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

#ifndef _REASONER_H
#define _REASONER_H

#include "globaldef.h"
#include "comerror.h"
#include "dlCompletionGraph.h"
#include "dlTBox.h"
#include "dlDag.h"
#include "modelCacheSingleton.h"
#include "modelCacheIan.h"
#include "tSaveStack.h"
#include "procTimer.h"
#include "DataReasoning.h"
#include "ToDoList.h"
#include "tFastSet.h"

// Enum for usage the Tactics to a ToDoEntry
enum tacticUsage { utUnusable, utClash, utDone };

/// class for gathering statistic both for session and totally
class AccumulatedStatistic
{
private:	// prohibit copy
		/// copy c'tor (unimplemented)
	AccumulatedStatistic ( const AccumulatedStatistic& );
		/// assignment (unimplemented)
	AccumulatedStatistic& operator = ( const AccumulatedStatistic& );

protected:	// static members
		/// all members are linked together
	static AccumulatedStatistic* root;

public:		// static methods
		/// accumulate all registered statistic elements
	static void accumulateAll ( void )
	{
#	ifndef RELEASE	// don't gather statistic for the release versions
		for ( AccumulatedStatistic* cur = root; cur; cur = cur->next )
			cur->accumulate();
#	endif
	}

protected:	// members
		/// accumulated statistic
	unsigned int total;
		/// current session statistic
	unsigned int local;
		/// link to the next element
	AccumulatedStatistic* next;

public:		// interface
		/// c'tor: link itself to the list
	AccumulatedStatistic ( void ) : total(0), local(0), next(root) { root = this; }
		/// d'tor: remove link from the list
	~AccumulatedStatistic ( void )
	{
		// all statistic elements are deleted in the inversed creation order
		if ( root == this )
		{
			root = next;
			return;
		}
		// we are here if it's multi-environment and arbitrary deletion order was chosen
		AccumulatedStatistic *prev = root, *cur = root->next;
		// find a pointer to current node
		for ( ; cur && cur != this; prev = cur, cur = cur->next )
			(void)NULL;
		if ( cur == this )
			prev->next = next;
	}

	// access to the elements

		/// get (RO) value of the element
	unsigned int get ( bool needLocal ) const { return needLocal ? local : total; }

	// general statistic operators

		/// increment local value
	void inc ( void )
	{
#	ifndef RELEASE	// don't gather statistic for the release versions
		++local;
#	endif
	}
		/// set local value to particular N
	void set ( unsigned int n )
	{
#	ifndef RELEASE	// don't gather statistic for the release versions
		local = n;
#	endif
	}
		/// add local value to a global one
	void accumulate ( void ) { total += local; local = 0; }

	// output

	void Print ( std::ostream& o, bool needLocal, const char* prefix, const char* suffix ) const
	{
		if ( get(needLocal) > 0 )
			o << prefix << get(needLocal) << suffix;
	}
}; // AccumulatedStatistic

class DlSatTester
{
protected:	// type definition
		/// TODO table type
	typedef ToDoList ToDoTableType;
		/// vector of singletons
	typedef TBox::SingletonVector SingletonVector;
		/// vector of edges
	typedef std::vector<DlCompletionTreeArc*> EdgeVector;
		/// RO label iterator
	typedef DlCompletionTree::const_label_iterator const_label_iterator;

protected:	// types
		/// possible flags of re-checking ALL-like expressions in new nodes
	enum { redoForall = 1, redoFunc = 2, redoAtMost = 4, redoIrr = 8 };
		/// reason of the branching
	enum BranchingTag
	{
		btOr,		// branching because of disjunction
		btLE,		// brunching because of <=-rule application
		btChoose,	// branching because of choose-rule
		btNN,		// brunching because of NN-rule
		btBarrier,	// not a real branching, just to remember a state
	};

protected:	// classes
		/// class for saving branching context of a Reasoner
	class BranchingContext
	{
	private:	// prevent copy
			/// no copy c'tor
		BranchingContext ( const BranchingContext& s );
			/// no assignment
		BranchingContext& operator = ( const BranchingContext& s );

	public:		// types
			/// short OR indexes
		typedef std::vector<BipolarPointer> OrIndex;
			/// short OR index iterator
		typedef OrIndex::const_iterator or_iterator;

	public:		// members
			/// currently processed node
		DlCompletionTree* curNode;
			/// currently processed concept
		ConceptWDep curConcept;
			/// positions of the Used members
		size_t pUsedIndex, nUsedIndex;
			/// current branching index; used in several branching rules
		unsigned int branchIndex;
			/// index of a merge-candidate (in LE concept)
		unsigned int mergeCandIndex;
			/// useful disjuncts (ready to add) in case of OR
		OrIndex applicableOrEntries;
			/// vector of edges to be merged
		DlSatTester::EdgeVector EdgesToMerge;
			/// dependences for branching clashes
		DepSet branchDep;
			/// branching tag
		BranchingTag tag;

	public:		// interface
			/// empty c'tor
		BranchingContext ( void ) : curNode (NULL), curConcept (bpINVALID) {}
			/// empty d'tor
		~BranchingContext ( void ) {}
			/// init tag and indeces
		void init ( BranchingTag btag )
		{
			tag = btag;
			branchIndex = btag == btNN ? 1 : 0;
			mergeCandIndex = 0;
		}
			/// correct mergeCandIndex after changing
		void resetMCI ( void ) { mergeCandIndex = EdgesToMerge.size()-1; }
			/// give the next branching alternative
		void nextOption ( void )
		{
			switch ( tag )
			{
			case btOr:	// OR branching
			case btNN:	// NN branching -- nothing to do (all will be done in tacticNN)
				++branchIndex;		// inc index in order to get next entry
				break;
			case btLE:	// LE branching
				--mergeCandIndex;	// get new merge candidate
				if ( mergeCandIndex == branchIndex )	// nothing more can be mergeable to BI node
				{
					++branchIndex;	// change the candidate to merge to
					resetMCI();
				}
				break;
			case btChoose:	// choose-rule branching
			case btBarrier:	// nothing to do
				break;
			default:	// safety
				assert(0);
			}
		}

		// access to the fields

			/// get FROM pointer to merge
		DlCompletionTreeArc* getFrom ( void ) const { return EdgesToMerge[mergeCandIndex]; }
			/// get FROM pointer to merge
		DlCompletionTreeArc* getTo ( void ) const { return EdgesToMerge[branchIndex]; }
			/// check if the current processing OR entry is the last one
		bool isLastOrEntry ( void ) const { return applicableOrEntries.size() == branchIndex+1; }
			/// check if the LE has no option to process
		bool noMoreLEOptions ( void ) const { return mergeCandIndex <= branchIndex; }
			/// check if the NN has no option to process
		bool noMoreNNOptions ( unsigned int n ) const { return branchIndex > n; }
			/// check if the LE is not applicable
		bool notApplicableLE ( unsigned int n ) const { return EdgesToMerge.size() <= n; }
			/// 1st element of OrIndex
		or_iterator orBeg ( void ) const { return applicableOrEntries.begin(); }
			/// current element of OrIndex
		or_iterator orCur ( void ) const { return orBeg() + branchIndex; }
	}; // BranchingContext

protected:	// members
		/// host TBox
	TBox& tBox;
		/// link to dag from TBox
	DLDag& DLHeap;
		/// all nominals defined in TBox
	SingletonVector Nominals;
		/// all the reflexive roles
	RoleMaster::roleSet ReflexiveRoles;

		/// Completion Graph of tested concept(s)
	DlCompletionGraph CGraph;
		/// TODO list
	ToDoTableType TODO;
		/// reasoning subsystem for the datatypes
	DataTypeReasoner DTReasoner;
		/// Used sets for pos- and neg- entries
	TFastSet<unsigned int> pUsed, nUsed;

		/// GCI-related KB flags
	TKBFlags GCIs;

		/// timer for the SAT tests (ie, cache creation)
	TsProcTimer satTimer;
		/// timer for the SUB tests (ie, general subsumption)
	TsProcTimer subTimer;

	// save/restore option

		/// stack for the local reasoner's state
	TSaveStack<BranchingContext> Stack;
		/// context from the restored branching rule
	BranchingContext* bContext;
		/// index of last non-det situation
	unsigned int tryLevel;

	// statistic elements

	AccumulatedStatistic
		nTacticCalls,
		nUseless,

		nIdCalls,
		nSingletonCalls,
		nOrCalls,
		nAndCalls,
		nSomeCalls,
		nAllCalls,
		nFuncCalls,
		nLeCalls,
		nGeCalls,

		nNNCalls,
		nMergeCalls,

		nSRuleAdd,
		nSRuleFire,

		nStateSaves,
		nStateRestores,
		nNodeSaves,
		nNodeRestores,

		nLookups,
		nCompareOps,

		// reasoning cache
		nCacheTry,
		nCacheFailedNoCache,
		nCacheFailedShallow,
		nCacheFailed,
		nCachedSat,
		nCachedUnsat;

	// current values

		/// currently processed CTree node
	DlCompletionTree* curNode;
		/// currently processed Concept
	ConceptWDep curConcept;

		/// last processed d-blocked node
	DlCompletionTree* dBlocked;

		/// size of the DAG with some extra space
	size_t dagSize;

		/// contains clash set if clash is encountered in a node label
	DepSet clashSet;

		/// flag for switching semantic branching
	bool useSemanticBranching;
		/// flag for switching backjumping
	bool useBackjumping;
		/// whether or not check blocking status as late as possible
	bool useLazyBlocking;
		/// flag for switching between Anywhere and Ancestor blockings
	bool useAnywhereBlocking;

	// session status flags:

		/// true if nominal-related expansion rule was fired during reasoning
	bool encounterNominal;
		/// flag to show if it is necessary to produce DT reasoning immideately
	bool checkDataNode;

private:	// no copy
		/// no copy c'tor
	DlSatTester ( const DlSatTester& );
		/// no assignment
	DlSatTester& operator = ( const DlSatTester& );

protected:	// methods

	/// resets all session flags
	void resetSessionFlags ( void );

	//------------  methods  ----------------------------
	// adds p to the labels of node n.
	// returns utDone, utClash or utUnused
	tacticUsage addToDoEntry ( DlCompletionTree* n, BipolarPointer c, const DepSet& dep,
							   const char* reason = NULL );
		/// insert P to the label of N; do necessary updates; may return Clash in case of data P
	tacticUsage insertToDoEntry ( DlCompletionTree* n, BipolarPointer c, const DepSet& dep,
								  DagTag tag, const char* reason );
		/// if something was added to cached node N, un- or re-cache it; @return result of re-cache
	tacticUsage correctCachedEntry ( DlCompletionTree* n );

	// label access interface

		/// check if it is possible to add a concept to a label given by TAG
	addConceptResult checkAddedConcept ( const CWDArray& lab, BipolarPointer p, const DepSet& dep );
		/// try to add a concept to a label given by TAG; ~P can't appear in the label
	bool findConcept ( const CWDArray& lab, BipolarPointer p );
		/// try to add a concept to a label given by TAG; ~P can't appear in the label; setup clash-set if found
	bool findConcept ( const CWDArray& lab, BipolarPointer p, const DepSet& dep );
		/// check if C or ~C is already in LAB
	addConceptResult tryAddConcept ( const CWDArray& lab, BipolarPointer c, const DepSet& dep );

		/** Adds ToDo entry which already exists in label of NODE. There is no need
			to add entry to label, but it is necessary to provide offset of existing concept.
			This is done by providing OFFSET of the concept in NODE's label
		 */
	void addExistingToDoEntry ( DlCompletionTree* node, int offset, const char* reason = NULL )
	{
		const ConceptWDep& C = node->label().getConcept(offset);
		BipolarPointer bp = C.bp();
		TODO.addEntry ( node, bp, DLHeap[bp].Type(), offset );
		if ( LLM.isWritable(llGTA) )
			logEntry ( node, bp, C.getDep(), reason );
	}
		/// add all elements from NODE label into TODO list
	void redoNodeLabel ( DlCompletionTree* node, const char* reason )
	{
		const CGLabel& lab = node->label();
		CGLabel::const_iterator p;
		for ( p = lab.begin_sc(); p != lab.end_sc(); ++p )
			addExistingToDoEntry ( node, lab.getSCOffset(p), reason );
		for ( p = lab.begin_cc(); p != lab.end_cc(); ++p )
			addExistingToDoEntry ( node, lab.getCCOffset(p), reason );
	}

		/// main reasoning function
	bool checkSatisfiability ( void );
		/// make sure that the DAG does not grow larger than that was recorded
	void ensureDAGSize ( void )
	{
		if ( dagSize < DLHeap.size() )
		{
			dagSize = DLHeap.maxSize();
			pUsed.ensureMaxSetSize(dagSize);
			nUsed.ensureMaxSetSize(dagSize);
		}
	}

		/// init some flags using an external option set
	void readConfig ( const ifOptionSet* Options );

//-----------------------------------------------------------------------------
//--		internal cache support
//-----------------------------------------------------------------------------

		/// return cache of given completion tree (implementation)
	const modelCacheInterface* createModelCache ( const DlCompletionTree* p ) const
		{ return new modelCacheIan ( DLHeap, p, encounterNominal ); }
		/// create cache entry for given singleton
	void registerNominalCache ( TIndividual* p ) const
		{ DLHeap.setCache ( p->pName, createModelCache(p->node->resolvePBlocker()) ); }

		/// generate necessary clash level if node's caching lead to clash
	void generateCacheClashLevel ( DlCompletionTree* node, modelCacheInterface* cache = NULL );
		/// check whether node may be (un)cached
	tacticUsage tryCacheNode1 ( DlCompletionTree* node );
		/// check whether node may be (un)cached; save node if something is changed
	tacticUsage tryCacheNode ( DlCompletionTree* node )
	{
		tacticUsage ret = tryCacheNode1(node);
		// node is cached if RET is utDone
		CGraph.saveRareCond(node->setCached(ret == utDone));
		return ret;
	}
		/// perform caching of the node (it is known that caching is possible)
	enum modelCacheState doCacheNode ( DlCompletionTree* node );

//-----------------------------------------------------------------------------
//--		internal nominal reasoning interface
//-----------------------------------------------------------------------------

		/// check whether reasoning with nominals is performed
	bool hasNominals ( void ) const { return !Nominals.empty(); }
		/// init single nominal node
	bool initNominalNode ( const TIndividual* nom )
	{
		DlCompletionTree* node = CGraph.getNewNode();
		node->setNominalLevel();
		const_cast<TIndividual*>(nom)->node = node;	// init nominal with associated node
		return initNewNode ( node, DepSet(), nom->pName ) == utClash;	// ABox is inconsistent
	}
		/// create nominal nodes for all individuals in TBox
	bool initNominalCloud ( void );
		/// make an R-edge between related nominals
	bool initRelatedNominals ( const TRelated* rel );

		/// check satisfiability of task which is set-up
	bool runSat ( void );

	/*
	 * Tactics description;
	 *
	 * Each tactic represents (possibly combined) expansion rule for
	 * certain type of concept expression (parameter CUR).
	 *
	 * Each tactic returns:
	 * - utUnusable	- if CUR can not be expanded
	 * - utClash		- if expansion of CUR lead to clash
	 * - utDone		- if CUR was successfully expanded
	 */

		/// main calling method; wrapper for Body
	tacticUsage commonTactic ( void );
		/// choose proper tactic based on type of a concept constructor
	tacticUsage commonTacticBody ( const DLVertex& cur );
		/// expansion rule for (non)primitive concept
	tacticUsage commonTacticBodyId ( const DLVertex& cur );
		/// expansion rule for (non)primitive singleton concept
	tacticUsage commonTacticBodySingleton ( const DLVertex& cur );
		/// expansion rule for conjunction
	tacticUsage commonTacticBodyAnd ( const DLVertex& cur );
		/// expansion rule for disjunction
	tacticUsage commonTacticBodyOr ( const DLVertex& cur );
		/// expansion rule for general existential quantifier
	tacticUsage commonTacticBodySome ( const DLVertex& cur );
		/// expansion rule for existential quantifier in the form \ER{nom}
	tacticUsage commonTacticBodyValue ( const TRole* R, const TIndividual* nom );
		/// expansion rule for universal restriction
	tacticUsage commonTacticBodyAll ( const DLVertex& cur );
		/// expansion rule for universal restriction with simple role using RA
	tacticUsage commonTacticBodyAllSimple ( const DLVertex& cur );
		/// expansion rule for universal restriction with non-simple role using RA
	tacticUsage commonTacticBodyAllComplex ( const DLVertex& cur );
		/// expansion rule for \E R{Self}
	tacticUsage commonTacticBodySomeSelf ( const TRole* R );
		/// expansion rule for \neg\E R{Self}
	tacticUsage commonTacticBodyIrrefl ( const TRole* R );
		/// expansion rule for at-least number restriction; applicable for logics without O/H
	tacticUsage commonTacticBodyGEsimple ( const DLVertex& cur );
		/// expansion rule for at-least number restriction; applicable for all logics
	tacticUsage commonTacticBodyGEusual ( const DLVertex& cur );
		/// expansion rule for at-least number restriction
	tacticUsage commonTacticBodyGE ( const DLVertex& cur )
	{
		if ( RKG_HIERARCHY_NR_TACTIC )
			return commonTacticBodyGEusual(cur);
		else
			return commonTacticBodyGEsimple(cur);
	}
		/// expansion rule for at-most number restriction
	tacticUsage commonTacticBodyLE ( const DLVertex& cur );
		/// expansion rule for choose-rule
	tacticUsage commonTacticBodyChoose ( const TRole* R, BipolarPointer C );
		/// expansion rule for functional restriction
	tacticUsage commonTacticBodyFunc ( const DLVertex& cur );
		/// expansion rule for at-most restriction in nominal node (NN-rule)
	tacticUsage commonTacticBodyNN ( const DLVertex& cur );

	// support for inapplicable tactics

		/// @return true iff current node is i-blocked (ie, no expansion necessary)
	bool isIBlocked ( void ) const { return curNode->isIBlocked(); }
		/// @return true iff NN-rule wrt (<= R.C) is applicable to the curNode
	bool isNNApplicable ( const TRole* r, BipolarPointer C ) const;
		/// apply rule-like actions for the concept P
	tacticUsage applyExtraRules ( const TConcept* p );
		/// apply rule-like actions for the concept P if necessary
	inline
	tacticUsage applyExtraRulesIf ( const TConcept* p )
	{
		if ( !p->hasExtraRules() )
			return utUnusable;
		assert ( p->isPrimitive() );
		return applyExtraRules(p);
	}

	// support for choose-rule

		/// apply choose-rule for given range of edges
	tacticUsage applyChooseRule ( DlCompletionTree* node, BipolarPointer C );

	// support for creating/updating edge methods

		/// check if current node is directly blocked (or became directly blocked)
	bool recheckNodeDBlocked ( void );
		/// apply all the generating rules for the (unblocked) current node
	void applyAllGeneratingRules ( DlCompletionTree* node );
		/// add C and T_G with given DEP-set to a NODE; @return DONE/CLASH
	tacticUsage initNewNode ( DlCompletionTree* node, const DepSet& dep, BipolarPointer C );
		/// apply reflexive roles to the (newly created) NODE with apropriate DEP; @return true for clash
	bool applyReflexiveRoles ( DlCompletionTree* node, const DepSet& dep );
		/// add necessary concepts to the NODE of the new edge, labelled with R
	tacticUsage initHeadOfNewEdge ( DlCompletionTree* node, const TRole* R, const DepSet& dep, const char* reason );

		/// adds T_G to the given node. returns result of addition
	tacticUsage addTG ( DlCompletionTree* Node, const DepSet& d );

		/// aux method for setting up new edge PA
	tacticUsage setupEdge ( DlCompletionTreeArc* pA, const DepSet& curDep, unsigned int flags );
		/// aux method for creating new edge from curNode with given ROLE edge label and CONCEPT at the final label
	tacticUsage createNewEdge ( const TRole* Role, BipolarPointer Concept, unsigned int flags );
		/// create new ROLE-neighbour with LEVEL to curNode; return edge to it
	DlCompletionTreeArc* createOneNeighbour ( const TRole* Role, const DepSet& dep,
											  CTNominalLevel level = BlockableLevel );

	// support for re-applying expansion rules for FORALL-like concepls in node label

		/**
			apply AR.C in and <= nR (if needed) in NODE's label where R is label of arcSample.
			Set of applicable concepts is defined by redoForallFlags value.
		*/
	tacticUsage applyUniversalNR ( DlCompletionTree* Node, const DlCompletionTreeArc* arcSample,
								   const DepSet& dep, unsigned int flags );

	// support for branching rules

		/// check if branching rule was called for the 1st time
	bool isFirstBranchCall ( void ) const { return bContext == NULL; }
		/// init branching context with given rule type
	void initBC ( BranchingTag tag )
	{
		bContext = Stack.push();
		bContext->branchDep = curConcept.getDep();
		bContext->init(tag);
	}
		/// clear branching context
	void clearBC ( void ) { bContext = NULL; }

	// support for disjunction

	/** Aux method for locating OR node characteristics.
		Fills branchDep with clashset's and indexVec with applicable concepts.
		@return -1 if some of OR entries are in Label of given Node
		@return 0 if all OR entries leads to clash;
		@return n>0 if there are n applicable entries;
	*/
	int getOrType ( const DLVertex& cur );
		/// aux method for disjunction processing
	tacticUsage contOrProcessing ( void );
		/// aux method for disjunction processing
	tacticUsage finOrProcessing ( void );
		/// process semantic branching for the OR entry within [BEG,END) with given dep-set
	template<class Iterator>
	void processSemanticBranching ( Iterator beg, Iterator end, const DepSet& dep )
	{
		if ( useSemanticBranching )
			for ( Iterator p = beg; p != end; ++p )
				if ( addToDoEntry ( curNode, inverse(*p), dep, "sb" ) != utDone )
					assert (0);	// Both Exists and Clash are errors
	}

	// support for (qualified) number restrictions

		/// create N R-neighbours of curNode with given Nominal LEVEL labelled with C
	tacticUsage createDifferentNeighbours ( const TRole* R, BipolarPointer C, const DepSet& dep,
											unsigned int n, CTNominalLevel level );

		/// check if ATLEAST and ATMOST entries are in clash. Both vertex MUST have dtLE type.
	bool checkNRclash ( const DLVertex& atleast, const DLVertex& atmost ) const
	{	// >= n R clash with <= m S iff...
		return atleast.getNumberGE() > atmost.getNumberLE() &&	// n is greater than m...
			   *atleast.getRole() <= *atmost.getRole();			// and R [= S
	}
		/// check if ATLEAST and ATMOST restrictions are in clash; setup depset from CUR
	bool isNRClash ( const DLVertex& atleast, const DLVertex& atmost, const ConceptWDep& reason );

		/// aux method that fills the dep-set for either C or ~C found in the label; @return whether C was found
	bool findChooseRuleConcept ( const CWDArray& label, BipolarPointer C, DepSet& Dep )
	{
		if ( C == bpTOP )
			return true;
		if ( findConcept ( label, C, Dep ) )
		{
			Dep = getClashSet();
			return true;
		}
		else if ( findConcept ( label, inverse(C), Dep ) )
		{
			Dep = getClashSet();
			return false;
		}
		else
			assert(0);
	}
		/// aux method which fills EdgesToMerge with *different* ROLE-neighbours of curNode
	void findNeighbours ( EdgeVector& EdgesToMerge, const TRole* Role, BipolarPointer C, DepSet& Dep );
		/// aux method that checks whether clash occurs during the merge of labels
	bool checkMergeClash ( const CGLabel& from, const CGLabel& to, const DepSet& dep, unsigned int nodeId );
		/// aux method that merge FROM label to the TO node with an appropriadte dep-set
	tacticUsage mergeLabels ( const CGLabel& from, DlCompletionTree* to, const DepSet& dep );
		/// merge FROM node into TO node with additional dep-set DEPF
	tacticUsage Merge ( DlCompletionTree* from, DlCompletionTree* to, const DepSet& depF );
		/// check whether clash occures due to new edge from FROM to TO labelled with R
	tacticUsage checkDisjointRoleClash ( DlCompletionTree* from, DlCompletionTree* to,
										 const TRole* R, const DepSet& dep );
		/// check whether clash occures EDGE to TO labelled with S disjoint with R
	bool checkDisjointRoleClash ( const DlCompletionTreeArc* edge, DlCompletionTree* to,
								  const TRole* R, const DepSet& dep )
	{	// clash found
		if ( edge->getArcEnd() == to && edge->getRole()->isDisjoint(R) )
		{
			setClashSet(dep);
			updateClashSet(edge->getDep());
			return true;
		}
		return false;
	}

	// support for FORALL expansion

		/** Perform expansion of (\neg \ER.Self).DEP to an EDGE */
	tacticUsage checkIrreflexivity ( const DlCompletionTreeArc* edge,
									 const TRole* R, const DepSet& dep )
	{
		// only loops counts here...
		if ( edge->getArcEnd() != edge->getReverse()->getArcEnd() )
			return utDone;
		// which are labelled either with R or with R-
		if ( !edge->isNeighbour(R) && !edge->isNeighbour(R->inverse()) )
			return utDone;

		// set up clash
		setClashSet(dep);
		updateClashSet(edge->getDep());
		return utClash;
	}

		/** Perform expansion of (C=\AR{state}.X).DEP to an EDGE with a given reason */
	tacticUsage applyAutomaton ( const DlCompletionTreeArc* edge,
								 const RoleAutomaton& A,
								 RAState state, BipolarPointer C,
								 const DepSet& dep, const char* reason = NULL );
		/** Perform expansion of (\AR.C).DEP to an EDGE for simple R with a given reason */
	tacticUsage applySimpleAutomaton ( const DlCompletionTreeArc* edge,
									   const RoleAutomaton& A,
									   BipolarPointer C, const DepSet& dep,
									   const char* reason = NULL );

	// datatype staff

		/// @return true iff given data node contains inconsistent data constraints
	bool hasDataClash ( const DlCompletionTree* node );
		/// @return utClash iff given data node contains inconsistent data constraints
	tacticUsage checkDataClash ( const DlCompletionTree* node )
	{
		if ( hasDataClash(node) )
		{
			setClashSet(DTReasoner.getClashSet());
			return utClash;
		}
		else
			return utDone;
	}

	// logging actions

		/// log indentation
	void logIndentation ( void ) const;
		/// log start of processing of a ToDo entry
	void logStartEntry ( void ) const;
		/// log finish of processing of a ToDo entry
	void logFinishEntry ( tacticUsage res ) const;
		/// log the result of processing ACTION with entry (N,C{DEP})/REASON
	void logNCEntry ( const DlCompletionTree* n, BipolarPointer c, const DepSet& dep,
					  const char* action, const char* reason ) const
	{
		CHECK_LL_RETURN(llGTA);	// useless, but safe

		LL << " " << action << "(";
		n->logNode();
		LL << "," << c << dep << ")";
		if ( reason )
			LL << reason;
	}
		/// log addition of the entry to ToDo list
	void logEntry ( const DlCompletionTree* n, BipolarPointer c, const DepSet& dep, const char* reason ) const
		{ logNCEntry ( n, c, dep, "+", reason ); }
		/// log clash happened during node processing
	void logClash ( const DlCompletionTree* n, BipolarPointer c, const DepSet& dep ) const
		{ logNCEntry ( n, c, dep, "x", DLHeap[c].getTagName() ); }
		/// write root subtree of CG with given LEVEL
	void writeRoot ( unsigned int level ) const
	{
		if ( LLM.isWritable(level) )
			CGraph.Print(LL);
	}

		/// merge session statistics to the global one
	void finaliseStatistic ( void );
		/// write down statistics wrt LOCAL flag
	void logStatisticData ( std::ostream& o, bool needLocal ) const;

	// save/restore methods

		/// save local state to current branching context
	void saveBC ( void );
		/// restore local state from current branching context
	void restoreBC ( void );
		/// use this method in ALL dependency stuff (never use tryLevel directly)
	unsigned int getCurLevel ( void ) const { return tryLevel; }
		/// set new branching level (never use tryLevel directly)
	void setCurLevel ( unsigned int level ) { tryLevel = level; }
		/// Get save/restore level based on either current- or DS level
	unsigned int getSaveRestoreLevel ( const DepSet& ds ATTR_UNUSED ) const
	{
		return	// FIXME!!! see more precise it later
#		ifdef RKG_IMPROVE_SAVE_RESTORE_DEPSET
			ds.level()+1;
#		else
			getCurLevel();
#		endif
	}
		/// save current reasoning state
	void save ( void );
		/// restore reasoning state to the latest saved position
	void restore ( void ) { return restore(getCurLevel()); }
		/// restore reasoning state to the NEWTRYLEVEL position
	void restore ( unsigned int newTryLevel );
		/// update level in N node and save it's state (if necessary)
	void updateLevel ( DlCompletionTree* n, const DepSet& ds ) { CGraph.saveNode ( n, getSaveRestoreLevel(ds) ); }
		/// finalize branching OP processing making deterministic op
	void determiniseBranchingOp ( void )
	{
		clearBC();		// clear context for the next branching op
		Stack.pop();	// remove unnecessary context from the stack
	}

	// access to global clashset, which contains result of clash during label addition

		/// get value of global dep-set
	const DepSet& getClashSet ( void ) const { return clashSet; }
		/// set value of global dep-set to D
	void setClashSet ( const DepSet& d ) { clashSet = d; }
		/// add D to global dep-set
	void updateClashSet ( const DepSet& d ) { clashSet.add(d); }
		/// get dep-set wrt current level
	DepSet getCurDepSet ( void ) const { return DepSet(getCurLevel()-1); }

		/// get RW access to current branching dep-set
	DepSet& getBranchDep ( void ) { return bContext->branchDep; }
		/// get RO access to current branching dep-set
	const DepSet& getBranchDep ( void ) const { return bContext->branchDep; }
		/// update cumulative branch-dep with current clash-set
	void updateBranchDep ( void ) { getBranchDep().add(getClashSet()); }
		/// prepare cumulative dep-set to usage
	void prepareBranchDep ( void ) { getBranchDep().restrict(getCurLevel()); }
		/// prepare cumulative dep-set and copy itto general clash-set
	void useBranchDep ( void )
	{
		prepareBranchDep();
		setClashSet(getBranchDep());
	}

		/// restore one level (no backjumping)
	bool straightforwardRestore ( void );
		/// restore if backjumping is used
	bool backJumpedRestore ( void );
		/// restore state based on usedBackjumping flag
	bool tunedRestore ( void );

		/// check if P was used during current reasoning session
	bool isUsed ( BipolarPointer p ) const { return ( isPositive(p) ? pUsed : nUsed ).in(getValue(p)); }
		/// set P as a used during current reasoning
	void setUsed ( BipolarPointer p ) { ( isPositive(p) ? pUsed : nUsed ).add(getValue(p)); }

public:		// rule's support
		/// @return true if the rule is applicable; set the dep-set accordingly
	bool applicable ( const TBox::TSimpleRule& rule );

public:		// blocking support
		/// re-apply all the relevant expantion rules to a given unblocked NODE
	void repeatUnblockedNode ( DlCompletionTree* node, bool direct )
	{
		if ( direct )		// not blocked -- clear blocked cache
			applyAllGeneratingRules(node);		// re-apply all the generating rules
		else
			redoNodeLabel ( node, "ubi" );
	}

public:
		/// c'tor
	DlSatTester ( TBox& tbox, const ifOptionSet* Options );
		/// d'tor
	~DlSatTester ( void ) {}

		/// prepare reasoner to a new run
	void prepareReasoner ( void );
		/// set-up satisfiability task for given pointers and run runSat on it
	bool runSat ( BipolarPointer p, BipolarPointer q = bpTOP )
	{
		prepareReasoner();

		// use general method to init node with P and add Q then
		if ( initNewNode ( CGraph.getRoot(), DepSet(), p ) == utClash ||
			 addToDoEntry ( CGraph.getRoot(), q, DepSet() ) == utClash )
			return false;		// concept[s] unsatisfiable

		// check satisfiability explicitly
		TsProcTimer& timer = q == bpTOP ? satTimer : subTimer;
		timer.Start();
		bool result = runSat();
		timer.Stop();
		return result;
	}

		/// init TODO list priority for classification
	void initToDoPriorities ( const ifOptionSet* OptionSet )
	{
		assert ( OptionSet != NULL );

		if ( TODO.initPriorities ( OptionSet, "IAOEFLG" ) )
			error ( "Wrong priority option given. Execution stopped." );
	}
		/// set blocking method for a session
	void setBlockingMethod ( bool hasInverse, bool hasQCR ) { CGraph.setBlockingMethod ( hasInverse, hasQCR ); }

		/// return [singleton] cache for given concept implementation
	const modelCacheInterface* createModelCache ( BipolarPointer p ) const
	{
		if ( p == bpTOP || p == bpBOTTOM )
			return new modelCacheConst(p==bpTOP);
		else
			return new modelCacheSingleton(p);
	}
		/// fills cache entry for given DAG node; @return cache
	const modelCacheInterface* fillsCache ( BipolarPointer p );
		/// build cache suitable for classification
	void prepareCascadedCache ( BipolarPointer p );
		/// create cache for given DAG node; @return cache
	const modelCacheInterface* createCache ( BipolarPointer p );
		/// create model cache for the just-classified entry
	const modelCacheInterface* createCacheByCGraph ( bool sat ) const
	{
		if ( sat )	// here we need actual (not a p-blocked) root of the tree
			return createModelCache(CGraph.getActualRoot());
		else		// unsat => cache is just bottom
			return createModelCache(bpBOTTOM);
	}

		/// init vector of nominals defined in TBox
	void initNominalVector ( void );
		/// check whether ontology with nominals is consistent
	bool consistentNominalCloud ( void );

	void writeTotalStatistic ( std::ostream& o )
	{
		AccumulatedStatistic::accumulateAll();	// ensure that the last reasoning results are in
		logStatisticData ( o, /*needLocal=*/false );
		printBlockingStat (o);
		clearBlockingStat();
		o << "\n";
	}

		/// print SAT/SUB timings to O; @return total time spend during reasoning
	float printReasoningTime ( std::ostream& o ) const;
}; // DlSatTester

// implementation

inline void DlSatTester :: resetSessionFlags ( void )
{
	// reflect possible change of DAG size
	ensureDAGSize();

	setUsed(bpTOP);
	setUsed(bpBOTTOM);

	encounterNominal = false;
	checkDataNode = true;
	dBlocked = NULL;
}

inline tacticUsage
DlSatTester :: addTG ( DlCompletionTree* Node, const DepSet& d )
{
	return tBox.getTG() != bpTOP ? addToDoEntry ( Node, tBox.getTG(), d ) : utUnusable;
}

inline tacticUsage
DlSatTester :: initNewNode ( DlCompletionTree* node, const DepSet& dep, BipolarPointer C )
{
	if ( node->isDataNode() )	// creating new data node -- do data check once in the end
		checkDataNode = false;
	node->setInit(C);
	if ( addToDoEntry ( node, C, dep ) == utClash )
		return utClash;
	if ( node->isDataNode() )
		return utDone;
	if ( addTG ( node, dep ) == utClash )
		return utClash;
	if ( GCIs.isReflexive() && applyReflexiveRoles ( node, dep ) )
		return utClash;

	return utDone;
}

// restore implementation
inline bool DlSatTester :: backJumpedRestore ( void )
{
	// if empty clash dep-set -- concept is unsatisfiable
	if ( getClashSet().empty () )
		return true;

	// some non-deterministic choices were done
	restore ( getClashSet().level() );
	return false;
}

inline bool DlSatTester :: straightforwardRestore ( void )
{
	if ( tryLevel == 1 )	// no non-deterministic choices was made
		return true;		// ... the concept is unsatisfiable
	else
	{	// restoring the state
		restore ();
		return false;
	}
}

inline bool DlSatTester :: tunedRestore ( void )
{
	if ( useBackjumping )
		return backJumpedRestore ();
	else
		return straightforwardRestore ();
}

inline tacticUsage DlSatTester :: commonTacticBodyAll ( const DLVertex& cur )
{
#ifdef ENABLE_CHECKING
	assert ( isPositive(curConcept.bp()) && cur.Type() == dtForall );
#endif

	// can't skip singleton models for complex roles due to empty transitions
	if ( !cur.getRole()->isSimple() )
		return commonTacticBodyAllComplex(cur);

	// ... but can for simple roles
	if ( curNode->hasChildren() || curNode->hasParent() )
		return commonTacticBodyAllSimple(cur);

	// if no children/parents => nothing to do
	nUseless.inc();
	return utUnusable;
}

//-----------------------------------------------------------------------------
//--		implemenation of reasoner-related parts of TBox
//-----------------------------------------------------------------------------

inline bool
TBox::TSimpleRule :: applicable ( DlSatTester& Reasoner ) const { return Reasoner.applicable(*this); }

/// set ToDo priorities using local OPTIONS
inline void
TBox :: setToDoPriorities ( void )
{
	stdReasoner->initToDoPriorities(pOptions);
	if ( nomReasoner )
		nomReasoner->initToDoPriorities(pOptions);
}

inline void
TBox :: initReasoner ( void )
{
	if ( stdReasoner == NULL )	// 1st action
	{
		assert ( nomReasoner == NULL );

		GCIs.setReflexive(RM.hasReflexiveRoles());

		stdReasoner = new DlSatTester ( *this, pOptions );
		if ( NCFeatures.hasSingletons() )
		{
			nomReasoner = new DlSatTester ( *this, pOptions );
			nomReasoner->initNominalVector();
		}
	}
}

/// init [singleton] cache for given concept implementation
inline void
TBox :: initSingletonCache ( BipolarPointer p )
{
	DLHeap.setCache ( p, stdReasoner->createModelCache(p) );
}

/// check if the ontology is consistent
inline bool
TBox :: performConsistencyCheck ( void )
{
	buildSimpleCache();

	TConcept* test = ( NCFeatures.hasSingletons() ? *i_begin() : NULL );
	prepareFeatures ( test, NULL );

	if ( test )
	{
		// make a cache for TOP if it is not there
		if ( DLHeap.getCache(bpTOP) == NULL )
			initSingletonCache(bpTOP);

		return nomReasoner->consistentNominalCloud();
	}
	else
		return isSatisfiable(pTop);
}

/// test if 2 concept non-subsumption can be determined by cache merging
inline enum modelCacheState
TBox :: testCachedNonSubsumption ( const TConcept* p, const TConcept* q )
{
	const modelCacheInterface* pCache = initCache(const_cast<TConcept*>(p));
	prepareFeatures ( NULL, q );	// make appropriate conditions for reasoning
	const modelCacheInterface* nCache = getReasoner()->fillsCache(inverse(q->pName));
	clearFeatures();
	return pCache->canMerge(nCache);
}

inline const modelCacheInterface*
TBox :: initCache ( TConcept* pConcept )
{
	const modelCacheInterface* cache = DLHeap.getCache(pConcept->pName);

	if ( cache == NULL )
	{
		prepareFeatures ( pConcept, NULL );
		cache = getReasoner()->fillsCache(pConcept->pName);
		clearFeatures();
	}

	return cache;
}

#endif
