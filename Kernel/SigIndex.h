/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2011 by Dmitry Tsarkov

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

#ifndef SIGINDEX_H
#define SIGINDEX_H

#include "tDLAxiom.h"
#include "tSignature.h"

class SigIndex
{
public:		// types
		/// set of axiom
	typedef std::set<TDLAxiom*> AxiomSet;
		/// iterator over set of axioms
	typedef AxiomSet::iterator iterator;

protected:	// types
		/// map between entities and axioms that contains them in their signature
	typedef std::map<const TNamedEntity*, AxiomSet> EntityAxiomMap;

protected:	// members
		/// map itself
	EntityAxiomMap Base;

public:		// interface
		/// empty c'tor
	SigIndex ( void ) {}
		/// empty d'tor
	~SigIndex ( void ) {}

	// work with axioms

		/// register an axiom
	void registerAx ( TDLAxiom* ax )
	{
		for ( TSignature::iterator p = ax->getSignature()->begin(), p_end = ax->getSignature()->end(); p != p_end; ++p )
			Base[*p].insert(ax);
	}
		/// unregister an axiom AX
	void unregisterAx ( TDLAxiom* ax )
	{
		for ( TSignature::iterator p = ax->getSignature()->begin(), p_end = ax->getSignature()->end(); p != p_end; ++p )
			Base[*p].erase(ax);
	}
		/// process an axiom wrt its Used status
	void processAx ( TDLAxiom* ax )
	{
		if ( ax->isUsed() )
			registerAx(ax);
		else
			unregisterAx(ax);
	}
		/// process the range [begin,end) of axioms
	template<class Iterator>
	void processRange ( Iterator begin, Iterator end )
	{
		while ( begin != end )
			processAx(*begin++);
	}

	// get the set by the index

		/// given an entity, return a set of all axioms that tontain this entity in a signature
	const AxiomSet& getAxioms ( const TNamedEntity* entity ) { return Base[entity]; }
}; // SigIndex

#endif
