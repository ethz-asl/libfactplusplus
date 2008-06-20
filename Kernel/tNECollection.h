/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2005-2008 by Dmitry Tsarkov

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

#ifndef _TNECOLLECTION_H
#define _TNECOLLECTION_H

#include <vector>

#include "globaldef.h"
#include "tNamedEntry.h"
#include "tNameSet.h"
#include "eFPPCantRegName.h"

/** class for collect TNamedEntry'es together. Template parameter should be
	inherited from TNamedEntry. Implemented as vector of T*,
	with Base[i]->getId() == i.
**/
template<class T>
class TNECollection
{
protected:	// typedefs
	typedef std::vector<T*> BaseType;

public:		// typedefs
	typedef typename BaseType::iterator iterator;
	typedef typename BaseType::const_iterator const_iterator;

protected:	// members
		/// vector of elements
	BaseType Base;
		/// nameset to hold the elements
	TNameSet<T> NameSet;
		/// name of the type
	std::string TypeName;
		/// flag to lock the nameset (ie, prohibit to add new names there)
	bool locked;

protected:	// methods
		/// virtual method for additional tuning of newly created element
	virtual void registerNew ( T* p ATTR_UNUSED ) {}
		/// register new element in a collection; return this element
	T* registerElem ( T* p )
	{
		p->setId(Base.size());
		Base.push_back(p);
		registerNew(p);
		return p;
	}

public:		// interface
		/// c'tor: clear 0-th element
	TNECollection ( const std::string& name )
		: TypeName(name)
		{ Base.push_back(NULL); }
		/// empty d'tor: all elements will be deleted in other place
	virtual ~TNECollection ( void ) {}

	// locked interface

		/// check if collection is locked
	bool isLocked ( void ) const { return locked; }
		/// set LOCKED value to a VAL; @return old value of LOCKED
	bool setLocked ( bool val ) { bool old = locked; locked = val; return old; }

	// add/remove elements

		/// check if name is registered in given collection
	bool isRegistered ( const TNamedEntry* name ) const
	{
		int id = name->getId();
		return  id > 0 &&						// defined entry
				size_t(id) < Base.size() &&		// ID does fit collection
				Base[id] == name;				// ID is an index of given name
	}
		/// get entry by NAME from the collection; register it if necessary
	T* get ( const std::string& name ) throw(EFPPCantRegName)
	{
		T* p = NameSet.get(name);

		// check if name is already defined
		if ( p != NULL )
			return p;

		// check if it is possible to insert name
		if ( isLocked() )
		{
			throw EFPPCantRegName ( name, TypeName );
			return NULL;
		}

		// register name in name set, and register it
		return registerElem(NameSet.add(name));
	}
		/// remove given entry from the collection; @return true iff it was NOT the last entry.
	bool Remove ( T* p )
	{
		if ( !isRegistered(p) )
			return true;
		// check if the entry is the last entry
		if ( Base.size() - p->getId() != 1 )
			return true;

		Base.resize(p->getId());
		NameSet.remove(p->getName());
		return false;
	}

	// access to elements

	iterator begin ( void ) { return Base.begin()+1; }
	const_iterator begin ( void ) const { return Base.begin()+1; }

	iterator end ( void ) { return Base.end(); }
	const_iterator end ( void ) const { return Base.end(); }

	size_t size ( void ) const { return Base.size()-1; }
}; // TNECollection

#endif
