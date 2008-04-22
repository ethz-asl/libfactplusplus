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
	}
		/// empty d'tor
	virtual ~EFPPSaveLoad ( void ) throw() {}

		/// reason
	virtual const char* what ( void ) const throw() { return str.c_str(); }
}; // EFppSaveLoad

#endif


