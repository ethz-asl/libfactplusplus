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

	forceReload();
}

