/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2005 by Dmitry Tsarkov

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

#ifndef _DELETELESSALLOCATOR_H
#define _DELETELESSALLOCATOR_H

#include "growingArrayP.h"

/**
 * Class for the allocator that does not allowed 'delete'. Instead
 * it allows user to reuse all allocated memory.
 */
template<class T>
class DeletelessAllocator: public growingArrayP<T>
{
public:
		/// c'tor: do nothing
	DeletelessAllocator ( void ) {}
		/// d'tor: do nothing (all done in ~gaP)
	virtual ~DeletelessAllocator ( void ) {}

		/// get a new object from the heap
	T* get ( void )
	{
		this->ensureHeapSize();
		return this->Base[this->last++];
	}
}; // DeletelessAllocator

#endif
