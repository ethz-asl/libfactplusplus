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

/// initialise the incremental bits on full reload
void
ReasoningKernel :: initIncremental ( void )
{
	OntologyBasedModularizer* ModExtractor = getModExtractor(false);
	// fill the module signatures of the concepts
	for ( TBox::c_const_iterator p = getTBox()->c_begin(), p_end = getTBox()->c_end(); p != p_end; ++p )
	{
		const TNamedEntity* entity = (*p)->getEntity();
		if ( entity == NULL )
			continue;
		TSignature sig;
		sig.add(entity);
		ModExtractor->getModule(sig,M_BOT);
		Name2Sig[*p] = new TSignature(ModExtractor->getModularizer()->getSignature());
	}
	getTBox()->setNameSigMap(&Name2Sig);
}

void
ReasoningKernel :: doIncremental ( void )
{
	std::cout << "Incremental!\n";
	// re-set the modularizer to use updated ontology
	delete ModSyn;
	ModSyn = NULL;
	// fill in M^+ and M^- sets
	LocalityChecker* lc = getModExtractor(false)->getModularizer()->getLocalityChecker();
	TOntology::iterator nb = Ontology.beginUnprocessed(), ne = Ontology.end(), rb = Ontology.beginRetracted(), re = Ontology.endRetracted();
	TLISPOntologyPrinter pr(std::cout);
	if ( nb != ne )
		(*nb)->accept(pr);
	if ( rb != re )
		(*rb)->accept(pr);
	// TODO: add new sig here
	std::set<const ClassifiableEntry*> MPlus, MMinus;
	for ( NameSigMap::iterator p = Name2Sig.begin(), p_end = Name2Sig.end(); p != p_end; ++p )
	{
		lc->setSignatureValue(*p->second);
		for ( TOntology::iterator notProcessed = nb; notProcessed != ne; ++notProcessed )
			if ( !lc->local(*notProcessed) )
			{
				MPlus.insert(p->first);
				std::cout << "Non-local NP axiom ";
				(*notProcessed)->accept(pr);
				std::cout << " wrt " << p->first->getName() << std::endl;
				break;
			}
		for ( TOntology::iterator retracted = rb; retracted != re; retracted++ )
			if ( !lc->local(*retracted) )
			{
				MMinus.insert(p->first);
				std::cout << "Non-local RT axiom ";
				(*retracted)->accept(pr);
				std::cout << " wrt " << p->first->getName() << std::endl;
				break;
			}
	}

	// fill in an order to
	std::queue<TaxonomyVertex*> queue;
	std::vector<TaxonomyVertex*> toProcess;
	Taxonomy* tax = getCTaxonomy();
	queue.push(tax->getTopVertex());
	while ( !queue.empty() )
	{
		TaxonomyVertex* cur = queue.front();
		queue.pop();
		if ( tax->isVisited(cur) )
			continue;
		tax->setVisited(cur);
		const ClassifiableEntry* entry = cur->getPrimer();
		if ( MPlus.find(entry) != MPlus.end() || MMinus.find(entry) != MMinus.end() )
			toProcess.push_back(cur);
		for ( TaxonomyVertex::iterator p = cur->begin(/*upDirection=*/false), p_end = cur->end(/*upDirection=*/false); p != p_end; ++p )
			queue.push(*p);
	}
	tax->clearVisited();

	for ( std::vector<TaxonomyVertex*>::iterator p = toProcess.begin(), p_end = toProcess.end(); p != p_end; ++p )
	{
		const ClassifiableEntry* entry = (*p)->getPrimer();
		reclassifyNode ( *p, MPlus.find(entry) != MPlus.end(), MMinus.find(entry) != MMinus.end() );
	}
	Ontology.setProcessed();
}

class ConceptActor: public Actor
{
public:		// types
	typedef Actor::SynVector SynVector;
	typedef Actor::SetOfNodes SetOfNodes;

public:		// interface
		/// init c'tor
	ConceptActor ( void ) { needConcepts(); }
		/// get RO access to found nodes
	const SetOfNodes& getNodes ( void ) const { return acc; }
}; // ConceptActor

/// reclassify (incrementally) NODE wrt ADDED or REMOVED flags
void
ReasoningKernel::reclassifyNode ( TaxonomyVertex* node, bool added, bool removed )
{
	const ClassifiableEntry* entry = node->getPrimer();
	const TNamedEntity* entity = entry->getEntity();
	std::cout << "Reclassify " << entity->getName() << std::endl;
	TSignature sig;
	sig.add(entity);
	AxiomVec Module = getModExtractor(false)->getModule(sig, M_BOT);
	// update Name2Sig
	const TSignature ModSig = getModExtractor(false)->getModularizer()->getSignature();
	Name2Sig[entry] = new TSignature(ModSig);
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
	node->clearLinks(/*upDirection=*/true);
	ConceptActor actor;
	reasoner.getSupConcepts ( static_cast<const TDLConceptName*>(entity), /*direct=*/true, actor );
	ConceptActor::SetOfNodes parents = actor.getNodes();
	for ( ConceptActor::SetOfNodes::iterator q = parents.begin(), q_end = parents.end(); q != q_end; ++q )
	{
		const ClassifiableEntry* parentCE = *q->begin();
		// this CE is of the Reasoner
		const TNamedEntity* parent = parentCE->getEntity();
		// note that the entity maps to the Reasoner, so we need to use saved map
		const TNamedEntry* localNE = KeepMap[parent];
		std::cout << "Set parent " << localNE->getName() << std::endl;
		node->addNeighbour ( /*upDirection=*/true, dynamic_cast<const ClassifiableEntry*>(localNE)->getTaxVertex() );
	}
	// clear an ontology in a safe way
	reasoner.getOntology().safeClear();
	// restore all signature-2-entry map
	for ( s = ModSig.begin(); s != s_end; s++ )
		const_cast<TNamedEntity*>(*s)->setEntry(KeepMap[*s]);
}

