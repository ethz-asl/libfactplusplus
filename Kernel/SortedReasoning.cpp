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

#include "globaldef.h"

#ifdef RKG_USE_SORTED_REASONING

#include "RoleMaster.h"
#include "dlDag.h"
#include "logging.h"

/// merge label of given role and all its super-roles
void TRole :: mergeSupersDomain ( void )
{
	for ( iterator p = begin_anc(); p != end_anc(); ++p )
		domLabel.merge((*p)->domLabel);

	// for reflexive role -- merge domain and range labels
	if ( isReflexive() )
		domLabel.merge(getRangeLabel());

	// for R1*R2*...*Rn [= R, merge dom(R) with dom(R1) and ran(R) with ran(Rn)
	for ( std::vector<roleSet>::iterator q = subCompositions.begin(); q != subCompositions.end(); ++q )
		if ( !q->empty() )
		{
			domLabel.merge((*q->begin())->domLabel);
			getRangeLabel().merge(q->back()->getRangeLabel());
		}
}

/// check if two BPs are of the same sort
bool DLDag :: haveSameSort ( unsigned int i, unsigned int j )
{
	assert ( i > 0 && j > 0 );	// sanity check

	// everything has the same label as TOP
	if ( i == 1 || j == 1 )
		return true;

	// if some concepts were added to DAG => nothing to say
	if ( i >= sortArraySize || j >= sortArraySize )
		return true;

	// check whether two sorts are identical
	return (*this)[i].getSort() == (*this)[j].getSort();
}

/// merge sorts for a given role
void DLDag :: mergeSorts ( TRole* R )
{
	// associate role domain labels
	R->mergeSupersDomain();
	merge ( R->getDomainLabel(), R->getBPDomain() );
	// also associate functional nodes (if any)
	for ( TRole::iterator q = R->begin_topfunc(), q_end = R->end_topfunc(); q != q_end; ++q )
		merge ( R->getDomainLabel(), (*q)->getFunctional() );
}

/// merge sorts for a given vertex
void DLDag :: mergeSorts ( DLVertex& v )
{
	switch ( v.Type() )
	{
	case dtLE:	// set R&D for role
	case dtForall:
		v.merge(const_cast<TRole*>(v.getRole())->getDomainLabel());	// domain(role)=cur
		merge ( const_cast<TRole*>(v.getRole())->getRangeLabel(), v.getC() );
		break;
	case dtIrr:	// equate R&D for role
		v.merge(const_cast<TRole*>(v.getRole())->getDomainLabel());
		v.merge(const_cast<TRole*>(v.getRole())->getRangeLabel());
		break;
	case dtAnd:
		for ( DLVertex::const_iterator q = v.begin(), q_end = v.end(); q < q_end; ++q )
			merge ( v.getSort(), *q );
		break;
	case dtNSingleton:
	case dtPSingleton:
	case dtPConcept:
	case dtNConcept:	// merge with description
		merge ( v.getSort(), v.getC() );
		break;

	case dtDataType:	// nothing to do
	case dtDataValue:
	case dtDataExpr:
		break;
	case dtTop:
	default:
		assert (0);
	}
}

/// build the sort system for given TBox
void DLDag :: determineSorts ( RoleMaster& RM )
{
	sortArraySize = Heap.size();

	RoleMaster::iterator p, p_end = RM.end();
	HeapType::iterator i, i_end = Heap.end();

	// init roles R&D sorts
	for ( p = RM.begin(); p < p_end; ++p )
		if ( !(*p)->isSynonym() )
			mergeSorts(*p);

	for ( i = Heap.begin()+2; i < i_end; ++i )
		mergeSorts(**i);

	CHECK_LL_RETURN(llAlways);
	unsigned int sum = 0;
	for ( i = Heap.begin()+2; i < i_end; ++i )
		if ( (*i)->getSort().isSample() )
			++sum;

	for ( p = RM.begin(); p < p_end; ++p )
		if ( !(*p)->isSynonym() && const_cast<TRole*>(*p)->getRangeLabel().isSample() )
			++sum;

	LL << "\nThere are ";
	if ( sum > 1 )
		LL << sum;
	else
		LL << "no";
	LL << " different sorts in TBox\n";
}

#endif	// RKG_USE_SORTED_REASONING
