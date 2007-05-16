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

#ifndef _DEPSET_BDD_H
#define _DEPSET_BDD_H

#include <cassert>

#include "bdd.h"

/**
 *  dep-set implementation based on BDDs. Maximal dep-level is stored together with a set
 */
class depSet_bdd
{
protected:	// classes
	class BddManager
	{
	private:	// constants
		static const unsigned int iVars;
		static const unsigned int iNodes;
		static const unsigned int iCacheSize;
	protected:	// members
			/// current number of vars
		unsigned int nvars;

	public:		// interface
			/// Default c'tor; inits BDD
		BddManager ( void ) : nvars(iVars)
		{
			bdd_init ( iNodes, iCacheSize );
			bdd_setvarnum (nvars);
			bdd_gbc_hook(NULL);		// don't print information about garbage collection
			bdd_error_hook(NULL);
		}
			/// d'tor
		~BddManager ( void ) { bdd_done(); }

			/// check if requested level is higher then number of vars
		void checkLevel ( unsigned int level )
		{
			if ( level >= nvars )
			{
				bdd_extvarnum(nvars);
				nvars *= 2;
			}
		}
	}; // BddManager

protected:	// members
		/// the only manager which works with all dependences
	static BddManager Manager;
		/// dependency
	bdd dep;
		/// max level of dep
	unsigned int lev;

protected:	// methods
		/// set up maximum dep-level for the cache entry
	void updateLevel ( unsigned int l ) { if ( l>lev ) lev = l; }
		/// internal safety check
	void invariant ( void ) const
	{
#	ifdef ENABLE_CHECKING
		assert ( (lev==0) == (dep==bddtrue) );
#	endif
	}

public:		// interface
		/// Empty c'tor
	depSet_bdd ( void ) : dep(bddtrue), lev(0) {}
		/// C'tor taking current dep-level
	explicit depSet_bdd ( unsigned int level ) : dep(bdd_ithvar(level)), lev(level) {}
		/// Copy C'tor
	depSet_bdd ( const depSet_bdd& d ) : dep(d.dep), lev(d.lev) {}
		/// Assignment
	depSet_bdd& operator = ( const depSet_bdd& d ) { dep = d.dep; lev = d.lev; return *this; }
		/// D'tor
	~depSet_bdd (void) {}

	// access methods

		/// Return maximal dep-value in set
	unsigned int level ( void ) const { return lev; }
	 	/// Check if dep-set is empty
	bool empty ( void ) const { invariant(); return (lev == 0); }
		/// Check if dep-set contains given level
	bool contains ( unsigned int level ) const
	{
		invariant();
		if ( level > lev )
			return false;
		else if ( level == lev )
			return true;
		else
			return (( dep & bdd_nithvar(level)) == bddfalse );
	}

		/// ensure that given branching level exists
	static void ensureLevel ( unsigned int level ) { Manager.checkLevel(level); }
		/// Adds given level to current dep-set
	void add ( unsigned int level )
	{
		invariant();
		dep &= bdd_ithvar(level);
		updateLevel(level);
		invariant();
	}
		/// Adds given dep-set to current dep.set
	void add ( const depSet_bdd& add )
	{
		invariant();
		dep &= add.dep;
		updateLevel(add.lev);
		invariant();
	}
	depSet_bdd& operator += ( const depSet_bdd& toadd )
	{
		add(toadd);
		return *this;
	}
		/// Restricts current dep-set with given level
	void restrict ( unsigned int level )
	{
		assert(level);	// should not restrict to 0 (=empty dep-set)
		invariant();
		if ( level > lev )
			return;
//		assert ( level == lev );	// this is not true if OR's priority is lowest

		// create an aux bdd
		bdd aux = dep;
		// clean current depset
		clear();

		while ( aux != bddtrue )
		{
			// remove next var from the set
			int var = bdd_var(aux);
#		ifdef ENABLE_CHECKING
			assert ( var >= 0 );
#		endif
			aux = bdd_exist ( aux, bdd_ithvar(var) );
			if ( level > (unsigned)var )
				add(var);	// keep this var in existing BDD
		}
		invariant();
	}
		/// Remove all information from dep-set
	void clear ( void ) { dep = bddtrue; lev = 0; }

		/// Print given dep-set to a standart stream
	template <class O> void Print ( O& o ) const
	{
		invariant();
		if ( dep == bddtrue )
			return;
		o << "{";
		bdd res = dep;
		int var = bdd_var(res);
		o << var;
		res = bdd_exist ( res, bdd_ithvar(var) );
		while (res != bddtrue)
		{
			var = bdd_var(res);
			o << "," << var;
			res = bdd_exist ( res, bdd_ithvar(var) );
		}
		o << "}";
	}

	friend depSet_bdd operator + ( const depSet_bdd& ds1, const depSet_bdd& ds2 );
}; // depSet_bdd

template <class O>
inline O& operator << ( O& o, const depSet_bdd& s )
{ s.Print(o); return o; }

inline depSet_bdd operator + ( const depSet_bdd& ds1, const depSet_bdd& ds2 )
{
	depSet_bdd ret(ds1);
	return ret += ds2;
}

#endif
