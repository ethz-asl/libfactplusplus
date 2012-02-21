/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2011-2012 by Dmitry Tsarkov

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
#include "ProgressIndicatorInterface.h"

/// remove tautologies (axioms that are always local) from the ontology temporarily
void
AtomicDecomposer :: removeTautologies ( TOntology* O, ModuleType type )
{
	// we might use it for another decomposition
	Tautologies.clear();
	unsigned long nAx = 0;
	for ( iterator p = O->begin(), p_end = O->end(); p != p_end; ++p )
		if ( likely((*p)->isUsed()) )
		{
			// check whether an axiom is local wrt its own signature
			Modularizer.extract ( p, p+1, *(*p)->getSignature(), type );
			if ( unlikely(!(*p)->isInModule()) )
			{
				Tautologies.push_back(*p);
				(*p)->setUsed(false);
			}
			else
				++nAx;
		}
	if ( PI )
		PI->setLimit(nAx);
}

/// build a module for given axiom AX and module type TYPE; use part [BEGIN,END) for the module search
TOntologyAtom*
AtomicDecomposer :: buildModule ( const TSignature& sig, ModuleType type, iterator begin, iterator end, TOntologyAtom* parent )
{
	// build a module for a given signature
	std::set<TDLAxiom*> Module;
	Modularizer.extract ( begin, end, sig, type, Module );
	// if module is empty (empty bottom atom) -- do nothing
	if ( Module.empty() )
		return NULL;
	// here the module is created; report it
	if ( PI )
		PI->incIndicator();
	// check if the module corresponds to a PARENT one; modules are the same iff their sizes are the same
	if ( parent && Module.size() == parent->getModule().size() )	// same module means same atom
		return parent;
	// create new atom with that module
	TOntologyAtom* atom = AOS->newAtom();
	atom->setModule(Module);
	return atom;
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
	// no empty modules should be here
	fpp_assert ( atom != NULL );
	// register axiom as a part of an atom
	atom->addAxiom(ax);
	// if atom is the same as parent -- nothing more to do
	if ( atom == parent )
		return parent;
	// not the same as parent: for all atom's axioms check their atoms and make ATOM depend on them
	AxiomVec Module ( atom->getModule().begin(), atom->getModule().end() );
	begin = Module.begin();
	end = Module.end();
	for ( iterator q = begin; q != end; ++q )
		if ( likely ( *q != ax ) )
			atom->addDepAtom ( createAtom ( *q, type, begin, end, atom ) );
	return atom;
}

/// get the atomic structure for given module type TYPE
AOStructure*
AtomicDecomposer :: getAOS ( TOntology* O, ModuleType type )
{
	// prepare a new AO structure
	delete AOS;
	AOS = new AOStructure();

	// we don't need tautologies here
	removeTautologies(O,type);

	// prepare SigIndex for the optimized modularization
	initSigIndex(O);

	// build the "bottom" atom for an empty signature
	TOntologyAtom* BottomAtom = buildModule ( TSignature(), type, O->begin(), O->end(), NULL );
	if ( BottomAtom )
		for ( TOntologyAtom::AxiomSet::const_iterator q = BottomAtom->getModule().begin(), q_end = BottomAtom->getModule().end(); q != q_end; ++q )
			BottomAtom->addAxiom(*q);

	// create atoms for all the axioms in the ontology
	for ( iterator p = O->begin(), p_end = O->end(); p != p_end; ++p )
		if ( (*p)->isUsed() && (*p)->getAtom() == NULL )
			createAtom ( *p, type, O->begin(), O->end(), NULL );

	// restore tautologies in the ontology
	restoreTautologies();

	if ( LLM.isWritable(llAlways) )
		LL << "\nThere were " << Modularizer.getNNonLocal() << " non-local axioms out of " << Modularizer.getNChecks() << " totally checked\n";

	// reduce graph
	AOS->reduceGraph();

	return AOS;
}
