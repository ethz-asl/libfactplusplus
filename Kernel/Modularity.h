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

#ifndef MODULARITY_H
#define MODULARITY_H

#include <queue>

#include "tOntology.h"
#include "SigIndex.h"

enum ModuleType { M_TOP, M_BOT, M_STAR };

/// class to create modules of an ontology wrt module type
template<class LocalityChecker>
class TModularizer
{
protected:	// types
		/// RO iterator over axiom vector
	typedef AxiomVec::const_iterator const_iterator;

protected:	// members
		/// shared signature signature
	TSignature sig;
		/// internal syntactic locality checker
	LocalityChecker Checker;
		/// module as a list of axioms
	AxiomVec Module;
		/// pointer to a sig index; if not NULL then use optimized algo
	SigIndex* sigIndex;
		/// queue of unprocessed entities
	std::queue<const TNamedEntity*> WorkQueue;
		/// number of locality check calls
	unsigned long long nChecks;
		/// number of non-local axioms
	unsigned long long nNonLocal;

protected:	// methods
		/// update SIG wrt the axiom signature
	void addAxiomSig ( const TSignature& axiomSig )
	{
		if ( sigIndex == NULL )
			sig.add(axiomSig);
		else
			for ( TSignature::iterator p = axiomSig.begin(), p_end = axiomSig.end(); p != p_end; ++p )
				if ( !sig.contains(*p) )	// new one
				{
					WorkQueue.push(*p);
					sig.add(*p);
				}
	}
		/// add an axiom to a module
	void addAxiomToModule ( TDLAxiom* axiom )
	{
		axiom->setInModule(true);
		Module.push_back(axiom);
		// update the signature
		addAxiomSig(axiom->getSignature());
	}
		/// @return true iff an AXiom is non-local
	bool isNonLocal ( const TDLAxiom* ax )
	{
		++nChecks;
		if ( Checker.local(ax) )
			return false;
		++nNonLocal;
		return true;
	}
		/// add an axiom if it is non-local (or in noCheck is true)
	void addNonLocal ( TDLAxiom* ax, bool noCheck )
	{
		if ( unlikely(noCheck) || unlikely(isNonLocal(ax)) )
			addAxiomToModule(ax);
	}
		/// mark the ontology O such that all the marked axioms creates the module wrt SIG
	void extractModuleLoop ( const_iterator begin, const_iterator end )
	{
		size_t sigSize;
		do
		{
			sigSize = sig.size();
			for ( const_iterator p = begin; p != end; ++p )
				if ( !(*p)->isInModule() && (*p)->isUsed() )
					addNonLocal ( *p, /*noCheck=*/false );

        } while ( sigSize != sig.size() );
	}
		/// add all the non-local axioms from given axiom-set AxSet
	void addNonLocal ( const SigIndex::AxiomCollection& AxSet, bool noCheck )
	{
		for ( SigIndex::const_iterator q = AxSet.begin(), q_end = AxSet.end(); q != q_end; ++q )
			if ( !(*q)->isInModule() && (*q)->isInSS() )	// in the given range but not in module yet
				addNonLocal ( *q, noCheck );
	}
		/// build a module traversing axioms by a signature
	void extractModuleQueue ( void )
	{
		// init queue with a sig
		for ( TSignature::iterator p = sig.begin(), p_end = sig.end(); p != p_end; ++p )
			WorkQueue.push(*p);
		// add all the axioms that are non-local wrt given value of a top-locality
		addNonLocal ( sigIndex->getNonLocal(sig.topCLocal()), /*noCheck=*/true );
		// main cycle
		while ( !WorkQueue.empty() )
		{
			const TNamedEntity* entity = WorkQueue.front();
			WorkQueue.pop();
			// for all the axioms that contains entity in their signature
			addNonLocal ( sigIndex->getAxioms(entity), /*noCheck=*/false );
		}
	}
		/// extract module wrt presence of a sig index
	void extractModule ( const_iterator begin, const_iterator end )
	{
		Module.clear();
		Module.reserve(end-begin);
		// clear the module flag in the input
		const_iterator p;
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

public:		// interface
		/// init c'tor
	TModularizer ( void )
		: Checker(&sig)
		, sigIndex(NULL)
		, nChecks(0)
		, nNonLocal(0)
		{}
		// d'tor
	~TModularizer ( void ) { delete sigIndex; }

		/// set sig index to a given value
	void setSigIndex ( SigIndex* p )
	{
		sigIndex = p;
		nChecks += 2*p->nProcessedAx();
		nNonLocal += p->getNonLocal(false).size() + p->getNonLocal(true).size();
	}
		/// allow the checker to preprocess an ontology if necessary
	void preprocessOntology ( const AxiomVec& vec ) { Checker.preprocessOntology(vec); }
		/// extract module wrt SIGNATURE and TYPE from the set of axioms [BEGIN,END)
	void extract ( const_iterator begin, const_iterator end, const TSignature& signature, ModuleType type )
	{
		bool topLocality = (type == M_TOP);

		sig = signature;
		sig.setLocality(topLocality);
 		extractModule ( begin, end );

		if ( type != M_STAR )
			return;

		// here there is a star: do the cycle until stabilization
		size_t size;
		AxiomVec oldModule;
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
		/// extract module wrt SIGNATURE and TYPE from the axiom vector VEC; @return result in the Set
	void extract ( const AxiomVec& Vec, const TSignature& signature, ModuleType type, std::set<TDLAxiom*>& Set )
		{ extract ( Vec.begin(), Vec.end(), signature, type ); }
		/// extract module wrt SIGNATURE and TYPE from O
	void extract ( TOntology& O, const TSignature& signature, ModuleType type )
		{ extract ( O.begin(), O.end(), signature, type ); }

		/// get the last computed module
	const AxiomVec& getModule ( void ) const { return Module; }
		/// get access to a signature
	const TSignature& getSignature ( void ) const { return sig; }
		/// get number of checks made
	unsigned long long getNChecks ( void ) const { return nChecks; }
		/// get number of axioms that were local
	unsigned long long getNNonLocal ( void ) const { return nNonLocal; }
}; // TModularizer

#endif
