/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2004 by Dmitry Tsarkov

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

#ifndef _GROWINGARRAY_H
#define _GROWINGARRAY_H

#include <vector>
#include <cassert>

/// implementation of an array which groves but not shrinkes
template <class C>
class growingArray
{
public:		// types
	/// define iterator
	typedef typename std::vector<C>::iterator iterator;
	/// define const_iterator
	typedef typename std::vector<C>::const_iterator const_iterator;

protected:	// members
	/// body of an aggay
	std::vector<C> Body;
	/// index of the element after last actual element in array
	unsigned int last;

protected:	// methods
	/// increase Body size to ensure 'last' points to valid place
	void grow ( unsigned int _size )
	{
		if ( _size >= Body.size() )
			Body.resize ( 2*_size+1 );
	}

public:		// interface
		/// the empty c'tor
	growingArray ( void ) : last(0) {}
		/// copy c'tor
	growingArray ( const growingArray<C>& ga ) : Body(ga.Body), last(ga.last) {}
		/// assignment
	growingArray& operator = ( const growingArray<C>& ga ) { Body=ga.Body; last=ga.last; return *this; }
	/// d'tor
	virtual ~growingArray ( void ) {}

	/// add the entry to the array
	void add ( const C& entry )
	{
		grow(last);
		Body[last++] = entry;
	}
	/// is array empty
	bool empty ( void ) const { return ( last == 0 ); }
	/// resize an array (leaving 'last' unchanged)
	void resize ( unsigned int i ) { if ( i >= last ) grow (i); }
	/// reset last element to given index
	void reset ( unsigned int i ) { resize (i); last = i; }
	/// clear the array
	void clear ( void ) { last = 0; }
	/// get the count of elements
	unsigned int size ( void ) const { return last; }

	// access to elements

	/// random access (non-const version)
	C& operator [] ( unsigned int i )
	{
#	ifdef ENABLE_CHECKING
		assert ( i < last );
		assert ( last <= Body.size() );
#	endif
		return Body[i];
	}

	/// random access (const version)
	const C& operator [] ( unsigned int i ) const
	{
#	ifdef ENABLE_CHECKING
		assert ( i < last );
		assert ( last <= Body.size() );
#	endif
		return Body[i];
	}

	// iterators
	iterator begin ( void ) { return Body.begin(); }
	const_iterator begin ( void ) const { return Body.begin(); }
	iterator end ( void ) { return begin()+last; }
	const_iterator end ( void ) const { return begin()+last; }
}; // growingArray

#endif
