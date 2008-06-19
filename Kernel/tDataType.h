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

#ifndef _TDATATYPE_H
#define _TDATATYPE_H

#include "tDataEntry.h"
#include "tNECollection.h"

/// class for representing general data type
class TDataType: public TNECollection<TDataEntry>
{
protected:	// types
		/// set of facets
	typedef std::vector<TDataInterval*> TFacets;

protected:	// members
		/// data type
	TDataEntry* Type;
		/// internal facets
	TFacets Facets;

private:	// no copy
		/// no copy c'tor
	TDataType ( const TDataType& );
		/// no assignment
	TDataType& operator = ( const TDataType& );

protected:	// methods
		/// register data value in the datatype
	virtual void registerNew ( TDataEntry* p ) { p->setHostType(Type); }

public:		// interface
		/// c'tor: create the TYPE entry
	TDataType ( const std::string& name )
		: TNECollection<TDataEntry>(name)
		{ Type = new TDataEntry(name); }
		/// d'tor: delete data type entry and all the facets
	virtual ~TDataType ( void )
	{
		for ( TFacets::iterator p = Facets.begin(), p_end = Facets.end(); p < p_end; ++p )
			delete *p;

		delete Type;
	}

	// access to the type

		/// get RW access to the type entry (useful for relevance etc)
	TDataEntry* getType ( void ) { return Type; }
		/// get RO access to the type entry
	const TDataEntry* getType ( void ) const { return Type; }

		/// create new expression of the type
	TDataEntry* getExpr ( void )
	{
		if ( isLocked() )
			return NULL;	// FIXME!! exception later
		return registerElem(new TDataEntry("expr"));
	}
		/// get new Facet
	TDataInterval* getFacet ( void )
	{
		TDataInterval* ret = new TDataInterval;
		Facets.push_back(ret);
		return ret;
	}
}; // TDataType

#endif
