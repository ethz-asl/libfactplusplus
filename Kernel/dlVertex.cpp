/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2009 by Dmitry Tsarkov

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

	// find apropiate place to insert
	unsigned int v = getValue (p);

	register unsigned int i, j;
	for ( i = 0; i < Child. size () && getValue(Child[i]) < v; ++i )
		(void)NULL;

	if ( i == Child. size() )	// finish
	{
		Child. push_back(p);
		return false;
	}

	// we finds a place with |Child[i]| >= v
	if ( Child [i] == p )	// concept already exists
		return false;
	else if ( Child [i] == inverse (p) )
	{	// clear all stuff; returns true
		Child. resize (0);
		Op = dtBad;
		return true;
	}

	// we need to insert p into set
	Child.push_back ( Child.back () );

	for ( j = Child. size ()-1; j>i; --j )
		Child [j] = Child [j-1];	// copy the tail

	Child [i] = p;

	// FIXME: add some simplification (about AR.C1, AR.c2 etc)
	return false;
}

/// gather statistic for the subgraph starting from given node
void
DLVertex :: gatherStat ( const DLDag& dag, bool pos )
{
	// this vertex is already processed
	if ( isProcessed(pos) )
		return;

	// in case of cycle: mark concept as such
	if ( isVisited(pos) )
	{
		setInCycle(pos);
		return;
	}

	setVisited(pos);

	// ensure that the statistic is gather for all sub-concepts of the expression
	switch ( Type() )
	{
	case dtCollection:	// if pos then behaves like and
		if ( !pos )		// ~Coll -- nothing to do
			break;
		// fallthrough
	case dtAnd:	// check all the conjuncts
		for ( const_iterator q = begin(), q_end = end(); q < q_end; ++q )
			const_cast<DLDag&>(dag)[*q].gatherStat ( dag, pos == isPositive(*q) );	// FIXME!! think later on
		break;
	case dtName:
	case dtForall:
	case dtUAll:
	case dtLE:	// check a single referenced concept
		const_cast<DLDag&>(dag)[getC()].gatherStat ( dag, pos == isPositive(getC()) );
		break;
	default:	// nothing to do
		break;
	}

	setProcessed(pos);

	// here all the necessary statistics is gathered -- use it in the init
	initStat ( dag, pos );
}

void DLVertex :: initStat ( const DLDag& dag, bool pos )
{
	StatType d = 0, s = 1, b = 0, g = 0, d_loc;

#define UPDATE_STAT()			\
	if ( v.isInCycle(posQ) )	\
		setInCycle(pos);		\
	d_loc = v.getDepth(posQ);	\
	s += v.getSize(posQ);		\
	b += v.getBranch(posQ);		\
	g += v.getGener(posQ);		\
	if ( d_loc > d ) d = d_loc

	if ( !omitStat(pos) )
	{
		if ( isValid(getC()) )
		{
			const DLVertex& v = dag[getC()];
			bool posQ = (pos == isPositive(getC()));
			UPDATE_STAT();
		}
		else
			for ( const_iterator q = begin(), q_end = end(); q < q_end; ++q )
			{
				const DLVertex& v = dag[*q];
				bool posQ = (pos == isPositive(*q));
				UPDATE_STAT();
			}
	}

#undef UPDATE_STAT

	// correct values wrt POS
	switch ( Type() )
	{
	case dtAnd:
		if ( !pos )
			++b;	// OR is branching
		break;
	case dtForall:
		++d;		// increase depth
		if ( !pos )
			++g;	// SOME is generating
		break;
	case dtLE:
		++d;		// increase depth
		if ( !pos )
			++g;	// >= is generating
		else if ( getNumberLE() != 1 )
			++b;	// <= is branching
		break;
	default:
		break;
	}

	setStatValues ( d, s, b, g, pos );
}

/// increment frequency of the sub-tree starting from the current node
void DLVertex :: incFreq ( DLDag& dag, bool pos )
{
	if ( isVisited(pos) )	// avoid cycles
		return;

	incFreqValue(pos);	// increment frequence of current vertex

	setVisited(pos);

	if ( omitStat(pos) )	// negation of primitive concept-like
		return;

	// increment frequence of all subvertex
	if ( isValid(getC()) )
		dag[getC()].incFreq ( dag, pos == isPositive(getC()) );
	else
		for ( const_iterator q = begin(); q != end(); ++q )
			dag[*q].incFreq ( dag, pos == isPositive(*q) );
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
	case dtDataType: return "data-type";
	case dtDataValue: return "data-value";
	case dtDataExpr: return "data-expr";
	case dtAnd:		return "and";
	case dtCollection:return "collection";
	case dtForall:	return "all";
	case dtLE:		return "at-most";
	case dtUAll:	return "all U";
	case dtIrr:		return "irreflexive";
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
	case dtTop:
	case dtUAll:
		break;

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

	default:
		std::cerr << "Error printing vertex of type " << getTagName() << "(" << Type() << ")";
		fpp_unreachable();
	}

	// print operands of the concept constructor
	for ( const_iterator q = begin(); q != end(); ++q )
		o << ' ' << *q;
}
