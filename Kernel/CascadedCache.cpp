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

#include "Reasoner.h"

// uncomment the following line to debug cascaded caching
//#define TMP_CACHE_DEBUG

const modelCacheInterface*
DlSatTester :: createCache ( BipolarPointer p )
{
	fpp_assert ( isValid(p) );	// safety check

	const modelCacheInterface* cache;

	// check if cache already calculated
	if ( (cache = DLHeap.getCache(p)) != NULL )
		return cache;

#ifdef TMP_CACHE_DEBUG
	std::cerr << "\nCCache for " << p << ":";
#endif
	prepareCascadedCache(p);

	// it may be a cycle and the cache for p is already calculated
	if ( (cache = DLHeap.getCache(p)) != NULL )
		return cache;

	// need to build cache
	cache = buildCache(p);
	DLHeap.setCache ( p, cache );
	return cache;
}

void
DlSatTester :: prepareCascadedCache ( BipolarPointer p )
{
	/// cycle found -- shall be processed without caching
	if ( inProcess.find(p) != inProcess.end() )
	{
#	ifdef TMP_CACHE_DEBUG
		std::cerr << " cycle with " << p << ";";
#	endif
		return;
	}

	const DLVertex& v = DLHeap[p];
	bool pos = isPositive(p);

	// check if a concept already cached
	if ( v.getCache(pos) != NULL )
		return;

	switch ( v.Type() )
	{
	case dtTop:
		break;

	case dtDataType:	// data things are checked by data inferer
	case dtDataValue:
	case dtDataExpr:
		break;

	case dtAnd:
	case dtCollection:
	{
		for ( DLVertex::const_iterator q = v.begin(), q_end = v.end(); q < q_end; ++q )
			prepareCascadedCache ( pos ? *q : inverse(*q) );
		break;
	}

	case dtPSingleton:
	case dtNSingleton:
	case dtNConcept:
	case dtPConcept:
		if ( isNegative(p) && isPNameTag(v.Type()) )
			return;
		inProcess.insert(p);
#	ifdef TMP_CACHE_DEBUG
		std::cerr << " expanding " << p << ";";
#	endif
		prepareCascadedCache ( pos ? v.getC() : inverse(v.getC()) );
		inProcess.erase(p);
		break;

	case dtForall:
	case dtLE:
	{
		const TRole* R = v.getRole();
		if ( R->isDataRole() )	// skip data-related stuff
			break;
		BipolarPointer x = pos ? v.getC() : inverse(v.getC());

		// build cache for C in \AR.C
		if ( x != bpTOP )
		{
			inProcess.insert(x);
#		ifdef TMP_CACHE_DEBUG
			std::cerr << " caching " << x << ";";
#		endif
			createCache(x);
			inProcess.erase(x);
		}

		// build cache for the Range(R) if necessary
		x = R->getBPRange();
		if ( x != bpTOP )
		{
			inProcess.insert(x);
#		ifdef TMP_CACHE_DEBUG
			std::cerr << " caching range(" << v.getRole()->getName() << ") = " << x << ";";
#		endif
			createCache(x);
			inProcess.erase(x);
		}

		break;
	}

	case dtIrr:
		break;

	default:
		fpp_unreachable();
	}
}

/// build cache for given DAG node using SAT tests; @return cache
const modelCacheInterface*
DlSatTester :: buildCache ( BipolarPointer p )
{
	if ( LLM.isWritable(llDagSat) )
	{
		LL << "\nChecking satisfiability of DAG entry " << p;
		tBox.PrintDagEntry(LL,p);
		LL << ":\n";
	}

#ifdef TMP_CACHE_DEBUG
	std::cerr << " building cache for " << p << "...";
#endif

	bool sat = runSat(p);

#ifdef TMP_CACHE_DEBUG
	std::cerr << " done";
#endif

	// unsat => P -> \bot
	if ( !sat && LLM.isWritable(llAlways) )
		LL << "\nDAG entry " << p << " is unsatisfiable\n";

	return buildCacheByCGraph(sat);
}

