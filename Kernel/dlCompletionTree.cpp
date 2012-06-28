/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2012 by Dmitry Tsarkov

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

#include "dlCompletionTree.h"

/// check if transitive R-successor of the NODE labelled with C
const DlCompletionTree*
DlCompletionTree :: isTSuccLabelled ( const TRole* R, BipolarPointer C ) const
{
	if ( isLabelledBy(C) )
		return this;
	// don't check nominal nodes (prevent cycles)
	if ( isNominalNode() )
		return NULL;

	// check all other successors
	const DlCompletionTree* ret = NULL;
	for ( const_edge_iterator p = begin(), p_end = end(); p < p_end; ++p )
		if ( (*p)->isSuccEdge() &&
			 (*p)->isNeighbour(R) &&
			 !(*p)->isReflexiveEdge() &&	// prevent cycles
			 (ret = (*p)->getArcEnd()->isTSuccLabelled(R,C)) != NULL )
			return ret;

	// not happens
	return NULL;
}

/// check if transitive R-predcessor of the NODE labelled with C; skip FROM node
const DlCompletionTree*
DlCompletionTree :: isTPredLabelled ( const TRole* R, BipolarPointer C, const DlCompletionTree* from ) const
{
	if ( isLabelledBy(C) )
		return this;
	// don't check nominal nodes (prevent cycles)
	if ( isNominalNode() )
		return NULL;

	// check all other successors
	const DlCompletionTree* ret = NULL;
	for ( const_edge_iterator p = begin(), p_end = end(); p < p_end; ++p )
		if ( (*p)->isSuccEdge() && (*p)->isNeighbour(R) && (*p)->getArcEnd() != from &&
			 (ret = (*p)->getArcEnd()->isTSuccLabelled(R,C)) != NULL )
			return ret;

	// check predecessor
	if ( hasParent() && isParentArcLabelled(R) )
		return getParentNode()->isTPredLabelled ( R, C, this );
	else
		return NULL;
}

const DlCompletionTree*
DlCompletionTree :: isNSomeApplicable ( const TRole* R, BipolarPointer C ) const
{
	for ( DlCompletionTree::const_edge_iterator p = begin(), p_end = end(); p < p_end; ++p )
		if ( (*p)->isNeighbour(R) && (*p)->getArcEnd()->isLabelledBy(C) )
			return (*p)->getArcEnd();	// already contained such a label

	return NULL;
}

const DlCompletionTree*
DlCompletionTree :: isTSomeApplicable ( const TRole* R, BipolarPointer C ) const
{
	const DlCompletionTree* ret = NULL;

	for ( DlCompletionTree::const_edge_iterator p = begin(), p_end = end(); p < p_end; ++p )
		if ( (*p)->isNeighbour(R) )
		{
			if ( (*p)->isPredEdge() )
				ret = (*p)->getArcEnd()->isTPredLabelled(R,C,this);
			else
				ret = (*p)->getArcEnd()->isTSuccLabelled(R,C);

			if ( ret )
				return ret;	// already contained such a label
		}

	return NULL;
}

#ifdef RKG_IR_IN_NODE_LABEL
	//----------------------------------------------
	// inequality relation methods
	//----------------------------------------------

// check if IR for the node contains C
bool DlCompletionTree :: inIRwithC ( const ConceptWDep& C, DepSet& dep ) const
{
	if ( IR.empty() )
		return false;

	for ( IRInfo::const_iterator p = IR.begin(); p != IR.end(); ++p )
		if ( p->bp() == C.bp() )
		{
			dep += p->getDep();
			dep += C.getDep();
			return true;
		}

	return false;
}

// check if the NODE's and current node's IR are labelled with the same level
bool DlCompletionTree :: nonMergable ( const DlCompletionTree* node, DepSet& dep ) const
{
	if ( IR.empty() || node->IR.empty() )
		return false;

	for ( IRInfo::const_iterator p = node->IR.begin(); p != node->IR.end(); ++p )
		if ( inIRwithC ( *p, dep ) )
			return true;

	return false;
}

/// update IR of the current node with IR from NODE and additional clash-set; @return restorer
TRestorer* DlCompletionTree :: updateIR ( const DlCompletionTree* node, const DepSet& toAdd )
{
	if ( node->IR.empty() )
		return NULL;	// nothing to do

	// save current state
	TRestorer* ret = new IRRestorer(this);

	// copy all elements from NODE's IR to current node.
	// FIXME!! do not check if some of them are already in there
	for ( IRInfo::const_iterator p = node->IR.begin(); p != node->IR.end(); ++p )
		IR.add ( ConceptWDep ( *p, toAdd ) );

	return ret;
}
#endif // RKG_IR_IN_NODE_LABEL

// saving/restoring
void DlCompletionTree :: save ( SaveState* nss ) const
{
	nss->curLevel = curLevel;
	nss->nNeighbours = Neighbour.size();
	Label.save(nss->lab);

	logSRNode("SaveNode");
}

void DlCompletionTree :: restore ( SaveState* nss )
{
	if ( nss == NULL )
		return;

	// level restore
	curLevel = nss->curLevel;

	// label restore
	Label.restore ( nss->lab, getCurLevel() );

	// remove new neighbours
#ifndef RKG_USE_DYNAMIC_BACKJUMPING
	Neighbour.resize(nss->nNeighbours);
#else
	for ( int j = Neighbour.size()-1; j >= 0; --j )
		if ( Neighbour[j]->Node->creLevel <= getCurLevel() )
		{
			Neighbour.resize(j+1);
			break;
		}
#endif

	// it's cheaper to dirty affected flag than to consistently save nodes
	affected = true;

	// clear memory
	delete nss;

	logSRNode("RestNode");
}

// output
void DlCompletionTree :: PrintBody ( std::ostream& o ) const
{
	o << id;
	if ( isNominalNode() )
		o << "o" << getNominalLevel();
	o << '(' << getCurLevel() << ')';

	// data node?
	if ( isDataNode() )
		o << "d";

	// label (if any)
	Label.print(o);

	// blocking status information
	logNodeBStatus(o);
}
