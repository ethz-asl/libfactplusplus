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

#ifndef _DIGFILEINTERFACE_H
#define _DIGFILEINTERFACE_H

#include "DIGinterface.h"
#include "strx.h"

class DIGFileInterface : public DIGInterface
{
protected:	// methods
		/// process a file given by name, transform all XML exceptions into DIG ones
	void processFile ( const char* filename ) throw(DIGParserException)
	{
		try
		{
			xmlParser->parse(filename);
		}
		catch (const XMLException& e)
		{
			throw ( DIGParserException ( 102, "Malformed Request", StrX(e.getMessage()).localForm() ) );
		}
	}

public:		// interface
		/// empty c'tor
	DIGFileInterface ( void ) : DIGInterface() {}
		/// empty d'tor
	virtual ~DIGFileInterface ( void ) {}

		/// process a file given by a FILENAME, write a responce to RES
	void processFile ( const char* filename, std::string& res );
}; // DIGFileInterface

#endif
