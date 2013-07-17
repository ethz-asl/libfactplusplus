/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2013 by Dmitry Tsarkov

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

// incremental reasoning implementation

#include "Kernel.h"
#include "OntologyBasedModularizer.h"
#include "Actor.h"
#include "tOntologyPrinterLISP.h"

/// setup Name2Sig for a given name C
AxiomVec
ReasoningKernel :: setupSig ( const ClassifiableEntry* C )
{
	AxiomVec ret;

	// get the entity; do nothing if doesn't exist
	const TNamedEntity* entity = C->getEntity();
	if ( entity == NULL )
		return ret;

	// prepare a place to update
	TSignature sig;
	NameSigMap::iterator insert = Name2Sig.find(C);
	if ( insert == Name2Sig.end() )
		insert = Name2Sig.insert(std::make_pair(C,&sig)).first;
	else
		delete insert->second;

	// calculate a module
	sig.add(entity);
	ret = getModExtractor(false)->getModule(sig,M_BOT);

	// perform update
	insert->second = new TSignature(getModExtractor(false)->getModularizer()->getSignature());

	return ret;
}

/// initialise the incremental bits on full reload
void
ReasoningKernel :: initIncremental ( void )
{
	delete ModSyn;
	ModSyn = NULL;
	// fill the module signatures of the concepts
	for ( TBox::c_const_iterator p = getTBox()->c_begin(), p_end = getTBox()->c_end(); p != p_end; ++p )
		setupSig(*p);

	getTBox()->setNameSigMap(&Name2Sig);
	// fill in ontology signature
	OntoSig = Ontology.getSignature();
}

void
ReasoningKernel :: doIncremental ( void )
{
	std::cout << "Incremental!\n";
	// re-set the modularizer to use updated ontology
	delete ModSyn;
	ModSyn = NULL;

	Taxonomy* tax = getCTaxonomy();
	std::cout << "Original Taxonomy:";
	tax->print(std::cout);
	getTBox()->ReloadTaxonomy();
	tax = getCTaxonomy();
	std::cout << "Reloaded Taxonomy:";
	tax->print(std::cout);
	std::cout.flush();

	std::set<const ClassifiableEntry*> MPlus, MMinus, MAll;

	// detect new- and old- signature elements
	TSignature NewSig = Ontology.getSignature();
	TSignature::BaseType RemovedEntities, AddedEntities;
	std::set_difference(OntoSig.begin(), OntoSig.end(), NewSig.begin(), NewSig.end(), inserter(RemovedEntities, RemovedEntities.begin()));
	std::set_difference(NewSig.begin(), NewSig.end(), OntoSig.begin(), OntoSig.end(), inserter(AddedEntities, AddedEntities.begin()));

	// deal with removed concepts
	TSignature::BaseType::iterator e, e_end;
	for ( e = RemovedEntities.begin(), e_end = RemovedEntities.end(); e != e_end; ++e )
		if ( const TConcept* C = dynamic_cast<const TConcept*>((*e)->getEntry()) )
		{
			// remove all links
			C->getTaxVertex()->remove();
			// update Name2Sig
			delete Name2Sig[C];
			Name2Sig.erase(C);
		}

	// deal with added concepts
	tax->deFinalise();
	for ( e = AddedEntities.begin(), e_end = AddedEntities.end(); e != e_end; ++e )
		if ( const TDLConceptName* cName = dynamic_cast<const TDLConceptName*>(*e) )
		{
			// register the name in TBox
			TreeDeleter TD(this->e(cName));
			TConcept* C = dynamic_cast<TConcept*>(cName->getEntry());
			// create sig for it
			setupSig(C);
			// init the taxonomy element
			TaxonomyVertex* cur = tax->getCurrent();
			cur->clear();
			cur->setSample(C);
			cur->addNeighbour ( /*upDirection=*/true, tax->getTopVertex() );
			tax->finishCurrentNode();
			std::cout << "Insert " << C->getName() << std::endl;
		}
	tax->finalise();
	OntoSig = NewSig;

	// fill in M^+ and M^- sets
	LocalityChecker* lc = getModExtractor(false)->getModularizer()->getLocalityChecker();
	TOntology::iterator p, nb = Ontology.beginUnprocessed(), ne = Ontology.end(), rb = Ontology.beginRetracted(), re = Ontology.endRetracted();
	TLISPOntologyPrinter pr(std::cout);
	for ( p = nb; p != ne; ++p )
	{
		std::cout << "Add:";
		(*p)->accept(pr);
	}
	for ( p = rb; p != re; ++p )
	{
		std::cout << "Del:";
		(*p)->accept(pr);
	}
	// TODO: add new sig here
	for ( NameSigMap::iterator p = Name2Sig.begin(), p_end = Name2Sig.end(); p != p_end; ++p )
	{
		lc->setSignatureValue(*p->second);
		for ( TOntology::iterator notProcessed = nb; notProcessed != ne; ++notProcessed )
			if ( !lc->local(*notProcessed) )
			{
				MPlus.insert(p->first);
				MAll.insert(p->first);
				std::cout << "Non-local NP axiom ";
				(*notProcessed)->accept(pr);
				std::cout << " wrt " << p->first->getName() << std::endl;
				break;
			}
		for ( TOntology::iterator retracted = rb; retracted != re; retracted++ )
			if ( !lc->local(*retracted) )
			{
				MMinus.insert(p->first);
				MAll.insert(p->first);
				std::cout << "Non-local RT axiom ";
				(*retracted)->accept(pr);
				std::cout << " wrt " << p->first->getName() << std::endl;
				break;
			}
	}

	std::cout << "Add/Del names Taxonomy:";
	tax->print(std::cout);
	for ( std::set<const ClassifiableEntry*>::iterator p = MAll.begin(), p_end = MAll.end(); p != p_end; ++p )
	{
		reclassifyNode ( (*p)->getTaxVertex(), MPlus.find(*p) != MPlus.end(), MMinus.find(*p) != MMinus.end() );
		tax->print(std::cout);
		std::cout.flush();
	}
	Ontology.setProcessed();
}

/// reclassify (incrementally) NODE wrt ADDED or REMOVED flags
void
ReasoningKernel :: reclassifyNode ( TaxonomyVertex* node, bool added, bool removed )
{
	const ClassifiableEntry* entry = node->getPrimer();
	const TNamedEntity* entity = entry->getEntity();
	std::cout << "Reclassify " << entity->getName() << std::endl;

	// update Name2Sig
	AxiomVec Module = setupSig(entry);
	const TSignature ModSig = getModExtractor(false)->getModularizer()->getSignature();

	// renew all signature-2-entry map
	std::map<const TNamedEntity*, TNamedEntry*> KeepMap;
	TSignature::iterator s, s_end = ModSig.end();
	for ( s = ModSig.begin(); s != s_end; s++ )
	{
		const TNamedEntity* entity = *s;
		KeepMap[entity] = entity->getEntry();
		const_cast<TNamedEntity*>(entity)->setEntry(NULL);
	}

	ReasoningKernel reasoner;
	std::cout << "Module: \n";
	TLISPOntologyPrinter pr(std::cout);
	for ( AxiomVec::iterator p = Module.begin(), p_end = Module.end(); p != p_end; ++p )
	{
		reasoner.getOntology().add(*p);
		(*p)->accept(pr);
	}
	// update top links
	node->removeLinks(/*upDirection=*/true);
	Actor actor;
	actor.needConcepts();
	reasoner.getSupConcepts ( static_cast<const TDLConceptName*>(entity), /*direct=*/true, actor );
	Actor::Array2D parents;
	actor.getFoundData(parents);
	for ( Actor::Array2D::iterator q = parents.begin(), q_end = parents.end(); q != q_end; ++q )
	{
		const ClassifiableEntry* parentCE = *q->begin();
		if ( parentCE == reasoner.getCTaxonomy()->getTopVertex()->getPrimer() )	// special case it
		{	// FIXME!! re-think after a proper taxonomy change
			node->addNeighbour ( /*upDirection=*/true, getCTaxonomy()->getTopVertex() );
			break;
		}
		// this CE is of the Reasoner
		const TNamedEntity* parent = parentCE->getEntity();
		// note that the entity maps to the Reasoner, so we need to use saved map
		const TNamedEntry* localNE = KeepMap[parent];
		std::cout << "Set parent " << localNE->getName() << std::endl;
		node->addNeighbour ( /*upDirection=*/true, dynamic_cast<const ClassifiableEntry*>(localNE)->getTaxVertex() );
	}
	// actually add node
	node->incorporate();
	// clear an ontology in a safe way
	reasoner.getOntology().safeClear();
	// restore all signature-2-entry map
	for ( s = ModSig.begin(); s != s_end; s++ )
		const_cast<TNamedEntity*>(*s)->setEntry(KeepMap[*s]);
}

