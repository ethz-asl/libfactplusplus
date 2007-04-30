/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2005 by Dmitry Tsarkov

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

#ifndef _TRARESAVESTACK_H
#define _TRARESAVESTACK_H

#include <vector>

#include "tRestorer.h"

/**
 *	Stack for Saving/Restoring rarely changing information.
 *	Uses self-contained Restorer as a way to update state of object.
 */
class TRareSaveStack
{
protected:	// classes
		/// class to represent restoring information together with level
	class RestoreElement
	{
	public:		// members
			/// level of restoring
		unsigned int level;
			/// pointer to restoring information
		TRestorer* p;
	public:		// interface
			/// empty c'tor
		RestoreElement ( void ) : level(0), p(NULL) {}
			/// create c'tor
		RestoreElement ( unsigned int l, TRestorer* q ) : level(l), p(q) {}
			/// copy c'tor
		RestoreElement ( const RestoreElement& e ) : level(e.level), p(e.p) {}
			/// assignment
		RestoreElement& operator = ( const RestoreElement& e )
		{
			if ( p != e.p )
			{
				delete p;
				level = e.level;
				p = e.p;
			}
			return *this;
		}
			/// d'tor
			// WARNING!! memory leaks!
		~RestoreElement ( void ) { /*delete p;*/ }
	}; // RestoreElement

protected:	// typedefs
		/// type of the heap
	typedef std::vector<RestoreElement> baseType;

protected:	// members
		/// heap of saved objects
	baseType Base;

public:		// interface
		/// empty c'tor: stack will most likely be empty
	TRareSaveStack ( void ) {}
		/// d'tor
	~TRareSaveStack ( void ) {}

	// stack operations

		/// check that stack is empty
	bool empty ( void ) const { return Base.empty(); }
		/// add a new object to the stack
	void push ( unsigned int level, TRestorer* p ) { Base.push_back(RestoreElement(level,p)); }
		/// get all object from the top of the stack with levels >= LEVEL
	void restore ( unsigned int level )
	{
		while ( !Base.empty() && Base.back().level > level )
		{
			// need to restore: restore last element, remove it from stack
			Base.back().p->restore();
			Base.resize(Base.size()-1);
		}
	}
		/// clear stack
	void clear ( void ) { Base.clear(); }
}; // TRareSaveStack

#endif
