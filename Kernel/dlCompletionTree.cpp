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

#include "dlCompletionTree.h"

unsigned int DlCompletionTree :: nSetCompareOps = 0;
DLDag* DlCompletionTree :: pDLHeap = NULL;
bool DlCompletionTree :: useLazyBlocking = true;
bool DlCompletionTree :: sessionHasInverseRoles = false;

/// check if transitive R-successor of the NODE labelled with C
bool
DlCompletionTree :: isTSuccLabelled ( const TRole* R, BipolarPointer C ) const
{
	if ( isLabelledBy(C) )
		return true;
	// don't check nominal nodes (prevent cycles)
	if ( isNominalNode() )
		return false;

	// check all other successors
	for ( const_edge_iterator p = begins(), p_end = ends(); p < p_end; ++p )
		if ( (*p)->isNeighbour(R) && (*p)->getArcEnd()->isTSuccLabelled(R,C) )
			return true;

	// not happens
	return false;
}

/// check if transitive R-predcessor of the NODE labelled with C; skip FROM node
bool
DlCompletionTree :: isTPredLabelled ( const TRole* R, BipolarPointer C, const DlCompletionTree* from ) const
{
	if ( isLabelledBy(C) )
		return true;
	// don't check nominal nodes (prevent cycles)
	if ( isNominalNode() )
		return false;

	// check all other successors
	const_edge_iterator p, p_end;
	for ( p = begins(), p_end = ends(); p < p_end; ++p )
		if ( (*p)->isNeighbour(R) && (*p)->getArcEnd() != from &&
			 (*p)->getArcEnd()->isTSuccLabelled(R,C) )
			return true;

	// check predecessor
	if ( hasParent() && isParentArcLabelled(R) )
		return getParentNode()->isTPredLabelled ( R, C, this );
	else
		return false;
}

bool
DlCompletionTree :: isNSomeApplicable ( const TRole* R, BipolarPointer C ) const
{
	DlCompletionTree::const_edge_iterator
		p, p_end = endp(), s_end = ends();
	for ( p = begins(); p < s_end; ++p )
		if ( (*p)->isNeighbour(R) && (*p)->getArcEnd()->isLabelledBy(C) )
			return true;	// already contained such a label

	if ( !sessionHasInverseRoles )
		return false;

	for ( p = beginp(); p < p_end; ++p )
		if ( (*p)->isNeighbour(R) && (*p)->getArcEnd()->isLabelledBy(C) )
			return true;	// already contained such a label

	return false;
}

bool
DlCompletionTree :: isTSomeApplicable ( const TRole* R, BipolarPointer C ) const
{
	DlCompletionTree::const_edge_iterator
		p, p_end = endp(), s_end = ends();
	for ( p = begins(); p < s_end; ++p )
		if ( (*p)->isNeighbour(R) && (*p)->getArcEnd()->isTSuccLabelled(R,C) )
			return true;	// already contained such a label

	if ( !sessionHasInverseRoles )
		return false;

	for ( p = beginp(); p < p_end; ++p )
		if ( (*p)->isNeighbour(R) && (*p)->getArcEnd()->isTPredLabelled(R,C,this) )
			return true;	// already contained such a label

	return false;
}

#ifdef RKG_IR_IN_NODE_LABEL
	//----------------------------------------------
	// inequality relation methods
	//----------------------------------------------

// check if IR for the node contains C
bool DlCompletionTree :: inIRwithC ( const ConceptWDep& C, const DepSet& ds ) const
{
	if ( IR.empty() )
		return false;

	for ( IRInfo::const_iterator p = IR.begin(); p != IR.end(); ++p )
		if ( p->bp() == C.bp() )
		{
			setClashSet ( p->getDep() + C.getDep() + ds );
			return true;
		}

	return false;
}

// check if the NODE's and current node's IR are labelled with the same level
bool DlCompletionTree :: nonMergable ( const DlCompletionTree* node, const DepSet& ds ) const
{
	if ( IR.empty() || node->IR.empty() )
		return false;

	for ( IRInfo::const_iterator p = node->IR.begin(); p != node->IR.end(); ++p )
		if ( inIRwithC ( *p, ds ) )
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
	nss->nPars = Parent.size();
	nss->nSons = Son.size();
	Label.save(nss->lab);
	nss->cached = cached;
#ifdef RKG_SAVE_AFFECTED
	nss->affected = affected;
#endif

	logSRNode("SaveNode");
}

#ifdef RKG_USE_DYNAMIC_BACKJUMPING
#	define __DYNAMIC_NODE_RESTORE
#endif

void DlCompletionTree :: restore ( SaveState* nss )
{
	if ( nss == NULL )
		return;

	// level restore
	curLevel = nss->curLevel;

	// label restore
	Label.restore ( nss->lab, curLevel );

	// we don't save blockers info, so just invalidate blockers
	iBlocker = NULL;
	dBlocker = NULL;

	// remove new parents
	Parent.resize ( nss->nPars );
	// remove new sons
#ifndef __DYNAMIC_NODE_RESTORE
#if RKG_DEFAULT_CTREE_ALLOCATION
	for ( unsigned int i = nss->nSons; i < Son.size (); ++i )
		delete Son[i];
#endif
	Son.resize ( nss->nSons );
#else
	for ( int j = Son.size()-1; j >= 0; --j )
		if ( Son[j]->Node->creLevel <= curLevel )
		{
			Son.resize(j+1);
			break;
		}
#	if RKG_DEFAULT_CTREE_ALLOCATION
		else
			delete Son[j];
#	endif
#endif

	// restore cached value
	cached = nss->cached;

#ifdef RKG_SAVE_AFFECTED
	// restore affected value
	affected = nss->affected;
#else
	// conservatively set affected value
	affected = true;
#endif

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
	o << '(' << curLevel << ')';

	// data node?
	if ( isDataNode() )
		o << "d";

	// label (if any)
	Label.print(o);

	// blocking status information
	logNodeBStatus(o);
}
