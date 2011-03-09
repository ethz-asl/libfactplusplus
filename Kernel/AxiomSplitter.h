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

#ifndef AXIOMSPLITTER_H
#define AXIOMSPLITTER_H

#include "Modularity.h"
#include <sstream>

class TAxiomSplitter
{
protected:	// types
		/// keep the single rename: named concept C in an axiom (C=D or C[=D) into a new name C' and new axiom C'=D or C'[=D
	struct TRecord
	{
		const TDLConceptName* oldName, *newName;
		TDLAxiom* oldAxiom, *newAxiom;
		TSignature newAxSig;
			/// set old axiom as an equivalent AX; create a new one
		void setEqAx ( TDLAxiomEquivalentConcepts* ax )
		{
			oldAxiom = ax;
			TDLAxiomEquivalentConcepts::ExpressionArray copy;
			for ( TDLAxiomEquivalentConcepts::iterator p = ax->begin(), p_end = ax->end(); p != p_end; ++p )
				if ( *p == oldName )
					copy.push_back(newName);
				else
					copy.push_back(*p);
			newAxiom = new TDLAxiomEquivalentConcepts(copy);
		}
			/// set old axiom as an implication AX; create a new one
		void setImpAx ( TDLAxiomConceptInclusion* ax )
		{
			oldAxiom = ax;
			newAxiom = new TDLAxiomConceptInclusion ( newName, ax->getSupC() );
		}
			/// register the new axiom and retract the old one
		void Register ( TOntology* O )
		{
			O->retract(oldAxiom);
			O->add(newAxiom);
		}
	};

protected:	// members
	std::set<const TDLConceptName*> SubNames;
	std::vector<TRecord*> Renames, R2;
	std::map<TDLAxiom*, TDLAxiom*> Replacement;
	std::map<const TDLConceptName*, std::set<TDLAxiomConceptInclusion*> > ImplNames;
	std::map<const TDLConceptName*, TRecord*> Introduction;
	TLISPOntologyPrinter pr;
	int newNameId;
	TModularizer mod;
	TSignature sig;	// seed signature
	TSignatureUpdater Updater;
	TOntology* O;

protected:	// methods
		/// rename old concept into a new one with a fresh name
	const TDLConceptName* rename ( const TDLConceptName* oldName )
	{
		std::stringstream s;
		s << oldName->getName() << "+" << ++newNameId;
		return dynamic_cast<const TDLConceptName*>(O->getExpressionManager()->Concept(s.str()));
	}
		/// add axiom CI in a form C [= D for D != TOP
	void addSingleCI ( TDLAxiomConceptInclusion* ci )
	{
		if ( ci != NULL && dynamic_cast<const TDLConceptTop*>(ci->getSupC()) == NULL )
		{	// skip axioms with RHS=TOP
			const TDLConceptName* name = dynamic_cast<const TDLConceptName*>(ci->getSubC());
			if ( name != NULL )
			{
				SubNames.insert(name);
				ImplNames[name].insert(ci);
			}
		}
	}
		/// register all axioms in a form C [= D
	void registerCIs ( void )
	{
		// FIXME!! check for the case (not D) [= (not C) later
		// FIXME!! disjoints here as well
		for ( TOntology::iterator p = O->begin(), p_end = O->end(); p != p_end; ++p )
			if ( (*p)->isUsed() )
				addSingleCI(dynamic_cast<TDLAxiomConceptInclusion*>(*p));
	}
		/// check whether an equivalent axiom is splittable; @return split name or NULL if not splittable
	const TDLConceptName* getEqSplit ( TDLAxiomEquivalentConcepts* ce )
	{
		// check whether it is not a synonym definition
		const TDLConceptName* splitName = NULL, *name;
		size_t size = ce->size();
		for ( TDLAxiomEquivalentConcepts::iterator q = ce->begin(), q_end = ce->end(); q != q_end; ++q )
			if ( (name = dynamic_cast<const TDLConceptName*>(*q)) != NULL )
			{
				if ( SubNames.count(name) > 0 )
				{	// found a split candidate; save the name
					if ( splitName == NULL )
						splitName = name;
					else	// more than one split candidates: do the split right now
							// FIXME!! now we jump out right now, later on we're going to do the same with changed axiom
						return splitName;
				}
				else
					--size;
			}
		return size > 1 ? splitName : NULL;
	}
		/// make the axiom split for the equivalence axiom
	void makeEqSplit ( TDLAxiomEquivalentConcepts* ce )
	{
		if ( ce == NULL )
			return;
		const TDLConceptName* splitName = getEqSplit(ce);
		if ( splitName == NULL )
			return;
		// create new record
		TRecord* rec = new TRecord();
		rec->oldName = splitName;
		rec->newName = rename(splitName);
		rec->setEqAx(ce);
		rec->Register(O);
		// register rec
		Renames.push_back(rec);
		std::cout << "split " << splitName->getName() << " into " << rec->newName->getName() << "\n";
		ce->accept(pr); rec->newAxiom->accept(pr);
	}
		/// split all possible EQ axioms
	void registerEQ ( void )
	{
		// use index instead of iterators will be invalidated during additions
		for ( size_t i = 0; i < O->size(); ++i )
			if ( (*O)[i]->isUsed() )
				makeEqSplit(dynamic_cast<TDLAxiomEquivalentConcepts*>((*O)[i]));
	}
		/// check whether the record is independent wrt modularity
	void checkSplitCorrectness ( TRecord* rec )
	{
		sig.clear();	// make sig a signature of a new axiom
		rec->newAxiom->accept(Updater);
//		O->clearModuleInfo();
		mod.extract ( *O, sig, M_STAR );	// build a module/signature for the axiom
//		std::cout << "Module for " << rec->oldName->getName() << ":\n";
//		for ( std::set<TDLAxiom*>::const_iterator z = module.begin(), z_end = module.end(); z != z_end; ++z )
//			(*z)->accept(pr);
		if ( mod.getSignature().contains(static_cast<const TNamedEntity*>(rec->oldName)) )
		{
			// restore the old axiom, get rid of the new one
			rec->oldAxiom->setUsed(true);
			rec->newAxiom->setUsed(false);
			std::cout << "unsplit " << rec->oldName->getName();
			delete rec;
		}
		else	// keep the split
		{
			rec->newAxSig = mod.getSignature();	// FIXME!! check that SIG wouldn't change after some axiom retractions
			R2.push_back(rec);
			Introduction[rec->newName] = rec;
			std::cout << "keep split " << rec->oldName->getName();
		}
//		std::cout << " with module size " << module.size() << "\n";
	}
		/// move all independent splits in R2; delete all the rest
	void keepIndependentSplits ( void )
	{
		for ( std::vector<TRecord*>::iterator r = Renames.begin(), r_end = Renames.end(); r != r_end; ++r )
			checkSplitCorrectness(*r);
		std::cout << "There were made " << R2.size() << " splits out of " << Renames.size() << " tries\n";
	}
		/// make the axiom split for the equivalence axiom
	void makeImpSplit ( TDLAxiomConceptInclusion* ci, const TDLConceptName* oldName, const TDLConceptName* newName )
	{
		TRecord* rec = new TRecord();
		rec->oldName = oldName;
		rec->newName = newName;
		rec->setImpAx(ci);
		rec->Register(O);
		R2.push_back(rec);
		ci->accept(pr); rec->newAxiom->accept(pr);
		rec->newAxiom->accept(Updater);	// keep the signature of a new item
	}
		/// split all implications corresponding to oldName; @return split pointer
	TSplitVar* splitImplicationsFor ( const TDLConceptName* oldName )
	{
		// check whether we already did translation for such a name
		if ( O->Splits.hasCN(oldName) )
			return O->Splits.get(oldName);
		const TDLConceptName* newName = rename(oldName);
		// build signatures
		sig.clear();
		size_t i = R2.size();
		std::cout << "split " << oldName->getName() << " into " << newName->getName() << "\n";
		for ( std::set<TDLAxiomConceptInclusion*>::iterator s = ImplNames[oldName].begin(), s_end = ImplNames[oldName].end(); s != s_end; ++s )
			makeImpSplit ( *s, oldName, newName );
		// create module for newName and put the extended signature
		mod.extract ( *O, sig, M_STAR );	// build a module/signature for the axiom
		for ( ; i < R2.size(); ++i )
			R2[i]->newAxSig = mod.getSignature();
		// create new split
		TSplitVar* split = new TSplitVar();
		split->oldName = oldName;
		split->splitNames.push_back(newName);
		O->Splits.set ( oldName, split );
		return split;
	}
		/// split all implications for which equivalences were split as well
	void splitImplications ( void )
	{
		Renames.swap(R2);
		R2.clear();
		for ( std::vector<TRecord*>::iterator r = Renames.begin(), r_end = Renames.end(); r != r_end; ++r )
			splitImplicationsFor((*r)->oldName)->splitNames.push_back((*r)->newName);
	}

public:		// interaface
	TAxiomSplitter ( TOntology* o ) : pr(std::cout), newNameId(0), Updater(sig), O(o) {}
	void buildSplit ( void )
	{
		// first make a set of named concepts C s.t. C [= D is in the ontology
		registerCIs();
		// now check if some of the C's contains in an equivalence axioms
		registerEQ();
		if ( Renames.size() == 0 )
			return;
		// here we have a maximal split; check whether modules are fine
		keepIndependentSplits();
		// now R2 contains all separated axioms; make one replacement for every C [= D axiom
		splitImplications();
	}
}; // TAxiomSplitter

#endif

