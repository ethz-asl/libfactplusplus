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

#ifndef _TDATAENTRY_H
#define _TDATAENTRY_H

#include <cstdlib>

#include "tNamedEntry.h"
#include "BiPointer.h"
#include "tLabeller.h"
#include "DataTypeComparator.h"

class TDataEntry;

// if min() and max() are defined, then they conflicts with a TDI members
#undef min
#undef max

/// class for representing general data restriction
class TDataInterval
{
public:		// members
		/// left border of the interval
	const TDataEntry* min;
		/// right border of the interval
	const TDataEntry* max;
		/// type of the left border
	bool minExcl;
		/// type of the right border
	bool maxExcl;

public:		// interface
		/// empty c'tor
	TDataInterval ( void ) : min(NULL), max(NULL) {}
		/// copy c'tor
	TDataInterval ( const TDataInterval& copy )
		: min(copy.min)
		, max(copy.max)
		, minExcl(copy.minExcl)
		, maxExcl(copy.maxExcl)
		{}
		/// assignment
	TDataInterval& operator = ( const TDataInterval& copy )
	{
		min = copy.min;
		max = copy.max;
		minExcl = copy.minExcl;
		maxExcl = copy.maxExcl;
		return *this;
	}
		/// empty d'tor
	~TDataInterval ( void ) {}

		/// clear an interval
	void clear ( void ) { min = max = NULL; }

		/// no constraints
	bool empty ( void ) const { return min == NULL && max == NULL; }
		/// check if min value range have been set
	bool hasMin ( void ) const { return min != NULL; }
		/// check if max value range have been set
	bool hasMax ( void ) const { return max != NULL; }
		/// closed interval
	bool closed ( void ) const { return min != NULL && max != NULL; }

		/// update MIN border of an interval with VALUE wrt EXCL
	bool updateMin ( bool excl, const TDataEntry* value );
		/// update MAX border of an interval with VALUE wrt EXCL
	bool updateMax ( bool excl, const TDataEntry* value );
		/// update given border of an interval with VALUE wrt EXCL
	bool update ( bool min, bool excl, const TDataEntry* value )
		{ return min ? updateMin ( excl, value ) : updateMax ( excl, value ); }
		/// update wrt another interval
	bool update ( const TDataInterval& Int )
	{
		bool ret = false;
		if ( Int.hasMin() )
			ret |= updateMin ( Int.minExcl, Int.min );
		if ( Int.hasMax() )
			ret |= updateMax ( Int.maxExcl, Int.max );
		return ret;
	}
		/// @return true iff all the data is consistent wrt given TYPE
	bool consistent ( const TDataEntry* type ) const;

		/// print an interval
	void Print ( std::ostream& o ) const;
}; // TDataInterval

/// class for representing general data entry ("name" of data type or data value)
class TDataEntry: public TNamedEntry
{
private:	// members
		/// label to use in relevant-only checks
	TLabeller::LabType rel;

protected:	// members
		/// corresponding type (Type has NULL in the field)
	const TDataEntry* Type;
		/// DAG index of the entry
	BipolarPointer pName;
		/// ComparableDT, used only for values
	ComparableDT comp;
		/// restriction to the entry
	TDataInterval Constraints;

private:	// no copy
		/// no copy c'tor
	TDataEntry ( const TDataEntry& );
		/// no assignment
	TDataEntry& operator = ( const TDataEntry& );

protected:	// methods
		/// set COMP for the (typed) data value
	void setComp ( const std::string& typeName )
	{
		if ( typeName == "String" )
			comp = ComparableDT(getName());
		else if ( typeName == "Number" )
			comp = ComparableDT((long)atoi(getName()));
		else if ( typeName == "Real" )
			comp = ComparableDT((float)atof(getName()));
	}

public:		// interface
		/// create data entry with given name
	TDataEntry ( const std::string& name )
		: TNamedEntry(name)
		, Type(NULL)
		, pName(bpINVALID)
		, comp()
		{}
		/// empty d'tor
	~TDataEntry ( void ) {}

	// type/value part

		/// check if data entry represents basic data type
	bool isBasicDataType ( void ) const { return Type == NULL && Constraints.empty(); }
		/// check if data entry represents restricted data type
	bool isRestrictedDataType ( void ) const { return !Constraints.empty(); }
		/// check if data entry represents data value
	bool isDataValue ( void ) const { return Type != NULL && Constraints.empty(); }

		/// set host data type for the data value
	void setHostType ( const TDataEntry* type ) { Type = type; setComp(type->getName()); }
		/// get host type
	const TDataEntry* getType ( void ) const { return Type; }

		/// get comparable variant of DE
	const ComparableDT& getComp ( void ) const { return comp; }

	// facet part

		/// get RW access to constraints of the DE
	TDataInterval* getFacet ( void ) { return &Constraints; }
		/// get RO access to constraints of the DE
	const TDataInterval* getFacet ( void ) const { return &Constraints; }

	// relevance part

		/// is given concept relevant to given Labeller's state
	bool isRelevant ( const TLabeller& lab ) const { return lab.isLabelled(rel); }
		/// make given concept relevant to given Labeller's state
	void setRelevant ( const TLabeller& lab ) { lab.set(rel); }

	// BP part

		/// get pointer to DAG entry correstonding to the data entry
	BipolarPointer getBP ( void ) const { return pName; }
		/// set DAG index of the data entry
	void setBP ( BipolarPointer p ) { pName = p; }
}; // TDataEntry

inline bool
TDataInterval :: consistent ( const TDataEntry* type ) const
{
	if ( hasMax() && max->getType() != type )
		return false;
	if ( hasMin() && min->getType() != type )
		return false;
	return true;
}

inline bool
TDataInterval :: updateMin ( bool excl, const TDataEntry* value )
{
	if ( hasMin() )	// another min value: check if we need update
	{
		// constraint is >= or >
		if ( min->getComp() > value->getComp() )	// was: {7,}; now: {5,}: no update needed
			return false;
		if ( min->getComp() == value->getComp() &&	// was: (5,}; now: [5,}: no update needed
			 minExcl && !excl )
			return false;
		// fallthrough: update is necessary for everything else
	}

	min = value;
	minExcl = excl;
	return true;
}

inline bool
TDataInterval :: updateMax ( bool excl, const TDataEntry* value )
{
	if ( hasMax() )	// another max value: check if we need update
	{
		// constraint is <= or <
		if ( max->getComp() < value->getComp() )	// was: {,5}; now: {,7}: no update needed
			return false;
		if ( max->getComp() == value->getComp() &&	// was: {,5); now: {,5]: no update needed
			 maxExcl && !excl )
			return false;
		// fallthrough: update is necessary for everything else
	}

	max = value;
	maxExcl = excl;
	return true;
}

inline void
TDataInterval :: Print ( std::ostream& o ) const
{
	o << (min ? (minExcl ? '(' : '[') : '{');
	if ( min ) o << min->getName();
	o << ',';
	if ( max ) o << max->getName();
	o << (max ? (maxExcl ? ')' : ']') : '}');
}

inline std::ostream&
operator << ( std::ostream& o, const TDataInterval& c )
{
	c.Print(o);
	return o;
}

#endif
