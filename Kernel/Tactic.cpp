/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2010 by Dmitry Tsarkov

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
	default: fpp_unreachable();\
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
	fpp_assert ( curConcept.bp() != bpINVALID );
#endif

	// check if Node is cached and we tries to expand existing result
	// also don't do anything for p-blocked nodes (can't be unblocked)
	if ( curNode->isCached() || curNode->isPBlocked() )
		return utUnusable;

	// informs about starting calculations...
	if ( LLM.isWritable(llGTA) )
		logStartEntry();

	tacticUsage ret = utUnusable;

	// apply tactic only if Node is not an i-blocked
	if ( !isIBlocked() )
		ret = commonTacticBody ( DLHeap[curConcept.bp()] );

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
	incStat(nTacticCalls);

	// call proper tactic
	switch ( cur.Type() )
	{
	case dtTop:
		fpp_unreachable();		// can't appear here; addToDoEntry deals with constants
		return utDone;

	case dtDataType:	// data things are checked by data inferer
	case dtDataValue:
		incStat(nUseless);
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
		if ( isFunctionalVertex(cur) )
			return commonTacticBodyFunc(cur);
		else
			return commonTacticBodyLE(cur);

	case dtProj:
		fpp_assert ( isPositive (curConcept.bp()) );
		return commonTacticBodyProj ( cur.getRole(), cur.getC(), cur.getProjRole() );

	default:
		fpp_unreachable();
		return utUnusable;
	}
}

tacticUsage DlSatTester :: commonTacticBodyId ( const DLVertex& cur )
{
#ifdef ENABLE_CHECKING
	fpp_assert ( isCNameTag(cur.Type()) );	// safety check
#endif

	incStat(nIdCalls);

	tacticUsage ret = utUnusable;

#ifdef RKG_USE_SIMPLE_RULES
	// check if we have some simple rules
	if ( isPositive(curConcept.bp()) )
		switchResult ( ret, applyExtraRulesIf(static_cast<const TConcept*>(cur.getConcept())) );
#endif

	// get either body(p) or inverse(body(p)), depends on sign of current ID
	BipolarPointer C = isPositive(curConcept.bp()) ? cur.getC() : inverse(cur.getC());
	switchResult ( ret, addToDoEntry ( curNode, C, curConcept.getDep() ) );

	return ret;
}

/// @return true if the rule is applicable; set the dep-set accordingly
bool
DlSatTester :: applicable ( const TBox::TSimpleRule& rule )
{
	BipolarPointer bp = curConcept.bp();
	const CWDArray& lab = curNode->label().getLabel(dtPConcept);
	// dep-set to keep track for all the concepts in a rule-head
	DepSet loc = curConcept.getDep();

	for ( TBox::TSimpleRule::const_iterator p = rule.Body.begin(), p_end = rule.Body.end(); p < p_end; ++p )
	{
		if ( (*p)->pName == bp )
			continue;
		if ( findConcept(lab,(*p)->pName,loc) )
			loc = getClashSet();	// such a concept exists -- rememeber clash set
		else	// no such concept -- can not fire a rule
			return false;
	}

	// rule will be fired -- set the dep-set
	setClashSet(loc);
	return true;
}

tacticUsage
DlSatTester :: applyExtraRules ( const TConcept* C )
{
	tacticUsage ret = utUnusable;

	for ( TConcept::er_iterator p = C->er_begin(), p_end=C->er_end(); p < p_end; ++p )
	{
		const TBox::TSimpleRule* rule = tBox.getSimpleRule(*p);
		incStat(nSRuleAdd);
		if ( rule->applicable(*this) )	// apply the rule's head
		{
			incStat(nSRuleFire);
			switchResult ( ret, addToDoEntry ( curNode, rule->bpHead, getClashSet() ) );
		}
	}

	return ret;
}

tacticUsage DlSatTester :: commonTacticBodySingleton ( const DLVertex& cur )
{
#ifdef ENABLE_CHECKING
	fpp_assert ( cur.Type() == dtPSingleton || cur.Type() == dtNSingleton );	// safety check
#endif

	incStat(nSingletonCalls);

	// can use this rule only in the Nominal reasoner
	fpp_assert ( hasNominals() );

	// if the test REALLY uses nominals, remember this
	encounterNominal = true;

	const TIndividual* C = static_cast<const TIndividual*>(cur.getConcept());
	fpp_assert ( C->node != NULL );

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
	fpp_assert ( isPositive(curConcept.bp()) && ( cur.Type() == dtAnd || cur.Type() == dtCollection ) );	// safety check
#endif

	incStat(nAndCalls);

	const DepSet& dep = curConcept.getDep();
	tacticUsage ret = utUnusable;

	// FIXME!! I don't know why, but performance is usually BETTER if using r-iters.
	// It's their only usage, so after investigation they can be dropped
	for ( DLVertex::const_reverse_iterator q = cur.rbegin(); q != cur.rend(); ++q )
		switchResult ( ret, addToDoEntry ( curNode, *q, dep ) );

	return ret;
}

tacticUsage DlSatTester :: commonTacticBodyOr ( const DLVertex& cur )	// for C \or D concepts
{
#ifdef ENABLE_CHECKING
	fpp_assert ( isNegative(curConcept.bp()) && cur.Type() == dtAnd );	// safety check
#endif

	incStat(nOrCalls);

	if ( isFirstBranchCall() )	// check the structure of OR operation (number of applicable concepts)
	{
		DepSet dep;
		if ( planOrProcessing ( cur, dep ) )
		{	// found existing component
			if ( LLM.isWritable(llGTA) )
				LL << " E(" << OrConceptsToTest.back() << ")";
			return utUnusable;
		}
		if ( OrConceptsToTest.empty() )
		{	// no more applicable concepts:
			// set global dep-set using accumulated deps
			setClashSet(dep);
			return utClash;
		}
			// not a branching: just add a single concept
		if ( OrConceptsToTest.size() == 1 )
		{
			BipolarPointer C = OrConceptsToTest.back();
			return insertToDoEntry ( curNode, C, dep, DLHeap[C].Type(), "bcp" );
		}

		// more than one alternative: use branching context
		createBCOr();
		bContext->branchDep = dep;
		static_cast<BCOr*>(bContext)->applicableOrEntries.swap(OrConceptsToTest);
	}

	// now it is OR case with 1 or more applicable concepts
	return processOrEntry();
}

bool DlSatTester :: planOrProcessing ( const DLVertex& cur, DepSet& dep )
{
	OrConceptsToTest.clear();
	dep = curConcept.getDep();

	// check all OR components for the clash
	const CGLabel& lab = curNode->label();
	const DepSet dummy;
	for ( DLVertex::const_iterator q = cur.begin(), q_end = cur.end(); q < q_end; ++q )
		switch ( tryAddConcept ( lab.getLabel(DLHeap[*q].Type()), inverse(*q), dummy ) )
		{
		case acrClash:	// clash found -- OK
			dep.add(getClashSet());
			continue;
		case acrExist:	// already have such concept -- save it to the 1st position
			OrConceptsToTest.resize(1);
			OrConceptsToTest[0] = inverse(*q);
			return true;
		case acrDone:
			OrConceptsToTest.push_back(inverse(*q));
			continue;
		default:		// safety check
			fpp_unreachable();
		}

	return false;
}

tacticUsage DlSatTester :: processOrEntry ( void )
{
	// save the context here as after save() it would be lost
	const BCOr* bcOr = static_cast<BCOr*>(bContext);
	BCOr::or_iterator p = bcOr->orBeg(), p_end = bcOr->orCur();
	BipolarPointer C = *p_end;
	const char* reason = NULL;
	DepSet dep;

	if ( bcOr->isLastOrEntry() )
	{
		// cumulative dep-set will be used
		prepareBranchDep();
		dep = getBranchDep();
		// no more branching decisions
		determiniseBranchingOp();
		reason = "bcp";
	}
	else
	{
		// save current state
		save();
		// new (just branched) dep-set
		dep = getCurDepSet();
	}

	// if semantic branching is in use -- add previous entries to the label
	if ( useSemanticBranching )
		for ( ; p < p_end; ++p )
			if ( addToDoEntry ( curNode, inverse(*p), dep, "sb" ) != utDone )
				fpp_unreachable();	// Both Exists and Clash are errors

	// add new entry to current node; we know the result would be DONE
	return
#	ifdef RKG_USE_DYNAMIC_BACKJUMPING
		addToDoEntry ( curNode, C, dep, reason );
#	else
		insertToDoEntry ( curNode, C, dep, DLHeap[C].Type(), reason );
#	endif
}

//-------------------------------------------------------------------------------
//	ALL processing
//-------------------------------------------------------------------------------

tacticUsage DlSatTester :: commonTacticBodyAllComplex ( const DLVertex& cur )
{
	tacticUsage ret = utUnusable;

	const DepSet& dep = curConcept.getDep();
	unsigned int state = cur.getState();
	BipolarPointer C = curConcept.bp()-state;	// corresponds to AR{0}.X
	const RAStateTransitions& RST = cur.getRole()->getAutomaton()[state];
	RAStateTransitions::const_iterator q, end = RST.end();

	// apply all empty transitions
	if ( RST.hasEmptyTransition() )
		for ( q = RST.begin(); q != end; ++q )
		{
			incStat(nAutoEmptyLookups);

			if ( (*q)->empty() )
				switchResult ( ret, addToDoEntry ( curNode, C+(*q)->final(), dep, "e" ) );
		}

	// apply final-state rule
	if ( state == 1 )
		switchResult ( ret, addToDoEntry ( curNode, cur.getC(), dep ) );

	// check whether automaton applicable to any edges
	incStat(nAllCalls);

	// check all neighbours
	for ( DlCompletionTree::const_edge_iterator p = curNode->begin(), p_end = curNode->end(); p < p_end; ++p )
		if ( RST.recognise((*p)->getRole()) )
			switchResult ( ret, applyTransitions ( (*p), RST, C, dep+(*p)->getDep() ) );

	return ret;
}

tacticUsage DlSatTester :: commonTacticBodyAllSimple ( const DLVertex& cur )
{
	tacticUsage ret = utUnusable;

	const RAStateTransitions& RST = cur.getRole()->getAutomaton()[0];
	const DepSet& dep = curConcept.getDep();
	BipolarPointer C = cur.getC();

	// check whether automaton applicable to any edges
	incStat(nAllCalls);

	// check all neighbours; as the role is simple then recognise() == applicable()
	for ( DlCompletionTree::const_edge_iterator p = curNode->begin(), p_end = curNode->end(); p < p_end; ++p )
		if ( RST.recognise((*p)->getRole()) )
			switchResult ( ret, addToDoEntry ( (*p)->getArcEnd(), C, dep+(*p)->getDep() ) );

	return ret;
}

//-------------------------------------------------------------------------------
//	Support for ALL processing
//-------------------------------------------------------------------------------

/** Perform expansion of (C=\AR{state}.X).DEP to an EDGE with a given reason */
tacticUsage DlSatTester :: applyTransitions ( const DlCompletionTreeArc* edge,
											const RAStateTransitions& RST,
											BipolarPointer C,
											const DepSet& dep, const char* reason )
{
	DlCompletionTree* node = edge->getArcEnd();
	// fast lane: the single transition which is applicable
	if ( RST.isSingleton() )
		return addToDoEntry ( node, C+RST.getTransitionEnd(), dep, reason );

	RAStateTransitions::const_iterator q, end = RST.end();
	const TRole* R = edge->getRole();
	tacticUsage ret = utUnusable;

	// try to apply all transitions to edge
	for ( q = RST.begin(); q != end; ++q )
	{
		incStat(nAutoTransLookups);
		if ( (*q)->applicable(R) )
			switchResult ( ret, addToDoEntry ( node, C+(*q)->final(), dep, reason ) );
	}

	return ret;
}

//-------------------------------------------------------------------------------
//	SOME processing
//-------------------------------------------------------------------------------

tacticUsage DlSatTester :: commonTacticBodySome ( const DLVertex& cur )	// for ER.C concepts
{
#ifdef ENABLE_CHECKING
	fpp_assert ( isNegative(curConcept.bp()) && cur.Type() == dtForall );
#endif

	const DepSet& dep = curConcept.getDep();
	const TRole* R = cur.getRole();
	BipolarPointer C = inverse(cur.getC());

	tacticUsage ret = utUnusable;

	// check if we already have R-neighbour labelled with C
	if ( isSomeExists ( R, C ) )
		return utUnusable;
	// try to check the case (some R (or C D)), where C is in the label of an R-neighbour
	if ( isNegative(C) && DLHeap[C].Type() == dtAnd )
		for ( DLVertex::const_iterator q = DLHeap[C].begin(), q_end = DLHeap[C].end(); q < q_end; ++q )
			if ( isSomeExists ( R, inverse(*q) ) )
				return utUnusable;

	// check for the case \ER.{o}
	if ( tBox.testHasNominals() && isPositive(C) )
	{
		const DLVertex& nom = DLHeap[C];
		if ( nom.Type() == dtPSingleton || nom.Type() == dtNSingleton )
			return commonTacticBodyValue ( R, static_cast<const TIndividual*>(nom.getConcept()) );
	}

	incStat(nSomeCalls);

	// check if we have functional role
	if ( R->isFunctional() )
		for ( TRole::const_iterator r = R->begin_topfunc(), r_end = R->end_topfunc(); r != r_end; ++r )
			switch ( tryAddConcept ( curNode->label().getLabel(dtLE), (*r)->getFunctional(), dep ) )
			{
			case acrClash:	// addition leads to clash
				return utClash;
			case acrDone:	// should be add to a label
			{
				// we are changing current Node => save it
				updateLevel ( curNode, dep );

				ConceptWDep rFuncRestriction ( (*r)->getFunctional(), dep );
				// NOTE! not added into TODO (because will be checked right now)
				CGraph.addConceptToNode ( curNode, rFuncRestriction, dtLE );
				setUsed(rFuncRestriction.bp());

				if ( LLM.isWritable(llGTA) )
					LL << " nf(" << rFuncRestriction << ")";
			}
				break;
			case acrExist:	// already exists
				break;
			default:		// safety check
				fpp_unreachable();
			}


	bool rFunc = false;				// flag is true if we have functional restriction with this Role name
	const TRole* RF = R;			// most general functional super-role of given one
	ConceptWDep rFuncRestriction;	// role's functional restriction w/dep

	// set up rFunc; rfRole contains more generic functional superrole of rName
	for ( DlCompletionTree::const_label_iterator pc = curNode->beginl_cc(); pc != curNode->endl_cc(); ++pc )
	{	// found such vertex (<=1 R)
		const ConceptWDep& LC = *pc;
		const DLVertex& ver = DLHeap[LC.bp()];

		if ( isPositive(LC.bp()) && isFunctionalVertex(ver) && *ver.getRole() >= *R )
			if ( !rFunc ||	// 1st functional restriction found or another one...
				 *ver.getRole() >= *RF )	// ... with more generic role
			{
				rFunc = true;
				RF = ver.getRole();
				rFuncRestriction = LC;
			}
	}

	if ( rFunc )	// functional role found => add new concept to existing node
	{
		const DlCompletionTreeArc* functionalArc = NULL;
		DepSet newDep;

		// check if we have an (R)-successor or (R-)-predecessor
		for ( DlCompletionTree::const_edge_iterator pr = curNode->begin(), pr_end = curNode->end(); !functionalArc && pr < pr_end ; ++pr )
			if ( (*pr)->isNeighbour ( RF, newDep ) )
				functionalArc = *pr;

		// perform actions if such arc was found
		if ( functionalArc != NULL )
		{
			if ( LLM.isWritable(llGTA) )
				LL << " f(" << rFuncRestriction << "):";

			DlCompletionTree* succ = functionalArc->getArcEnd();

			// add current dependences (from processed entry)
			newDep.add(dep);

			// check if merging will lead to clash because of disjoint roles
			if ( R->isDisjoint() &&
				 checkDisjointRoleClash ( curNode, succ, R, newDep ) == utClash )
				return utClash;

			// add current role label (to both arc and its reverse)
			functionalArc = CGraph.addRoleLabel ( curNode, succ, functionalArc->isPredEdge(), R, newDep );

			// adds concept to the end of arc
			switchResult ( ret, addToDoEntry ( succ, C, newDep ) );

			// if new role label was added...
			if ( RF != R )
			{
				// add Range and Domain of a new role; this includes functional, so remove it from the latter
				switchResult ( ret, initHeadOfNewEdge ( curNode, R, newDep, "RD" ) );
				switchResult ( ret, initHeadOfNewEdge ( succ, R->inverse(), newDep, "RR" ) );

				// check AR.C in both sides of functionalArc
				// FIXME!! for simplicity, check the functionality here (see bEx017). It seems
				// only necessary when R has several functional super-roles, so the condition
				// can be simplified
				switchResult ( ret, applyUniversalNR ( curNode, functionalArc, newDep, redoForall | redoFunc ) );
				// if new role label was added to a functionalArc, some functional restrictions
				// in the SUCC node might became applicable. See bFunctional1x test
				switchResult ( ret, applyUniversalNR ( succ, functionalArc->getReverse(), newDep,
													   redoForall | redoFunc | redoAtMost ) );
			}

			return ret;
		}
	}

	//------------------------------------------------
	// no functional role or 1st arc -- create new arc
	//------------------------------------------------

	// there no such neighbour - create new successor
	// all FUNCs are already checked; no (new) irreflexivity possible
	return createNewEdge ( cur.getRole(), C, redoForall|redoAtMost );
}

/// expansion rule for existential quantifier in the form ER {o}
tacticUsage DlSatTester :: commonTacticBodyValue ( const TRole* R, const TIndividual* nom )
{
	DepSet dep(curConcept.getDep());

	// check blocking conditions
	if ( recheckNodeDBlocked() )
		return utUnusable;

	incStat(nSomeCalls);

	fpp_assert ( nom->node != NULL );

	// if node for NOM was purged due to merge -- find proper one
	DlCompletionTree* realNode = nom->node->resolvePBlocker(dep);

	// check if merging will lead to clash because of disjoint roles
	if ( R->isDisjoint() && checkDisjointRoleClash ( curNode, realNode, R, dep ) == utClash )
		return utClash;

	// here we are sure that there is a nominal connected to a root node
	encounterNominal = true;

	// create new edge between curNode and the given nominal node
	DlCompletionTreeArc* edge =
		CGraph.addRoleLabel ( curNode, realNode, /*linkToParent=*/false, R, dep );

	// add all necessary concepts to both ends of the edge
	return
		setupEdge ( edge, dep, redoForall|redoFunc|redoAtMost|redoIrr ) == utClash
			? utClash
			: utDone;
}

//-------------------------------------------------------------------------------
//	Support for SOME processing
//-------------------------------------------------------------------------------

tacticUsage DlSatTester :: createNewEdge ( const TRole* R, BipolarPointer C, unsigned int flags )
{
	const DepSet& dep = curConcept.getDep();

	// check blocking conditions
	if ( recheckNodeDBlocked() )
	{
		incStat(nUseless);
		return utUnusable;
	}

	DlCompletionTreeArc* pA = createOneNeighbour ( R, dep );

	// add necessary label
	if ( initNewNode ( pA->getArcEnd(), dep, C ) == utClash ||
		 setupEdge ( pA, dep, flags ) == utClash )
		return utClash;
	else
		return utDone;
}

/// create new ROLE-neighbour to curNode; return edge to it
DlCompletionTreeArc* DlSatTester :: createOneNeighbour ( const TRole* R, const DepSet& dep, CTNominalLevel level )
{
	// check whether is called from NN-rule
	bool forNN = level != BlockableLevel;

	// create a proper neighbour
	DlCompletionTreeArc* pA = CGraph.createNeighbour ( curNode, /*isPredEdge=*/forNN, R, dep );
	DlCompletionTree* node = pA->getArcEnd();

	// set nominal node's level if necessary
	if ( forNN )
		node->setNominalLevel(level);

	// check whether created node is data node
	if ( R->isDataRole() )
		node->setDataNode();

	// log newly created node
	CHECK_LL_RETURN_VALUE(llGTA,pA);

	if ( R->isDataRole() )
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
	// FIXME!! if you remove the 1st check, more unblocks would be possible during reasoning
	if ( !curNode->isBlocked() && curNode->isAffected() )
	{
		updateLevel ( curNode, curConcept.getDep() );
		CGraph.updateDBlockedStatus(curNode);
	}

	// if node became d-blocked -- update cache
	if ( curNode->isDBlocked() )
	{
		dBlocked = curNode;
		return true;
	}

	// clear d-blocker cache
	dBlocked = NULL;

	// if node became i-blocked -- don't need to do anything
	if ( curNode->isIBlocked() )
		return true;

	// not blocked
	return false;
}

void
DlSatTester :: applyAllGeneratingRules ( DlCompletionTree* node )
{
	const CGLabel& label = node->label();
	for ( CGLabel::const_iterator p = label.begin_cc(), p_end = label.end_cc(); p != p_end; ++p )
	{
		// need only ER.C or >=nR.C concepts
		if ( isPositive(p->bp()) )
			continue;

		switch ( DLHeap[p->bp()].Type() )
		{
		case dtForall:
		case dtLE:
			addExistingToDoEntry ( node, label.getCCOffset(p), "ubd" );
			break;

		default:
			break;
		}
	}
}

tacticUsage
DlSatTester :: setupEdge ( DlCompletionTreeArc* pA, const DepSet& dep, unsigned int flags )
{
	tacticUsage ret = utUnusable;

	DlCompletionTree* child = pA->getArcEnd();
	DlCompletionTree* from = pA->getReverse()->getArcEnd();

	// adds Range and Domain
	switchResult ( ret, initHeadOfNewEdge ( from, pA->getRole(), dep, "RD" ) );
	switchResult ( ret, initHeadOfNewEdge ( child, pA->getReverse()->getRole(), dep, "RR" ) );

	// check if we have any AR.X concepts in current node
	switchResult ( ret, applyUniversalNR ( from, pA, dep, flags ) );

	// for nominal children and loops -- just apply things for the inverses
	if ( pA->isPredEdge() || child->isNominalNode() || child == from )
		switchResult ( ret, applyUniversalNR ( child, pA->getReverse(), dep, flags ) );
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
											  const DepSet& dep_, unsigned int flags )
{
	// check whether a flag is set
	if ( flags == 0 )
		return utUnusable;

	tacticUsage ret = utUnusable;
	const TRole* R = arcSample->getRole();
	DepSet dep = dep_ + arcSample->getDep();

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
		{
			if ( (flags & redoForall) == 0 )
				break;

			/// check whether transition is possible
			const RAStateTransitions& RST = vR->getAutomaton()[v.getState()];
			if ( !RST.recognise(R) )
				break;

			if ( vR->isSimple() )	// R is recognised so just add the final state!
				switchResult ( ret, addToDoEntry ( arcSample->getArcEnd(), v.getC(), dep+p->getDep(), "ae" ) );
			else
				switchResult ( ret, applyTransitions ( arcSample, RST, p->bp()-v.getState(), dep+p->getDep(), "ae" ) );
			break;
		}

		case dtLE:
			if ( isFunctionalVertex(v) )
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
	TRole::const_iterator r, r_end;

	// if R is functional, then add FR with given DEP-set to NODE
	if ( R->isFunctional() )
		for ( r = R->begin_topfunc(), r_end = R->end_topfunc(); r != r_end; ++r )
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
	fpp_assert ( isPositive(curConcept.bp()) && isFunctionalVertex(cur) );
#endif

	// check whether we need to apply NN rule first
	if ( isNNApplicable ( cur.getRole(), bpTOP, curConcept.bp()+1 ) )
		return commonTacticBodyNN(cur);	// after application func-rule would be checked again

	incStat(nFuncCalls);

	if ( isQuickClashLE(cur) )
		return utClash;

	// locate all R-neighbours of curNode
	DepSet dummy;	// not used
	findNeighbours ( cur.getRole(), bpTOP, dummy );

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
	fpp_assert ( isPositive(curConcept.bp()) && ( cur.Type() == dtLE ) );
#endif

	incStat(nLeCalls);
	BipolarPointer C = cur.getC();
	const TRole* R = cur.getRole();

	tacticUsage ret = utUnusable;

	BCLE* bcLE = NULL;

	if ( !isFirstBranchCall() )
	{
		if ( dynamic_cast<BCChoose*>(bContext) != NULL )
			goto applyCh;	// clash in choose-rule: redo all
		if ( dynamic_cast<BCNN*>(bContext) != NULL )
			goto applyNN;	// clash in NN-rule: skip choose-rule
		if ( dynamic_cast<BCLE*>(bContext) != NULL )
			goto applyLE;	// clash in LE-rule: skip all the rest

		// only these three cases can be here
		fpp_unreachable();
	}

applyCh:

	// check if we have Qualified NR
	if ( C != bpTOP )
		switchResult ( ret, commonTacticBodyChoose ( R, C ) );

	// check whether we need to apply NN rule first
	if ( isNNApplicable ( R, C, /*stopper=*/curConcept.bp()+cur.getNumberLE() ) )
	{
applyNN:
		return commonTacticBodyNN(cur);	// after application <=-rule would be checked again
	}

	// if we are here that it IS first LE call

	if ( isQuickClashLE(cur) )
		return utClash;

	// we need to repeate merge until there will be necessary amount of edges
	while (1)
	{
		if ( isFirstBranchCall() )
		{
			DepSet dep;
			// check the amount of neighbours we have
			findNeighbours ( R, C, dep );

			// if the number of R-neighbours satisfies condition -- nothing to do
			if ( EdgesToMerge.size() <= cur.getNumberLE() )
				return ret;

			// init context
			createBCLE();
			bContext->branchDep += dep;

			// setup BCLE
			bcLE = static_cast<BCLE*>(bContext);

			bcLE->EdgesToMerge.swap(EdgesToMerge);
			bcLE->resetMCI();
		}

applyLE:	// skip init, because here we are after restoring

		bcLE = static_cast<BCLE*>(bContext);

		if ( bcLE->noMoreLEOptions() )
		{	// set global clashset to cummulative one from previous branch failures
			useBranchDep();
			return utClash;
		}

		// get from- and to-arcs using corresponding indexes in Edges
		DlCompletionTreeArc* from = bcLE->getFrom();
		DlCompletionTreeArc* to = bcLE->getTo();

		DepSet dep;	// empty dep-set
		// fast check for from->end() and to->end() are in \neq
		if ( CGraph.nonMergable ( from->getArcEnd(), to->getArcEnd(), dep ) )
		{
			if ( C == bpTOP )	// dep-set is known now
				setClashSet(dep);
			else	// QCR: update dep-set wrt C
			{
				// here we know that C is in both labels; set a proper clash-set
				DagTag tag = DLHeap[C].Type();
				bool test;

				// here dep contains the clash-set
				test = findConcept(from->getArcEnd()->label().getLabel(tag), C, dep );
				fpp_assert(test);
				dep = getClashSet();	// save new dep-set

				test = findConcept(to->getArcEnd()->label().getLabel(tag), C, dep );
				fpp_assert(test);
				// both clash-sets are now in common clash-set
			}

			updateBranchDep();
			bContext->nextOption();
			goto applyLE;
		}

		save();

		// add depset from current level and FROM arc and to current dep.set
		DepSet curDep(getCurDepSet());
		curDep.add(from->getDep());

		switchResult ( ret, Merge ( from->getArcEnd(), to->getArcEnd(), curDep ) );
		// it might be the case (see bIssue28) that after the merge there is an R-neigbour
		// that have neither C or ~C in its label (it was far in the nominal cloud)
		if ( C != bpTOP )
			switchResult ( ret, commonTacticBodyChoose ( R, C ) );
	}
}

tacticUsage DlSatTester :: commonTacticBodyGE ( const DLVertex& cur )	// for >=nR.C concepts
{
#ifdef ENABLE_CHECKING
	fpp_assert ( isNegative(curConcept.bp()) && cur.Type() == dtLE );
#endif

	// check blocking conditions
	if ( recheckNodeDBlocked() )
		return utUnusable;

	incStat(nGeCalls);

	if ( isQuickClashGE(cur) )
		return utClash;

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
		// don't care about return value as clash can't occur
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
	setClashSet ( curConcept.getDep() + reason.getDep() );

	// log clash reason
	if ( LLM.isWritable(llGTA) )
		logClash ( curNode, reason.bp(), reason.getDep() );

	return true;
}

/// aux method that checks whether clash occurs during the merge of labels
bool
DlSatTester :: checkMergeClash ( const CGLabel& from, const CGLabel& to, const DepSet& dep, unsigned int nodeId )
{
	CGLabel::const_iterator p, p_end;
	DepSet clashDep(dep);
	bool clash = false;
	for ( p = from.begin_sc(), p_end = from.end_sc(); p < p_end; ++p )
		if ( isUsed(inverse(p->bp()))
			 && findConcept ( to.getLabel(dtPConcept), inverse(p->bp()), p->getDep() ) )
		{
			clash = true;
			clashDep.add(getClashSet());
			// log the clash
			if ( LLM.isWritable(llGTA) )
				LL << " x(" << nodeId << "," << p->bp() << getClashSet()+dep << ")";
		}
	for ( p = from.begin_cc(), p_end = from.end_cc(); p < p_end; ++p )
		if ( isUsed(inverse(p->bp()))
			 && findConcept ( to.getLabel(dtForall), inverse(p->bp()), p->getDep() ) )
		{
			clash = true;
			clashDep.add(getClashSet());
			// log the clash
			if ( LLM.isWritable(llGTA) )
				LL << " x(" << nodeId << "," << p->bp() << getClashSet()+dep << ")";
		}
	if ( clash )
		setClashSet(clashDep);
	return clash;
}

tacticUsage DlSatTester :: mergeLabels ( const CGLabel& from, DlCompletionTree* to, const DepSet& dep )
{
	tacticUsage ret = utUnusable;
	CGLabel::const_iterator p, p_end;
	CGLabel& lab(to->label());
	// arrays for simple- and complex concepts in the merged-to vector
	CWDArray& sc(lab.getLabel(dtPConcept));
	CWDArray& cc(lab.getLabel(dtForall));

	// due to merging, all the concepts in the TO label
	// should be updated to the new dep-set DEP
	for ( p = sc.begin(), p_end = sc.end(); p < p_end; ++p )
		CGraph.saveRareCond ( sc.updateDepSet ( p->bp(), dep ) );
	for ( p = cc.begin(), p_end = cc.end(); p < p_end; ++p )
		CGraph.saveRareCond ( cc.updateDepSet ( p->bp(), dep ) );

	// if the concept is already exists in the node label --
	// we still need to update it with a new dep-set (due to merging)
	// note that DEP is already there
	for ( p = from.begin_sc(), p_end = from.end_sc(); p < p_end; ++p )
		if ( findConcept ( sc, p->bp() ) )
			CGraph.saveRareCond ( sc.updateDepSet ( p->bp(), p->getDep() ) );
		else
			switchResult ( ret, insertToDoEntry ( to, p->bp(), dep+p->getDep(), DLHeap[p->bp()].Type(), "M" ) );
	for ( p = from.begin_cc(), p_end = from.end_cc(); p < p_end; ++p )
		if ( findConcept ( cc, p->bp() ) )
			CGraph.saveRareCond ( cc.updateDepSet ( p->bp(), p->getDep() ) );
		else
			switchResult ( ret, insertToDoEntry ( to, p->bp(), dep+p->getDep(), DLHeap[p->bp()].Type(), "M" ) );

	return ret;
}

tacticUsage DlSatTester :: Merge ( DlCompletionTree* from, DlCompletionTree* to, const DepSet& depF )
{
	// if node is already purged -- nothing to do
	fpp_assert ( !from->isPBlocked() );

	// prevent node to be merged to itself
	fpp_assert ( from != to );

	// never merge nominal node to blockable one
	fpp_assert ( to->getNominalLevel() <= from->getNominalLevel() );

	if ( LLM.isWritable(llGTA) )
		LL << " m(" << from->getId() << "->" << to->getId() << ")";

	incStat(nMergeCalls);

	// can't merge 2 nodes which are in inequality relation
	DepSet dep(depF);
	if ( CGraph.nonMergable ( from, to, dep ) )
	{
		setClashSet(dep);
		return utClash;
	}

	tacticUsage ret = utUnusable;

	// check for the clash before doing anything else
	if ( checkMergeClash ( from->label(), to->label(), depF, to->getId() ) )
		return utClash;

	// copy all node labels
	switchResult ( ret, mergeLabels ( from->label(), to, depF ) );

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
	// try to check whether link from->to labelled with something disjoint with R
	for ( DlCompletionTree::const_edge_iterator p = from->begin(), p_end = from->end(); p != p_end; ++p )
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

void DlSatTester :: findNeighbours ( const TRole* Role, BipolarPointer C, DepSet& Dep )
{
	EdgesToMerge.clear();
	DagTag tag = DLHeap[C].Type();

	for ( DlCompletionTree::const_edge_iterator p = curNode->begin(), p_end = curNode->end(); p < p_end; ++p )
		if ( (*p)->isNeighbour(Role)
			 && isNewEdge ( (*p)->getArcEnd(), EdgesToMerge.begin(), EdgesToMerge.end() )
			 && findChooseRuleConcept ( (*p)->getArcEnd()->label().getLabel(tag), C, Dep ) )
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

	// apply choose-rule for every R-neighbour
	for ( DlCompletionTree::const_edge_iterator p = curNode->begin(), p_end = curNode->end(); p < p_end; ++p )
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
		createBCCh();
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
	incStat(nNNCalls);

	if ( isFirstBranchCall() )
		createBCNN();

	const BCNN* bcNN = static_cast<BCNN*>(bContext);

	// check whether we did all possible tries
	if ( bcNN->noMoreNNOptions(cur.getNumberLE()) )
	{	// set global clashset to cummulative one from previous branch failures
		useBranchDep();
		return utClash;
	}

	// take next NN number; save it as SAVE() will reset it to 0
	unsigned int NN = bcNN->branchIndex;

	tacticUsage ret = utDone;	// we WILL create several entries

	// prepare to addition to the label
	save();

	// new (just branched) dep-set
	const DepSet curDep(getCurDepSet());

	// make a stopper to mark that NN-rule is applied
	switchResult ( ret, addToDoEntry ( curNode, curConcept.bp() + cur.getNumberLE(), DepSet(), "NNs" ) );

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
bool DlSatTester :: isNNApplicable ( const TRole* r, BipolarPointer C, BipolarPointer stopper ) const
{
	// NN rule is only applicable to nominal nodes
	if ( !curNode->isNominalNode() )
		return false;

	// check whether the NN-rule was already applied here for a given concept
	if ( curNode->isLabelledBy(stopper) )
		return false;

	// check for the real applicability of the NN-rule here
	DlCompletionTree::const_edge_iterator p, p_end = curNode->end(), q;
	for ( p = curNode->begin(); p != p_end; ++p )
	{
		const DlCompletionTree* suspect = (*p)->getArcEnd();

		// if there is an edge that require to run the rule, then we need it
		if ( (*p)->isPredEdge() && suspect->isBlockableNode() && (*p)->isNeighbour(r) && suspect->isLabelledBy(C) )
		{
			if ( LLM.isWritable(llGTA) )
				LL << " NN(" << suspect->getId() << ")";

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
	// check blocking conditions
	if ( recheckNodeDBlocked() )
		return utUnusable;

	// nothing to do if R-loop already exists
	for ( DlCompletionTree::const_edge_iterator p = curNode->begin(), p_end = curNode->end(); p < p_end; ++p )
		if ( (*p)->getArcEnd() == curNode && (*p)->isNeighbour(R) )
			return utUnusable;

	// create an R-loop through curNode
	const DepSet& dep = curConcept.getDep();
	DlCompletionTreeArc* pA = CGraph.createLoop ( curNode, R, dep );
	return setupEdge ( pA, dep, redoForall|redoFunc|redoAtMost|redoIrr );
}

tacticUsage DlSatTester :: commonTacticBodyIrrefl ( const TRole* R )
{
	// return clash if R-loop is found
	for ( DlCompletionTree::const_edge_iterator p = curNode->begin(), p_end = curNode->end(); p < p_end; ++p )
		if ( checkIrreflexivity ( *p, R, curConcept.getDep() ) == utClash )
			return utClash;

	return utDone;
}

//-------------------------------------------------------------------------------
//	Support for projection R\C -> P
//-------------------------------------------------------------------------------

tacticUsage DlSatTester :: commonTacticBodyProj ( const TRole* R, BipolarPointer C, const TRole* ProjR )
{
	// find an R-edge, try to apply projection-rule to it
	tacticUsage ret = utUnusable;

	// if ~C is in the label of curNode, do nothing
	if ( curNode->isLabelledBy(inverse(C)) )
		return utUnusable;

	// FIXME!! checkProjection() might change curNode's edge vector and thusly invalidate iterators
	DlCompletionTree::const_edge_iterator p = curNode->begin(), p_end = curNode->end();

	for ( int i = 0, n = p_end - p; i < n; ++i )
	{
		p = curNode->begin() + i;
		if ( (*p)->isNeighbour(R) )
			switchResult ( ret, checkProjection ( *p, C, ProjR ) );
	}

	return ret;
}

tacticUsage DlSatTester :: checkProjection ( DlCompletionTreeArc* pA, BipolarPointer C, const TRole* ProjR )
{
	// nothing to do if pA is labelled by ProjR as well
	if ( pA->isNeighbour(ProjR) )
		return utUnusable;
	// if ~C is in the label of curNode, do nothing
	if ( curNode->isLabelledBy(inverse(C)) )
		return utUnusable;

	// neither C nor ~C are in the label: make a choice
	tacticUsage ret = utUnusable;

	DepSet dep(curConcept.getDep());
	dep.add(pA->getDep());

	if ( !curNode->isLabelledBy(C) )
	{
		if ( isFirstBranchCall() )
		{
			createBCCh();
			// save current state
			save();

			return addToDoEntry ( curNode, inverse(C), getCurDepSet(), "cr0" );
		}
		else
		{
			prepareBranchDep();
			dep.add(getBranchDep());
			determiniseBranchingOp();
			switchResult ( ret, addToDoEntry ( curNode, C, dep, "cr1" ) );
		}
	}
	// here C is in the label of curNode: add ProjR to the edge if necessary
	DlCompletionTree* child = pA->getArcEnd();
	pA = CGraph.addRoleLabel ( curNode, child, pA->isPredEdge(), ProjR, dep );
	int flags = redoForall|redoFunc|redoAtMost|redoIrr;
	switchResult ( ret, setupEdge ( pA, dep, flags ) );

	return ret;
}

