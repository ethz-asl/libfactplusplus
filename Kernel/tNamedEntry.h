/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2011 by Dmitry Tsarkov

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

#ifndef TNAMEDENTRY_H
#define TNAMEDENTRY_H

#include <string>
#include <iostream>

#include "flags.h"

//#include "SmallObj.h"

class TNamedEntity;

class TNamedEntry: public Flags//: public Loki::SmallObject<>
{
private:	// no copy
		/// copy c'tor (unimplemented)
	TNamedEntry ( const TNamedEntry& );
		/// assignment (unimplemented)
	TNamedEntry& operator = ( const TNamedEntry& );

protected:	// members
		/// name of the entry
	std::string extName;
		/// entry identifier
	int extId;
		/// original entity
	const TNamedEntity* entity;

public:		// interface
		/// the only c'tor
	explicit TNamedEntry ( const std::string& name )
		: extName (name)		// copy name
		, extId (0)				// sets local id
		, entity(NULL)
		{}
		/// empty d'tor
	virtual ~TNamedEntry ( void ) {}

		/// gets name of given entry
	const char* getName ( void ) const { return extName.c_str(); }

		/// set internal ID
	void setId ( int id ) { extId = id; }
		/// get internal ID
	int getId ( void ) const { return extId; }

		/// set entity
	void setEntity ( const TNamedEntity* e ) { entity = e; }
		/// get entity
	const TNamedEntity* getEntity ( void ) const { return entity; }

		/// register a System flag
	FPP_ADD_FLAG(System,0x1);

	// hierarchy interface

		/// register a Top-of-the-hierarchy flag
	FPP_ADD_FLAG(Top,0x1000);
		/// register a Bottom-of-the-hierarchy flag
	FPP_ADD_FLAG(Bottom,0x2000);

	virtual void Print ( std::ostream& o ) const { o << getName (); }

	// save/load interface; implementation is in SaveLoad.cpp

		/// save entry
	virtual void Save ( std::ostream& o ) const;
		/// load entry
	virtual void Load ( std::istream& i );
}; // TNamedEntry

#endif
