/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2008 by Dmitry Tsarkov

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

#ifndef EAXIOMLOADFAILUREEXCEPTION_H
#define EAXIOMLOADFAILUREEXCEPTION_H

#include <string>

#include "eFaCTPlusPlus.h"

class EAxiomLoadFailureException : public EFaCTPlusPlus
{
protected:	// members
		// reason of the failure
	std::string What;

public:		// interface
		/// c'tor with a name of a failed axiom
	EAxiomLoadFailureException ( const std::string& name ) : EFaCTPlusPlus()
	{
		What = "Axiom '";
		What += name;
		What += "' failed to load";
	}
		/// empty d'tor
	~EAxiomLoadFailureException ( void ) throw() {}
		/// reason
	virtual const char* what ( void ) const throw() { return What.c_str(); }
}; // EAxiomLoadFailureException

#endif