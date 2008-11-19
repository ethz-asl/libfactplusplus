/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2004 by Dmitry Tsarkov

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

#ifndef _DIGNAMESMANAGER_H
#define _DIGNAMESMANAGER_H

#include "DIGNamesManagerInterface.h"

/// MAP-based implementation of DIG v1.x
class NamesManager_DIG_1x : public DIGNamesManagerInterface
{
public:	// interface
		/// default c'tor
	NamesManager_DIG_1x ( void );	// a lot of addName's here
		/// virtual d'tor
	virtual ~NamesManager_DIG_1x ( void ) {}
}; // DIGNames_v1x

/// MAP-based implementation of DIG v2.x
class NamesManager_DIG_2x : public DIGNamesManagerInterface
{
public:	// interface
		/// default c'tor
	NamesManager_DIG_2x ( void );	// a lot of addName's here
		/// virtual d'tor
	virtual ~NamesManager_DIG_2x ( void ) {}
}; // DIGNames_v1x

#endif
