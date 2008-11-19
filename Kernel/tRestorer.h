/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2005 by Dmitry Tsarkov

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

#ifndef _TRESTORER_H
#define _TRESTORER_H

/**
 *	Generic class for restore some property.
 *	Usually inherited class has a pointer to object to be restored and restore info
 */
class TRestorer
{
public:	// interface
		/// empty c'tor
	TRestorer ( void ) {}
		/// empty d'tor
	virtual ~TRestorer ( void ) {}
		/// restore an object based on saved information
	virtual void restore ( void ) = 0;
}; // TRestorer

#endif
