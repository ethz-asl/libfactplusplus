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

#include "tSplitExpansionRules.h"
#include "dlDag.h"
#include "Modularity.h"
#include "tConcept.h"

/// init entity map using given DAG. note that this should be done AFTER rule splits are created!
void
TSplitRules :: initEntityMap ( const DLDag& Dag )
{
	const size_t size = Dag.size();
	EntityMap.resize(size);
	EntityMap[0] = EntityMap[1] = NULL;
	for ( size_t i = 2; i < size-1; ++i )
		EntityMap[i] = getSingleEntity(Dag[i].getConcept());
}

TSplitRules::SigSet
TSplitRules :: buildSet ( const TSignature& sig, const TNamedEntity* entity )
{
	SigSet Set;
//	std::cout << "Building set for " << entity->getName() << "\n";
	for ( TSignature::iterator p = sig.begin(), p_end = sig.end(); p != p_end; ++p )
		if ( *p != entity && dynamic_cast<const TDLConceptName*>(*p) != NULL )
		{
//			std::cout << "In the set: " << (*p)->getName() << "\n";
			Set.insert(*p);
		}
//	std::cout << "done\n";
	// register all elements in the set in PossibleSignature
	PossibleSignature.insert ( Set.begin(), Set.end() );
	return Set;
}

void
TSplitRules :: initSplit ( TSplitVar* split )
{
//	std::cout << "Processing split for " << split->oldName->getName() << ":\n";
	TSplitVar::iterator p = split->begin(), p_end = split->end();
	SigSet impSet = buildSet ( p->sig, p->name );
	BipolarPointer bp = split->C->pBody+1;	// choose-rule stays next to a split-definition of C
	while ( ++p != p_end )
	{
		if ( p->Module.size() == 1 )
			addSplitRule ( buildSet ( p->sig, p->name ), impSet, bp );
		else
		{
			// make set of all the seed signatures of for p->Module
			std::set<TSignature> Out;
			// prepare vector of available entities
			SigVec Allowed;
//			std::cout << "\n\n\nMaking split for module with " << p->name->getName();
			AxiomVec Module ( p->Module.begin(), p->Module.end() );
			// prepare signature for the process
			TSignature sig = p->sig;
			prepareStartSig ( Module, sig, Allowed );
			// build all the seed sigs for p->sig
			BuildAllSeedSigs ( Allowed, sig, Module, Out );
			for ( std::set<TSignature>::const_iterator q = Out.begin(), q_end = Out.end(); q != q_end; ++q )
				addSplitRule ( buildSet ( *q, p->name ), impSet, bp );
		}
	}
}

void
TSplitRules :: prepareStartSig ( const AxiomVec& Module, TSignature& sig, SigVec& Allowed ) const
{
	// remove all defined concepts from signature
	for ( AxiomVec::const_iterator p = Module.begin(), p_end = Module.end(); p != p_end; ++p )
	{
		const TDLAxiomEquivalentConcepts* ax = dynamic_cast<const TDLAxiomEquivalentConcepts*>(*p);
		if ( ax != NULL )	// we don't need class names here
			for ( TDLAxiomEquivalentConcepts::iterator q = ax->begin(), q_end = ax->end(); q != q_end; ++q )
			{	// FIXME!! check for the case A=B for named classes
				const TDLConceptName* cn = dynamic_cast<const TDLConceptName*>(*q);
				if ( cn != NULL )
					sig.remove(cn);
			}
		else
		{
			const TDLAxiomConceptInclusion* ci = dynamic_cast<const TDLAxiomConceptInclusion*>(*p);
			if ( ci == NULL )
				continue;
			// don't need the left-hand part either if it is a name
			const TDLConceptName* cn = dynamic_cast<const TDLConceptName*>(ci->getSubC());
			if ( cn != NULL )
				sig.remove(cn);
		}
	}
	// now put every concept name into Allowed
	for ( TSignature::iterator r = sig.begin(), r_end = sig.end(); r != r_end; ++r )
		if ( dynamic_cast<const TDLConceptName*>(*r) != NULL )	// concept name
			Allowed.push_back(*r);
}

/// build all the seed signatures
void
TSplitRules :: BuildAllSeedSigs ( const SigVec& Allowed, const TSignature& StartSig, AxiomVec& Module, std::set<TSignature>& Out ) const
{
	// copy the signature
	TSignature sig = StartSig;
//	std::cout << "\nBuilding seed signatures:";
	// create a set of allowed entities for the next round
	SigVec RecAllowed, Keepers;
	TModularizer<SyntacticLocalityChecker> mod;
	SigVec::const_iterator p, p_end;
	for ( p = Allowed.begin(), p_end = Allowed.end(); p != p_end; ++p )
		if ( likely(sig.contains(*p)))
		{
			sig.remove(*p);
//			std::cout << "\nTrying " << (*p)->getName() << ": ";
			mod.extract ( Module.begin(), Module.end(), sig, M_STAR );
			if ( mod.getModule().size() == Module.size() )
			{	// possible to remove one
//				std::cout << "remove";
				RecAllowed.push_back(*p);
			}
			else
			{
//				std::cout << "keep";
				Keepers.push_back(*p);
			}
			sig.add(*p);
		}
//	std::cout << "\nDone with " << RecAllowed.size() << " sigs left";
	if ( RecAllowed.empty() )	// minimal seed signature
	{
		Out.insert(StartSig);
		return;
	}
	if ( !Keepers.empty() )
	{
		for ( p = RecAllowed.begin(), p_end = RecAllowed.end(); p != p_end; ++p )
			sig.remove(*p);
		mod.extract ( Module.begin(), Module.end(), sig, M_STAR );
		if ( mod.getModule().size() == Module.size() )
		{
			Out.insert(sig);
			return;
		}
	}
	// need to try smaller sigs
	sig = StartSig;
	for ( p = RecAllowed.begin(), p_end = RecAllowed.end(); p != p_end; ++p )
	{
		sig.remove(*p);
		BuildAllSeedSigs ( RecAllowed, sig, Module, Out );
		sig.add(*p);
	}
}
