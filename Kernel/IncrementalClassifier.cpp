/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2013 by Dmitry Tsarkov

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

/*******************************************************\
|* Implementation of taxonomy building for the FaCT++  *|
\*******************************************************/

#include "Reasoner.h"
#include "IncrementalClassifier.h"
#include "procTimer.h"
#include "globaldef.h"
#include "logging.h"

/********************************************************\
|* 			Implementation of class Taxonomy			*|
\********************************************************/

bool IncrementalClassifier :: testSub ( const TConcept* q )
{
	fpp_assert ( q != NULL );

	if ( q->isTop() )
		return true;

	if ( q->isSingleton()		// singleton on the RHS is useless iff...
		 && q->isPrimitive()	// it is primitive
		 && !q->isNominal() )	// nominals should be classified as usual concepts
		return false;

	if ( LLM.isWritable(llTaxTrying) )
		LL << "\nTAX: trying '" << Current->getName() << "' [= '" << q->getName() << "'... ";

//	std::cout << "testing " << Current->getName() << " [= " << q->getName() << "... ";

	if ( isNotInModule(q->getEntity()) )
	{
		if ( LLM.isWritable(llTaxTrying) )
			LL << "NOT holds (module result)";
//		std::cout << " not in module" << std::endl;

		++nModuleNegative;
		return false;
	}

	bool ret = testSubTBox(q);
//	std::cout << ret << std::endl;
	return ret;
}

void IncrementalClassifier :: print ( std::ostream& o ) const
{
	o << "Totally " << nTries << " subsumption tests was made\nAmong them ";

	unsigned int n = ( nTries ? nTries : 1 );

	o << nPositives << " (" << (unsigned long)(nPositives*100/n) << "%) successful\n";
	if ( nModuleNegative )
		o << "Modular reasoning deals with " << nModuleNegative << " non-subsumptions\n";
	o << "There were made " << nSearchCalls << " search calls\nThere were made " << nSubCalls
	  << " Sub calls, of which " << nNonTrivialSubCalls << " non-trivial\n";
	o << "Current efficiency (wrt Brute-force) is " << nEntries*(nEntries-1)/n << "\n";

	TaxonomyCreator::print(o);
}

// Baader procedures
void
IncrementalClassifier :: searchBaader ( TaxonomyVertex* cur )
{
	// label 'visited'
	pTax->setVisited(cur);
//	std::cout << "visiting " << cur->getPrimer()->getName() << std::endl;


	++nSearchCalls;
	bool noPosSucc = true;

	// check if there are positive successors; use DFS on them.
	for ( TaxonomyVertex::iterator p = cur->begin(upDirection), p_end = cur->end(upDirection); p != p_end; ++p )
		if ( enhancedSubs(*p) )
		{
			if ( !pTax->isVisited(*p) )
				searchBaader(*p);

			noPosSucc = false;
		}

	// in case current node is unchecked (no BOTTOM node) -- check it explicitly
	if ( !isValued(cur) )
		setValue ( cur, testSubsumption(cur) );

	// mark labelled leaf node as a parent
	if ( noPosSucc && cur->getValue() )
		curNode->addNeighbour ( !upDirection, cur );
}

bool
IncrementalClassifier :: enhancedSubs1 ( TaxonomyVertex* cur )
{
	++nNonTrivialSubCalls;

	// need to be valued -- check all parents
	// propagate false
	for ( TaxonomyVertex::iterator p = cur->begin(!upDirection), p_end = cur->end(!upDirection); p != p_end; ++p )
		if ( !enhancedSubs(*p) )
			return false;

	// all subsumptions holds -- test current for subsumption
	return testSubsumption(cur);
}

void 		/// fill candidates
IncrementalClassifier :: fillCandidates ( TaxonomyVertex* cur )
{
//	std::cout << "fill candidates: " << cur->getPrimer()->getName() << std::endl;
	if ( isValued(cur) )
	{
		if ( getValue(cur) )	// positive value -- nothing to do
			return;
	}
	else
		candidates.insert(cur);

	for ( TaxonomyVertex::iterator p = cur->begin(true), p_end = cur->end(true); p != p_end; ++p )
		fillCandidates(*p);
}

void
IncrementalClassifier :: reclassify ( TaxonomyVertex* node, ReasoningKernel* r, const TSignature* s, bool added, bool removed )
{
	reasoner = r;
	sig = s;
	curNode = node;
	Current = getCName(node->getPrimer());
//	std::cout << "in reclassify " << std::endl;

	// FIXME!! check the unsatisfiability later

	fpp_assert ( added || removed );
	typedef std::vector<TaxonomyVertex*> TVArray;
	clearLabels();

	if ( node->noNeighbours(true) )
		node->addNeighbour(true, pTax->getTopVertex());

	// we use candidates set if nothing was added (so no need to look further from current subs)
	useCandidates = !added;
	candidates.clear();
	if ( removed )	// re-check all parents
	{
//		std::cout << "in removed " << std::endl;
		TVArray pos, neg;
		for ( TaxonomyVertex::iterator p = node->begin(true), p_end = node->end(true); p != p_end; ++p )
		{
//			std::cout << "check parent " << (*p)->getPrimer()->getName() << std::endl;
			bool sub = testSubsumption(*p);
			setValue ( *p, sub );
			if ( sub )
			{
				pos.push_back(*p);
				propagateTrueUp(*p);
			}
			else
				neg.push_back(*p);
		}
		node->removeLinks(true);
		for ( TVArray::iterator q = pos.begin(), q_end = pos.end(); q != q_end; ++q )
			node->addNeighbour(true, *q);
		if ( useCandidates )
			for ( TVArray::iterator q = neg.begin(), q_end = neg.end(); q != q_end; ++q )
				fillCandidates(*q);
	}
	else	// all parents are there
	{
		for ( TaxonomyVertex::iterator p = node->begin(true), p_end = node->end(true); p != p_end; ++p )
		{
			setValue ( *p, true );
			propagateTrueUp(*p);
		}
		node->removeLinks(true);
	}

	// FIXME!! for now. later check the equivalence etc
	setValue ( node, false );
//	std::cout << "do Baader" << std::endl;
	// the landscape is prepared
	searchBaader(pTax->getTopVertex());
//	std::cout << "Incorporate" << std::endl;
	curNode->incorporate();
//	std::cout << "done" << std::endl;
	clearLabels();
}
