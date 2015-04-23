/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2006-2015 by Dmitry Tsarkov

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

#ifndef ACTOR_H
#define ACTOR_H

#include "taxVertex.h"
#include "WalkerInterface.h"

class TIndividual;

/// class for acting with concept taxonomy
class Actor: public WalkerInterface
{
public:		// types
		/// entry in an output
	typedef ClassifiableEntry EntryType;
		/// 1D vector of entries
	typedef std::vector<const EntryType*> Array1D;
		/// 2D vector of entries
	typedef std::vector<Array1D> Array2D;

protected:	// members
		/// vertices that satisfy the condition
	std::vector<const TaxonomyVertex*> found;
		/// flag to look at concept-like or role-like entities
	bool isRole;
		/// flag to look at concepts or object roles
	bool isStandard;
		/// flag to throw exception at the 1st found
	bool interrupt;

protected:	// methods
		/// check whether actor is applicable to the ENTRY
	bool applicable ( const EntryType* entry ) const;
		/// @return true iff current entry is visible
	bool tryEntry ( const EntryType* p ) const { return !p->isSystem() && applicable(p);}
		/// @return true if at least one entry of a vertex V is visible
	bool tryVertex ( const TaxonomyVertex& v ) const
	{
		if ( tryEntry(v.getPrimer()) )
			return true;
		for ( TaxonomyVertex::syn_iterator p = v.begin_syn(), p_end=v.end_syn(); p != p_end; ++p )
			if ( tryEntry(*p) )
				return true;
		return false;
	}
		/// fills an array with all suitable data from the vertex
	void fillArray ( const TaxonomyVertex& v, Array1D& array ) const
	{
		if ( tryEntry(v.getPrimer()) )
			array.push_back(v.getPrimer());
		for ( TaxonomyVertex::syn_iterator p = v.begin_syn(), p_end=v.end_syn(); p != p_end; ++p )
			if ( tryEntry(*p) )
				array.push_back(*p);
	}

public:		// interface
		/// empty c'tor
	Actor ( void ) {}
		/// empty d'tor
	virtual ~Actor ( void ) {}


	void clear ( void ) { found.clear(); }

	// flags setup

		/// set the actor to look for classes
	void needConcepts ( void ) { isRole = false; isStandard = true; }
		/// set the actor to look for individuals
	void needIndividuals ( void ) { isRole = false; isStandard = false; }
		/// set the actor to look for object properties
	void needObjectRoles ( void ) { isRole = true; isStandard = true; }
		/// set the actor to look for individuals
	void needDataRoles ( void ) { isRole = true; isStandard = false; }
		/// set the interrupt parameter to VALUE
	void setInterruptAfterFirstFound ( bool value ) { interrupt = value; }

	// fill structures according to what's in the taxonomy

		/// return data as a 1D-array
	void getFoundData ( Array1D& array ) const
	{
		array.clear();
		for ( size_t i = 0; i < found.size(); i++ )
			fillArray ( *found[i], array );
	}
		/// return data as a 2D-array
	void getFoundData ( Array2D& array ) const
	{
		array.clear();
		array.resize(found.size());
		for ( size_t i = 0; i < found.size(); i++ )
			fillArray ( *found[i], array[i] );
	}

		/// taxonomy walking method.
		/// @return true if node was processed
		/// @return false if node can not be processed in current settings
	virtual bool apply ( const TaxonomyVertex& v )
	{
		if ( tryVertex(v) )
		{
			found.push_back(&v);
			return true;
		}
		return false;
	}
}; // Actor

#endif
