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
#include "dlDag.h"
#include "dlCompletionGraph.h"

// statistic for calling blocking
unsigned long nBlockingCalls = 0,
			  nSuccessfullBlocks = 0,
			  nB2Tries = 0,
			  nB3Tries = 0,
			  nB4Tries = 0,
			  nB5Tries = 0,
			  nB6Tries = 0,
			  nB1Fails = 0,
			  nB2Fails = 0,
			  nB3Fails = 0,
			  nB4Fails = 0,
			  nB5Fails = 0,
			  nB6Fails = 0;

void printBlockingStat ( std::ostream& o )
{
	if ( nBlockingCalls == 0 )	// nothing to inform
		return;

	// else -- print precize statistics
	o << "\nThere were made " << nBlockingCalls << " blocking tests of which "
	  << nSuccessfullBlocks << " successfull.\nBlocking rules failure statistic: "
	  << nB1Fails << "/" << nBlockingCalls << ", "
	  << nB2Fails << "/" << nB2Tries << ", "
	  << nB3Fails << "/" << nB3Tries << ", "
	  << nB4Fails << "/" << nB4Tries << ", "
	  << nB5Fails << "/" << nB5Tries << ", "
	  << nB6Fails << "/" << nB6Tries;
}

void clearBlockingStat ( void )
{
	nBlockingCalls = nSuccessfullBlocks = nB2Tries = nB3Tries = nB4Tries = nB5Tries =
	nB6Tries = nB1Fails = nB2Fails = nB3Fails = nB4Fails = nB5Fails = nB6Fails = 0;
}

// SHIQ double-blocking method
bool DlCompletionTree :: isBlockedBy_SHIQ_db ( const DlCompletionTree* p ) const
{
	// here is classical double-blocking method,
	// current node is x, blocking node (p) is y

	assert ( hasParent () );	// there exists x'

	if ( !p->hasParent () )
		return false;	// we cannot find y'

	return ( isBlockedBy_SHI( getParentNode() )  		// L(x)=L(x')
			 && p->isBlockedBy_SHI( p->getParentNode() )  	// L(y)=L(y')
			 //FIXME!!! not just '==' but 2 equal sets
//			 && getParent()->Label == p->getParent()->Label 	// L((x',x))=L((y',y))
			 );
}

// optimized blocking for SHIQ
bool DlCompletionTree :: isBlockedBy_SHIQ_ob ( const DlCompletionTree* p ) const
{
	assert ( pDLHeap != NULL );

	++nBlockingCalls;

	// optimized double-blocking method
	if ( isCommonlyBlockedBy(p) && ( isCBlockedBy(p) || isABlockedBy(p) ) )
	{
		++nSuccessfullBlocks;
		return true;
	}

	return false;
}

bool DlCompletionTree :: isCommonlyBlockedBy ( const DlCompletionTree* p ) const
{
	// common B1:
	if ( !B1(p) )
		return false;

	for ( const_label_iterator q = p->beginl_cc(), q_end = p->endl_cc(); q < q_end; ++q )
	{
		BipolarPointer bp = q->bp();
		const DLVertex& v = (*pDLHeap)[bp];

		if ( v.Type() == dtForall && isPositive(bp) )
		{	// (all S C) \in L(w')
			if ( !B2 ( v, bp ) )
				return false;
		}
	}

	return true;
}

bool DlCompletionTree :: isABlockedBy ( const DlCompletionTree* p ) const
{
	// current = w; p = w'; parent = v
#ifdef ENABLE_CHECKING
	assert ( hasParent () );	// there exists v
#endif

	// B3,B4
	for ( const_label_iterator q = p->beginl_cc(), q_end = p->endl_cc(); q < q_end; ++q )
	{
		BipolarPointer bp = q->bp();
		const DLVertex& v = (*pDLHeap)[bp];

		if ( v.Type() == dtForall && isNegative(bp) )
		{	// (some T E) \in L(w')
			if ( !B4 ( p, 1, v.getRole(), inverse(v.getC()) ) )
				return false;
		}
		else if ( v.Type() == dtLE )
		{
			if ( isPositive(bp) )
			{	// (<= n S C) \in L(w')
				if ( !B3 ( p, v.getNumberLE(), v.getRole(), v.getC() ) )
					return false;
			}
			else
			{	// (>= m T E) \in L(w')
				if ( !B4 ( p, v.getNumberGE(), v.getRole(), v.getC() ) )
					return false;
			}
		}
	}

	// all other is OK -- done;
	return true;
}

bool DlCompletionTree :: isCBlockedBy ( const DlCompletionTree* p ) const
{
	// current = w; p = w'; parent = v
#ifdef ENABLE_CHECKING
	assert ( hasParent () );	// there exists v
#endif

	// B5
	const_label_iterator q, q_end;
	for ( q = p->beginl_cc(), q_end = p->endl_cc(); q < q_end; ++q )
	{
		BipolarPointer bp = q->bp();
		const DLVertex& v = (*pDLHeap)[bp];

		if ( v.Type() == dtLE && isPositive(bp) )
		{	// (<= n T E) \in L(w')
			if ( !B5 ( v.getRole(), v.getC() ) )
				return false;
		}
	}

	// B6
	const DlCompletionTree* par = getParentNode();
	for ( q = par->beginl_cc(), q_end = par->endl_cc(); q < q_end; ++q )
	{
		BipolarPointer bp = q->bp();
		const DLVertex& v = (*pDLHeap)[bp];

		if ( v.Type() == dtLE && isNegative(bp) )
		{	// (>= m U F) \in L(v)
			if ( !B6 ( v.getRole(), v.getC() ) )
				return false;
		}
	}

	return true;
}

//----------------------------------------------------------------------
//--   B1 to B6 conditions implementation
//--  WARNING!! 19-06-2005. All blockable nodes has the only parent
//--    (with probably several links to it). So we should check all of them
//----------------------------------------------------------------------


	/// check if B1 holds for a given vertex (p is a candidate for blocker)
bool DlCompletionTree :: B1 ( const DlCompletionTree* p ) const
{
	if ( isBlockedBy_SH(p) )
		return true;

	++nB1Fails;
	return false;
}

	/// check if B2 holds for (AS C) a simple automaton A for S
bool DlCompletionTree :: B2 ( const RoleAutomaton& A, BipolarPointer C ) const
{
	const DlCompletionTree* parent = getParentNode();
	const CGLabel& parLab = parent->label();
	RATransition* trans = *A.begin(0);
	++nB2Tries;

	for ( const_edge_iterator p = beginp(), p_end = endp(); p < p_end; ++p )
		if ( !(*p)->isIBlocked() && (*p)->getArcEnd() == parent && trans->applicable((*p)->getRole()) )
		{
			if ( !parLab.contains(C) )
			{
				++nB2Fails;
				return false;
			}
			else
				return true;
		}

	return true;
}

	/// check if B2 holds for C=(AS{n} X) a complex automaton A for S
bool DlCompletionTree :: B2 ( const RoleAutomaton& A, BipolarPointer C, RAState n ) const
{
	const DlCompletionTree* parent = getParentNode();
	const CGLabel& parLab = parent->label();
	RoleAutomaton::const_trans_iterator q, end = A.end(n);
	++nB2Tries;

	for ( const_edge_iterator p = beginp(), p_end = endp(); p < p_end; ++p )
	{
		if ( (*p)->isIBlocked() || (*p)->getArcEnd() != parent )
			continue;
		const TRole* R = (*p)->getRole();

		for ( q = A.begin(n); q != end; ++q )
			if ( (*q)->applicable(R) )
				if ( !parLab.containsCC(C-n+(*q)->final()) )
				{
					++nB2Fails;
					return false;
				}
	}

	return true;
}

	/// check if B3 holds for (<= n S.C)\in w' (p is a candidate for blocker)
bool DlCompletionTree :: B3 ( const DlCompletionTree* p, unsigned int n, const TRole* S, BipolarPointer C ) const
{
#ifdef ENABLE_CHECKING
	assert ( hasParent () );	// safety check
#endif
	++nB3Tries;

	bool ret;
	// if (<= n S C) \in L(w') then
	// a) w is an inv(S)-succ of v or
	if ( !isParentArcLabelled(S) )
		ret = true;
	// b) w is an inv(S) succ of v and ~C\in L(v) or
	else if ( getParentNode()->isLabelledBy(inverse(C)) )
		ret = true;
	// c) w is an inv(S) succ of v and C\in L(v)...
	else if ( !getParentNode()->isLabelledBy(C) )
		ret = false;
	else
	{	// ...and <=n-1 S-succ. z with C\in L(z)
		register unsigned int m = 0;
		for ( const_edge_iterator q = p->begins(), q_end = p->ends(); q < q_end; ++q )
			if ( (*q)->isNeighbour(S) && (*q)->getArcEnd()->isLabelledBy(C) )
				++m;

		ret = ( m < n );
	}

	if ( !ret )
		++nB3Fails;

	return ret;
}

	/// check if B4 holds for (>= m T.E)\in w' (p is a candidate for blocker)
bool DlCompletionTree :: B4 ( const DlCompletionTree* p, unsigned int m, const TRole* T, BipolarPointer E ) const
{
#ifdef ENABLE_CHECKING
	assert ( hasParent () );	// safety check
#endif
	++nB4Tries;

	// if (>= m T E) \in L(w') then
	// b) w is an inv(T) succ of v and E\in L(v) and m == 1 or
	if ( isParentArcLabelled(T) && m == 1 && getParentNode()->isLabelledBy(E) )
		return true;

	// a) w' has at least m T-succ z with E\in L(z)
	// check all sons
	register unsigned int n = 0;
	for ( const_edge_iterator q = p->begins(), q_end = p->ends(); q < q_end; ++q )
		if ( (*q)->isNeighbour(T) && (*q)->getArcEnd()->isLabelledBy(E) )
			if ( ++n >= m )		// check if node has enough successors
				return true;

	// rule check fails
	++nB4Fails;
	return false;
}

	/// check if B5 holds for (<= n T.E)\in w'
bool DlCompletionTree :: B5 ( const TRole* T, BipolarPointer E ) const
{
#ifdef ENABLE_CHECKING
	assert ( hasParent () );	// safety check
#endif
	++nB5Tries;

	// if (<= n T E) \in L(w'), then
	// either w is not an inv(T)-successor of v...
	if ( !isParentArcLabelled(T) )
		return true;
	// or ~E \in L(v)
	if ( getParentNode()->isLabelledBy ( inverse(E) ) )
		return true;

	++nB5Fails;
	return false;
}

	/// check if B6 holds for (>= m U.F)\in v
bool DlCompletionTree :: B6 ( const TRole* U, BipolarPointer F ) const
{
#ifdef ENABLE_CHECKING
	assert ( hasParent () );	// safety check
#endif
	++nB6Tries;

	// if (>= m U F) \in L(v), and
	// w is U-successor of v...
	if ( !isParentArcLabelled(U->inverse()) )
		return true;
	// then ~F\in L(w)
	if ( isLabelledBy ( inverse(F) ) )
		return true;

	++nB6Fails;
	return false;
}

//----------------------------------------------------------------------
//--   changing blocked status
//----------------------------------------------------------------------

void
DlCompletionGraph :: updateIBlockedStatus ( DlCompletionTree* node )
{
	if ( !node->isIBlocked()		// was not i-blocked (don't care)
		 || node->isPBlocked()		// p-blocked (can't be unblocked)
		 || !node->isAffected() )	// does not change since previous check
		return;

	if ( node->iBlocker->isAffected() )
		updateDBlockedStatus(const_cast<DlCompletionTree*>(node->iBlocker));
	else	// iBlocker does not changed, so it is still d-blocked, so current is still i-blocked
		node->clearAffected();
	// FIXME!! for now
//	assert ( !node->isAffected() );
}

void DlCompletionGraph :: detectBlockedStatus ( DlCompletionTree* node )
{
	DlCompletionTree* p = node;
	bool wasBlocked = node->isBlocked();

	while ( p->hasParent() && p->isBlockableNode() && p->isAffected() )
	{
		findDBlocker(p);
		p->clearAffected();
		if ( p->isBlocked() )
			return;
		p = p->getParentNode();
	}
	p->clearAffected();
	if ( wasBlocked && !node->isBlocked() )
	{
		node->logNodeUnblocked();
		unblockNodeChildren(node);
	}
}

/// propagate blocked status to children/check it
void DlCompletionTree :: propagateIBlockedStatus ( const DlCompletionTree* p )
{
	for ( const_edge_iterator q = begins(), q_end = ends(); q < q_end; ++q )
		if ( !(*q)->isIBlocked() )
			(*q)->getArcEnd()->setIBlocked(p);
}

void DlCompletionTree :: setIBlocked ( const DlCompletionTree* p )
{
	// nominal nodes can't be blocked
	if ( isPBlocked() || isNominalNode() )
		return;

	clearAffected();

	if ( iBlocker == p ||	// already iBlocked -- nothing changes
		 this == p )		// prevent node to be IBlocked due to reflexivity
		return;

	iBlocker = p;
	logNodeIBlocked();
	propagateIBlockedStatus(p);
}

void DlCompletionGraph :: findDAncestorBlocker ( DlCompletionTree* node ) const
{
	register const DlCompletionTree* p = node;

	while ( p->hasParent() && p->isBlockableNode() )
	{
		p = p->getParentNode();

		if ( isBlockedBy ( node, p ) )
		{
			node->setDBlocked(p);
			return;
		}
	}
}

void DlCompletionGraph :: findDAnywhereBlocker ( DlCompletionTree* node ) const
{
	for ( const_iterator q = begin(), q_end = end(); q < q_end && *q != node; ++q )
	{
		const DlCompletionTree* p = *q;

		// node was merge to smth with the larger ID or is blocked itself
		if ( p->isBlocked() || p->isPBlocked() )
			continue;

		if ( isBlockedBy ( node, p ) )
		{
			node->setDBlocked(p);
			return;
		}
	}
}
