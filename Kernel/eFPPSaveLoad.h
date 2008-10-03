/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2006-2008 by Dmitry Tsarkov

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

#ifndef _EFPPSAVELOAD_H
#define _EFPPSAVELOAD_H

#include <string>

#include "eFaCTPlusPlus.h"

/// exception thrown in case name can't be registered
class EFPPSaveLoad: public EFaCTPlusPlus
{
public:		// members
		/// error string
	std::string str;

public:		// interface
		/// c'tor with a given "what" string
	EFPPSaveLoad ( const std::string& why ) throw()
		: EFaCTPlusPlus()
		, str(why)
	{
		reason = str.c_str();
	}
		/// c'tor "Char not found"
	explicit EFPPSaveLoad ( const char c ) throw()
		: EFaCTPlusPlus()
	{
		str = "Expected character '";
		str += c;
		str += "' not found";
		reason = str.c_str();
	}
		/// c'tor: create an output string for the bad filename
	EFPPSaveLoad ( const std::string& filename, bool save ) throw()
		: EFaCTPlusPlus()
	{
		const char* action = save ? "save" : "load";
		const char* prep = save ? "to" : "from";
		str = "Unable to ";
		str += action;
		str += " internal state ";
		str += prep;
		str += " file '";
		str += filename;
		str += "'";
		reason = str.c_str();
	}
		/// empty d'tor
	virtual ~EFPPSaveLoad ( void ) throw() {}
}; // EFppSaveLoad

#endif


