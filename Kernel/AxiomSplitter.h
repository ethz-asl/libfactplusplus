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
		typedef std::vector<TDLAxiom*> AxVec;
		const TDLConceptName* oldName, *newName;
		AxVec oldAxioms;
		TDLAxiom* newAxiom;
		TSignature newAxSig;
			/// set old axiom as an equivalent AX; create a new one
		void setEqAx ( TDLAxiomEquivalentConcepts* ax )
		{
			oldAxioms.push_back(ax);
			TDLAxiomEquivalentConcepts::ExpressionArray copy;
			for ( TDLAxiomEquivalentConcepts::iterator p = ax->begin(), p_end = ax->end(); p != p_end; ++p )
				if ( *p == oldName )
					copy.push_back(newName);
				else
					copy.push_back(*p);
			newAxiom = new TDLAxiomEquivalentConcepts(copy);
		}
			/// set a new implication axiom based on a (known) set of old ones
		void setImpAx ( const TDLConceptExpression* Desc )
		{
			newAxiom = new TDLAxiomConceptInclusion ( newName, Desc );
		}
			/// register the new axiom and retract the old one
		void Register ( TOntology* O )
		{
			for ( AxVec::iterator p = oldAxioms.begin(), p_end = oldAxioms.end(); p != p_end; ++p )
				O->retract(*p);
			O->add(newAxiom);
		}
			/// un-register record
		void Unregister ( void )
		{
			for ( AxVec::iterator p = oldAxioms.begin(), p_end = oldAxioms.end(); p != p_end; ++p )
				(*p)->setUsed(true);
			newAxiom->setUsed(false);
		}
	};

protected:	// members
	std::set<const TDLConceptName*> SubNames, Rejects;
	std::vector<TRecord*> Renames, R2;
	std::map<const TDLConceptName*, TRecord*> ImpRens;
	std::map<const TDLConceptName*, std::set<TDLAxiomConceptInclusion*> > ImplNames;
	TLISPOntologyPrinter pr;
	int newNameId;
	TModularizer mod;
	TSignature sig;	// seed signature
	TSignatureUpdater Updater;
	std::set<TSplitVar*> RejSplits;
	TOntology* O;

protected:	// methods
		/// rename old concept into a new one with a fresh name
	const TDLConceptName* rename ( const TDLConceptName* oldName )
	{
		std::stringstream s;
		s << oldName->getName() << "+" << ++newNameId;
		return dynamic_cast<const TDLConceptName*>(O->getExpressionManager()->Concept(s.str()));
	}
		/// create a signature of a module corresponding to a new axiom in record
	void buildSig ( TRecord* rec )
	{
		sig.clear();	// make sig a signature of a new axiom
		rec->newAxiom->accept(Updater);
		mod.extract ( *O, sig, M_STAR );	// build a module/signature for the axiom
//		std::cout << "Module for " << rec->oldName->getName() << ":\n";
//		for ( std::set<TDLAxiom*>::const_iterator z = module.begin(), z_end = module.end(); z != z_end; ++z )
//			(*z)->accept(pr);
		rec->newAxSig = mod.getSignature();	// FIXME!! check that SIG wouldn't change after some axiom retractions
//		std::cout << " with module size " << module.size() << "\n";
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
		/// make implication split for a given old NAME
	TRecord* makeImpSplit ( const TDLConceptName* oldName )
	{
		const TDLConceptName* newName = rename(oldName);
		std::cout << "split " << oldName->getName() << " into " << newName->getName() << "\n";
		TRecord* rec = new TRecord();
		rec->oldName = oldName;
		rec->newName = newName;
		O->getExpressionManager()->newArgList();
		for ( std::set<TDLAxiomConceptInclusion*>::iterator s = ImplNames[oldName].begin(), s_end = ImplNames[oldName].end(); s != s_end; ++s )
		{
			rec->oldAxioms.push_back(*s);
			O->getExpressionManager()->addArg((*s)->getSupC());
			(*s)->accept(pr);
		}
		rec->setImpAx(O->getExpressionManager()->And());
		buildSig(rec);
		rec->Register(O);
		rec->newAxiom->accept(pr);
		return rec;
	}
		/// get imp record of a given name; create if necessary
	TRecord* getImpRec ( const TDLConceptName* oldName )
	{
		if ( ImpRens.find(oldName) == ImpRens.end() )
			ImpRens[oldName] = makeImpSplit(oldName);
		return ImpRens[oldName];
	}
		/// check whether the record is independent wrt modularity
	void checkSplitCorrectness ( TRecord* rec )
	{
		if ( Rejects.count(rec->oldName) > 0 )
		{
		unsplit:	// restore the old axiom, get rid of the new one
			rec->Unregister();
			std::cout << "unsplit " << rec->oldName->getName() << "\n";
			delete rec;
			return;
		}

		TRecord* imp = getImpRec(rec->oldName);
		buildSig(rec);
		if ( rec->newAxSig.contains(static_cast<const TNamedEntity*>(rec->oldName)) )
//			|| !intersect(rec->newAxSig, imp->newAxSig).empty() )
		{
			// mark name as rejected, un-register imp
			Rejects.insert(rec->oldName);
			imp->Unregister();
			goto unsplit;
		}
		else	// keep the split
		{
			R2.push_back(rec);
			std::cout << "keep split " << rec->oldName->getName() << "\n";
		}
	}
		/// move all independent splits in R2; delete all the rest
	void keepIndependentSplits ( void )
	{
		for ( std::vector<TRecord*>::iterator r = Renames.begin(), r_end = Renames.end(); r != r_end; ++r )
			checkSplitCorrectness(*r);
		std::cout << "There were made " << R2.size() << " splits out of " << Renames.size() << " tries\n";
	}
		/// split all implications corresponding to oldName; @return split pointer
	TSplitVar* splitImplicationsFor ( const TDLConceptName* oldName )
	{
		// check whether we already did translation for such a name
		if ( O->Splits.hasCN(oldName) )
			return O->Splits.get(oldName);

		TRecord* rec = getImpRec(oldName);
		// create new split
		TSplitVar* split = new TSplitVar();
		split->oldName = oldName;
		split->splitNames.push_back(rec->newName);
		split->Sigs.push_back(rec->newAxSig);
		O->Splits.set ( oldName, split );
		return split;
	}
		/// split all implications for which equivalences were split as well
	void splitImplications ( void )
	{
		Renames.swap(R2);
		R2.clear();
		for ( std::vector<TRecord*>::iterator r = Renames.begin(), r_end = Renames.end(); r != r_end; ++r )
			if ( Rejects.count((*r)->oldName) == 0 )
			{
				TSplitVar* split = splitImplicationsFor((*r)->oldName);
				split->splitNames.push_back((*r)->newName);
				split->Sigs.push_back((*r)->newAxSig);
			}
			else
				(*r)->Unregister();
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

