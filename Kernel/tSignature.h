/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2011-2013 by Dmitry Tsarkov

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
#include <algorithm>
#include <iterator>

#include "tDLExpression.h"

/// class to hold the signature of a module
class TSignature
{
public:		// types
		/// set of entities as a base underlying type os a signature
	typedef std::set<const TNamedEntity*> BaseType;
		/// RO iterator over a set of entities
	typedef BaseType::const_iterator iterator;

protected:	// members
		/// set to keep all the elements in signature
	BaseType Set;
		/// true if concept TOP-locality; false if concept BOTTOM-locality
	bool topCLocality;
		/// true if role TOP-locality; false if role BOTTOM-locality
	bool topRLocality;

public:		// interface
		/// empty c'tor
	TSignature ( void ) : topCLocality(false), topRLocality(false) {}
		/// copy c'tor
	TSignature ( const TSignature& copy ) : Set(copy.Set), topCLocality(copy.topCLocality), topRLocality(copy.topRLocality) {}
		/// assignment
	TSignature& operator= ( const TSignature& copy )
	{
		Set = copy.Set;
		topCLocality = copy.topCLocality;
		topRLocality = copy.topRLocality;
		return *this;
	}
		/// empty d'tor
	~TSignature ( void ) {}

	// add names to signature

		/// add pointer to named object to signature
	void add ( const TNamedEntity* p ) { Set.insert(p); }
		/// add set of named entities to signature
	void add ( const BaseType& aSet ) { Set.insert ( aSet.begin(), aSet.end() ); }
		/// add another signature to a given one
	void add ( const TSignature& Sig ) { add(Sig.Set); }
		/// remove given element from a signature
	void remove ( const TNamedEntity* p ) { Set.erase(p); }
		/// set new locality polarity
	void setLocality ( bool topC, bool topR ) { topCLocality = topC; topRLocality = topR; }
		/// set new locality polarity
	void setLocality ( bool top ) { setLocality ( top, top ); }

	// comparison

		/// check whether 2 signatures are the same
	bool operator == ( const TSignature& sig ) const { return Set == sig.Set; }
		/// check whether 2 signatures are different
	bool operator != ( const TSignature& sig ) const { return Set != sig.Set; }
		/// operator <
	bool operator < ( const TSignature& sig ) const { return Set < sig.Set; }
		/// @return true iff signature contains given element
	bool contains ( const TNamedEntity* p ) const { return Set.count(p) > 0; }
		/// @return true iff signature contains given element
	bool contains ( const TDLExpression* p ) const
	{
		if ( const TNamedEntity* e = dynamic_cast<const TNamedEntity*>(p) )
			return contains(e);
		if ( const TDLObjectRoleInverse* inv = dynamic_cast<const TDLObjectRoleInverse*>(p) )
			return contains(inv->getOR());

		return false;
	}
		/// @return size of the signature
	size_t size ( void ) const { return Set.size(); }
		/// clear the signature
	void clear ( void ) { Set.clear(); }

		/// RO access to the elements of signature
	iterator begin ( void ) const { return Set.begin(); }
		/// RO access to the elements of signature
	iterator end ( void ) const { return Set.end(); }

		/// @return true iff concepts are treated as TOPs
	bool topCLocal ( void ) const { return topCLocality; }
		/// @return true iff concepts are treated as BOTTOMs
	bool botCLocal ( void ) const { return !topCLocality; }
		/// @return true iff roles are treated as TOPs
	bool topRLocal ( void ) const { return topRLocality; }
		/// @return true iff roles are treated as BOTTOMs
	bool botRLocal ( void ) const { return !topRLocality; }
}; // TSignature

inline TSignature::BaseType
intersect ( const TSignature& s1, const TSignature& s2 )
{
	TSignature::BaseType ret;
	set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(), inserter(ret, ret.begin()));
	return ret;
}

#endif
