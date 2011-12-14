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

/// representation of the ontology atom
class TOntologyAtom
{
protected:	// internal types
		/// type to compare 2 atoms
	struct AtomLess
	{
		bool operator()(const TOntologyAtom* a1, const TOntologyAtom* a2) const
			{ return a1->getId() < a2->getId(); }
	};

public:		// typedefs
		/// set of atoms
	typedef std::set<TOntologyAtom*, AtomLess> AtomSet;

protected:	// members
		/// set of axioms in the atom
	AxiomSet AtomAxioms;
		/// set of axioms in the module (Atom's ideal)
	AxiomSet ModuleAxioms;
		/// set of atoms current one depends on
	AtomSet DepAtoms;
		/// unique atom's identifier
	size_t Id;

public:		// interface
		/// empty c'tor
	TOntologyAtom ( void ) : Id(0) {}
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

		/// get the value of the id
    size_t getId() const { return Id; }
    	/// set the value of the id to ID
    void setId ( size_t id ) { Id = id; }
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
		ret->setId(Atoms.size());
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

/// atomical decomposer of the ontology
class AtomicDecomposer
{
protected:	// types
		/// remove all declarations from the ontology for the lifetime of the object
	class DeclRemover
	{
	protected:	// types
			/// axiom array
		typedef TOntology::TAxiomArray TAxiomArray;
			/// iterator over the axiom array
		typedef TAxiomArray::iterator iterator;

	protected:	// members
			/// set of declaration axioms to remember
		TAxiomArray Declarations;

	public:		// interface
			/// c'tor: mark all used declarations as unused
		DeclRemover ( TOntology* O )
		{
			for ( iterator p = O->begin(), p_end = O->end(); p != p_end; ++p )
				if ( likely((*p)->isUsed()) && dynamic_cast<TDLAxiomDeclaration*>(*p) != NULL )
				{
					(*p)->setUsed(false);
					Declarations.push_back(*p);
				}
		}
			/// d'tor: mark all removed declarations as used
		~DeclRemover ( void )
		{
			for ( iterator p = Declarations.begin(), p_end = Declarations.end(); p != p_end; ++p )
				(*p)->setUsed(true);
		}
	}; // DeclRemover

protected:	// members
		/// atomic structure to build
	AOStructure* AOS;
		/// typedef for the iterator
	typedef TOntology::iterator iterator;
		/// modularizer to build modules
	TModularizer Modularizer;

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
