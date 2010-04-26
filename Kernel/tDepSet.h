/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2006-2010 by Dmitry Tsarkov

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

#ifndef TDEPSET_H
#define TDEPSET_H

#include <map>
#include <iostream>

#include "fpp_assert.h"
#include "growingArrayP.h"
#include "tHeadTailCache.h"

/**
 *  dep-set implementation based on lists that shared tails
 */
class TDepSetManager;

/// implementation of DSE
class TDepSetElement
{
protected:	// members
		/// reference to the containing manager
	TDepSetManager* Manager;
		/// current dependency level
	unsigned int Level;
		/// pointer to the rest of the dep-set
	TDepSetElement* Tail;

public:		// interface
		/// init c'tor
	explicit TDepSetElement ( TDepSetManager* manager, unsigned int level, TDepSetElement* tail )
		: Manager(manager)
		, Level(level)
		, Tail(tail)
	{
#	ifdef TMP_DEPSET_DEBUG
		std::cout << "Created DSE "; Print(std::cout); std::cout << std::endl;
#	endif
	}
		/// d'tor
	~TDepSetElement ( void )
	{
#	ifdef TMP_DEPSET_DEBUG
		std::cout << "Deleted DSE "; Print(std::cout); std::cout << std::endl;
#	endif
	}

		/// get level of DSE
	unsigned int level ( void ) const { return Level; }
		/// get pointer to the Tail DSE
	TDepSetElement* tail ( void ) const { return Tail; }
		/// merge this element with ELEM; use Manager for this
	TDepSetElement* merge ( TDepSetElement* elem );
		/// Print given dep-set to a standart stream
	template <class O>
	void Print ( O& o ) const
	{
		if ( Tail )	// print the rest of dep-set, then current element
		{
			Tail->Print(o);
			o << ',' << level();
		}
		else	// leaf element (from basement)
			o << level();
	}
}; // TDepSetElement

/// class for the cache element. Contains all the pointers to the dep-sets
/// started from the same element
class TDepSetCache: public THeadTailCache<TDepSetElement,TDepSetElement>
{
protected:	// members
		/// reference to the containing manager
	TDepSetManager* Manager;
		/// element head.NULL
	TDepSetElement* HeadDepSet;
		/// head element of the given cache
	unsigned int Level;

protected:	// methods
		/// the way to create an object by a given tail
	virtual TDepSetElement* build ( TDepSetElement* tail ) { return new TDepSetElement ( Manager, Level, tail ); }

public:		// interface
		/// stub c'tor for growingArrayP()
	TDepSetCache ( void ) { fpp_unreachable(); }
		/// c'tor: create head dep-set
	explicit TDepSetCache ( TDepSetManager* manager, unsigned int level )
		: Manager(manager)
		, Level(level)
	{
		HeadDepSet = new TDepSetElement ( Manager, Level, NULL );
	}
		/// d'tor: delete all the cached dep-sets and the head element
	~TDepSetCache ( void )
	{
		delete HeadDepSet;
	}

		/// get dep-set corresponding to a Head.Tail; treat NULL tail separately
	TDepSetElement* getDS ( TDepSetElement* tail )
	{
		// special case the empty tail: most common case
		if ( tail == NULL )
			return HeadDepSet;
		return get(tail);
	}
}; // TDepSetCache

/// implementation of Manager
class TDepSetManager: public growingArrayP<TDepSetCache>
{
protected:	// methods
		/// create a new entry with an improved level
	virtual TDepSetCache* createNew ( void ) { return new TDepSetCache ( this, last++ ); }

public:		// interface
		/// c'tor: init N basement elements
	TDepSetManager ( unsigned int n ) : growingArrayP<TDepSetCache>(0) { ensureHeapSize(n); }
		/// d'tor: delete all basement elements
	virtual ~TDepSetManager ( void ) {}

		/// ensure that size of vector is enough to keep N elements
	void ensureLevel ( unsigned int n ) { ensureHeapSize(n); }
		/// get concatenation of N'th level element and TAIL
	TDepSetElement* get ( unsigned int n, TDepSetElement* tail = NULL ) const { return Base[n]->getDS(tail); }
		/// merge two dep-sets into a single one
	TDepSetElement* merge ( TDepSetElement* d1, TDepSetElement* d2 )
	{
		// if any dep-set is NULL -- return another one
		if ( d1 == NULL )
			return d2;
		if ( d2 == NULL )
			return d1;
		if ( d1 == d2 )
			return d1;
		// take the largest level, and add to it the result of merging tails
		if ( d1->level() > d2->level() )
			return get ( d1->level(), merge(d1->tail(),d2) );
		if ( d1->level() < d2->level() )
			return get ( d2->level(), merge(d1,d2->tail()) );
		// here d1.level == d2.level
		return get ( d1->level(), merge(d1->tail(),d2->tail()) );
	}
}; // TDepSetManager

class TDepSet
{
protected:	// members
		/// pointer to the appropriate dep-set element
	TDepSetElement* dep;

public:		// interface
		/// default c'tor: create empty dep-set
	TDepSet ( void ) : dep(NULL) {}
		/// main c'tor
	explicit TDepSet ( TDepSetElement* depp ) { dep = depp; }
		/// copy c'tor
	TDepSet ( const TDepSet& d ) : dep(d.dep) {}
		/// assignment
	TDepSet& operator = ( const TDepSet& d ) { dep = d.dep; return *this; }
		/// empty d'tor: no need to delete element as it is registered in manager
	~TDepSet ( void ) {}

	// access methods

		/// return latest branching point in the dep-set
	unsigned int level ( void ) const { return dep ? dep->level() : 0; }
	 	/// check if the dep-set is empty
	bool empty ( void ) const { return dep == NULL; }
		/// check if the dep-set contains given level
	bool contains ( unsigned int level ) const
	{
		for ( TDepSetElement* p = dep; p; p = p->tail() )
			if ( level > p->level() )		// missed one
				return false;
			else if ( level == p->level() )	// found one
				return true;

		// not found
		return false;
	}

		/// Adds given dep-set to current dep-set
	void add ( const TDepSet& toAdd ) { dep = dep ? dep->merge(toAdd.dep) : toAdd.dep; }
		/// Adds given dep-set to current dep-set
	TDepSet& operator += ( const TDepSet& toAdd ) { add(toAdd); return *this; }
		/// Remove all information from dep-set
	void clear ( void ) { dep = NULL; }
		/// remove parts of the current dep-set that larger than given level
	void restrict ( unsigned int level )
	{
		if ( empty() )
			return;
		if ( dep->level() == level )
		{
			dep = dep->tail();
			return;
		}
		TDepSetElement* p = dep;
		// find part of the dep-set with level <= given
		while ( p && level <= p->level() )
			p = p->tail();

		// check for empty dep-set
		if ( p )
			dep = p;
		else
			clear();
	}

		/// Print given dep-set to a standart stream
	template <class O>
	void Print ( O& o ) const
	{
		if ( !empty() )
		{
			o << "{";
			dep->Print(o);
			o << "}";
		}
	}
}; // TDepSet

/// merge this element with ELEM; use Manager for this
inline TDepSetElement*
TDepSetElement :: merge ( TDepSetElement* elem )
{
#ifdef ENABLE_CHECKING
	if ( elem == NULL )
		return this;
	fpp_assert ( Manager == elem->Manager );
#endif
	return Manager->merge ( this, elem );
}

#endif
