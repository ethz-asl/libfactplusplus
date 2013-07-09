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

void
ReasoningKernel :: doIncremental ( void )
{
	// fill in M^+ and M^- sets
	LocalityChecker* lc = getModExtractor(false)->getModularizer()->getLocalityChecker();
	TOntology::iterator nb = Ontology.beginUnprocessed(), ne = Ontology.end(), rb = Ontology.beginRetracted(), re = Ontology.endRetracted();
	// TODO: add new sig here
	std::set<const ClassifiableEntry*> MPlus, MMinus;
	for ( NameSigMap::iterator p = Name2Sig.begin(), p_end = Name2Sig.end(); p != p_end; ++p )
	{
		lc->setSignatureValue(*p->second);
		for ( TOntology::iterator notProcessed = nb; notProcessed != ne; ++notProcessed )
			if ( !lc->local(*notProcessed) )
			{
				MPlus.insert(p->first);
				break;
			}
		for ( TOntology::iterator retracted = rb; retracted != re; retracted++ )
			if ( !lc->local(*retracted) )
			{
				MMinus.insert(p->first);
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
//	forceReload();
}

class MyActor: public Actor
{
public:
	typedef Actor::SynVector SynVector;
	typedef Actor::SetOfNodes SetOfNodes;
	MyActor(){ needConcepts(); }
	const SetOfNodes& getNodes ( void ) const { return acc; }
}; // MyActor

/// reclassify (incrementally) NODE wrt ADDED or REMOVED flags
void
ReasoningKernel::reclassifyNode ( TaxonomyVertex* node, bool added, bool removed )
{
	const ClassifiableEntry* entry = node->getPrimer();
	const TNamedEntity* entity = entry->getEntity();
	TSignature sig;
	sig.add(entity);
	AxiomVec Module = getModExtractor(false)->getModule(sig, M_BOT);
	// update Name2Sig
	Name2Sig[entry] = new TSignature(getModExtractor(false)->getModularizer()->getSignature());
	ReasoningKernel reasoner;
	for ( AxiomVec::iterator p = Module.begin(), p_end = Module.end(); p != p_end; ++p )
		reasoner.getOntology().add(*p);
	// update top links
	node->clearLinks(/*upDirection=*/true);
	MyActor actor;
	reasoner.getSupConcepts ( static_cast<const TDLConceptName*>(entity), /*direct=*/true, actor );
	MyActor::SetOfNodes parents = actor.getNodes();
	for ( MyActor::SetOfNodes::iterator q = parents.begin(), q_end = parents.end(); q != q_end; ++q )
		node->addNeighbour ( /*upDirection=*/true, (*q->begin())->getTaxVertex() );
	// clear an ontology FIXME!! later
//	reasoner.getOntology().clear();
}

