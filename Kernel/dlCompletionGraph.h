/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2005-2008 by Dmitry Tsarkov

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

#ifndef _DLCOMPLETIONGRAPH_H
#define _DLCOMPLETIONGRAPH_H

#include <vector>

#include "globaldef.h"
#include "DeletelessAllocator.h"
#include "dlCompletionTree.h"
#include "dlCompletionTreeArc.h"
#include "tSaveStack.h"
#include "tRareSaveStack.h"

class DlSatTester;

/**
 * Class for maintaining graph of CT nodes. Behaves like
 * deleteless allocator for nodes, plus some obvious features
 */
class DlCompletionGraph
{
protected:	// typedefs
		/// type of the heap
	typedef std::vector<DlCompletionTree*> nodeBaseType;
		/// heap's RW iterator
	typedef nodeBaseType::iterator iterator;
		/// heap's RO iterator
	typedef nodeBaseType::const_iterator const_iterator;

protected:	// types
		/// class for S/R local state
	class SaveState
	{
	public:		// members
			/// number of valid nodes
		unsigned int nNodes;
			/// end pointer of saved nodes
		unsigned int sNodes;

	public:		// interface
			/// empty c'tor
		SaveState ( void ) : nNodes(0), sNodes(0) {}
			/// empty d'tor
		~SaveState ( void ) {}
	}; // SaveState

private:	// constants
		/// initial value of branching level
	static const unsigned int initBranchingLevel = 1;
		/// initial value of IR level
	static const unsigned int initIRLevel = 0;

private:	// members
		/// allocator for edges
	DeletelessAllocator<DlCompletionTreeArc> CTEdgeHeap;

protected:	// members
		/// heap itself
	nodeBaseType NodeBase;
		/// nodes, saved on current branching level
	nodeBaseType SavedNodes;
		/// host reasoner
	DlSatTester* pReasoner;
		/// remember the last generated ID for the node
	unsigned int nodeId;
		/// index of the next unallocated entry
	unsigned int endUsed;
		/// current branching level (synchronised with resoner's one)
	unsigned int branchingLevel;
		/// current IR level (should be valid BP)
	unsigned int IRLevel;
		/// stack for rarely changed information
	TRareSaveStack RareStack;
		/// stack for usual saving/restoring
	TSaveStack<SaveState> Stack;

	// statistical members

		/// number of node' saves
	unsigned int nNodeSaves;
		/// number of node' saves
	unsigned int nNodeRestores;

	// flags

		/// use or not lazy blocking (ie test blocking only expanding exists)
	bool useLazyBlocking;
		/// whether to use Anywhere blocking as opposed to an ancestor one
	bool useAnywhereBlocking;

		/// check if session has inverse roles
	bool sessionHasInverseRoles;
		/// check if session has number restrictions
	bool sessionHasNumberRestrictions;

protected:	// methods
		/// init vector [B,E) with new objects T
	void initNodeArray ( iterator b, iterator e )
	{
		for ( iterator p = b; p != e; ++p )
			*p = new DlCompletionTree(nodeId++);
	}
		/// increase heap size
	void grow ( void )
	{
		NodeBase.resize(NodeBase.size()*2);
		initNodeArray ( NodeBase.begin()+NodeBase.size()/2, NodeBase.end() );
	}
		/// init root node
	void initRoot ( void )
	{
		assert ( endUsed == 0 );
		getNewNode();
	}
		/// create edge between nodes with given label and creation level; @return from->to arc
	DlCompletionTreeArc* createEdge (
		DlCompletionTree* from,
		DlCompletionTree* to,
		bool isUpLink,
		const TRole* roleName,
		const DepSet& dep );

		/// Aux method for Merge(): add EDGE to the NODE wrt flag ISUPLINK and dep-set DEP
	DlCompletionTreeArc* moveEdge (
		DlCompletionTree* node,
		DlCompletionTreeArc* edge,
		bool isUpLink, const DepSet& dep );

		/// save rarely appeared info
	void saveRare ( TRestorer* p ) { RareStack.push ( branchingLevel, p ); }
		/// invalidate EDGE, save restoring info
	void invalidateEdge ( DlCompletionTreeArc* edge ) { saveRareCond(edge->save()); }

		/// begin (RO) of USED nodes
	const_iterator begin ( void ) const { return NodeBase.begin(); }
		/// end (RO) of USED nodes
	const_iterator end ( void ) const { return NodeBase.begin()+endUsed; }
		/// begin (RW) of USED nodes
	iterator begin ( void ) { return NodeBase.begin(); }
		/// end (RW) of USED nodes
	iterator end ( void ) { return NodeBase.begin()+endUsed; }

	//----------------------------------------------
	// inequality relation methods
	//----------------------------------------------

		/// update IR in P with IR from Q and additional dep-set
	void updateIR ( DlCompletionTree* p, const DlCompletionTree* q, const DepSet& toAdd );

	//----------------------------------------------
	// re-building blocking hierarchy
	//----------------------------------------------

		/// check whether NODE is blocked by a BLOCKER
	bool isBlockedBy ( const DlCompletionTree* node, const DlCompletionTree* blocker ) const;
		/// check if d-blocked node is still d-blocked
	bool isStillDBlocked ( const DlCompletionTree* node ) const { return node->isDBlocked() && isBlockedBy ( node, node->Blocker ); }
		/// detect blocked status of current node by checking whether NODE and/or its ancestors are d-blocked
	void detectBlockedStatus ( DlCompletionTree* node );
		/// try to find d-blocker for a node using ancestor blocking
	void findDAncestorBlocker ( DlCompletionTree* node );
		/// try to find d-blocker for a node using anywhere blocking
	void findDAnywhereBlocker ( DlCompletionTree* node );
		/// try to find d-blocker for a node
	void findDBlocker ( DlCompletionTree* node )
	{
		saveNode ( node, branchingLevel );
		node->clearAffected();
		if ( node->isBlocked() )
			saveRareCond(node->setUBlocked());
		if ( useAnywhereBlocking )
			findDAnywhereBlocker(node);
		else
			findDAncestorBlocker(node);
	}
		/// unblock all the children of the node
	void unblockNodeChildren ( DlCompletionTree* node )
	{
		for ( DlCompletionTree::const_edge_iterator q = node->begins(), q_end = node->ends(); q < q_end; ++q )
			if ( !(*q)->isIBlocked() && !(*q)->isReflexiveEdge() )	// all of them are i-blocked
				unblockNode ( (*q)->getArcEnd(), false );
	}
		/// mark node unblocked; unblock all the hierarchy
	void unblockNode ( DlCompletionTree* node, bool wasDBlocked );

		/// mark NODE as a d-blocked by a BLOCKER
	void setNodeDBlocked ( DlCompletionTree* node, const DlCompletionTree* blocker )
	{
		saveRareCond(node->setDBlocked(blocker));
		propagateIBlockedStatus ( node, node );
	}
		/// mark NODE as an i-blocked by a BLOCKER
	void setNodeIBlocked ( DlCompletionTree* node, const DlCompletionTree* blocker )
	{
		// nominal nodes can't be blocked
		if ( node->isPBlocked() || node->isNominalNode() )
			return;

		node->clearAffected();

		// already iBlocked -- nothing changes
		if ( node->isIBlocked() && node->Blocker == blocker )
			return;
		// prevent node to be IBlocked due to reflexivity
		if ( node == blocker )
			return;

		saveRareCond(node->setIBlocked(blocker));
		propagateIBlockedStatus ( node, blocker );
	}
		/// propagate i-blocked status to all children of NODE
	void propagateIBlockedStatus ( DlCompletionTree* node, const DlCompletionTree* blocker )
	{
		for ( DlCompletionTree::const_edge_iterator q = node->begins(), q_end = node->ends(); q < q_end; ++q )
			if ( !(*q)->isIBlocked() )
				setNodeIBlocked ( (*q)->getArcEnd(), blocker );
	}

public:		// interface
		/// c'tor: make INIT_SIZE objects
	DlCompletionGraph ( unsigned int initSize, DlSatTester* p )
		: NodeBase(initSize)
		, pReasoner(p)
		, nodeId(0)
		, endUsed(0)
		, branchingLevel(initBranchingLevel)
		, IRLevel(initIRLevel)
	{
		initNodeArray ( NodeBase.begin(), NodeBase.end() );
		clearStatistics();
		initRoot();
	}
		/// d'tor: delete all allocated nodes
	~DlCompletionGraph ( void )
	{
		for ( iterator p = NodeBase.begin(); p != NodeBase.end(); ++p )
			delete *p;
	}

	// flag setting

		/// set flags for blocking
	void initContext ( DLDag* pDag, bool useLB, bool useAB )
	{
		DlCompletionTree::initContext(pDag);
		useLazyBlocking = useLB;
		useAnywhereBlocking = useAB;
	}
		/// set blocking method for a session
	void setBlockingMethod ( bool hasInverse, bool hasQCR )
	{
		sessionHasInverseRoles = hasInverse;
		sessionHasNumberRestrictions = hasQCR;
	}
		/// add concept C of a type TAG to NODE; call blocking check if appropriate
	void addConceptToNode ( DlCompletionTree* node, const ConceptWDep& c, DagTag tag )
	{
		node->addConcept(c,tag);

		if ( useLazyBlocking )
			node->setAffected();
		else
			detectBlockedStatus(node);
	}

	// access to nodes

		/// get a root node (non-const)
	DlCompletionTree* getRoot ( void ) { return NodeBase[0]; }
		/// get a root node (const)
	const DlCompletionTree* getRoot ( void ) const { return NodeBase[0]; }
		/// get actual root of the CGraph (in case root was merged to a nominal node)
	const DlCompletionTree* getActualRoot ( void ) const { return getRoot()->resolvePBlocker(); }
		/// get a node by it's ID
	const DlCompletionTree* getNode ( unsigned int id ) const
	{
		if ( id >= endUsed )
			return NULL;
		return NodeBase[id];
	}
		/// get new node (with internal level)
	DlCompletionTree* getNewNode ( void )
	{
		if ( endUsed >= NodeBase.size() )
			grow();
		DlCompletionTree* ret = NodeBase[endUsed++];
		ret->init(branchingLevel);
		return ret;
	}

	// blocking

		/// update blocked status for d-blocked node
	void updateDBlockedStatus ( DlCompletionTree* node )
	{
		if ( !node->isAffected() )
			return;
		if ( isStillDBlocked(node) )
			// FIXME!! clear affected in all children
			node->clearAffected();
		else
			detectBlockedStatus(node);
		assert ( !node->isAffected() );
	}
		/// retest every d-blocked node in the CG. Use it after the CG was build
	void retestCGBlockedStatus ( void )
	{
		for ( iterator p = begin(), p_end = end(); p < p_end; ++p )
			if ( (*p)->isDBlocked() )
				updateDBlockedStatus(*p);
	}

	// fairness support

		/// make a check for the reactive fairness constraint C
	DlCompletionTree* checkFairness ( BipolarPointer C )
	{
		for ( iterator p = begin(), p_end = end(); p < p_end; ++p )
			if ( (*p)->isDBlocked() && !(*p)->isLoopLabelled(C) )
				return const_cast<DlCompletionTree*>((*p)->Blocker);
		return NULL;
	}

		/// clear all the session statistics
	void clearStatistics ( void )
	{
		nNodeSaves = 0;
		nNodeRestores = 0;
	}
		/// mark all heap elements as unused
	void clear ( void )
	{
		CTEdgeHeap.clear();
		endUsed = 0;
		branchingLevel = initBranchingLevel;
		IRLevel = initIRLevel;
		RareStack.clear();
		Stack.clear();
		SavedNodes.clear();
		initRoot();
	}
		/// get number of nodes in the CGraph
	size_t maxSize ( void ) const { return NodeBase.size(); }

		/// save rarely appeared info if P is non-NULL
	void saveRareCond ( TRestorer* p ) { if (p) saveRare(p); }

	//----------------------------------------------
	// role/node
	//----------------------------------------------

		/// add RNAME with dep-set RDEP to the label of TO arc
	DlCompletionTreeArc* addRoleLabel (
		DlCompletionTree* from,
		DlCompletionTree* to,
		bool isUpLink,
		const TRole* rName,	// name of role (arc label)
		const DepSet& rDep )	// dep-set of the arc label
	{
		// check if GCraph already has FROM->TO edge labelled with RNAME
		DlCompletionTreeArc* ret = from->getEdgeLabelled ( rName, to, isUpLink );
		if ( ret == NULL )
			ret = createEdge ( from, to, isUpLink, rName, rDep );
		else
			saveRareCond(ret->addDep(rDep));

		return ret;
	}
		/// Create an empty R-neighbour of FROM; @return an edge to created node
	DlCompletionTreeArc* createNeighbour (
		DlCompletionTree* from,
		bool isUpLink,
		const TRole* r,		// name of role (arc label)
		const DepSet& dep )	// dep-set of the arc label
	{
#	ifdef RKG_IMPROVE_SAVE_RESTORE_DEPSET
		assert ( branchingLevel == r.getDep().level()+1 );
#	endif
		return createEdge ( from, getNewNode(), isUpLink, r, dep );
	}

		/// merge node FROM to node TO (do NOT copy label); fill EDGES with new edges added to TO
	void Merge ( DlCompletionTree* from, DlCompletionTree* to, const DepSet& toAdd,
				 std::vector<DlCompletionTreeArc*>& edges );
		/// purge node P with given ROOT and DEP-set
	void purgeNode ( DlCompletionTree* p, const DlCompletionTree* root, const DepSet& dep );
		/// purge edge E with given ROOT and DEP-set
	void purgeEdge ( DlCompletionTreeArc* e, const DlCompletionTree* root, const DepSet& dep );

	//----------------------------------------------
	// inequality relation interface
	//----------------------------------------------

		/// init new IR set
	void initIR ( void );
		/// make given NODE member of current IR set; @return true iff clash occurs
	bool setCurIR ( DlCompletionTree* node, const DepSet& ds );
		/// finilise current IR set
	void finiIR ( void );

		/// check if P and Q are in IR; if so, put the clash-set to DEP
	bool nonMergable ( const DlCompletionTree* p, const DlCompletionTree* q, DepSet& dep ) const;

	//----------------------------------------------
	// save/restore
	//----------------------------------------------

		/// save given node wrt level
	void saveNode ( DlCompletionTree* node, unsigned int level )
	{
		if ( node->needSave(level) )
		{
			node->save(level);
			SavedNodes.push_back(node);
			++nNodeSaves;
		}
	}
		/// restore given node wrt level
	void restoreNode ( DlCompletionTree* node, unsigned int level )
	{
		if ( node->needRestore(level) )
		{
			node->restore(level);
			++nNodeRestores;
		}
	}
		/// save local state
	void save ( void );
		/// restore state for the given LEVEL
	void restore ( unsigned int level );

	// statistics

		/// get number of nodes saved during session
	unsigned int getNNodeSaves ( void ) const { return nNodeSaves; }
		/// get number of nodes restored during session
	unsigned int getNNodeRestores ( void ) const { return nNodeRestores; }

	// print

		/// print graph starting from the root
	void Print ( std::ostream& o ) const;
}; // DlCompletionGraph

// blocking

#if defined(RKG_IR_IN_NODE_LABEL)
inline bool
DlCompletionGraph :: nonMergable ( const DlCompletionTree* p, const DlCompletionTree* q, DepSet& dep ) const
{
	return p->nonMergable ( q, dep );
}

inline void
DlCompletionGraph :: updateIR ( DlCompletionTree* p, const DlCompletionTree* q, const DepSet& toAdd )
{
	saveRareCond ( p->updateIR ( q, toAdd ) );
}

inline void
DlCompletionGraph :: initIR ( void )
{
	++IRLevel;
}

inline bool
DlCompletionGraph :: setCurIR ( DlCompletionTree* node, const DepSet& ds )
{
	return node->initIR ( IRLevel, ds );
}

inline void
DlCompletionGraph :: finiIR ( void ) {}

#endif	// RKG_IR_IN_NODE_LABEL

#endif
