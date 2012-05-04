/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2012 by Dmitry Tsarkov

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

// methods to work with Atomic Decomposition

#include <fstream>

#include "AtomicDecomposer.h"
#include "tOntologyPrinterLISP.h"	// AD prints
#include "procTimer.h"
#include "cppi.h"

// defined in FaCT.cpp
extern std::ofstream Out;

// print axioms of an atom
static void
printAtomAxioms ( const TOntologyAtom::AxiomSet& Axioms )
{
	static TLISPOntologyPrinter LP(Out);
	// do cycle via set to keep the order
	typedef std::set<TDLAxiom*> AxSet;
	const AxSet M ( Axioms.begin(), Axioms.end() );
	for ( AxSet::const_iterator p = M.begin(); p != M.end(); ++p )
		(*p)->accept(LP);
}

/// print dependencies of an atom
static void
printAtomDeps ( const TOntologyAtom::AtomSet& Dep )
{
	if ( unlikely(Dep.empty()) )
		Out << "Ground";
	else
		Out << "Depends on:";
	for ( TOntologyAtom::AtomSet::const_iterator q = Dep.begin(), q_end = Dep.end(); q != q_end; ++q )
		Out << " " << (*q)->getId();
	Out << "\n";
}

/// print the atom with an index INDEX of the AD
static void
printADAtom ( TOntologyAtom* atom )
{
	const TOntologyAtom::AxiomSet& Axioms = atom->getAtomAxioms();
	Out << "Atom " << atom->getId() << " (size " << Axioms.size() << ", module size " << atom->getModule().size() << "):\n";
	printAtomAxioms(Axioms);
	printAtomDeps(atom->getDepAtoms());
}

/// @return all the axioms in the AD
static size_t
sizeAD ( AOStructure* AOS )
{
	size_t ret = 0;
	for ( AOStructure::iterator p = AOS->begin(), p_end = AOS->end(); p != p_end; ++p )
		ret += (*p)->getAtomAxioms().size();
	return ret;
}

void
CreateAD ( TOntology* Ontology )
{
	std::cerr << "\n";
	// do the atomic decomposition
	TsProcTimer timer;
	timer.Start();
	AtomicDecomposer* AD = new AtomicDecomposer();
	AD->setProgressIndicator(new CPPI());
	AOStructure* AOS = AD->getAOS ( Ontology, M_BOT );
	timer.Stop();
	Out << "Atomic structure built in " << timer << " seconds\n";
	size_t sz = sizeAD(AOS);
	Out << "Atomic structure (" << sz << " axioms in " << AOS->size() << " atoms; " << Ontology->size()-sz << " tautologies):\n";
	for ( AOStructure::iterator p = AOS->begin(), p_end = AOS->end(); p != p_end; ++p )
		printADAtom(*p);
	delete AD;
}
