/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2008 by Dmitry Tsarkov

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

#ifndef _TSTTREE_HH_
#define _TSTTREE_HH_

#include <cstdlib>		// NULL

//#include "SmallObj.h"

template < class T >
class TsTTree//: public Loki::SmallObject<>
{
private:	// members
		/// element in the tree node
	T elem;
		/// pointer to left subtree
	TsTTree *left;
		/// pointer to right subtree
	TsTTree *right;

private:	// prevent copy
		/// no copy c'tor
	TsTTree ( const TsTTree& );
		/// no assignment
	TsTTree& operator = ( const TsTTree& );

public:		// interface
		/// default c'tor
	TsTTree ( const T& Init, TsTTree *l = NULL, TsTTree *r = NULL )
		: elem(Init)
		, left(l)
		, right(r)
		{}
		/// d'tor
	~TsTTree ( void ) {}

	// access to members

	T& Element ( void )	{ return elem; }
	const T& Element ( void ) const	{ return elem; }

	TsTTree* Left ( void ) const { return left; }
	TsTTree* Right ( void ) const { return right; }

	void SetLeft ( TsTTree *l ) { left = l; }
	void SetRight ( TsTTree *r ) { right = r; }

	TsTTree* clone ( void ) const
	{
		TsTTree* p = new TsTTree(Element());
		if ( left )
			p->SetLeft(left->clone());
		if ( right )
			p->SetRight(right->clone());
		return p;
	}
}; // TsTTree

/// delete the whole tree
template <class T>
void deleteTree ( TsTTree<T>* t )
{
	if ( t )
	{
		deleteTree(t->Left());
		deleteTree(t->Right());
		delete t;
	}
}

#endif
