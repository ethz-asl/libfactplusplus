/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2007 by Dmitry Tsarkov

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

#ifndef _EFPPCYCLEINRIA_H
#define _EFPPCYCLEINRIA_H

#include "eFaCTPlusPlus.h"

/// exception thrown in case there is a (disallowed) cycle in a role-inclusion axiom
class EFPPCycleInRIA: public EFaCTPlusPlus
{
public:		// members
		/// saved name of the role
	const std::string roleName;
		/// error string
	std::string str;

public:		// interface
		/// c'tor: create an output string
	EFPPCycleInRIA ( const std::string& name ) throw()
		: EFaCTPlusPlus()
		, roleName(name)
	{
		str = "Role '";
		str += name;
		str += "' appears in a cyclical role inclusion axioms";
	}
		/// empty d'tor
	virtual ~EFPPCycleInRIA ( void ) throw() {}

		/// reason
	virtual const char* what ( void ) const throw() { return str.c_str(); }
		/// access to the role
	const char* getRoleName ( void ) const { return roleName.c_str(); }
}; // EFPPCycleInRIA

#endif


