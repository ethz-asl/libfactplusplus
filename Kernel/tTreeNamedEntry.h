/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2009 by Dmitry Tsarkov

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

#ifndef TTREENAMEDENTRY_H
#define TTREENAMEDENTRY_H

#include "tNamedEntry.h"

class TTreeNamedEntry
{
private:	// no copy
		/// copy c'tor (unimplemented)
	TTreeNamedEntry ( const TTreeNamedEntry& );
		/// assignment (unimplemented)
	TTreeNamedEntry& operator = ( const TTreeNamedEntry& );

protected:	// members
		/// name of the entry
	std::string name;
		/// pointer to the implementation class
	TNamedEntry* impl;

public:		// interface
		/// init c'tor
	explicit TTreeNamedEntry ( const std::string& Name )
		: name(Name)		// copy name
		, impl(NULL)
		{}
		/// transparent c'tor
	TTreeNamedEntry ( TNamedEntry* ne )
		: name(ne->getName())		// copy name
		, impl(ne)
		{}
		/// empty d'tor
	~TTreeNamedEntry ( void ) {}

		/// gets name of given entry
	const char* getName ( void ) const { return name.c_str(); }

		/// set implementation
	void setImpl ( TNamedEntry* id ) { impl = id; }
		/// get internal ID
	TNamedEntry* getImpl ( void ) const { return impl; }

	void Print ( std::ostream& o ) const { o << getName(); }
}; // TTreeNamedEntry

#endif
