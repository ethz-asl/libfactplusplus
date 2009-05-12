/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2009 by Dmitry Tsarkov

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

#ifndef _TNAMESET_H
#define _TNAMESET_H

#include <string>
#include <map>

/// base class for creating Named Entries; template parameter should be derived from TNamedEntry
template<class T>
class TNameCreator
{
public:		// interface
		/// empty c'tor
	TNameCreator ( void ) {}
		/// empty d'tor
	virtual ~TNameCreator ( void ) {}

		/// create new Named Entry
	virtual T* makeEntry ( const std::string& name ) const { return new T(name); }
}; // TNameCreator


/// Implementation of NameSets by binary trees; template parameter should be derived from TNamedEntry
template<class T>
class TNameSet
{
protected:	// types
	typedef std::map <std::string, T*> NameTree;

protected:	// members
		/// Base holding all names
	NameTree Base;
		/// creator of new name
	TNameCreator<T>* Creator;

private:	// no copy
		/// no copy c'tor
	TNameSet ( const TNameSet& );
		/// no assignment
	TNameSet& operator = ( const TNameSet& );

public:		// interface
		/// c'tor (empty)
	TNameSet ( void ) : Creator(new TNameCreator<T>) {}
		/// c'tor (with given Name Creating class)
	TNameSet ( TNameCreator<T>* p ) : Creator(p) {}
		/// d'tor (delete all entries)
	virtual ~TNameSet ( void ) { clear(); delete Creator; }

		/// return pointer to existing id or NULL if no such id defined
	T* get ( const std::string& id ) const;
		/// unconditionally add new element with name ID to the set; return new element
	T* add ( const std::string& id );
		/// Insert id to a nameset (if necessary); @return pointer to id structure created by external creator
	T* insert ( const std::string& id );
		/// remove given entry from the set
	void remove ( const std::string& id );
		/// clear name set
	void clear ( void );
}; // TNameSet

template<class T>
void TNameSet<T> :: clear ( void )
{
	for ( typename NameTree::iterator p = Base.begin(); p != Base.end(); ++p )
		delete p->second;

	Base.clear();
}

template<class T>
T* TNameSet<T> :: insert ( const std::string& id )
{
	T* pne = get(id);

	if ( pne == NULL )	// no such Id
		pne = add(id);

	return pne;
}

template<class T>
T* TNameSet<T> :: add ( const std::string& id )
{
	T* pne = Creator->makeEntry(id);
	Base[id] = pne;
	return pne;
}

template<class T>
T* TNameSet<T> :: get ( const std::string& id ) const
{
	typename NameTree::const_iterator p = Base.find(id);
	return p == Base.end() ? NULL : p->second;
}

template<class T>
void TNameSet<T> :: remove ( const std::string& id )
{
	typename NameTree::iterator p = Base.find(id);

	if ( p != Base.end () )	// founs such Id
	{
		delete p->second;
		Base.erase(p);
	}
}

#endif
