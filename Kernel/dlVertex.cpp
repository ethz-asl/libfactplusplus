/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2011 by Dmitry Tsarkov

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

#include "dlVertex.h"
#include "dlDag.h"
#include "tDataEntry.h"

// adds a child to 'AND' vertex.
// if finds a contrary pair of concepts -- returns TRUE and became dtBad;
// else return false
bool DLVertex :: addChild ( BipolarPointer p )
{
	// if adds to broken vertex -- do nothing;
	if ( Op == dtBad )
		return true;

	// if adds TOP -- nothing to do
	if ( p == bpTOP )
		return false;

	// if adding BOTTOM -- return clash (empty vertex) immediately
	// this can happen in case of nested simplifications; see bNested1
	if ( p == bpBOTTOM )
	{
clash:	// clash found: clear all stuff; returns true
		Child.resize(0);
		Op = dtBad;
		return true;
	}

	// find apropiate place to insert
	unsigned int v = getValue (p);

	BaseType::iterator q = Child.begin(), q_end = Child.end();
	for ( ; q != q_end && getValue(*q) < v; ++q )
		(void)NULL;

	if ( q == q_end )	// finish
	{
		Child.push_back(p);
		return false;
	}

	// we finds a place with |Child[i]| >= v
	if ( *q == p )	// concept already exists
		return false;
	else if ( *q == inverse(p) )
		goto clash;

	// we need to insert p into set
	unsigned int offset = q - Child.begin();
	Child.push_back(Child.back());

	for ( q_end = Child.begin()+offset, q = Child.end()-1; q != q_end; --q )
		*q=*(q-1);	// copy the tail

	*q = p;

	// FIXME: add some simplification (about AR.C1, AR.c2 etc)
	return false;
}

// Sort given entry in the order defined by flags in a DAG.
// the overall sorted entry structure looks like
//   fffA..M..Zlll if sortAscend set, and
//   fffZ..M..Alll if sortAscend cleared.
// here 's' means "always first" entries, like neg-primconcepts,
//  and 'l' means "always last" entries, like looped concepts
void DLVertex :: sortEntry ( const DLDag& dag )
{
	// safety check
	if ( Type() != dtAnd )
		return;

	register BipolarPointer x;	// value of moved element
	register int j;
	unsigned int size = Child.size();

	for ( register unsigned int i = 1; i < size; ++i )
	{
		x = Child[i];

		// put x to the place s.t. SxL, where S <= x < L wrt dag.less()
		for ( j = i-1; j >= 0 && dag.less ( x, Child[j] ); --j )
			Child[j+1] = Child[j];

		// insert new element on it's place
		Child[j+1] = x;
	}
}

/*
// sortirovka vstavkami s minimumom (na budushhee)
template<class T>
inline void insertSortGuarded(T a[], long size) {
  T x;
  long i, j;
  T backup = a[0];			// ????????? ?????? ?????? ???????

  setMin(a[0]);				// ???????? ?? ???????????

  // ????????????? ??????
  for ( i=1; i < size; i++) {
    x = a[i];

    for ( j=i-1; a[j] > x; j--)
	  a[j+1] = a[j];

	a[j+1] = x;
  }

  // ???????? backup ?? ?????????? ?????
  for ( j=1; j<size && a[j] < backup; j++)
    a[j-1] = a[j];

  // ??????? ????????
  a[j-1] = backup;
}
*/

const char* DLVertex :: getTagName ( void ) const
{
	switch (Op)
	{
	case dtTop:		return "*TOP*";
	case dtBad:		return "bad-tag";
	case dtNConcept:return "concept";
	case dtPConcept:return "primconcept";
	case dtPSingleton:return "prim-singleton";
	case dtNSingleton:return "singleton";
	case dtSplitConcept:return "split-concept";
	case dtDataType: return "data-type";
	case dtDataValue: return "data-value";
	case dtDataExpr: return "data-expr";
	case dtAnd:		return "and";
	case dtCollection:return "collection";
	case dtForall:	return "all";
	case dtLE:		return "at-most";
	case dtIrr:		return "irreflexive";
	case dtProj:	return "projection";
	case dtNN:		return "NN-stopper";
	case dtChoose:	return "choose";
	default:		return "UNKNOWN";
	};
}


void
DLVertex :: Print ( std::ostream& o ) const
{
	o << "[d(" << getDepth(true) << "/" << getDepth(false)
	  << "),s(" << getSize(true) << "/" << getSize(false)
	  << "),b(" << getBranch(true) << "/" << getBranch(false)
	  << "),g(" << getGener(true) << "/" << getGener(false)
	  << "),f(" << getFreq(true) << "/" << getFreq(false) << ")] ";
	o << getTagName();

	switch ( Type() )
	{
	case dtAnd:		// nothing to do (except for printing operands)
	case dtCollection:
	case dtSplitConcept:
		break;

	case dtTop:		// nothing to do
	case dtNN:
		return;

	case dtDataExpr:
		o << ' ' << *static_cast<const TDataEntry*>(getConcept())->getFacet();
		return;

	case dtDataValue:	// named entry -- just like concept names
	case dtDataType:

	case dtPConcept:
	case dtNConcept:
	case dtPSingleton:
	case dtNSingleton:
		o << '(' << getConcept()->getName() << ") " << (isNNameTag(Type()) ? "=" : "[=") << ' ' << getC();
		return;

	case dtLE:
		o << ' ' << getNumberLE() << ' ' << getRole()->getName() << ' ' << getC();
		return;

	case dtForall:
		o << ' ' << getRole()->getName() << '{' << getState() << '}' << ' ' << getC();
		return;

	case dtIrr:
		o << ' ' << getRole()->getName();
		return;

	case dtProj:
		o << ' ' << getRole()->getName() << ", " << getC() << " => " << getProjRole()->getName();
		return;

	case dtChoose:
		o << ' ' << getC();
		return;

	default:
		std::cerr << "Error printing vertex of type " << getTagName() << "(" << Type() << ")";
		fpp_unreachable();
	}

	// print operands of the concept constructor
	for ( const_iterator q = begin(); q != end(); ++q )
		o << ' ' << *q;
}
