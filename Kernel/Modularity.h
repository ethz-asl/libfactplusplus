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

#ifndef MODULARITY_H
#define MODULARITY_H

#include "tOntology.h"
#include "SyntacticLocalityChecker.h"
#include "tSignatureUpdater.h"

enum ModuleType { M_TOP, M_BOT, M_STAR };

/// class to create modules of an ontology wrt module type
class TModularizer
{
protected:	// members
		/// shared signature signature
	TSignature sig;
		/// internal syntactic locality checker
	SyntacticLocalityChecker Checker;
		/// signature updater
	TSignatureUpdater Updater;
		/// module as a list of axioms
	std::vector<TDLAxiom*> Module;

protected:	// methods
		/// add an axiom to a module
	void addAxiomToModule ( TDLAxiom* axiom )
	{
		axiom->setInModule(true);
		Module.push_back(axiom);
		// update the signature
		axiom->accept(Updater);
	}
		/// mark the ontology O such that all the marked axioms creates the module wrt SIG
	template<class Iterator>
	void extractModule ( Iterator begin, Iterator end )
	{
		size_t sigSize;
		Module.clear();
		Module.reserve(end-begin);
		// clear the module flag in the input
		Iterator p;
		for ( p = begin; p != end; ++p )
			(*p)->setInModule(false);
		do
		{
			sigSize = sig.size();
			for ( p = begin; p != end; ++p )
				if ( !(*p)->isInModule() && (*p)->isUsed() && !Checker.local(*p) )
					addAxiomToModule(*p);

        } while ( sigSize != sig.size() );
	}

public:
		/// init c'tor
	TModularizer ( void )
		: Checker(&sig)
		, Updater(sig)
		{}
		// empty d'tor
	~TModularizer ( void ) {}

		/// extract module wrt SIGNATURE and TYPE from O
	void extract ( TOntology& O, const TSignature& signature, ModuleType type )
	{
		bool topLocality = (type == M_TOP);

		sig = signature;
		sig.setLocality(topLocality);
 		extractModule ( O.begin(), O.end() );

		if ( type != M_STAR )
			return;

		// here there is a star: do the cycle until stabilizastion
		size_t size;
		std::vector<TDLAxiom*> oldModule;
		do
		{
			size = Module.size();
			oldModule.swap(Module);
			topLocality = !topLocality;

			sig = signature;
			sig.setLocality(topLocality);
	 		extractModule ( oldModule.begin(), oldModule.end() );
		} while ( size != Module.size() );
	}
		/// extract module wrt SIGNATURE and TYPE from O; @return result in the Set
	void extract ( TOntology& O, const TSignature& signature, ModuleType type, std::set<TDLAxiom*>& Set )
	{
		extract ( O, signature, type );
		Set.clear();
		Set.insert ( Module.begin(), Module.end() );
	}
		/// get access to a signature
	const TSignature& getSignature ( void ) const { return sig; }
}; // TModularizer

#endif
