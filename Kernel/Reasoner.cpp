/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2007 by Dmitry Tsarkov

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

#include "Reasoner.h"
#include "logging.h"

// comment the line out for printing tree info before save and after restoring
//#define __DEBUG_SAVE_RESTORE
// comment the line out for flushing LL after dumping significant piece of info
//#define __DEBUG_FLUSH_LL

AccumulatedStatistic* AccumulatedStatistic::root = NULL;

DlSatTester :: DlSatTester ( TBox& tbox, const ifOptionSet* Options )
	: tBox(tbox)
	, DLHeap(tbox.DLHeap)
	, CGraph(1)
	, TODO(new ToDoTableType)
	, DTReasoner(tbox.DLHeap)
	, GCIs(tbox.GCIs)
	, bContext(NULL)
	, tryLevel(1)
{
	// init local options
	readConfig ( Options );
	// init static part of CTree
	DlCompletionTree::initContext ( &tbox.DLHeap, useLazyBlocking );
	// init datatype reasoner
	tBox.getDataTypeCenter().initDataTypeReasoner(DTReasoner);
	// init vector of nominals defined in TBox
	initNominalVector();
	// init set of reflexive roles
	GCIs.setReflexive(tbox.getRM()->fillReflexiveRoles(ReflexiveRoles));

	resetSessionFlags ();
}

// load init values from config file
void DlSatTester :: readConfig ( const ifOptionSet* Options )
{
	assert ( Options != NULL );	// safety check

// define a macro for registering boolean option
#	define addBoolOption(name)				\
	name = Options->getBool ( #name );		\
	if ( LLM.isWritable(llAlways) )			\
		LL << "Init " #name " = " << name << "\n"
	addBoolOption(useSemanticBranching);
	addBoolOption(useBackjumping);
	addBoolOption(useLazyBlocking);
	addBoolOption(useDagCache);
#undef addBoolOption
}

// register all nominals defined in TBox
void DlSatTester :: initNominalVector ( void )
{
	Nominals.clear();

	for ( TBox::i_iterator pi = tBox.i_begin(); pi != tBox.i_end(); ++pi )
		if ( !(*pi)->isSynonym() )
			Nominals.push_back(*pi);
}


void DlSatTester :: clearNominalCloud ( void )
{
	if ( Nominals.empty() )
		return;

	SingletonVector::iterator p = Nominals.begin();

	// check if nominals were touched.
	// FIXME!! check later on where not ALL nominals will be touched
//	if ( (*p)->node == NULL )
//		return;

	// clear node link in all nominals
	for ( ; p != Nominals.end(); ++p )
		(*p)->node = NULL;
}

void DlSatTester :: clear ( void )
{
	CGraph.clear();

	// clear last session information
	resetSessionFlags ();

	AccumulatedStatistic::accumulateAll();

	Stack.clear();

	TODO->clear ();
	clearNominalCloud();

	bContext = NULL;
	tryLevel = 1;
}

addConceptResult
DlSatTester :: tryAddConcept ( const CGLabel& lab, DagTag tag, BipolarPointer c,
							   const DepSet& dep ) const
{
	// check whether C or ~C can occurs in a node label
	bool canC = isUsed(c);
	bool canNegC = isUsed(inverse(c));

	// if either C or ~C is used already, it's not new in a label
	if ( canC )
	{
		if ( canNegC )	// both C and ~C can be in the label
			return lab.checkAddedConcept(tag,c,dep);
		else			// C but not ~C can be in the label
			return lab.checkAddedConceptP(tag,c);
	}
	else
	{
		if ( canNegC )	// ~C but not C can be in the label
			return lab.checkAddedConceptN(tag,c,dep);
		else			// neither C nor ~C can be in the label
			return acrDone;
	}
}

tacticUsage DlSatTester :: addToDoEntry ( DlCompletionTree* n, BipolarPointer c, const DepSet& dep,
										  const char* reason )
{
	if ( c == bpTOP )	// simplest things first
		return utUnusable;
	if ( c == bpBOTTOM )
	{
		DlCompletionTree::setClashSet(dep);
		if ( LLM.isWritable(llGTA) )
			logClash ( n, c, dep );
		return utClash;
	}

	const DLVertex& v = DLHeap[c];
	DagTag tag = v.Type();

	// try to add a concept to a node label
	switch ( tryAddConcept ( n->label(), tag, c, dep ) )
	{
	case acrClash:	// clash -- return
		if ( LLM.isWritable(llGTA) )
			logClash ( n, c, dep );
		return utClash;
	case acrExist:	// already exists -- nothing new
		return utUnusable;
	case acrDone:	// try was done
		return insertToDoEntry ( n, c, dep, tag, reason );
	default:	// safety check
		assert (0);
	}
}

/// insert P to the label of N; do necessary updates; may return Clash in case of data node N
tacticUsage DlSatTester :: insertToDoEntry ( DlCompletionTree* n, BipolarPointer c, const DepSet& dep,
											 DagTag tag, const char* reason = NULL )
{
	// we will need this tag for TODO entry anyway
	ConceptWDep p ( c, dep );

	// we will change current Node => save it if necessary
	updateLevel ( n, dep );
	n->addConcept ( p, tag );

	setUsed(c);

	if ( n->isDataNode() )	// data concept -- run data center for it
		return checkDataNode ? checkDataClash(n) : utUnusable;

	if ( n->isCached() )
	{
		tacticUsage ret = correctCachedEntry(n);
		return ret != utUnusable ? ret : utDone;
	}

	// add new info in TODO list
	TODO->addEntry ( n, c, tag );

	// inform about it
	if ( LLM.isWritable(llGTA) )
		logEntry ( n, c, dep, reason );

	return utDone;
}

tacticUsage DlSatTester :: correctCachedEntry ( DlCompletionTree* n )
{
	assert ( n->isCached() );	// safety check

	// FIXME!! check if it is possible to leave node cached in more efficient way
	tacticUsage ret = tryCacheNode(n);

	if ( ret != utUnusable )
		return ret;

	// uncheck cached node status and add all elements in TODO list
	n->setCached(false);

	redoNodeLabel ( n, "ce" );
	return ret;
}

void DlSatTester :: generateCacheClashLevel ( DlCompletionTree* node,
											  modelCacheInterface* cache ATTR_UNUSED )
{
	// set clash level by merging together all dep-sets of a node label
	//FIXME!! this could be done better
	DepSet cur;
	DlCompletionTree::const_label_iterator p;

	for ( p = node->beginl_sc(); p != node->endl_sc(); ++p )
		cur.add(p->getDep());
	for ( p = node->beginl_cc(); p != node->endl_cc(); ++p )
		cur.add(p->getDep());

	DlCompletionTree::setClashSet(cur);
}

/// @return false iff given data node contains data contradiction
tacticUsage DlSatTester :: checkDataClash ( const DlCompletionTree* Node )
{
	assert ( Node && Node->isDataNode() );	// safety check

	DTReasoner.clear();

	// data node may contain only "simple" concepts in there
	for ( DlCompletionTree::const_label_iterator r = Node->beginl_sc(); r != Node->endl_sc(); ++r )
		if ( DTReasoner.addDataEntry(*r) )	// clash found
			return utClash;

	return DTReasoner.checkClash() ? utClash : utDone;
}

bool
DlSatTester :: consistentNominalCloud ( void )
{
	if ( LLM.isWritable(llBegSat) )
		LL << "\n--------------------------------------------\n"
			  "Checking consistency of an ontology with individuals:";
	if ( LLM.isWritable(llGTA) )
		LL << "\n";

	bool result = false;

	do
	{	// reserve the root for the forthcoming reasoning
		if ( initNewNode ( CGraph.getRoot(), DepSet(), bpTOP ) == utClash ||
			 initNominalCloud(DepSet()) )	// clash during initialisation
			break;

		// make reasoning up-to-branching

		// finish normal reasoning
		result = runSat();
	} while(0);

	if ( LLM.isWritable(llSatResult) )
		LL << "\nThe ontology is " << (result ? "consistent" : "INCONSISTENT");

	if ( !result )
		return false;

	// ABox is consistent -> create cache for every nominal in KB
	for ( SingletonVector::iterator p = Nominals.begin(); p != Nominals.end(); ++p )
		registerNominalCache(*p);

	return true;
}

bool DlSatTester :: runSat ( void )
{
	// for time calculation
	TsProcTimer pt;
	pt.Start ();
	bool result = checkSatisfiability ();
	pt.Stop ();

	// log the result ant calculation time
	nAddOps.set(DlCompletionTree::getNAddOps());
	nCompareOps.set(DlCompletionTree::getNSetCompareOps());
	nNodeSaves.set(CGraph.getNNodeSaves());
	nNodeRestores.set(CGraph.getNNodeRestores());


	if ( LLM.isWritable(llSatTime) )
		LL << "\nChecking time was " << pt << " seconds";

	if ( LLM.isWritable(llRStat) )
		logStatisticData ( LL, /*needLocal=*/true );

	if ( result )
		writeRoot(llRStat);

	return result;
}

	/// create nominal nodes for all individuals in TBox
bool DlSatTester :: initNominalCloud ( const DepSet& dep )
{
	// prepare to clear nominal links in case of clash
	CGraph.saveRareCond(new TNominalCloudRestorer(this));

	// create nominal nodes and fills them with initial values
	for ( SingletonVector::iterator p = Nominals.begin(); p != Nominals.end(); ++p )
		if ( initNominalNode ( *p, dep ) )
			return true;	// ABox is inconsistent

	// create edges between related nodes
	if ( !tBox.isPrecompleted() )	// not needed for precompleted KB
		for ( TBox::RelatedCollection::const_iterator q = tBox.RelatedI.begin(); q != tBox.RelatedI.end(); ++q, ++q )
			if ( initRelatedNominals(*q) )
				return true;	// ABox is inconsistent

	// create disjoint markers on nominal nodes
	if ( tBox.Different.empty() )
		return false;

	for ( TBox::DifferentIndividuals::const_iterator
		  r = tBox.Different.begin(); r != tBox.Different.end(); ++r )
	{
		CGraph.initIR();
		for ( SingletonVector::const_iterator p = r->begin(); p != r->end(); ++p )
			if ( CGraph.setCurIR ( (*p)->node, dep ) )	// different(c,c)
				return true;
		CGraph.finiIR();
	}

	// init was OK
	return false;
}

bool DlSatTester :: initRelatedNominals ( const TRelated* rel )
{
	DlCompletionTree* from = rel->a->node;
	DlCompletionTree* to = rel->b->node;
	TRole* R = rel->R;
	const DepSet& dep = curConcept.getDep();

	// check if merging will lead to clash because of disjoint roles
	if ( R->isDisjoint() && checkDisjointRoleClash ( from, to, R, dep ) == utClash )
		return true;

	// create new edge between FROM and TO
	DlCompletionTreeArc* pA =
		CGraph.addRoleLabel ( from, to, /*isUpLink=*/false, R, dep );

	// return OK iff setup new enge didn't lead to clash
	// do NOT need to re-check anything: nothing was processed yet
	return setupEdge ( pA, from, dep, 0 ) == utClash;
}

bool DlSatTester :: applyReflexiveRoles ( DlCompletionTree* node, const DepSet& dep )
{
	for ( RoleMaster::roleSet::const_iterator p = ReflexiveRoles.begin(); p != ReflexiveRoles.end(); ++p )
	{
		// create R-loop through the NODE
		DlCompletionTreeArc* pA = CGraph.addRoleLabel ( node, node, /*isUpLink=*/false, *p, dep );
		if ( setupEdge ( pA, node, dep, 0 ) == utClash )
			return true;
	}

	// no clash found
	return false;
}

const modelCacheInterface* DlSatTester :: fillsCache ( BipolarPointer p )
{
	assert ( isValid(p) );	// safety check

	const modelCacheInterface* ret = DLHeap.getCache(p);

	// check if cache already calculated
	if ( ret != NULL )
		return ret;

//	std::cout << "\nCCache for " << p << ":";
	prepareCascadedCache(p);
	return DLHeap.setCache ( p, createCache(p) );
}

void
DlSatTester :: prepareCascadedCache ( BipolarPointer p )
{
	static std::set<BipolarPointer> inProcess;

	/// cycle found -- shall be processed without caching
	if ( inProcess.find(p) != inProcess.end() )
	{
//		std::cout << " cycle with " << p << ";";
		return;
	}

	const DLVertex& v = DLHeap[p];
	bool pos = isPositive(p);

	// check if a concept already cached
	if ( v.getCache(pos) != NULL )
		return;

	switch ( v.Type() )
	{
	case dtTop:
		break;

	case dtDataType:	// data things are checked by data inferer
	case dtDataValue:
	case dtDataExpr:
		break;

	case dtAnd:
	{
		for ( DLVertex::const_iterator q = v.begin(), q_end = v.end(); q < q_end; ++q )
			prepareCascadedCache ( pos ? *q : inverse(*q) );
		break;
	}

	case dtPSingleton:
	case dtNSingleton:
	case dtNConcept:
	case dtPConcept:
		if ( isNegative(p) && isPNameTag(v.Type()) )
			return;
		inProcess.insert(p);
//		std::cout << " expanding " << p << ";";
		prepareCascadedCache(pos ? v.getC() : inverse(v.getC()));
		inProcess.erase(p);
		break;

	case dtForall:
	case dtLE:
	{
		if ( v.getRole()->isDataRole() )	// skip data-related stuff
			break;
		BipolarPointer x = pos ? v.getC() : inverse(v.getC());

		// build cache for C in \AR.C
		if ( x != bpTOP )
		{
			inProcess.insert(x);
//			std::cout << " caching " << x << ";";
			fillsCache(x);
			inProcess.erase(x);
		}

		// build cache for the Range(R) if necessary
		x = v.getRole()->getBPRange();
		if ( x != bpTOP )
		{
			inProcess.insert(x);
//			std::cout << " caching range(" << v.getRole()->getName() << ") = " << x << ";";
			fillsCache(x);
			inProcess.erase(x);
		}

		break;
	}

	case dtIrr:
		break;

	default:
		assert(0);
	}
}

/// create cache for given DAG node; @return cache
modelCacheInterface*
DlSatTester :: createCache ( BipolarPointer p )
{
	if ( LLM.isWritable(llDagSat) )
	{
		LL << "\nChecking satisfiability of DAG entry " << p << ":\n";
#	ifdef __DEBUG_FLUSH_LL
		LL.flush ();
#	endif
	}

	bool sat = runSat(p);

	if ( sat )	// here we need actual (not a p-blocked) root of the tree
		return createModelCache(CGraph.getActualRoot());

	// fails to prove => P -> \bot
	if ( LLM.isWritable(llAlways) )
		LL << "\nDAG entry " << p << " is unsatisfiable\n";
	return createModelCache(bpBOTTOM);
}

bool DlSatTester :: checkSatisfiability ( void )
{
	tacticUsage ret = utDone;	// if nothing to do -- OK.

	while ( !TODO->empty () )
	{
		// FIXME: insert here a list of tactics
		const ToDoEntry* curTDE = TODO->getNextEntry ();
		assert ( curTDE != NULL );

		// setup current context
		curNode = curTDE->Node;
		curConcept = curNode->label().getConcept(curTDE->index);

		ret = commonTactic();

		// if tactic returns clash...
		while ( ret == utClash )
		{
			if ( tunedRestore () )	// restoring the state
				return false;		// ... the concept is unsatisfiable
			else					//  ... try to use next alternative
				ret = commonTactic();
		}
	}

	// we are here if either no tactic is usable (ret = utUnusable)
	// or there are no entryes to deal with (then the answer depends on ret)
	return ( ret != utClash );
}

/********************************************************************************
  *
  *  Save/Restore section
  *
  ******************************************************************************/

	/// save local state to BContext
void DlSatTester :: saveBC ( void )
{
	// save reasoning context
	bContext->curNode = curNode;
	bContext->curConcept = curConcept;
}
	/// restore local state from BContext
void DlSatTester :: restoreBC ( void )
{
	// restore reasoning context
	curNode = bContext->curNode;
	curConcept = bContext->curConcept;

	// update branch dep-set
	updateBranchDep();
	bContext->nextOption();
}

void DlSatTester :: save ( void )
{
	// save local vars
	saveBC();

	// save tree
	CGraph.save();

	// save ToDoList
	TODO->save ();

	// increase tryLevel
	++tryLevel;
	DepSet::ensureLevel(getCurLevel());

	// init BC
	clearBC();

	nStateSaves.inc();

	if ( LLM.isWritable(llSRState) )
		LL << " ss(" << getCurLevel()-1 << ")";
#ifdef __DEBUG_SAVE_RESTORE
	writeRoot(llSRState);
#endif
}

void DlSatTester :: restore ( unsigned int newTryLevel )
{
	assert ( !Stack.empty () );
	assert ( newTryLevel > 0 );

	// skip all intermediate restorings
	setCurLevel(newTryLevel);

	// restore local
	bContext = Stack.top(getCurLevel());
	restoreBC();

	// restore tree
	CGraph.restore(getCurLevel());

	// restore TODO list
	TODO->restore(getCurLevel());

	nStateRestores.inc();

	if ( LLM.isWritable(llSRState) )
		LL << " sr(" << getCurLevel() << ")";
#ifdef __DEBUG_SAVE_RESTORE
	writeRoot(llSRState);
#endif
}

/**
  * logging methods
  */

void DlSatTester :: logStartEntry ( void ) const
{
	CHECK_LL_RETURN(llGTA);	// useless but safe

	LL << "\n";
	for ( register unsigned int i = getCurLevel(); --i; )
		LL << " ";
	LL << "[*(";
	curNode->logNode ();
	LL << "," << curConcept << "){";
	if ( isNegative (curConcept.bp()) )
		LL << "~";
	LL << DLHeap[curConcept.bp()].getTagName() << "}:";
}

void DlSatTester :: logFinishEntry ( tacticUsage res ) const
{
	CHECK_LL_RETURN(llGTA);	// useless but safe

	LL << "] ";
	switch (res)
	{
	case utDone:
		LL << "utDone";
		break;
	case utUnusable:
		LL << "utUnusable";
		break;
	case utClash:
		LL << "utClash" << DlCompletionTree::getClashSet();
		break;
	default:	// safety check
		assert (0);
	}
#ifdef __DEBUG_FLUSH_LL
	LL.flush ();
#endif
}

void DlSatTester :: logStatisticData ( std::ostream& o, bool needLocal ) const
{
	nTacticCalls.Print	( o, needLocal, "\nThere were made ", " tactic operations, of which:" );
	nIdCalls.Print		( o, needLocal, "\n    CN   operations: ", "" );
	nSingletonCalls.Print(o, needLocal, "\n           including ", " singleton ones" );
	nOrCalls.Print		( o, needLocal, "\n    OR   operations: ", "" );
	nAndCalls.Print		( o, needLocal, "\n    AND  operations: ", "" );
	nSomeCalls.Print	( o, needLocal, "\n    SOME operations: ", "" );
	nAllCalls.Print		( o, needLocal, "\n    ALL  operations: ", "" );
	nFuncCalls.Print	( o, needLocal, "\n    Func operations: ", "" );
	nLeCalls.Print		( o, needLocal, "\n    LE   operations: ", "" );
	nGeCalls.Print		( o, needLocal, "\n    GE   operations: ", "" );
	nUseless.Print		( o, needLocal, "\n    N/A  operations: ", "" );

	nNNCalls.Print		( o, needLocal, "\nThere were made ", " NN rule application" );
	nMergeCalls.Print	( o, needLocal, "\nThere were made ", " merging operations" );

	nStateSaves.Print		( o, needLocal, "\nThere were made ", " save(s) of global state" );
	nStateRestores.Print	( o, needLocal, "\nThere were made ", " restore(s) of global state" );
	nNodeSaves.Print		( o, needLocal, "\nThere were made ", " save(s) of tree state" );
	nNodeRestores.Print		( o, needLocal, "\nThere were made ", " restore(s) of tree state" );
	nAddOps.Print			( o, needLocal, "\nThere were made ", " concept insertion ops" );
	nCompareOps.Print		( o, needLocal, "\nThere were made ", " set compare ops" );

	nCachedSat.Print		( o, needLocal, "\nThere were build ", " cached satisfiable nodes" );
	nCachedUnsat.Print		( o, needLocal, "\nThere were build ", " cached unsatisfiable nodes" );
	nCacheFailures.Print	( o, needLocal, "\nThere were ", " cached failures" );
	if ( !needLocal )
		o << "\nThe maximal graph size is " << CGraph.maxSize() << " nodes";
}

float
DlSatTester :: printReasoningTime ( std::ostream& o ) const
{
	o << "\n     SAT takes " << satTimer << " seconds\n     SUB takes " << subTimer << " seconds";
	return satTimer + subTimer;
}

