/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2008 by Dmitry Tsarkov

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "globaldef.h"

#include <algorithm>	// for std::sort

#include "Reasoner.h"
#include "modelCacheIan.h"
#include "logging.h"

#define switchResult(ret,expr)\
do {\
	switch ( expr )\
	{\
	case utClash: return utClash;\
	case utUnusable: break;\
	case utDone: ret = utDone; break;\
	default: assert (0);\
	}\
} while (0)\

/********************************************************************************
  * Tactics section;
  *
  * Each Tactic should have a (small) Usability function <name>
  * and a Real tactic function <name>Body
  *
  * Each tactic returns:
  * - utUnusable	- if it couldn't be used for such vertex;
  * - utClash		- if there is a clash while used
  * - utDone		- if it was successfully used
  *
  ******************************************************************************/

// main local Tactic
tacticUsage DlSatTester :: commonTactic ( void )
{
#ifdef ENABLE_CHECKING
	assert ( curConcept.bp() != bpINVALID );
#endif

	// check if Node is cached and we tries to expand existing result
	// don't do anything for p- and/or i-blocked nodes (can't be unblocked)
	if ( curNode->isCached() || isPIBlocked() )
		return utUnusable;

	// informs about starting calculations...
	if ( LLM.isWritable(llGTA) )
		logStartEntry();

	tacticUsage ret = commonTacticBody ( DLHeap[curConcept.bp()] );

	if ( LLM.isWritable(llGTA) )
		logFinishEntry(ret);

	return ret;
}

//-------------------------------------------------------------------------------
//	Simple tactics
//-------------------------------------------------------------------------------

tacticUsage DlSatTester :: commonTacticBody ( const DLVertex& cur )
{
	// show DAG usage
#ifdef RKG_PRINT_DAG_USAGE
	const_cast<DLVertex&>(cur).incUsage(isPositive(curConcept.bp()));
#endif
	nTacticCalls.inc();

	// call proper tactic
	switch ( cur.Type() )
	{
	case dtTop:
		assert(0);		// can't appear here; addToDoEntry deals with constants
		return utDone;

	case dtDataType:	// data things are checked by data inferer
	case dtDataValue:
		nUseless.inc();
		return utUnusable;

	case dtPSingleton:
	case dtNSingleton:
		dBlocked = NULL;	// invalidate cache for the blocked node
		if ( isPositive (curConcept.bp()) )	// real singleton
			return commonTacticBodySingleton(cur);
		else	// negated singleton -- nothing to do with.
			return commonTacticBodyId(cur);

	case dtNConcept:
	case dtPConcept:
		dBlocked = NULL;	// invalidate cache for the blocked node
		return commonTacticBodyId(cur);

	case dtAnd:
		dBlocked = NULL;	// invalidate cache for the blocked node
		if ( isPositive (curConcept.bp()) )	// this is AND vertex
			return commonTacticBodyAnd(cur);
		else	// OR
			return commonTacticBodyOr(cur);

	case dtForall:
		if ( isNegative(curConcept.bp()) )	// SOME vertex
			return commonTacticBodySome(cur);

		// ALL vertex
		dBlocked = NULL;	// invalidate cache for the blocked node
		return commonTacticBodyAll(cur);

	case dtIrr:
		if ( isNegative(curConcept.bp()) )	// SOME R.Self vertex
			return commonTacticBodySomeSelf(cur.getRole());
		else	// don't need invalidate cache, as IRREFL can only lead to CLASH
			return commonTacticBodyIrrefl(cur.getRole());

	case dtLE:
		if ( isNegative (curConcept.bp()) )	// >= vertex
			return commonTacticBodyGE(cur);

		// <= vertex
		dBlocked = NULL;	// invalidate cache for the blocked node

		if ( cur.isFunctional() )
			return commonTacticBodyFunc(cur);
		else
			return commonTacticBodyLE(cur);

	default:
		assert(0);
		return utUnusable;
	}
}

bool DlSatTester :: isPIBlocked ( void )
{
	// check for p-blocked node (can't be unblocked)
	if ( curNode->isPBlocked() )
		return true;

	// check for i-blocked nodes
	if ( curNode->isIBlocked() )
	{
		// check if it is the same i-blocked node as earlier
		if ( curNode == iBlocked )
			return true;

		// check whether node became unblocked
		curNode->updateIBlockedStatus();
		if ( curNode->isIBlocked() )
		{
			// cache value of i-blocked node
			iBlocked = curNode;
			return true;
		}
	}

	// clear i-blocked cache
	iBlocked = NULL;
	return false;
}

tacticUsage DlSatTester :: commonTacticBodyId ( const DLVertex& cur )
{
#ifdef ENABLE_CHECKING
	assert ( isCNameTag(cur.Type()) );	// safety check
#endif

	nIdCalls.inc();

	tacticUsage ret = utUnusable;

	// check if we have some DJ statement
	if ( isPositive(curConcept.bp()) )
		switchResult ( ret, applyExtraRulesIf(static_cast<const TConcept*>(cur.getConcept())) );

	// get either body(p) or inverse(body(p)), depends on sign of current ID
	BipolarPointer p = isPositive(curConcept.bp()) ? cur.getC() : inverse(cur.getC());
	switchResult ( ret, addToDoEntry ( curNode, p, curConcept.getDep() ) );

	return ret;
}

tacticUsage
DlSatTester :: applyExtraRules ( const TConcept* C )
{
	tacticUsage ret = utUnusable;

	const DepSet& dep = curConcept.getDep();
	CGLabel& lab = const_cast<CGLabel&>(curNode->label());
	for ( TConcept::er_iterator p = C->er_begin(), p_end=C->er_end(); p < p_end; ++p )
		if ( lab.addExtraConcept ( *p, dep ) )
		{
			nSRuleFire.inc();
			switchResult ( ret,
						   addToDoEntry ( curNode,
						   				  tBox.getExtraRuleHead(*p), DlCompletionTree::getClashSet() ) );
		}
		else
			nSRuleAdd.inc();

	return ret;
}

tacticUsage DlSatTester :: commonTacticBodySingleton ( const DLVertex& cur )
{
#ifdef ENABLE_CHECKING
	assert ( cur.Type() == dtPSingleton || cur.Type() == dtNSingleton );	// safety check
#endif

	nSingletonCalls.inc();

	// if the test REALLY uses nominals, remember this
	encounterNominal = true;

	const TIndividual* C = static_cast<const TIndividual*>(cur.getConcept());
	assert ( C->node != NULL );

	// if node for C was purged due to merge -- find proper one
	DepSet dep = curConcept.getDep();
	DlCompletionTree* realNode = C->node->resolvePBlocker(dep);

	if ( realNode != curNode )	// check if o-rule is applicable
		// apply o-rule: merge 2 nodes
		// don't need to actually expand P: it was/will be done in C->node
		return Merge ( curNode, realNode, dep );

	// singleton behaves as a general named concepts besides nominal cloud
	return commonTacticBodyId(cur);
}

//-------------------------------------------------------------------------------
//	AND/OR processing
//-------------------------------------------------------------------------------

tacticUsage DlSatTester :: commonTacticBodyAnd ( const DLVertex& cur )
{
#ifdef ENABLE_CHECKING
	assert ( isPositive(curConcept.bp()) && ( cur.Type() == dtAnd || cur.Type() == dtCollection ) );	// safety check
#endif

	nAndCalls.inc();

	register const DepSet& curDep = curConcept.getDep ();
	tacticUsage ret = utUnusable;

	// FIXME!! I don't know why, but performance is usually BETTER if using r-iters.
	// It's their only usage, so after investigation they can be dropped
	for ( DLVertex::const_reverse_iterator q = cur.rbegin(); q != cur.rend(); ++q )
		switchResult ( ret, addToDoEntry ( curNode, *q, curDep ) );

	return ret;
}

tacticUsage DlSatTester :: commonTacticBodyOr ( const DLVertex& cur )	// for C \or D concepts
{
#ifdef ENABLE_CHECKING
	assert ( isNegative(curConcept.bp()) && cur.Type() == dtAnd );	// safety check
#endif

	nOrCalls.inc();

	if ( isFirstBranchCall() )	// check the structure of OR operation (number of applicable concepts)
		switch ( getOrType(cur) )
		{
		case -1:	// found existing component
			if ( LLM.isWritable(llGTA) )
				LL << " E(" << *bContext->orBeg() << ")";

			// we create extra (useless) BC; now we shall get rid of it
			determiniseBranchingOp();
			return utUnusable;

		case 0:		// no more applicable concepts
			// set global dep-set using accumulated deps in branchDep
			DlCompletionTree::setClashSet(getBranchDep());
			return utClash;

		default:	// complex OR case;
			break;
		}

	// now it is OR case with 1 or more applicable concepts
	if ( bContext->isLastOrEntry() )
		return finOrProcessing();
	else
		return contOrProcessing();
}

int DlSatTester :: getOrType ( const DLVertex& cur )
{
	initBC(btOr);
	BranchingContext::OrIndex& indexVec = bContext->applicableOrEntries;
	indexVec.clear();

	// check all OR components for the clash
	const CGLabel& lab = curNode->label();
	for ( DLVertex::const_iterator q = cur.begin(), q_end = cur.end(); q < q_end; ++q )
		switch ( tryAddConcept ( lab, DLHeap[*q].Type(), inverse(*q), DepSet() ) )
		{
		case acrClash:	// clash found -- OK
			updateBranchDep();
			continue;
		case acrExist:	// already have such concept -- save it to the 1st position
			indexVec.resize(1);
			indexVec[0] = inverse(*q);
			return -1;
		case acrDone:
			indexVec.push_back(inverse(*q));
			continue;
		default:		// safety check
			assert (0);
		}

	// return number of items to be processed
	return indexVec.size();
}

tacticUsage DlSatTester :: contOrProcessing ( void )
{
	BranchingContext::or_iterator beg = bContext->orBeg(), end = bContext->orCur();
	BipolarPointer C = *end;

	// save current state
	save();

	// new (just branched) dep-set
	const DepSet curDep(getCurDepSet());

	// if semantic branching is in use -- add previous entries to the label
	processSemanticBranching ( beg, end, curDep );

	// add new entry to current node; we know the result would be DONE
	return
#	ifdef RKG_USE_DYNAMIC_BACKJUMPING
		addToDoEntry ( curNode, C, curDep );
#	else
		insertToDoEntry ( curNode, C, curDep, DLHeap[C].Type(), NULL );
#	endif
}

tacticUsage DlSatTester :: finOrProcessing ( void )
{
	// we will use cumulative dep-set
	prepareBranchDep();

	const DepSet& branchDep = getBranchDep();
	BipolarPointer C = *bContext->orCur();

	// if semantic branching is in use -- add previous entries to the label
	processSemanticBranching ( bContext->orBeg(), bContext->orCur(), branchDep );

	// add new entry to current node; we know the result would be DONE
	tacticUsage ret =
#	ifdef RKG_USE_DYNAMIC_BACKJUMPING
		addToDoEntry ( curNode, C, branchDep, "bcp" );
#	else
		insertToDoEntry ( curNode, C, branchDep, DLHeap[C].Type(), "bcp" );
#	endif

	// we did no save here => drop branching index by hands
	determiniseBranchingOp();
	return ret;
}

//-------------------------------------------------------------------------------
//	ALL processing
//-------------------------------------------------------------------------------

tacticUsage DlSatTester :: commonTacticBodyAllComplex ( const DLVertex& cur )
{
	tacticUsage ret = utUnusable;

	const RoleAutomaton& A = cur.getRole()->getAutomaton();
	const DepSet& dep = curConcept.getDep();
	BipolarPointer C = curConcept.bp();
	unsigned int state = cur.getState();
	RoleAutomaton::const_trans_iterator q, end = A.end(state);

	// apply all empty transitions
	for ( q = A.begin(state); q != end; ++q )
		if ( (*q)->empty() )
			switchResult ( ret, addToDoEntry ( curNode, C-state+(*q)->final(), dep, "e" ) );

	// apply final-state rule
	if ( state == 1 )
		switchResult ( ret, addToDoEntry ( curNode, cur.getC(), dep ) );

	// check whether automaton applicable to any edges
	nAllCalls.inc();
	DlCompletionTree::const_edge_iterator p;

	// check all parents and all sons
	for ( p = curNode->beginp(); p != curNode->endp(); ++p )
		switchResult ( ret, applyAutomaton ( (*p), A, state, C, dep ) );
	for ( p = curNode->begins(); p != curNode->ends(); ++p )
		switchResult ( ret, applyAutomaton ( (*p), A, state, C, dep ) );

	return ret;
}

tacticUsage DlSatTester :: commonTacticBodyAllSimple ( const DLVertex& cur )
{
	tacticUsage ret = utUnusable;

	const RoleAutomaton& A = cur.getRole()->getAutomaton();
	const DepSet& dep = curConcept.getDep();
	BipolarPointer C = cur.getC();

	// check whether automaton applicable to any edges
	nAllCalls.inc();
	DlCompletionTree::const_edge_iterator p;

	// check all parents and all sons
	for ( p = curNode->beginp(); p != curNode->endp(); ++p )
		switchResult ( ret, applySimpleAutomaton ( (*p), A, C, dep ) );
	for ( p = curNode->begins(); p != curNode->ends(); ++p )
		switchResult ( ret, applySimpleAutomaton ( (*p), A, C, dep ) );

	return ret;
}

//-------------------------------------------------------------------------------
//	Support for ALL processing
//-------------------------------------------------------------------------------

/** Perform expansion of (C=\AR{state}.X).DEP to an EDGE with a given reason */
tacticUsage DlSatTester :: applyAutomaton ( const DlCompletionTreeArc* edge,
											const RoleAutomaton& A,
											RAState state, BipolarPointer C,
											const DepSet& dep, const char* reason )
{
	if ( edge->isIBlocked() )
		return utUnusable;

	RoleAutomaton::const_trans_iterator q, end = A.end(state);
	const TRole* R = edge->getRole();
	DlCompletionTree* node = edge->getArcEnd();
	tacticUsage ret = utUnusable;

	// try to apply all transitions to edge
	for ( q = A.begin(state); q != end; ++q )
		if ( (*q)->applicable(R) )
			switchResult ( ret,
				addToDoEntry ( node, C-state+(*q)->final(), dep+edge->getDep(), reason ) );

	return ret;
}

/** Perform expansion of (\AR.C).DEP to an EDGE for simple R with a given reason */
tacticUsage DlSatTester :: applySimpleAutomaton ( const DlCompletionTreeArc* edge,
												  const RoleAutomaton& A,
												  BipolarPointer C,
												  const DepSet& dep, const char* reason )
{
	if ( edge->isIBlocked() )
		return utUnusable;

	// simple automaton has the only (meta-)transition
	if ( (*A.begin(0))->applicable(edge->getRole()) )
		return addToDoEntry ( edge->getArcEnd(), C, dep+edge->getDep(), reason );

	return utUnusable;
}

//-------------------------------------------------------------------------------
//	SOME processing
//-------------------------------------------------------------------------------

tacticUsage DlSatTester :: commonTacticBodySome ( const DLVertex& cur )	// for ER.C concepts
{
#ifdef ENABLE_CHECKING
	assert ( isNegative(curConcept.bp()) && cur.Type() == dtForall );
#endif

	register const DepSet& curDep = curConcept.getDep ();
	const TRole* rName = cur.getRole();				// role name for Some
	BipolarPointer cToAdd = inverse(cur.getC());	// concept to be add to the Some

	tacticUsage ret = utUnusable;

	// check if we already have R-neighbour labelled with C
	if ( isUsed(cToAdd) && curNode->isSomeApplicable ( rName, cToAdd ) )
		return utUnusable;

	// check for the case \ER.{o}
	if ( tBox.testHasNominals() && isPositive(cToAdd) )
	{
		const DLVertex& nom = DLHeap[cToAdd];
		if ( nom.Type() == dtPSingleton || nom.Type() == dtNSingleton )
			return commonTacticBodyValue ( rName, static_cast<const TIndividual*>(nom.getConcept()) );
	}

	nSomeCalls.inc();

	// check if we have functional role
	if ( rName->isFunctional() )
		for ( TRole::iterator r = rName->begin_topfunc(); r != rName->end_topfunc(); ++r )
			switch ( tryAddConcept ( curNode->label(), dtLE, (*r)->getFunctional(), curDep ) )
			{
			case acrClash:	// addition leads to clash
				return utClash;
			case acrDone:	// should be add to a label
			{
				// we are changing current Node => save it
				updateLevel ( curNode, curDep );

				ConceptWDep rFuncRestriction ( (*r)->getFunctional(), curDep );
				// NOTE! not added into TODO (because will be checked right now)
				curNode->addConcept ( rFuncRestriction, dtLE );
				setUsed(rFuncRestriction.bp());

				if ( LLM.isWritable(llGTA) )
					LL << " nf(" << rFuncRestriction << ")";
			}
				break;
			case acrExist:	// already exists
				break;
			default:		// safety check
				assert (0);
			}


	bool rFunc = false;				// flag is true if we have functional restriction with this Role name
	const TRole* rfRole = rName;	// most general functional super-role of given one
	ConceptWDep rFuncRestriction;	// role's functional restriction w/dep

	// set up rFunc; rfRole contains more generic functional superrole of rName
	for ( DlCompletionTree::const_label_iterator pc = curNode->beginl_cc(); pc != curNode->endl_cc(); ++pc )
	{	// found such vertex (<=1 R)
		const ConceptWDep& C = *pc;
		const DLVertex& ver = DLHeap[C.bp()];

		if ( isPositive(C.bp()) && ver.isFunctional() && *ver.getRole() >= *rName )	// FIXME!!! think later
		{
			if ( rFunc )	// second functional restriction
			{	// is more generic role...
				if ( *ver.getRole() >= *rfRole )
				{	// setup roles
					rfRole = ver.getRole();
					rFuncRestriction = C;
				}
			}
			else	// 1st functional role found
			{
				rFunc = true;
				rfRole = ver.getRole();
				rFuncRestriction = C;
			}
		}
	}

	if ( rFunc )	// functional role found => add new concept to existing node
	{
		const DlCompletionTreeArc* functionalArc = NULL;
		DepSet newDep;
		bool linkToParent = false;

		// check if we have an (R)-successor or (R-)-predecessor
		DlCompletionTree::const_edge_iterator pr;
		for ( pr = curNode->beginp(); !functionalArc && pr != curNode->endp(); ++pr )
			if ( (*pr)->isNeighbour ( rfRole, newDep ) )
			{
				functionalArc = *pr;
				linkToParent = true;
			}

		for ( pr = curNode->begins(); !functionalArc && pr != curNode->ends(); ++pr )
			if ( (*pr)->isNeighbour ( rfRole, newDep ) )
				functionalArc = *pr;

		// perform actions if such arc was found
		if ( functionalArc != NULL )
		{
			if ( LLM.isWritable(llGTA) )
				LL << " f(" << rFuncRestriction << "):";

			DlCompletionTree* succ = functionalArc->getArcEnd();

			// add current dependences (from processed entry)
			newDep.add(curDep);

			// check if merging will lead to clash because of disjoint roles
			if ( rName->isDisjoint() &&
				 checkDisjointRoleClash ( curNode, succ, rName, newDep ) == utClash )
				return utClash;

			// add current role label (to both arc and its reverse)
			functionalArc = CGraph.addRoleLabel ( curNode, succ, linkToParent, rName, newDep );

			// adds concept to the end of arc
			switchResult ( ret, addToDoEntry ( succ, cToAdd, newDep ) );

			// if new role label was added -- check AR.C in both sides of functionalArc
			if ( rfRole != rName )
			{
				switchResult ( ret, applyUniversalNR ( curNode, functionalArc, newDep, redoForall ) );
				switchResult ( ret, applyUniversalNR ( succ, functionalArc->getReverse(), newDep, redoForall ) );
			}

			return ret;
		}
	}

	//------------------------------------------------
	// no functional role or 1st arc -- create new arc
	//------------------------------------------------

	// there no such neighbour - create new successor
	// all FUNCs are already checked; no (new) irreflexivity possible
	return createNewEdge ( cur.getRole(), cToAdd, redoForall|redoAtMost );
}

/// expansion rule for existential quantifier in the form ER {o}
tacticUsage DlSatTester :: commonTacticBodyValue ( const TRole* R, const TIndividual* nom )
{
	DepSet curDep = curConcept.getDep();

	// check blocking conditions
	if ( recheckNodeDBlocked() )
		return utUnusable;

	nSomeCalls.inc();

	assert ( nom->node != NULL );

	// if node for NOM was purged due to merge -- find proper one
	DlCompletionTree* realNode = nom->node->resolvePBlocker(curDep);

	// check if merging will lead to clash because of disjoint roles
	if ( R->isDisjoint() && checkDisjointRoleClash ( curNode, realNode, R, curDep ) == utClash )
		return utClash;

	// here we are sure that there is a nominal connected to a root node
	encounterNominal = true;

	// create new edge between curNode and the given nominal node
	DlCompletionTreeArc* edge =
		CGraph.addRoleLabel ( curNode, realNode, /*linkToParent=*/false, R, curDep );

	// add all necessary concepts to both ends of the edge
	return
		setupEdge ( edge, curDep, redoForall|redoFunc|redoAtMost|redoIrr ) == utClash
			? utClash
			: utDone;
}

//-------------------------------------------------------------------------------
//	Support for SOME processing
//-------------------------------------------------------------------------------

tacticUsage DlSatTester :: createNewEdge ( const TRole* Role, BipolarPointer Concept, unsigned int flags )
{
	const DepSet& curDep = curConcept.getDep();

	// check blocking conditions
	if ( recheckNodeDBlocked() )
	{
		nUseless.inc();
		return utUnusable;
	}

	DlCompletionTreeArc* pA = createOneNeighbour ( Role, curDep );

	// add necessary label
	if ( initNewNode ( pA->getArcEnd(), curDep, Concept ) == utClash ||
		 setupEdge ( pA, curDep, flags ) == utClash )
		return utClash;
	else
		return utDone;
}

/// create new ROLE-neighbour to curNode; return edge to it
DlCompletionTreeArc* DlSatTester :: createOneNeighbour ( const TRole* Role, const DepSet& dep, CTNominalLevel level )
{
	// check whether is called from NN-rule
	bool forNN = level != BlockableLevel;

	// create a proper neighbour
	DlCompletionTreeArc* pA = CGraph.createNeighbour ( curNode, /*isUpLink=*/forNN, Role, dep );
	DlCompletionTree* node = pA->getArcEnd();

	// set nominal node's level if necessary
	if ( forNN )
		node->setNominalLevel(level);

	// check whether created node is data node
	if ( Role->isDataRole() )
		node->setDataNode();

	// log newly created node
	CHECK_LL_RETURN_VALUE(llGTA,pA);

	if ( Role->isDataRole() )
		LL << " DN(";
	else
		LL << " cn(";
	LL << node->getId() << dep << ")";

	return pA;
}

bool DlSatTester :: recheckNodeDBlocked ( void )
{
	// for non-lazy blocking blocked status is correct
	if ( !useLazyBlocking )
		return curNode->isBlocked();

	// several \E concepts in a row with the same node
	if ( curNode == dBlocked )
		return true;

	// update node's blocked status
	if ( curNode->isAffected() )
	{
		updateLevel ( curNode, curConcept.getDep() );
		curNode->updateDBlockedStatus();
	}

	// if node became d-blocked -- update cache
	if ( curNode->isDBlocked() )
	{
		dBlocked = curNode;
		return true;
	}

	// clear d-blocker cache
	dBlocked = NULL;

	// if node became i-blocked -- update appropriate cache
	if ( curNode->isIBlocked() )
	{
		iBlocked = curNode;
		return true;
	}

	// not blocked
	return false;
}

tacticUsage
DlSatTester :: setupEdge ( DlCompletionTreeArc* pA, const DepSet& curDep, unsigned int flags )
{
	tacticUsage ret = utUnusable;

	DlCompletionTree* child = pA->getArcEnd();
	DlCompletionTree* from = pA->getReverse()->getArcEnd();

	// adds Range and Domain
	switchResult ( ret, initHeadOfNewEdge ( from, pA->getRole(), curDep, "RD" ) );
	switchResult ( ret, initHeadOfNewEdge ( child, pA->getReverse()->getRole(), curDep, "RR" ) );

	// check if we have any AR.X concepts in current node
	switchResult ( ret, applyUniversalNR ( from, pA, curDep, flags ) );

	// for nominal children and loops -- just apply things for the inverces
	if ( child->isNominalNode() || child == from )
		switchResult ( ret, applyUniversalNR ( child, pA->getReverse(), curDep, flags ) );
	else
	{
		if ( child->isDataNode() )
		{
			checkDataNode = true;
			switchResult ( ret, checkDataClash(child) );
		}
		else	// check if it is possible to use cache for new node
			switchResult ( ret, tryCacheNode(child) );
	}

	// all done
	return ret;
}

tacticUsage DlSatTester :: applyUniversalNR ( DlCompletionTree* Node,
											  const DlCompletionTreeArc* arcSample,
											  const DepSet& dep, unsigned int flags )
{
	// check whether a flag is set
	if ( flags == 0 )
		return utUnusable;

	tacticUsage ret = utUnusable;
	const TRole* R = arcSample->getRole();

	for ( DlCompletionTree::const_label_iterator
		  p = Node->beginl_cc(), p_end = Node->endl_cc(); p != p_end; ++p )
	{
		// need only AR.C concepts where ARC is labelled with R
		if ( isNegative(p->bp()) )
			continue;

		const DLVertex& v = DLHeap[p->bp()];
		const TRole* vR = v.getRole();

		switch ( v.Type() )
		{
		case dtIrr:
			if ( flags & redoIrr )
				switchResult ( ret, checkIrreflexivity ( arcSample, vR, dep ) );
			break;

		case dtForall:
			if ( (flags & redoForall) == 0 )
				break;

			if ( vR->isSimple() )
				switchResult ( ret, applySimpleAutomaton ( arcSample, vR->getAutomaton(),
													 	   v.getC(), p->getDep()+dep, "ae" ) );
			else
				switchResult ( ret, applyAutomaton ( arcSample, vR->getAutomaton(),
													 v.getState(), p->bp(), p->getDep()+dep, "ae" ) );
			break;

		case dtLE:
			if ( v.isFunctional() )
			{
				if ( (flags & redoFunc) && (*vR >= *R) )
				{
					addExistingToDoEntry ( Node, Node->label().getCCOffset(p), "f" );
					ret = utDone;
				}
			}
			else
				if ( (flags & redoAtMost) && (*vR >= *R) )
				{
					addExistingToDoEntry ( Node, Node->label().getCCOffset(p), "le" );
					ret = utDone;
				}
			break;

		default:
			break;
		}
	}

	return ret;
}

	/// add necessary concepts to the head of the new EDGE
tacticUsage
DlSatTester :: initHeadOfNewEdge ( DlCompletionTree* node, const TRole* R, const DepSet& dep, const char* reason )
{
	// define return value
	tacticUsage ret = utUnusable;
	TRole::iterator r, r_end;

	// if R is functional, then add FR with given DEP-set to NODE
	if ( R->isFunctional() )
		for ( r = R->begin_topfunc(), r_end = R->end_topfunc(); r < r_end; ++r )
			switchResult ( ret, addToDoEntry ( node, (*r)->getFunctional(), dep, "fr" ) );

	// setup Domain for R
	switchResult ( ret, addToDoEntry ( node, R->getBPDomain(), dep, reason ) );

#	ifndef RKG_UPDATE_RND_FROM_SUPERROLES
		for ( r = R->begin_anc(), r_end = R->end_anc(); r < r_end; ++r )
			switchResult ( ret, addToDoEntry ( node, (*r)->getBPDomain(), dep, reason ) );
#	endif

	return ret;
}

//-------------------------------------------------------------------------------
//	Func/LE/GE processing
//-------------------------------------------------------------------------------

tacticUsage DlSatTester :: commonTacticBodyFunc ( const DLVertex& cur )	// for <=1 R concepts
{
#ifdef ENABLE_CHECKING
	assert ( isPositive(curConcept.bp()) && cur.isFunctional() );
#endif

	// check whether we need to apply NN rule first
	if ( isNNApplicable ( cur.getRole(), bpTOP ) )
		return commonTacticBodyNN(cur);	// after application func-rule would be checked again

	nFuncCalls.inc();

	if ( !RKG_HIERARCHY_NR_TACTIC )
		// check if we have a clash with the other NR
		for ( DlCompletionTree::const_label_iterator q = curNode->beginl_cc(); q != curNode->endl_cc(); ++q )
			if ( isNegative(q->bp())		// need at-least restriction
				 && isNRClash ( DLHeap[q->bp()], cur, *q ) )
				return utClash;

	// locate all R-neighbours of curNode
	EdgeVector EdgesToMerge;
	findNeighbours ( EdgesToMerge, cur.getRole(), bpTOP );

	// check if we have nodes to merge
	if ( EdgesToMerge.size() < 2 )
		return utUnusable;

	tacticUsage ret = utUnusable;

	// merge all nodes to the first (the least wrt nominal hierarchy) found node
	EdgeVector::iterator q = EdgesToMerge.begin();
	DlCompletionTree* sample = (*q)->getArcEnd();
	DepSet depF (curConcept.getDep());	// dep-set for merging
	depF.add((*q)->getDep());

	// merge all elements to sample (sample wouldn't be merge)
	for ( ++q; q != EdgesToMerge.end(); ++q )
		// during merge EdgesToMerge may became purged (see Nasty4) => check this
		if ( !(*q)->getArcEnd()->isPBlocked() )
			switchResult ( ret, Merge ( (*q)->getArcEnd(), sample, depF+(*q)->getDep() ) );

	return ret;
}


tacticUsage DlSatTester :: commonTacticBodyLE ( const DLVertex& cur )	// for <=nR.C concepts
{
#ifdef ENABLE_CHECKING
	assert ( isPositive(curConcept.bp()) && ( cur.Type() == dtLE ) );
#endif

	nLeCalls.inc();
	BipolarPointer C = cur.getC();

	tacticUsage ret = utUnusable;

	if ( !isFirstBranchCall() )
		switch ( bContext->tag )
		{
		case btChoose:	break;			// clash in choose-rule: redo all
		case btNN:		goto applyNN;	// clash in NN-rule: skip choose-rule
		case btLE:		goto applyLE;	// clash in LE-rule: skip all the rest
		default:		assert(0);		// no way to reach here
		}

	// check if we have Qualified NR
	if ( C != bpTOP )
		switchResult ( ret, commonTacticBodyChoose ( cur.getRole(), C ) );

	// check whether we need to apply NN rule first
	if ( isNNApplicable ( cur.getRole(), C ) )
	{
	applyNN:
		return commonTacticBodyNN(cur);	// after application <=-rule would be checked again
	}

	// if we are here that it IS first LE call

	// NOTE: this is not necessary if we have *F but not *N
	if ( !RKG_HIERARCHY_NR_TACTIC )
		// check if we have a clash with the other NR
		for ( DlCompletionTree::const_label_iterator q = curNode->beginl_cc(); q != curNode->endl_cc(); ++q )
			if ( isNegative(q->bp())		// need at-least restriction
				 && isNRClash ( DLHeap[q->bp()], cur, *q ) )
				return utClash;

	// we need to repeate merge until there will be necessary amount of edges
	while (1)
	{
		if ( isFirstBranchCall() )
		{
			// init context
			initBC(btLE);

			// check the amount of neighbours we have
			findNeighbours ( bContext->EdgesToMerge, cur.getRole(), C );

			// if the number of R-heighs satisfies condition -- nothing to do
			if ( bContext->notApplicableLE(cur.getNumberLE()) )
			{
				determiniseBranchingOp();
				return ret;
			}

			// setup mergeCandIndex
			bContext->resetMCI();
		}

applyLE:	// skip init, because here we are after restoring

		if ( bContext->noMoreLEOptions() )
		{	// set global clashset to cummulative one from previous branch failures
			useBranchDep();
			return utClash;
		}

		// get from- and to-arcs using corresponding indexes in Edges
		DlCompletionTreeArc* from = bContext->getFrom();
		DlCompletionTreeArc* to = bContext->getTo();

		// fast check for from->end() and to->end() are in \neq
		bool isNonMergable;
		if ( C == bpTOP )
			isNonMergable = CGraph.nonMergable ( from->getArcEnd(), to->getArcEnd(), DepSet() );
		else
			isNonMergable = CGraph.nonMergable ( from->getArcEnd(), to->getArcEnd(), DepSet(), C, DLHeap[C].Type() );

		if ( isNonMergable )
		{
			updateBranchDep();
			bContext->nextOption();
			goto applyLE;
		}

		save();

		// add depset from current level and FROM arc and to current dep.set
		DepSet curDep(getCurDepSet());
		curDep.add(from->getDep());

		switchResult ( ret, Merge ( from->getArcEnd(), to->getArcEnd(), curDep ) );
	}
}

tacticUsage DlSatTester :: commonTacticBodyGEsimple ( const DLVertex& cur )	// for >=nR.C concepts
{
#ifdef ENABLE_CHECKING
	assert ( isNegative(curConcept.bp()) && cur.Type() == dtLE );
#endif

	nGeCalls.inc();

	// check if we have a clash with the other NR
	for ( DlCompletionTree::const_label_iterator q = curNode->beginl_cc(); q != curNode->endl_cc(); ++q )
		if ( isPositive(q->bp())		// need at-most restriction
			 && isNRClash ( cur, DLHeap[q->bp()], *q ) )
			return utClash;

	const TRole* Role = cur.getRole();

	// check if we have an arc with the corresponding role

	// note that only SUCCESSORS are counted here: we have to multiply
	// the node easily, which is not the case for the predecessor.
	// TODO: try to found an example here
	for ( DlCompletionTree::const_edge_iterator
		  p = curNode->begins(); p != curNode->ends(); ++p )
		if ( (*p)->isNeighbour(Role) )
			return utUnusable;	// don't need to inform about this: return type is unique for the action

	// don't find proper arc -- create new one; no Irr check necessary
	return createNewEdge ( Role, cur.getC(), redoForall|redoFunc|redoAtMost );
}

tacticUsage DlSatTester :: commonTacticBodyGEusual ( const DLVertex& cur )	// for >=nR.C concepts
{
#ifdef ENABLE_CHECKING
	assert ( isNegative(curConcept.bp()) && cur.Type() == dtLE );
#endif

	nGeCalls.inc();

	// check blocking conditions
	if ( recheckNodeDBlocked() )
	{
		nUseless.inc();
		return utUnusable;
	}

	// create N new different edges
	return createDifferentNeighbours ( cur.getRole(), cur.getC(), curConcept.getDep(), cur.getNumberGE(), BlockableLevel );
}

//-------------------------------------------------------------------------------
//	Support for Func/LE/GE processing
//-------------------------------------------------------------------------------

/// create N R-neighbours of curNode with given Nominal LEVEL labelled with C
tacticUsage DlSatTester :: createDifferentNeighbours ( const TRole* R, BipolarPointer C, const DepSet& dep,
													   unsigned int n, CTNominalLevel level )
{
	tacticUsage ret = utDone;	// we WILL create several edges

	// create N new edges with the same IR
	DlCompletionTreeArc* pA = NULL;
	CGraph.initIR();
	for ( unsigned int i = 0; i < n; ++i )
	{
		pA = createOneNeighbour ( R, dep, level );
		DlCompletionTree* child = pA->getArcEnd();

		// make CHILD different from other created nodes
		CGraph.setCurIR ( child, dep );

		// add necessary new node labels and setup new edge
		switchResult ( ret, initNewNode ( child, dep, C ) );
		switchResult ( ret, setupEdge ( pA, dep, redoForall ) );
	}
	CGraph.finiIR();

	// re-apply all <= NR in curNode; do it only once for all created nodes; no need for Irr
	switchResult ( ret, applyUniversalNR ( curNode, pA, dep, redoFunc|redoAtMost ) );

	return ret;
}

		/// check if ATLEAST and ATMOST restrictions are in clash; setup depset from CUR
bool DlSatTester :: isNRClash ( const DLVertex& atleast, const DLVertex& atmost, const ConceptWDep& reason )
{
	if ( atmost.Type() != dtLE || atleast.Type() != dtLE )
		return false;
	if ( !checkNRclash ( atleast, atmost ) )	// no clash between them
		return false;

	// clash exists: create dep-set
	DlCompletionTree::setClashSet ( curConcept.getDep() + reason.getDep() );

	// log clash reason
	if ( LLM.isWritable(llGTA) )
		logClash ( curNode, reason.bp(), reason.getDep() );

	return true;
}

tacticUsage DlSatTester :: Merge ( DlCompletionTree* from, DlCompletionTree* to, const DepSet& depF )
{
	// if node is already purged -- nothing to do
	assert ( !from->isPBlocked() );

	// prevent node to be merged to itself
	assert ( from != to );

	// never merge nominal node to blockable one
	assert ( to->getNominalLevel() <= from->getNominalLevel() );

	if ( LLM.isWritable(llGTA) )
		LL << " m(" << from->getId() << "->" << to->getId() << ")";

	nMergeCalls.inc();

	// can't merge 2 nodes which are in inequality relation
	if ( CGraph.nonMergable ( from, to, depF ) )
		return utClash;	// clash-set was updated by IR

	tacticUsage ret = utUnusable;

	// copy all node labels
	DlCompletionTree::const_label_iterator p;
	for ( p = from->beginl_sc(); p != from->endl_sc(); ++p )
		switchResult ( ret, addToDoEntry ( to, p->bp(), depF+p->getDep(), "M" ) );
	for ( p = from->beginl_cc(); p != from->endl_cc(); ++p )
		switchResult ( ret, addToDoEntry ( to, p->bp(), depF+p->getDep(), "M" ) );

	// correct graph structure
	typedef std::vector<DlCompletionTreeArc*> edgeVector;
	edgeVector edges;
	CGraph.Merge ( from, to, depF, edges );

	// check whether a disjoint roles lead to clash
	edgeVector::const_iterator q, q_end = edges.end();

	for ( q = edges.begin(); q != q_end; ++q )
		if ( (*q)->getRole()->isDisjoint() &&
			 checkDisjointRoleClash ( (*q)->getReverse()->getArcEnd(), (*q)->getArcEnd(),
			 						  (*q)->getRole(), depF ) == utClash )
			return utClash;

	// nothing more to do with data nodes
	if ( to->isDataNode() )	// data concept -- run data center for it
		return checkDataClash(to);

	// for every node added to TO, every ALL, Irr and <=-node should be checked
	for ( q = edges.begin(); q != q_end; ++q )
		switchResult ( ret, applyUniversalNR ( to, *q, depF, redoForall|redoFunc|redoAtMost|redoIrr ) );

	// we do real action here, so the return value
	return utDone;
}

tacticUsage
DlSatTester :: checkDisjointRoleClash ( DlCompletionTree* from, DlCompletionTree* to,
										const TRole* R, const DepSet& dep )
{
	DlCompletionTree::const_edge_iterator p, p_end;

	// try to check whether link from->to labelled with something disjoint with R
	for ( p = from->beginp(), p_end = from->endp(); p != p_end; ++p )
		if ( checkDisjointRoleClash ( *p, to, R, dep ) )
			return utClash;
	for ( p = from->begins(), p_end = from->ends(); p != p_end; ++p )
		if ( checkDisjointRoleClash ( *p, to, R, dep ) )
			return utClash;
	return utDone;
}

// compare 2 CT edges wrt blockable/nominal nodes at their ends
class EdgeCompare
{
public:
	bool operator() ( DlCompletionTreeArc* pa, DlCompletionTreeArc* qa ) const
		{ return *(pa)->getArcEnd() < *(qa)->getArcEnd(); }
}; // EdgeCompare

/// aux method to check whether edge ended to NODE should be added to EdgetoMerge
template<class Iterator>
bool isNewEdge ( const DlCompletionTree* node, Iterator begin, Iterator end )
{
	for ( Iterator q = begin; q != end; ++q )
		if ( (*q)->getArcEnd() == node )	// skip edges to the same node
			return false;

	return true;
}

void DlSatTester :: findNeighbours ( EdgeVector& EdgesToMerge, const TRole* Role, BipolarPointer C ) const
{
	EdgesToMerge.clear();

	DlCompletionTree::const_edge_iterator p;

	for ( p = curNode->beginp(); p != curNode->endp(); ++p )
		if ( (*p)->isNeighbour(Role) && (*p)->getArcEnd()->isLabelledBy(C)
			 && isNewEdge ( (*p)->getArcEnd(), EdgesToMerge.begin(), EdgesToMerge.end() ) )
			EdgesToMerge.push_back(*p);

	for ( p = curNode->begins(); p != curNode->ends(); ++p )
		if ( (*p)->isNeighbour(Role) && (*p)->getArcEnd()->isLabelledBy(C)
			 && isNewEdge ( (*p)->getArcEnd(), EdgesToMerge.begin(), EdgesToMerge.end() ) )
			EdgesToMerge.push_back(*p);

	// sort EdgesToMerge: From named nominals to generated nominals to blockable nodes
	std::sort ( EdgesToMerge.begin(), EdgesToMerge.end(), EdgeCompare() );
}

//-------------------------------------------------------------------------------
//	Choose-rule processing
//-------------------------------------------------------------------------------

tacticUsage DlSatTester :: commonTacticBodyChoose ( const TRole* R, BipolarPointer C )
{
	tacticUsage ret = utUnusable;
	DlCompletionTree::edge_iterator p;

	// apply choose-rule for every R-predecessor
	for ( p = curNode->beginp(); p != curNode->endp(); ++p )
		if ( (*p)->isNeighbour(R) )
			switchResult ( ret, applyChooseRule ( (*p)->getArcEnd(), C ) );

	// apply choose-rule for every R-succecessor
	for ( p = curNode->begins(); p != curNode->ends(); ++p )
		if ( (*p)->isNeighbour(R) )
			switchResult ( ret, applyChooseRule ( (*p)->getArcEnd(), C ) );

	return ret;
}

//-------------------------------------------------------------------------------
//	Support for choose-rule processing
//-------------------------------------------------------------------------------

/// apply choose-rule to given node
tacticUsage DlSatTester :: applyChooseRule ( DlCompletionTree* node, BipolarPointer C )
{
	if ( node->isLabelledBy(C) || node->isLabelledBy(inverse(C)) )
		return utUnusable;

	// now node will be labelled with ~C or C
	if ( isFirstBranchCall() )
	{
		initBC(btChoose);
		// save current state
		save();

		return addToDoEntry ( node, inverse(C), getCurDepSet(), "cr0" );
	}
	else
	{
		prepareBranchDep();
		DepSet dep(getBranchDep());
		determiniseBranchingOp();
		return addToDoEntry ( node, C, dep, "cr1" );
	}
}

//-------------------------------------------------------------------------------
//	NN rule processing
//-------------------------------------------------------------------------------

tacticUsage DlSatTester :: commonTacticBodyNN ( const DLVertex& cur )	// NN-rule
{
	// here we KNOW that NN-rule is applicable, so skip some tests
	nNNCalls.inc();

	if ( isFirstBranchCall() )
		initBC(btNN);

	// check whether we did all possible tries
	if ( bContext->noMoreNNOptions(cur.getNumberLE()) )
	{	// set global clashset to cummulative one from previous branch failures
		useBranchDep();
		return utClash;
	}

	// take next NN number; save it as SAVE() will reset it to 0
	unsigned int NN = bContext->branchIndex;

	tacticUsage ret = utDone;	// we WILL create several entries

	// prepare to addition to the label
	save();

	// new (just branched) dep-set
	const DepSet curDep(getCurDepSet());

	// create curNN new different edges
	switchResult ( ret,
		createDifferentNeighbours ( cur.getRole(), cur.getC(), curDep, NN, curNode->getNominalLevel()+1 ) );

	// now remember NR we just created: it is (<= curNN R), so have to find it
	switchResult ( ret, addToDoEntry ( curNode, curConcept.bp() + (cur.getNumberLE()-NN), curDep, "NN" ) );

	return ret;
}

//-------------------------------------------------------------------------------
//	Support for NN rule processing
//-------------------------------------------------------------------------------

/// @return true iff NN-rule wrt (<= R) is applicable to the curNode
bool DlSatTester :: isNNApplicable ( const TRole* r, BipolarPointer C ) const
{
	// NN rule is only applicable to nominal nodes
	if ( !curNode->isNominalNode() )
		return false;

	unsigned int level = curNode->getNominalLevel()+1;

	// check for the real applicability of the NN-rule here
	DlCompletionTree::const_edge_iterator p, p_end = curNode->endp(), q;
	for ( p = curNode->beginp(); p != p_end; ++p )
	{
		const DlCompletionTree* suspect = (*p)->getArcEnd();

		if ( suspect->isBlockableNode() && (*p)->isNeighbour(r) && suspect->isLabelledBy(C) )
		{
			// have to fire NN-rule. Check whether we did this earlier
			for ( q = curNode->beginp(); q != p_end; ++q )
				if ( (*q)->getArcEnd()->isNominalNode(level) && (*q)->isNeighbour(r)
					 && (*q)->getArcEnd()->isLabelledBy(C) )
					return false;	// we already created at least one edge based on that rule

			if ( LLM.isWritable(llGTA) )
				LL << " NN(" << suspect->getId() << ")";

			// no such edges were created => have to run NN-rule
			return true;
		}
	}

	// can't apply NN-rule
	return false;
}

//-------------------------------------------------------------------------------
//	Support for (\neg) \E R.Self
//-------------------------------------------------------------------------------

tacticUsage DlSatTester :: commonTacticBodySomeSelf ( const TRole* R )
{
	// nothing to do if R-loop already exists
	DlCompletionTree::const_edge_iterator p, p_end;
	for ( p = curNode->beginp(), p_end = curNode->endp(); p != p_end; ++p )
		if ( (*p)->getArcEnd() == curNode && (*p)->isNeighbour(R) )
			return utUnusable;
	for ( p = curNode->begins(), p_end = curNode->ends(); p != p_end; ++p )
		if ( (*p)->getArcEnd() == curNode && (*p)->isNeighbour(R) )
			return utUnusable;

	// create an R-loop through curNode
	const DepSet& dep = curConcept.getDep();
	DlCompletionTreeArc* pA = CGraph.addRoleLabel ( curNode, curNode, /*isUpLink=*/false, R, dep );
	return setupEdge ( pA, dep, redoForall|redoFunc|redoAtMost|redoIrr );
}

tacticUsage DlSatTester :: commonTacticBodyIrrefl ( const TRole* R )
{
	DlCompletionTree::const_edge_iterator p, p_end;
	// return clash if R-loop is found
	for ( p = curNode->beginp(), p_end = curNode->endp(); p != p_end; ++p )
		if ( checkIrreflexivity ( *p, R, curConcept.getDep() ) == utClash )
			return utClash;
	for ( p = curNode->begins(), p_end = curNode->ends(); p != p_end; ++p )
		if ( checkIrreflexivity ( *p, R, curConcept.getDep() ) == utClash )
			return utClash;

	return utDone;
}

//-------------------------------------------------------------------------------
//	Support for cached reasoning deep in the tree
//-------------------------------------------------------------------------------

tacticUsage DlSatTester :: tryCacheNode ( DlCompletionTree* node )
{
	tacticUsage ret = utUnusable;
	DlCompletionTree::const_label_iterator p;
	node->setCached(false);
	bool shallow = true;
	unsigned int size = 0;

	// nominal nodes can not be cached
	if ( node->isNominalNode() )
		return utUnusable;

	// check applicability of the caching
	for ( p = node->beginl_sc(); p != node->endl_sc(); ++p )
	{
		if ( DLHeap.getCache(p->bp()) == NULL )
			return utUnusable;

		shallow &= DLHeap.getCache(p->bp())->shallowCache();
		++size;
	}

	for ( p = node->beginl_cc(); p != node->endl_cc(); ++p )
	{
		if ( DLHeap.getCache(p->bp()) == NULL )
			return utUnusable;

		shallow &= DLHeap.getCache(p->bp())->shallowCache();
		++size;
	}

	// it's useless to cache shallow nodes
	if ( shallow && size != 0 )
		return utUnusable;

	if ( size == 0 )
		ret = utDone;
	else
		ret = doCacheNode(node);

	if ( ret == utDone )
	{
		node->setCached(true);
		if ( LLM.isWritable(llGTA) )
			LL << " cached(" << node->getId() << ")";
	}

	return ret;
}

/// perform caching of the node (it is known that caching is possible)
tacticUsage
DlSatTester :: doCacheNode ( DlCompletionTree* node )
{
	tacticUsage ret = utUnusable;
	DlCompletionTree::const_label_iterator p;

	// It's unsafe to have a cache that touchs nominal here; set flagNominals to prevent it
	modelCacheIan cache(true);

	for ( p = node->beginl_sc(); p != node->endl_sc(); ++p )
		// try to merge cache of a node label element with accumulator
		if ( (ret = processCacheResultCompletely ( cache.merge(DLHeap.getCache(p->bp())), node )) != utDone )
			return ret; // caching of node fails

	for ( p = node->beginl_cc(); p != node->endl_cc(); ++p )
		// try to merge cache of a node label element with accumulator
		if ( (ret = processCacheResultCompletely ( cache.merge(DLHeap.getCache(p->bp())), node )) != utDone )
			return ret; // caching of node fails

	// all concepts in label are mergable; now try to add input arc
	if ( node->hasParent() )
	{
		modelCacheIan cachePar(false);
		cachePar.initRolesFromArcs(node);	// the only arc is parent

		ret = processCacheResultCompletely ( cache.merge(&cachePar), node );
	}

	return ret;
}
