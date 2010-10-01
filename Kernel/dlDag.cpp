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

#include "dlDag.h"

#include "logging.h"
#include "tDataEntry.h"

DLDag :: DLDag ( const ifOptionSet* Options )
	: indexAnd(*this)
	, indexAll(*this)
	, indexLE(*this)
	, nCacheHits(0)
	, useDLVCache(true)
{
	Heap.push_back ( new DLVertex (dtBad) );	// empty vertex -- bpINVALID
	Heap.push_back ( new DLVertex (dtTop) );

	readConfig ( Options );
}

DLDag :: ~DLDag ( void )
{
	for ( HeapType::iterator p = Heap.begin(), p_end = Heap.end(); p < p_end; ++p )
		delete *p;
}

void
DLDag :: removeAfter ( size_t n )
{
	fpp_assert ( n < size() );
	for ( HeapType::iterator p = Heap.begin()+n, p_end = Heap.end(); p < p_end; ++p )
	{
		// make sure data elements can be reused
		switch ( (*p)->Type() )
		{
		case dtData:
			static_cast<TDataEntry*>((*p)->getConcept())->setBP(bpINVALID);
			break;
		default:
			break;
		}
		delete *p;
	}
	Heap.resize (n);
}

void DLDag :: readConfig ( const ifOptionSet* Options )
{
	fpp_assert ( Options != NULL );	// safety check

	orSortSat = Options->getText ( "orSortSat" ).c_str();
	orSortSub = Options->getText ( "orSortSub" ).c_str();

	if ( !isCorrectOption(orSortSat) || !isCorrectOption(orSortSub) )
		throw EFaCTPlusPlus ( "DAG: wrong OR sorting options" );
}

/// set defaults of OR orderings
void
DLDag :: setOrderDefaults ( const char* defSat, const char* defSub )
{
	// defaults should be correct
	fpp_assert ( isCorrectOption(defSat) && isCorrectOption(defSub) );

	if ( LLM.isWritable(llAlways) )
		LL << "orSortSat: initial=" << orSortSat << ", default=" << defSat;
	if ( orSortSat[0] == '0' )
		orSortSat = defSat;
	if ( LLM.isWritable(llAlways) )
		LL << ", used=" << orSortSat << "\n"
		   << "orSortSub: initial=" << orSortSub << ", default=" << defSub;
	if ( orSortSub[0] == '0' )
		orSortSub = defSub;
	if ( LLM.isWritable(llAlways) )
		LL << ", used=" << orSortSub << "\n";
}

/// set OR sort flags based on given option string
void DLDag :: setOrderOptions ( const char* opt )
{
	// 0x means not to use OR sort
	if ( opt[0] == '0' )
		return;

	sortAscend = (opt[1] == 'a');
	preferNonGen = (opt[2] == 'p');

	// all statistics use negative version (as it is used in disjunctions)
	iSort = opt[0] == 'S' ? DLVertex::getStatIndexSize(false)
		  : opt[0] == 'D' ? DLVertex::getStatIndexDepth(false)
		  : opt[0] == 'B' ? DLVertex::getStatIndexBranch(false)
		  : opt[0] == 'G' ? DLVertex::getStatIndexGener(false)
		  				  : DLVertex::getStatIndexFreq(false);

	Recompute();
}

/// gather vertex freq statistics
void
DLDag :: computeVertexFreq ( BipolarPointer p )
{
	DLVertex& v = (*this)[p];
	bool pos = isPositive(p);

	if ( v.isVisited(pos) )	// avoid cycles
		return;

	v.incFreqValue(pos);	// increment frequence of current vertex
	v.setVisited(pos);

	if ( v.omitStat(pos) )	// negation of primitive concept-like
		return;

	// increment frequence of all subvertex
	if ( isValid(v.getC()) )
		computeVertexFreq ( createBiPointer ( v.getC(), pos ) );
	else
		for ( DLVertex::const_iterator q = v.begin(), q_end = v.end(); q != q_end; ++q )
			computeVertexFreq ( createBiPointer ( *q, pos ) );
}

void
DLDag :: gatherStatistic ( void )
{
	// gather main statistics for disjunctions
	for ( StatVector::iterator p = listAnds.begin(), p_end = listAnds.end(); p < p_end; ++p )
		Heap[*p]->gatherStat ( *this, /*pos=*/false );

	// if necessary -- gather frequency
	if ( orSortSat[0] != 'F' && orSortSub[0] != 'F' )
		return;

	clearDFS();

	for ( int i = size()-1; i > 1; --i )
	{
		if ( isCNameTag((*this)[i].Type()) )
			computeVertexFreq(i);
	}
}

//---------------------------------------------------
// change order of ADD elements with respect to Freq
//---------------------------------------------------

/// return true if p1 is less than p2 using chosen sort order

// the overall sorted entry structure looks like
//   fffA..M..Zlll if sortAscend set, and
//   fffZ..M..Alll if sortAscend cleared.
// here 's' means "always first" entries, like neg-primconcepts,
//  and 'l' means "always last" entries, like looped concepts

// note that the statistics is given for disjunctions,
// so inversed (neg) values are taken into account
bool DLDag :: less ( BipolarPointer p1, BipolarPointer p2 ) const
{
#	ifdef ENABLE_CHECKING
		fpp_assert ( isValid(p1) && isValid(p2) );
#	endif

	// idea: any positive entry should go first
	if ( preferNonGen )
	{
		if ( isNegative(p1) && isPositive(p2) )
			return true;
		if ( isPositive(p1) && isNegative(p2) )
			return false;
	}

	const DLVertex& v1 = (*this)[p1];
	const DLVertex& v2 = (*this)[p2];
/*
	// prefer non-cyclical
	if ( !v1.isInCycle(false) && v2.isInCycle(false) )
		return true;
	if ( !v2.isInCycle(false) && v1.isInCycle(false) )
		return false;
*/
	DLVertex::StatType key1 = v1.getStat(iSort);
	DLVertex::StatType key2 = v2.getStat(iSort);

	// return "less" wrt sortAscend
	if ( sortAscend )
		return key1 < key2;
	else
		return key2 < key1;
}

#ifdef RKG_PRINT_DAG_USAGE
/// print usage of DAG
void DLDag :: PrintDAGUsage ( std::ostream& o ) const
{
	unsigned int n = 0;	// number of no-used DAG entries
	unsigned int total = Heap.size()*2-2;	// number of total DAG entries

	for ( HeapType::const_iterator i = Heap.begin(), i_end = Heap.end(); i < i_end; ++i )
	{
		if ( (*i)->getUsage(true) == 0 )	// positive and...
			++n;
		if ( (*i)->getUsage(false) == 0 )	// negative ones
			++n;
	}

	o << "There are " << n << " unused DAG entries (" << (unsigned long)(n*100/total)
	  << "% of " << total << " total)\n";
}
#endif
