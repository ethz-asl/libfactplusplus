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

#ifndef EFPPFAILEDREASONING_H
#define EFPPFAILEDREASONING_H

#include "eFaCTPlusPlus.h"

class EFPPFailedReasoning : public EFaCTPlusPlus
{
public:		// interface
		/// empty c'tor
	EFPPFailedReasoning ( void ) throw() : EFaCTPlusPlus() {}
		/// empty d'tor
	~EFPPFailedReasoning ( void ) throw() {}
		/// reason
	virtual const char* what ( void ) const throw() { return "Can't classify KB because of previous errors"; }
}; // EAxiomLoadFailureException

#endif
