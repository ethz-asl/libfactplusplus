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

#include "AtomicDecomposer.h"
#include "logging.h"

/// build a module for given axiom AX and module type TYPE; use part [BEGIN,END) for the module search
TOntologyAtom*
AtomicDecomposer :: buildModule ( const TSignature& sig, ModuleType type, iterator begin, iterator end, TOntologyAtom* parent )
{
	// build a module for a given signature
	std::set<TDLAxiom*> Module;
	Modularizer.extract ( begin, end, sig, type, Module );
	// if module is empty -- do nothing
	if ( Module.empty() )
		return NULL;
	// check if the module corresponds to a PARENT one
	if ( parent && Module == parent->getModule() )	// same module means same atom
		return parent;
	// create new atom with that module
	TOntologyAtom* atom = AOS->newAtom();
	atom->setModule(Module);
	return atom;
}

/// get the atomic structure for given module type TYPE
AOStructure*
AtomicDecomposer :: getAOS ( TOntology* O, ModuleType type )
{
	// prepare a new AO structure
	delete AOS;
	AOS = new AOStructure();
	// we don't need declarations here
	Declarations.clear();
	iterator p = O->begin(), p_end = O->end();
	for ( ; p != p_end; ++p )
		if ( likely((*p)->isUsed()) && dynamic_cast<TDLAxiomDeclaration*>(*p) != NULL )
		{
			(*p)->setUsed(false);
			Declarations.push_back(*p);
		}
	// prepare SigIndex for the optimized modularization
	SigIndex* SI = new SigIndex();
	SI->processRange ( O->begin(), O->end() );
	Modularizer.setSigIndex(SI);

	// build the "bottom" atom for an empty signature
	TOntologyAtom* BottomAtom = buildModule ( TSignature(), type, O->begin(), O->end(), NULL );
	if ( BottomAtom )
		for ( AxiomSet::iterator q = BottomAtom->getModule().begin(), q_end = BottomAtom->getModule().end(); q != q_end; ++q )
			BottomAtom->addAxiom(*q);
	// create an atom for all the axioms in the ontology
	for ( p = O->begin(); p != p_end; ++p )
		if ( (*p)->isUsed() && (*p)->getAtom() == NULL )
			createAtom ( *p, type, O->begin(), O->end(), NULL );
	if ( LLM.isWritable(llAlways) )
		LL << "\nThere were " << Modularizer.getNNonLocal() << " non-local axioms out of " << Modularizer.getNChecks() << " totally checked\n";
	// return declaration back to the ontology
	for ( p = Declarations.begin(), p_end = Declarations.end(); p != p_end; ++p )
		(*p)->setUsed(true);

	return AOS;
}

/// create atom for given axiom AX and module type TYPE; use part [BEGIN,END) for the module search
TOntologyAtom*
AtomicDecomposer :: createAtom ( TDLAxiom* ax, ModuleType type, iterator begin, iterator end, TOntologyAtom* parent )
{
	// check whether axiom already has an atom
	if ( ax->getAtom() != NULL )
		return const_cast<TOntologyAtom*>(ax->getAtom());
	// build an atom: use a module to find atomic dependencies
	TOntologyAtom* atom = buildModule( *ax->getSignature(), type, begin, end, parent );
	// empty modules means nothing to do
	if ( atom == NULL )
		return NULL;
	// register axiom as a part of an atom
	atom->addAxiom(ax);
	// if atom is the same as parent -- nothing more to do
	if ( atom == parent )
		return parent;
	// not the same as parent: for all atom's axioms check their atoms and make ATOM depend on them
	typedef std::vector<TDLAxiom*> AxVec;
	// create a new module base
	AxVec Module ( atom->getModule().begin(), atom->getModule().end() );
	begin = Module.begin();
	end = Module.end();
	for ( iterator q = begin; q != end; ++q )
		if ( likely ( *q != ax ) )
			atom->addDepAtom ( createAtom ( *q, type, begin, end, atom ) );
	return atom;
}
