/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2011 by Dmitry Tsarkov

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

#ifndef TSIGNATURE_H
#define TSIGNATURE_H

#include <set>

#include "tDLExpression.h"

/// class to hold the signature of a module
class TSignature
{
protected:	// members
		/// set to keep all the elements in signature
	std::set<const TNamedEntity*> Set;
		/// true if TOP-locality; false if BOTTOM-locality
	bool topLocality;

public:		// interface
		/// empty c'tor
	TSignature ( void ) {}
		/// copy c'tor
	TSignature ( const TSignature& copy ) : Set(copy.Set) {}
		/// assignment
	TSignature& operator= ( const TSignature& copy )
	{
		Set = copy.Set;
		return *this;
	}
		/// empty d'tor
	~TSignature ( void ) {}

	// add names to signature

		/// add pointer to named object to signature
	void add ( const TNamedEntity* p ) { Set.insert(p); }
		/// set new locality polarity
	void setLocality ( bool top ) { topLocality = top; }

	// comparison

		/// check whether 2 signatures are the same
	bool operator == ( const TSignature& sig ) const { return Set == sig.Set; }
		/// check whether 2 signatures are different
	bool operator != ( const TSignature& sig ) const { return Set != sig.Set; }
		/// @return true iff signature contains given element
	bool contains ( const TNamedEntity* p ) const { return Set.count(p) > 0; }
		/// @return true iff signature contains given element
	bool contains ( const TDLExpression* p ) const
	{
		const TNamedEntity* e = dynamic_cast<const TNamedEntity*>(p);
		if ( e != NULL )
			return contains(e);
		const TDLObjectRoleInverse* inv = dynamic_cast<const TDLObjectRoleInverse*>(p);
		if ( inv != NULL )
			return contains(inv->getOR());

		return false;
	}
		/// @return size of the signature
	size_t size ( void ) const { return Set.size(); }

		/// @return true iff concepts are treated as TOPs
	bool topCLocal ( void ) const { return topLocality; }
		/// @return true iff roles are treated as TOPs
	bool topRLocal ( void ) const { return topLocality; }
}; // TSignature

#endif
