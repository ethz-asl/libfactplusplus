/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2006 by Dmitry Tsarkov

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

#ifndef _TKBFLAGS_H
#define _TKBFLAGS_H

#include "flags.h"

/// flags that reflects KB structure: GCIs, etc
class TKBFlags: public Flags
{
public:		// interface
		/// empty c'tor
	TKBFlags ( void ) : Flags() {}
		/// copy c'tor
	TKBFlags ( const TKBFlags& flags ) : Flags(flags) {}
		/// empty d'tor
	~TKBFlags ( void ) {}

		/// register flag for GCIs
	FPP_ADD_FLAG(GCI,0x1);
		/// register flag for Range and Domain axioms
	FPP_ADD_FLAG(RnD,0x2);
		/// register flag for Reflexive roles
	FPP_ADD_FLAG(Reflexive,0x4);
}; // TKBFlags

#endif
