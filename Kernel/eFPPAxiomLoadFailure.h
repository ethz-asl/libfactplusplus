/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2008 by Dmitry Tsarkov

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

#ifndef EFPPAXIOMLOADFAILURE_H
#define EFPPAXIOMLOADFAILURE_H

#include <string>

#include "eFaCTPlusPlus.h"

class EFPPAxiomLoadFailure : public EFaCTPlusPlus
{
protected:	// members
		// reason of the failure
	std::string What;

public:		// interface
		/// c'tor with a name of a failed axiom
	EFPPAxiomLoadFailure ( const std::string& name ) : EFaCTPlusPlus()
	{
		What = "Axiom '";
		What += name;
		What += "' failed to load";
	}
		/// empty d'tor
	~EFPPAxiomLoadFailure ( void ) throw() {}
		/// reason
	virtual const char* what ( void ) const throw() { return What.c_str(); }
}; // EFPPAxiomLoadFailure

#endif
