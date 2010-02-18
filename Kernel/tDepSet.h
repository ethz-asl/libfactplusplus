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

/**
 *  dep-set implementation based on lists that shared tails
 */
class TDepSet
{
protected:	// classes
		/// element for the dep-set (to be defined)
	class TDepSetElement;
		/// typedef for the pointer
	typedef TDepSetElement* DSPointer;

		/// implementation of DSE
	class TDepSetElement
	{
	protected:	// members
			/// current dependency level
		unsigned int Level;
			/// pointer to the rest of the dep-set
		DSPointer Tail;

	public:		// interface
			/// stub c'tor for growingArrayP()
		TDepSetElement ( void ) { fpp_unreachable(); }
			/// init c'tor
		explicit TDepSetElement ( unsigned int level, DSPointer tail )
			: Level(level)
			, Tail(tail)
		{
#		ifdef TMP_DEPSET_DEBUG
			std::cout << "Created DSE "; Print(std::cout); std::cout << std::endl;
#		endif
		}
			/// d'tor
		~TDepSetElement ( void )
		{
#		ifdef TMP_DEPSET_DEBUG
			std::cout << "Deleted DSE "; Print(std::cout); std::cout << std::endl;
#		endif
		}

			/// get level of DSE
		unsigned int level ( void ) const { return Level; }
			/// get pointer to the Tail DSE
		DSPointer tail ( void ) const { return Tail; }
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
	class TDepSetCache
	{
	protected:	// types
			/// auxiliary map
		typedef std::map<DSPointer,DSPointer> CacheMap;
			/// map RW iterator
		typedef CacheMap::iterator iterator;
			/// map RO iterator
		typedef CacheMap::const_iterator const_iterator;

	protected:	// members
			/// map dep-set tail into depset head.tail
		CacheMap Map;
			/// element head.NULL
		DSPointer HeadDepSet;
			/// head element of the given cache
		unsigned int Level;

	public:		// interface
			/// c'tor: create head dep-set
		TDepSetCache ( unsigned int level = 0 )
			: HeadDepSet(new TDepSetElement ( level, NULL ))
			, Level(level)
			{}
			/// d'tor: delete all the cached dep-sets and the head element
		~TDepSetCache ( void )
		{
			// don't delete tails as they are referenced outside
			for ( iterator p = Map.begin(), p_end = Map.end(); p != p_end; ++p )
				delete p->second;
			delete HeadDepSet;
		}

			/// get dep-set corresponding to a Head.Tail
		DSPointer get ( DSPointer tail = NULL )
		{
			// special case the empty tail: most common case
			if ( tail == NULL )
				return HeadDepSet;

			// try to find cached dep-set
			const_iterator p = Map.find(tail);
			if ( p != Map.end() )
				return p->second;

			// no cached entry -- create a new one and cache it
			DSPointer concat = new TDepSetElement ( Level, tail );
			Map[tail] = concat;
			return concat;
		}
	}; // TDepSetCache

		/// implementation of Manager
	class TDepSetManager: public growingArrayP<TDepSetCache>
	{
	protected:	// methods
			/// create a new entry with an improved level
		virtual TDepSetCache* createNew ( void ) { return new TDepSetCache(last++); }

	public:		// interface
			/// c'tor: init N basement elements
		TDepSetManager ( unsigned int n ) { ensureHeapSize(n); }
			/// d'tor: delete all basement elements
		virtual ~TDepSetManager ( void ) {}

			/// ensure that size of vector is enough to keep N elements
		void ensureSize ( unsigned int n ) { ensureHeapSize(n); }
			/// get concatenation of N'th level element and TAIL
		DSPointer get ( unsigned int n, DSPointer tail = NULL ) const { return Base[n]->get(tail); }
			/// merge two dep-sets into a single one
		DSPointer merge ( DSPointer d1, DSPointer d2 )
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

protected:	// static members
		/// unique basement for all dep-sets
	static TDepSetManager Manager;

protected:	// members
		/// pointer to the appropriate dep-set element
	DSPointer dep;

public:		// interface
		/// default c'tor: create empty dep-set
	TDepSet ( void ) : dep(NULL) {}
		/// main c'tor
	explicit TDepSet ( unsigned int level ) { dep = Manager.get(level); }
		/// copy c'tor
	TDepSet ( const TDepSet& d ) : dep(d.dep) {}
		/// assignment
	TDepSet& operator = ( const TDepSet& d ) { dep = d.dep; return *this; }
		/// empty d'tor
	~TDepSet ( void ) {}

	// access methods

		/// return latest branching point in the dep-set
	unsigned int level ( void ) const { return dep ? dep->level() : 0; }
	 	/// check if the dep-set is empty
	bool empty ( void ) const { return dep == NULL; }
		/// check if the dep-set contains given level
	bool contains ( unsigned int level ) const
	{
		for ( DSPointer p = dep; p; p = p->tail() )
			if ( level > p->level() )		// missed one
				return false;
			else if ( level == p->level() )	// found one
				return true;

		// not found
		return false;
	}

		/// ensure that given branching level exists
	static void ensureLevel ( unsigned int level ) { Manager.ensureSize(level); }
		/// Adds given level to current dep-set
	void add ( unsigned int level ) { dep = Manager.merge ( dep, Manager.get(level) ); }
		/// Adds given dep-set to current dep-set
	void add ( const TDepSet& toAdd ) { dep = Manager.merge ( dep, toAdd.dep ); }
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
		DSPointer p = dep;
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

#endif
