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

#ifndef ATOMICDECOMPOSER_H
#define ATOMICDECOMPOSER_H

#include "tDLAxiom.h"
#include "tSignature.h"
#include "Modularity.h"

typedef SigIndex::AxiomSet AxiomSet;

/// represeination of the ontology atom
class TOntologyAtom
{
public:		// typedefs
		/// set of atoms
	typedef std::set<TOntologyAtom*> AtomSet;

protected:	// members
		/// set of axioms in the atom
	AxiomSet AtomAxioms;
		/// set of axioms in the module (Atom's ideal)
	AxiomSet ModuleAxioms;
		/// set of atoms current one depends on
	AtomSet DepAtoms;
		/// label (int right now)
	size_t Label;

public:		// interface
		/// empty c'tor
	TOntologyAtom ( void ) : Label(0) {}
		/// d'tor
	~TOntologyAtom ( void ) {}

	// fill in the sets

		/// set the module axioms
	void setModule ( const AxiomSet& module ) { ModuleAxioms = module; }
		/// add axiom AX to an atom
	void addAxiom ( TDLAxiom* ax )
	{
		AtomAxioms.insert(ax);
		ax->setAtom(this);
	}
		/// add atom to the dependency set
	void addDepAtom ( TOntologyAtom* atom )
	{
		if ( likely(atom != NULL) && atom != this )
			DepAtoms.insert(atom);
	}

	// access to axioms

		/// get all the atom's axioms
	const AxiomSet& getAtomAxioms ( void ) const { return AtomAxioms; }
		/// get all the module axioms
	const AxiomSet& getModule ( void ) const { return ModuleAxioms; }
		/// get atoms a given one depends on
	const AtomSet& getDepAtoms ( void ) const { return DepAtoms; }

    size_t getLabel() const { return Label; }
    void setLabel ( size_t label ) { Label = label; }
}; // TOntologyAtom

/// atomical ontology structure
class AOStructure
{
public:		// types
		/// vector of atoms as a type
	typedef std::vector<TOntologyAtom*> AtomVec;
		/// RW iterator of it
	typedef AtomVec::iterator iterator;

protected:	// members
		/// all the atoms
	AtomVec Atoms;

public:		// interface
		/// empty c'tor
	AOStructure ( void ) {}
		/// d'tor: delete all atoms
	~AOStructure ( void ) {}

		/// create a new atom and get a pointer to it
	TOntologyAtom* newAtom ( void )
	{
		TOntologyAtom* ret = new TOntologyAtom();
		ret->setLabel(Atoms.size());
		Atoms.push_back(ret);
		return ret;
	}

		/// RW iterator begin
	iterator begin ( void ) { return Atoms.begin(); }
		/// RW iterator end
	iterator end ( void ) { return Atoms.end(); }
		/// get RW atom by its index
	TOntologyAtom* operator[] ( unsigned int index ) { return Atoms[index]; }
		/// get RO atom by its index
	const TOntologyAtom* operator[] ( unsigned int index ) const { return Atoms[index]; }
		/// size of the structure
	size_t size ( void ) const { return Atoms.size(); }
}; // AOStructure

/// atomical structure of the ontology
class AtomicDecomposer
{
protected:	// members
		/// atomic structure to build
	AOStructure* AOS;
		/// typedef for the iterator
	typedef TOntology::iterator iterator;
		/// modularizer to build modules
	TModularizer Modularizer;
		/// set of declaration axioms (not used in AD)
	TOntology::TAxiomArray Declarations;

protected:	// methods
		/// build a module for given signature SIG and module type TYPE; use part [BEGIN,END) for the module search
	TOntologyAtom* buildModule ( const TSignature& sig, ModuleType type, iterator begin, iterator end, TOntologyAtom* parent );
		/// create atom for given axiom AX and module type TYPE; use part [BEGIN,END) for the module search
	TOntologyAtom* createAtom ( TDLAxiom* ax, ModuleType type, iterator begin, iterator end, TOntologyAtom* parent );

public:		// interface
		/// init c'tor
	AtomicDecomposer ( void ) : AOS(NULL) {}
		/// d'tor
	~AtomicDecomposer ( void ) { delete AOS; }

		/// get the atomic structure for given module type TYPE
	AOStructure* getAOS ( TOntology* O, ModuleType type );
		/// get already created atomic structure
	const AOStructure* getAOS ( void ) const { return AOS; }
}; // AtomicDecomposer

#endif
