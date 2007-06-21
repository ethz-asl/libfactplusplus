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

#ifndef _MODELCACHEINTERFACE_H
#define _MODELCACHEINTERFACE_H

#include "globaldef.h"

/// status of model cache or merge operation
enum modelCacheState
{
	csInvalid,	///> clash in model/merging fails because of direct contradiction;
	csValid,	///> valid model/success in merging;
	csFailed,	///> incorrect model/merging fails because of incompleteness of procedure;
	csUnknown	///> untested model cache.
};

/// create united status for 2 given statuses
inline modelCacheState mergeStatus ( modelCacheState s1, modelCacheState s2 )
{
	// if one of caches is definitely UNSAT, then merge will be the same
	if ( s1 == csInvalid || s2 == csInvalid )
		return csInvalid;
	// if one of caches is unsure then result will be the same
	if ( s1 == csFailed || s2 == csFailed )
		return csFailed;
	// if one of caches is not inited, than result would be the same
	if ( s1 == csUnknown || s2 == csUnknown )
		return csUnknown;
	else	// valid+valid = valid
		return csValid;
}

/// interface for general model caching.
class modelCacheInterface
{
public:		// types
	enum modelCacheType
	{
		mctBadType,		// not implemented
		mctSingleton,	// contains just 1 concept
		mctIan,			// root-level concepts, ER and AR concepts are cached
	};

protected:	// members
		/// flag to show that model contains nominals
	bool hasNominalNode;

public:		// interface
		/// Create cache model with given precense of nominals
	modelCacheInterface ( bool flagNominals ) : hasNominalNode(flagNominals) {}
		/// empty d'tor
	virtual ~modelCacheInterface ( void ) {}

		/// check whether both models have nominals; in this case, merge is impossible
	bool hasNominalClash ( const modelCacheInterface* p ) const
		{ return hasNominalNode && p->hasNominalNode; }
		/// update knoweledge about nominals in the model after merging
	void updateNominalStatus ( const modelCacheInterface* p ) { hasNominalNode |= p->hasNominalNode; }

	// mergable part

		/// Check the model cache internal state.
	virtual modelCacheState getState ( void ) const = 0;
		/// check whether two caches can be merged; @return state of "merged" model
	virtual modelCacheState canMerge ( const modelCacheInterface* p ) const = 0;
		/// Merge given model to current one; return state of the merged model
	virtual modelCacheState merge ( const modelCacheInterface* p ) = 0;

		/// Get the tag identifying the cache type
	virtual modelCacheType getCacheType ( void ) const { return mctBadType; }
		/// get type of cache (deep or shallow)
	virtual bool shallowCache ( void ) const { return true; }
		/// log this cache entry (with given level)
	virtual void logCacheEntry ( unsigned int level ATTR_UNUSED ) const {}
}; // modelCacheInterface

#endif
