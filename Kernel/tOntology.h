/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2008 by Dmitry Tsarkov

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

#ifndef TONTOLOGY_H
#define TONTOLOGY_H

#include <vector>
#include "tDLAxiom.h"

/// define ontology as a set of axioms
class TOntology
{
protected:	// types
		/// base type for the set of axioms
	typedef std::vector<TDLAxiom*> TAxiomArray;
		/// RW iterator ovet base type
	typedef TAxiomArray::iterator iterator;

protected:	// members
		/// all the axioms
	TAxiomArray Axioms;
		/// id to be given to the next axiom
	unsigned int axiomId;

protected:	// methods

public:		// interface
		/// empty c'tor
	TOntology ( void ) : axiomId(0) {}
		/// d'tor
	~TOntology ( void ) { clear(); }

		/// add given axiom to the ontology
	void add ( TDLAxiom* p )
	{
		p->setId(++axiomId);
		Axioms.push_back(p);
	}
		/// load ontology to a given KB
	void load ( TBox& kb )
	{
		for ( iterator p = Axioms.begin(), p_end = Axioms.end(); p < p_end; ++p )
			(*p)->load(kb);
	}
		/// clear the ontology
	void clear ( void )
	{
		for ( iterator p = Axioms.begin(), p_end = Axioms.end(); p < p_end; ++p )
			delete *p;
		Axioms.clear();
	}
}; // TOntology

#endif
