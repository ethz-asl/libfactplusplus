/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2006 by Dmitry Tsarkov

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

#ifndef _TDEPSET_H
#define _TDEPSET_H

#include <vector>
#include "SmartPtr.h"
#include "growingArrayP.h"

/**
 *  dep-set implementation based on ...
 *	Maximal dep-level is stored together with a set
 */
class TDepSet
{
protected:	// classes
		/// element for the dep-set (to be defined)
	class TDepSetElement;
		/// typedef for the pointer
	typedef Loki::SmartPtr<TDepSetElement> DSPointer;

		/// implementation of DSE
	class TDepSetElement
	{
	protected:	// members
			/// current dependency level
		unsigned int Level;
			/// pointer to the element of the next level
		DSPointer Next;

	public:		// interface
			/// stub c'tor for growingArrayP()
		TDepSetElement ( void ) { assert(0); }
			/// init c'tor
		explicit TDepSetElement ( unsigned int level, DSPointer next )
			: Level(level)
			, Next(next)
			{}
			/// d'tor
		~TDepSetElement ( void ) {}

			/// get level of DSE
		unsigned int level ( void ) const { return Level; }
			/// get pointer to the next DSE
		DSPointer next ( void ) const { return Next; }
			/// Print given dep-set to a standart stream
		template <class O>
		void Print ( O& o ) const
		{
			if ( Next )	// print the rest of dep-set, then current element
			{
				Next->Print(o);
				o << ',' << level();
			}
			else	// leaf element (from basement)
				o << level();
		}
	}; // TDepSetElement

		/// implementation of Basement
	class TDepSetBasement: public growingArrayP<TDepSetElement>
	{
	protected:	// methods
			/// tunable method for creating new object
		virtual TDepSetElement* createNew ( void ) { return new TDepSetElement ( last++, NULL ); }

	public:		// interface
			/// c'tor: init N basement elements
		TDepSetBasement ( unsigned int n ) : growingArrayP<TDepSetElement>(n) {}
			/// d'tor: delete all basement elements
		~TDepSetBasement ( void ) {}

			/// ensure that size of vector is enough to keep N elements
		void ensureSize ( unsigned int n )
		{
			if ( n >= Base.size() )
				ensureHeapSize(n);
		}
			/// get N'th basement element
		DSPointer get ( unsigned int n ) const { return Base[n]; }
	}; // TDepSetBasement

protected:	// static members
		/// unique basement for all dep-sets
	static TDepSetBasement Basement;

protected:	// members
		/// pointer to the appropriate dep-set element
	DSPointer dep;

protected:	// methods
		/// merge two dep-sets into a single one
	DSPointer merge ( DSPointer d1, DSPointer d2 )
	{
		if ( d1 == NULL )
			return d2;
		if ( d2 == NULL )
			return d1;
		if ( d1->level() > d2->level() )
			return new TDepSetElement ( d1->level(), merge(d1->next(),d2) );
		if ( d1->level() < d2->level() )
			return new TDepSetElement ( d2->level(), merge(d1,d2->next()) );
		// here d1.level == d2.level
		return new TDepSetElement ( d1->level(), merge(d1->next(),d2->next()) );
	}

public:		// interface
		/// default c'tor: create empty dep-set
	TDepSet ( void ) : dep(NULL) {}
		/// main c'tor
	explicit TDepSet ( unsigned int level ) : dep(Basement.get(level)) {}
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
		for ( DSPointer p = dep; p; p = p->next() )
			if ( level > p->level() )		// missed one
				return false;
			else if ( level == p->level() )	// found one
				return true;

		// not found
		return false;
	}

		/// ensure that given branching level exists
	static void ensureLevel ( unsigned int level ) { Basement.ensureSize(level); }
		/// Adds given level to current dep-set
	void add ( unsigned int level ) { dep = merge ( dep, Basement.get(level) ); }
		/// Adds given dep-set to current dep-set
	void add ( const TDepSet& add ) { dep = merge ( dep, add.dep ); }
		/// Adds given dep-set to current dep-set
	TDepSet& operator += ( const TDepSet& toadd ) { add(toadd); return *this; }
		/// remove parts of the current dep-set that larger than given level
	void restrict ( unsigned int level )
	{
		if ( empty() )
			return;
		if ( dep->level() == level )
		{
			dep = dep->next();
			return;
		}
		DSPointer p = dep;
		// find part of the dep-set with level <= given
		while ( p && level <= p->level() )
			p = p->next();

		// check for empty dep-set
		if ( p )
			dep = p;
		else
			clear();
	}
		/// Remove all information from dep-set
	void clear ( void ) { dep = NULL; }

		/// Print given dep-set to a standart stream
	template <class O>
	void Print ( O& o ) const
	{
		if ( empty() )
			return;
		o << "{";
		dep->Print(o);
		o << "}";
	}

	friend TDepSet operator + ( const TDepSet& ds1, const TDepSet& ds2 );
}; // TDepSet_bdd

template <class O>
inline O& operator << ( O& o, const TDepSet& s )
{ s.Print(o); return o; }

inline TDepSet operator + ( const TDepSet& ds1, const TDepSet& ds2 )
{
	TDepSet ret(ds1);
	return ret += ds2;
}

#endif
