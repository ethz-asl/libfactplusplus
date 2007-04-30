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

#ifndef _MODELCACHESINGLETON_H
#define _MODELCACHESINGLETON_H

#include "logging.h"
#include "modelCacheInterface.h"
#include "BiPointer.h"

/** Model caching implementation for singleton models.
	Such models contains only one [negated] concept in completion tree.
	Reduced set of operations, but very efficient.
*/
class modelCacheSingleton: public modelCacheInterface
{
protected:	// members
		/// the singleton itself
	BipolarPointer Singleton;

protected:	// methods
		/// return result of merge of two given BPs
	static BipolarPointer mergeSingletons ( BipolarPointer bp1, BipolarPointer bp2 )
	{
		// if some of BPs are BOTTOM => result is contradiction
		if ( bp1 == bpBOTTOM || bp2 == bpBOTTOM )
			return bpBOTTOM;
		// if some of BPs are INVALID => result is undefined
		if ( bp1 == bpINVALID || bp2 == bpINVALID )
			return bpINVALID;
		// check for direct contradiction
		if ( bp1 == inverse(bp2) )
			return bpBOTTOM;
		// check for top in a singleton => just copy the other
		if ( bp1 == bpTOP )
			return bp2;
		// check for top in a singleton => just copy the other
		if ( bp2 == bpTOP )
			return bp1;
		// check for 2 same singletons
		if ( bp1 == bp2 )
			return bp1;
		// we couldn't produce valid Singleton for this case
		return bpINVALID;
	}
		/// get model state for given bp
	static modelCacheState getState ( BipolarPointer bp )
	{
		switch ( bp )
		{
		case bpBOTTOM:	return csInvalid;
		case bpINVALID:	return csFailed;
		default:		return csValid;
		}
	}

public:
		/// c'tor: no nominals can be here
	modelCacheSingleton ( BipolarPointer bp )
		: modelCacheInterface(/*flagNominals=*/false)
		, Singleton(bp)
		{}
		/// copy c'tor
	modelCacheSingleton ( const modelCacheSingleton& m )
		: modelCacheInterface(m.hasNominalNode)
		, Singleton(m.Singleton)
		{}
		/// empty d'tor
	virtual ~modelCacheSingleton ( void ) {}

		/// Check if the model contains clash
	virtual modelCacheState getState ( void ) const { return getState(getValue()); }
		/// Copy of the current entry
	virtual modelCacheInterface* copy ( void ) const { return new modelCacheSingleton(*this); }
		/// access to internal value
	BipolarPointer getValue ( void ) const { return Singleton; }

	// mergable part

		/// check whether two caches can be merged; @return state of "merged" model
	modelCacheState canMerge ( const modelCacheInterface* p ) const
	{
		if ( hasNominalClash(p) )	// fail to merge due to nominal precense
			return csFailed;
		if ( p->getCacheType() == mctSingleton )
			return getState ( mergeSingletons ( Singleton,
				static_cast<const modelCacheSingleton*>(p)->getValue() ) );
		else
			return p->canMerge(this);
	}
		/// merge general-type cache to singleton one
	virtual modelCacheState merge ( const modelCacheInterface* p )
	{
		BipolarPointer ms = bpINVALID;
		if ( p->getCacheType() == mctSingleton && !hasNominalClash(p) )
			ms = static_cast<const modelCacheSingleton*>(p)->getValue();

		Singleton = mergeSingletons ( Singleton, ms );
		updateNominalStatus(p);
		return getState();
	}
	/// Get the tag identifying the cache type
	virtual modelCacheType getCacheType ( void ) const { return mctSingleton; }
	/// log this cache entry (with given level)
	virtual void logCacheEntry ( unsigned int level ) const
	{
		if ( LLM.isWritable(level) )
			LL << "\nSingleton cache: element " << getValue();
	}
}; // modelCacheSingleton

#endif
