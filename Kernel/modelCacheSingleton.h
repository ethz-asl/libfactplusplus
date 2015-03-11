/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2015 by Dmitry Tsarkov

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

#ifndef MODELCACHESINGLETON_H
#define MODELCACHESINGLETON_H

#include "modelCacheConst.h"
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
		/// log a particular implementation of a cache entry
	virtual void logCacheEntryImpl ( void ) const { LL << "\nSingleton cache: element " << getValue(); }

public:		// interface
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
	virtual modelCacheState getState ( void ) const { return csValid; }
		/// access to internal value
	BipolarPointer getValue ( void ) const { return Singleton; }

	// mergable part

		/// check whether two caches can be merged; @return state of "merged" model
	modelCacheState canMerge ( const modelCacheInterface* p ) const
	{
		switch ( p->getCacheType() )
		{
		case mctConst:		// TOP/BOTTOM: the current node can't add anything to the result
			return p->getState();
		case mctSingleton:	// it can be a clash
			return static_cast<const modelCacheSingleton*>(p)->getValue()
				   == inverse(getValue()) ? csInvalid : csValid;
		case mctIan:		// ask more intelligent object
			return p->canMerge(this);
		case mctBadType:	// error
		default:
			return csUnknown;
		}
	}
		/// Get the tag identifying the cache type
	virtual modelCacheType getCacheType ( void ) const { return mctSingleton; }
}; // modelCacheSingleton

#endif
