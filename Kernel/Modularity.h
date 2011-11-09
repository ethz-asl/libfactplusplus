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

#include <queue>

#include "tOntology.h"
#include "SyntacticLocalityChecker.h"
#include "SigIndex.h"

enum ModuleType { M_TOP, M_BOT, M_STAR };

/// class to create modules of an ontology wrt module type
class TModularizer
{
protected:	// types
		/// copy axiom array definition from an ontology
	typedef TOntology::TAxiomArray TAxiomArray;
		/// iterator over axiom array
	typedef TAxiomArray::iterator iterator;

protected:	// members
		/// shared signature signature
	TSignature sig;
		/// internal syntactic locality checker
	SyntacticLocalityChecker Checker;
		/// module as a list of axioms
	std::vector<TDLAxiom*> Module;
		/// pointer to a sig index; if not NULL then use optimized algo
	SigIndex* sigIndex;
		/// queue of unprocessed entities
	std::queue<const TNamedEntity*> WorkQueue;

protected:	// methods
		/// update SIG wrt the axiom signature
	void addAxiomSig ( TDLAxiom* axiom )
	{
		const TSignature& axiomSig = *axiom->getSignature();
		if ( sigIndex )
		{
			for ( TSignature::iterator p = axiomSig.begin(), p_end = axiomSig.end(); p != p_end; ++p )
				if ( !sig.contains(*p) )	// new one
					WorkQueue.push(*p);
		}
		sig.add(axiomSig);
	}
		/// add an axiom to a module
	void addAxiomToModule ( TDLAxiom* axiom )
	{
		axiom->setInModule(true);
		Module.push_back(axiom);
		// update the signature
		addAxiomSig(axiom);
	}
		/// mark the ontology O such that all the marked axioms creates the module wrt SIG
	void extractModuleLoop ( iterator begin, iterator end )
	{
		size_t sigSize;
		do
		{
			sigSize = sig.size();
			for ( iterator p = begin; p != end; ++p )
				if ( !(*p)->isInModule() && (*p)->isUsed() && !Checker.local(*p) )
					addAxiomToModule(*p);

        } while ( sigSize != sig.size() );
	}
		/// add all t
	void extractModuleQueue ( void )
	{
		// init queue with a sig
		for ( TSignature::iterator p = sig.begin(), p_end = sig.end(); p != p_end; ++p )
			WorkQueue.push(*p);
		// main cycle
		while ( !WorkQueue.empty() )
		{
			const TNamedEntity* entity = WorkQueue.front();
			WorkQueue.pop();
			// for all the axioms that contains entity in their signature
			const SigIndex::AxiomSet& AxSet = sigIndex->getAxioms(entity);
			for ( SigIndex::iterator q = AxSet.begin(), q_end = AxSet.end(); q != q_end; ++q )
				if ( !(*q)->isInModule() &&	// not in module already
					 (*q)->isInSS() &&		// in the given range
					 !Checker.local(*q) )	// non-local
					addAxiomToModule(*q);
		}

	}
		/// extract module wrt presence of a sig index
	void extractModule ( iterator begin, iterator end )
	{
		Module.clear();
		Module.reserve(end-begin);
		// clear the module flag in the input
		iterator p;
		for ( p = begin; p != end; ++p )
			(*p)->setInModule(false);
		if ( sigIndex != NULL )
		{
			for ( p = begin; p != end; ++p )
				if ( (*p)->isUsed() )
					(*p)->setInSS(true);
			extractModuleQueue();
			for ( p = begin; p != end; ++p )
				(*p)->setInSS(false);
		}
		else
			extractModuleLoop ( begin, end );
	}
public:
		/// init c'tor
	TModularizer ( void )
		: Checker(&sig)
		, sigIndex(NULL)
		{}
		// empty d'tor
	~TModularizer ( void ) {}

		/// set sig index to a given value
	void setSigIndex ( SigIndex* p ) { sigIndex = p; }
		/// extract module wrt SIGNATURE and TYPE from the set of axioms [BEGIN,END)
	void extract ( iterator begin, iterator end, const TSignature& signature, ModuleType type )
	{
		bool topLocality = (type == M_TOP);

		sig = signature;
		sig.setLocality(topLocality);
 		extractModule ( begin, end );

		if ( type != M_STAR )
			return;

		// here there is a star: do the cycle until stabilization
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
		/// extract module wrt SIGNATURE and TYPE from the set of axioms [BEGIN,END); @return result in the Set
	void extract ( iterator begin, iterator end, const TSignature& signature, ModuleType type, std::set<TDLAxiom*>& Set )
	{
		extract ( begin, end, signature, type );
		Set.clear();
		Set.insert ( Module.begin(), Module.end() );
	}
		/// extract module wrt SIGNATURE and TYPE from O
	void extract ( TOntology& O, const TSignature& signature, ModuleType type )
		{ extract ( O.begin(), O.end(), signature, type ); }
		/// extract module wrt SIGNATURE and TYPE from O; @return result in the Set
	void extract ( TOntology& O, const TSignature& signature, ModuleType type, std::set<TDLAxiom*>& Set )
		{ extract ( O.begin(), O.end(), signature, type, Set ); }
		/// get access to a signature
	const TSignature& getSignature ( void ) const { return sig; }
}; // TModularizer

#endif
