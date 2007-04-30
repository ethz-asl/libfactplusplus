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

#ifndef _COMMON_PARSER_H
#define _COMMON_PARSER_H

#include "scanner.h"

/// generic class for parsing with usage of TsScanner
class CommonParser
{
protected:	// members
		/// used scanner
	TsScanner scan;
		/// last scanned token
	Token Current;

protected:	// methods
		/// get current token
	Token Code ( void ) const { return Current; }
		/// receive (and save) next token
	void NextLex ( void ) { Current = scan. GetLex (); }
		/// ensure that current token has given value; return error if it's not a case
	void MustBe ( Token t, const char* p = NULL ) const
	{
		if ( Current != t )
			scan. error (p);
	}
		/// ensure that current token has given value; return error if it's not a case; get new token
	void MustBeM ( Token t, const char* p = NULL )
		{ MustBe ( t, p ); NextLex (); }
		/// general error message
	void parseError ( const char* p ) const { MustBe ( UNUSED, p ); }

public:		// interface
		/// c'tor
	CommonParser ( std::istream* in ) : scan ( in ) { NextLex (); }
		/// empty d'tor
	virtual ~CommonParser ( void ) {}
};	// CommonParser

#endif // _COMMON_PARSER_HPP
