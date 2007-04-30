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

#ifndef _DLVHASHIMPL_H
#define _DLVHASHIMPL_H

// implementation of DLVertex Hash; to be included after DLDag definition

inline BipolarPointer
dlVHashTable :: locate ( const HashLeaf& leaf, const DLVertex& v ) const
{
	for ( HashLeaf::const_iterator p = leaf.begin(), p_end = leaf.end(); p != p_end; ++p )
		if ( v == host[*p] )
			return *p;

	return bpINVALID;
}

inline void
dlVHashTable :: addElement ( BipolarPointer pos )
{
	insert ( Table[hash(host[pos])], pos );
}

#endif

