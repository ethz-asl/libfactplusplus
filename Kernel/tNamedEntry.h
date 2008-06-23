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

#ifndef _TNAMEDENTRY_H
#define _TNAMEDENTRY_H

#include <string>
#include <iostream>

#include "flags.h"

//#include "SmallObj.h"

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

public:		// interface
		/// the only c'tor
	explicit TNamedEntry ( const std::string& name )
		: extName (name)		// copy name
		, extId (0)				// sets local id
		{}
		/// empty d'tor
	virtual ~TNamedEntry ( void ) {}

		/// gets name of given entry
	const char* getName ( void ) const { return extName.c_str(); }

		/// set internal ID
	void setId ( int id ) { extId = id; }
		/// get internal ID
	int getId ( void ) const { return extId; }

		/// register a System flag
	FPP_ADD_FLAG(System,0x1);

	virtual void Print ( std::ostream& o ) const { o << getName (); }

	// save/load interface; implementation is in SaveLoad.cpp

		/// save entry
	virtual void Save ( std::ostream& o ) const;
		/// load entry
	virtual void Load ( std::istream& i );
}; // TNamedEntry

#endif
