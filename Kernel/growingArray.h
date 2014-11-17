/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2014 by Dmitry Tsarkov

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

#ifndef GROWINGARRAY_H
#define GROWINGARRAY_H

#include <vector>

#include "fpp_assert.h"

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
	size_t last;

public:		// interface
		/// the empty c'tor
	growingArray ( void ) : last(0) {}
		/// copy c'tor
	growingArray ( const growingArray<C>& ga ) : Body(ga.Body), last(ga.last) {}
		/// assignment
	growingArray& operator = ( const growingArray<C>& ga ) { Body=ga.Body; last=ga.last; return *this; }
		/// empty d'tor
	virtual ~growingArray ( void ) {}

		/// check whether array is empty
	bool empty ( void ) const { return ( last == 0 ); }
		/// make sure array is big enough to keep N elements (leaving 'last' unchanged)
	void reserve ( size_t n )
	{
		if ( n >= Body.size() )
			Body.resize(2*n+1);
	}
		/// set the last element to given index
	void resize ( size_t n ) { reserve(n); last = n; }
		/// clear the array
	void clear ( void ) { last = 0; }
		/// get the count of elements
	size_t size ( void ) const { return last; }

	// access to elements

		/// add the entry to the array
	void add ( const C& entry )
	{
		reserve(last);
		Body[last++] = entry;
	}
		/// random access (non-const version)
	C& operator [] ( size_t i )
	{
#	ifdef ENABLE_CHECKING
		fpp_assert ( i < last );
		fpp_assert ( last <= Body.size() );
#	endif
		return Body[i];
	}

		/// random access (const version)
	const C& operator [] ( size_t i ) const
	{
#	ifdef ENABLE_CHECKING
		fpp_assert ( i < last );
		fpp_assert ( last <= Body.size() );
#	endif
		return Body[i];
	}

	// iterators
	iterator begin ( void ) { return Body.begin(); }
	const_iterator begin ( void ) const { return Body.begin(); }
	iterator end ( void ) { return begin()+(long)last; }
	const_iterator end ( void ) const { return begin()+(long)last; }
}; // growingArray

#endif
