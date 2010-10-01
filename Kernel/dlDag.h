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

#ifndef DLDAG_H
#define DLDAG_H

#include <vector>
#include <cstring>	// strlen

#include "globaldef.h"	// for statistic printed
#include "fpp_assert.h"
#include "dlVertex.h"
#include "dlVHash.h"
#include "ifOptions.h"
#include "tRole.h"

class RoleMaster;
class TConcept;

/** DAG of DL Verteces used in FaCT++ reasoner */
class DLDag
{
public:		// types
	typedef std::vector<DLVertex*> HeapType;
	typedef std::vector<unsigned int> StatVector;
		/// typedef for the hash-table
	typedef dlVHashTable HashTable;

protected:	// members
		/// body of DAG
	HeapType Heap;
		/// all the AND nodes (needs to recompute)
	StatVector listAnds;
		/// hash-table for verteces (and, all, LE) fast search
	HashTable indexAnd, indexAll, indexLE;

		/// cache efficiency -- statistic purposes
	unsigned int nCacheHits;

#ifdef RKG_USE_SORTED_REASONING
		/// size of sort array
	unsigned int sortArraySize;
#endif

	// tunable flags (set by readConfig)

		/// sort strings: option[0] for SAT/cache tests, option[1] for SUB/classify tests
	const char* orSortSat;
	const char* orSortSub;
		/// sort index (if necessary). Possible values are Size, Depth, Freq
	unsigned int iSort;
		/// whether or not sorting order is ascending
	bool sortAscend;
		/// prefer non-generating rules in OR orderings
	bool preferNonGen;

		/// flag whether cache should be used
	bool useDLVCache;

private:	// no copy
		/// no copy c'tor
	DLDag ( const DLDag& );
		/// no assignment
	DLDag& operator= ( const DLDag& );

protected:	// methods
		/// setup flags by given option set
	void readConfig ( const ifOptionSet* Options );
		/// check if given string is correct sort ordering representation
	bool isCorrectOption ( const char* str )
	{
		if ( str == NULL )
			return false;
		unsigned int n = strlen(str);
		if ( n < 1 || n > 3 )
			return false;
		char Method = str[0],
			 Order = n >= 2 ? str[1] : 'a',
			 NGPref = n == 3 ? str[2] : 'p';
		return ( Method == 'S' || Method == 'D' || Method == 'F' ||
				 Method == 'B' || Method == 'G' || Method == '0' )
			&& ( Order == 'a' || Order == 'd' ) && ( NGPref == 'p' || NGPref == 'n' );
	}
		/// gather vertex freq statistics
	void computeVertexFreq ( BipolarPointer p );
		/// change order of ADD elements wrt statistic
	void Recompute ( void )
	{
		for ( StatVector::const_iterator p = listAnds.begin(), p_end = listAnds.end(); p < p_end; ++p )
			Heap[*p]->sortEntry(*this);
	}
		/// set OR sort flags based on given option string; Recompute if necessary
	void setOrderOptions ( const char* opt );
		/// clear all DFS info from elements of DAG
	void clearDFS ( void )
	{
		for ( unsigned int i = Heap.size()-1; i > 0; --i )
			Heap[i]->clearDFS();
	}

		/// get index corresponding to DLVertex's tag
	const HashTable& getIndex ( DagTag tag ) const
	{
		switch(tag)
		{
		case dtCollection:
		case dtAnd:		return indexAnd;
		case dtIrr:
		case dtUAll:
		case dtForall:	return indexAll;
		case dtLE:		return indexLE;
		default:		fpp_unreachable();	// no such index
		}
	}
		/// update index corresponding to DLVertex's tag
	void updateIndex ( DagTag tag, BipolarPointer value )
	{
		switch(tag)
		{
		case dtCollection:
		case dtAnd:		indexAnd.addElement(value); listAnds.push_back(value); break;
		case dtIrr:
		case dtUAll:
		case dtForall:	indexAll.addElement(value); break;
		case dtLE:		indexLE.addElement(value); break;
		default:		break;	// nothing to do
		}
	}

#ifdef RKG_USE_SORTED_REASONING
	// internal sort interface

		/// merge sorts for a given role
	void mergeSorts ( TRole* R );
		/// merge sorts for a given vertex
	void mergeSorts ( DLVertex& v );
#endif

#ifdef RKG_PRINT_DAG_USAGE
		/// print usage of DAG
	void PrintDAGUsage ( std::ostream& o ) const;
#endif

public:		// interface
		/// the only c'tor
	DLDag ( const ifOptionSet* Options );
		/// d'tor
	~DLDag ( void );

	// construction methods

		/// get index of given vertex; include vertex to DAG if necessary
	BipolarPointer add ( DLVertex* v );
		/// add vertex to the end of DAG and calculate it's statistic if necessary
	BipolarPointer directAdd ( DLVertex* v )
	{
		Heap.push_back(v);
		// return an index of just added entry
		return Heap.size()-1;
	}
		/// add vertex to the end of DAG and calculate it's statistic if necessary; put it into cache
	BipolarPointer directAddAndCache ( DLVertex* v )
	{
		BipolarPointer ret = directAdd(v);
		if ( useDLVCache )
			updateIndex ( v->Type(), ret );
		return ret;
	}
		/// check if given index points to the last DAG entry
	bool isLast ( BipolarPointer p ) const { return (getValue(p) == Heap.size()-1); }

	// access methods

		/// whether to use cache for nodes
	void setExpressionCache ( bool val ) { useDLVCache = val; }
		/// return true if p1 is less than p2 using chosen sort order
	bool less ( BipolarPointer p1, BipolarPointer p2 ) const;

		/// access by index (non-const version)
	DLVertex& operator [] ( BipolarPointer i )
	{
#	ifdef ENABLE_CHECKING
		fpp_assert ( isValid(i) );
		fpp_assert ( getValue(i) < size() );
#	endif
		return *Heap[getValue(i)];
	}
		/// access by index (const version)
	const DLVertex& operator [] ( BipolarPointer i ) const
	{
#	ifdef ENABLE_CHECKING
		fpp_assert ( isValid(i) );
		fpp_assert ( getValue(i) < size() );
#	endif
		return *Heap[getValue(i)];
	}

		/// get size of DAG
	size_t size ( void ) const { return Heap.size (); }
		/// get approximation of the size after query is added
	size_t maxSize ( void ) const { return size() + ( size() < 220 ? 10 : size()/20 ); }
		/// resize DAG to a given number (for clearing intermediate/deleting temp)
	void removeAfter ( size_t n );

	// option interface

		/// set defaults of OR orderings
	void setOrderDefaults ( const char* defSat, const char* defSub );
		/// use SUB options to OR ordering
	void setSubOrder ( void ) { setOrderOptions(orSortSub); }
		/// use SAT options to OR ordering;
	void setSatOrder ( void ) { setOrderOptions(orSortSat); }
		/// gather statistics necessary for the OR ordering
	void gatherStatistic ( void );

	// cache interface

		/// get cache for given BiPointer (may return NULL if no cache defined)
	const modelCacheInterface* getCache ( BipolarPointer p ) const
		{ return operator[](p).getCache(isPositive(p)); }
		/// set cache for given BiPointer; @return given cache
	void setCache ( BipolarPointer p, const modelCacheInterface* cache )
		{ operator[](p).setCache ( isPositive(p), cache ); }

	// sort interface

#ifdef RKG_USE_SORTED_REASONING
		/// merge two given DAG entries
	void merge ( mergableLabel& ml, BipolarPointer p )
	{
		if ( p != bpINVALID && p != bpTOP && p != bpBOTTOM )
			(*this)[p].merge(ml);
	}
		/// build the sort system for given TBox
	void determineSorts ( RoleMaster& ORM, RoleMaster& DRM );
		/// update sorts for <a,b>:R construction
	void updateSorts ( BipolarPointer a, TRole* R, BipolarPointer b )
	{
		merge ( R->getDomainLabel(), a );
		merge ( R->getRangeLabel(), b );
	}
		/// check if two BPs are of the same sort
	bool haveSameSort ( unsigned int p, unsigned int q )
	{
		fpp_assert ( p > 0 && q > 0 );	// sanity check

		// everything has the same label as TOP
		if ( p == 1 || q == 1 )
			return true;

		// if some concepts were added to DAG => nothing to say
		if ( p >= sortArraySize || q >= sortArraySize )
			return true;

		// check whether two sorts are identical
		return (*this)[p].getSort() == (*this)[q].getSort();
	}
#else
	bool haveSameSort ( unsigned int p, unsigned int q ) { return true; }
#endif

	// output interface

		/// print DAG size and number of cache hits, together with DAG usage
	void PrintStat ( std::ostream& o ) const
	{
		o << "Heap size = " << Heap.size () << " nodes\n"
		  << "There were " << nCacheHits << " cache hits\n";
#	ifdef RKG_PRINT_DAG_USAGE
		PrintDAGUsage(o);
#	endif
	}
		/// print the whole DAG
	void Print ( std::ostream& o ) const
	{
		o << "\nDag structure";

		for ( unsigned int i = 1; i < size(); ++i )
		{
			o << "\n" << i << " ";
			(*this)[i].Print(o);
		}

		o << std::endl;
	}

	// save/load interface; implementation is in SaveLoad.cpp

		/// save entry
	void Save ( std::ostream& o ) const;
		/// load entry
	void Load ( std::istream& i );
}; // DLDag

#include "dlVHashImpl.h"

inline BipolarPointer
DLDag :: add ( DLVertex* v )
{
	BipolarPointer ret = useDLVCache ? getIndex(v->Type()).locate(*v) : bpINVALID;

	if ( !isValid(ret) )	// we fail to find such vertex -- it's new
		return directAddAndCache(v);

	// node was found in cache
	++nCacheHits;
	delete v;
	return ret;
}

#endif
