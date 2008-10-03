/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2007-2008 by Dmitry Tsarkov

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

#ifndef _EFACTPLUSPLUS_H
#define _EFACTPLUSPLUS_H

#include <exception>

/// general FaCT++ exception
class EFaCTPlusPlus: public std::exception
{
protected:
		/// reason of the exception
	const char* reason;

public:
		/// empty c'tor
	EFaCTPlusPlus ( void ) throw()
		: exception()
		, reason("FaCT++.Kernel: General exception")
		{}
		/// init c'tor
	EFaCTPlusPlus ( const char* str ) throw()
		: exception()
		, reason(str)
		{}
		/// empty d'tor
	virtual ~EFaCTPlusPlus ( void ) throw() {}

		/// reason
	virtual const char* what ( void ) const throw() { return reason; }
}; // EFaCTPlusPlus

#endif
