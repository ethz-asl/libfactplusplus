/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2006 by Dmitry Tsarkov

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

#ifndef _TRELATED_H
#define _TRELATED_H

#include "tIndividual.h"
#include "tRole.h"

/// class for represent individual relation <a,b>:R
class TRelated
{
public:		// members
	TIndividual* a;
	TIndividual* b;
	TRole* R;

public:		// interface
		/// empty c'tor
	TRelated ( void ) : a(NULL), b(NULL), R(NULL) {}
		/// init c'tor
	TRelated ( TIndividual* a_, TIndividual* b_, TRole* R_ ) : a(a_), b(b_), R(R_) {}
		/// copy c'tor
	TRelated ( const TRelated& c ) : a(c.a), b(c.b), R(c.R) {}
		/// assignment
	TRelated& operator = ( const TRelated& c )
	{
		a = c.a;
		b = c.b;
		R = c.R;
		return *this;
	}
		/// empty d'tor
	~TRelated ( void ) {}

		/// simplify structure wrt synonyms
	void simplify ( void )
	{
		R = resolveSynonym(R);
		a = resolveSynonym(a);
		b = resolveSynonym(b);
		a->addRelated(this);
	}
		/// get access to role wrt the FROM direction
	TRole* getRole ( void ) const { return R; }
}; // TRelated

// TIndividual RELATED-dependent method' implementation
inline void
TIndividual :: updateToldFromRelated ( void )
{
	updateTold ( RelatedIndex.begin(), RelatedIndex.end() );
}

#endif
