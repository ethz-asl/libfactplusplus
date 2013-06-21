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
#include "DLConceptTaxonomy.h"
#include "procTimer.h"
#include "globaldef.h"
#include "logging.h"

/********************************************************\
|* 			Implementation of class Taxonomy			*|
\********************************************************/

bool DLConceptTaxonomy :: testSub ( const TConcept* p, const TConcept* q )
{
	fpp_assert ( p != NULL );
	fpp_assert ( q != NULL );

	if ( q->isSingleton()		// singleton on the RHS is useless iff...
		 && q->isPrimitive()	// it is primitive
		 && !q->isNominal() )	// nominals should be classified as usual concepts
		return false;

	if ( unlikely(inSplitCheck) )
	{
		if ( q->isPrimitive() )	// only defined ones in split checks
			return false;
		return testSubTBox ( p, q );
	}

	if ( LLM.isWritable(llTaxTrying) )
		LL << "\nTAX: trying '" << p->getName() << "' [= '" << q->getName() << "'... ";

	if ( tBox.testSortedNonSubsumption ( p, q ) )
	{
		if ( LLM.isWritable(llTaxTrying) )
			LL << "NOT holds (sorted result)";

		++nSortedNegative;
		return false;
	}

	if ( isNotInModule(q->getEntity()) )
	{
		if ( LLM.isWritable(llTaxTrying) )
			LL << "NOT holds (module result)";

		++nModuleNegative;
		return false;
	}

	switch ( tBox.testCachedNonSubsumption ( p, q ) )
	{
	case csValid:	// cached result: satisfiable => non-subsumption
		if ( LLM.isWritable(llTaxTrying) )
			LL << "NOT holds (cached result)";

		++nCachedNegative;
		return false;

	case csInvalid:	// cached result: unsatisfiable => subsumption holds
		if ( LLM.isWritable(llTaxTrying) )
			LL << "holds (cached result)";

		++nCachedPositive;
		return true;

	default:		// need extra tests
		if ( LLM.isWritable(llTaxTrying) )
			LL << "wasted cache test";

		break;
	}

	return testSubTBox ( p, q );
}

bool
DLConceptTaxonomy :: isNotInModule ( const TNamedEntity* entity ) const
{
	if ( upDirection )	// bottom-up phase
		return false;
	const TSignature* sig = sigStack.top();
	if ( sig && entity && !sig->contains(entity) )
		return true;
	return false;
}

TaxonomyCreator::KnownSubsumers*
DLConceptTaxonomy :: buildKnownSubsumers ( ClassifiableEntry* ce )
{
	return TaxonomyCreator::buildKnownSubsumers(ce);
}

/// prepare signature for given entry
TSignature*
DLConceptTaxonomy :: buildSignature ( ClassifiableEntry* p )
{
	if ( tBox.pName2Sig == NULL )
		return NULL;
	TBox::NameSigMap::iterator found = tBox.pName2Sig->find(p);
	if ( found == tBox.pName2Sig->end() )
		return NULL;
	return found->second;
}

void DLConceptTaxonomy :: print ( std::ostream& o ) const
{
	o << "Totally " << nTries << " subsumption tests was made\nAmong them ";

	unsigned int n = ( nTries ? nTries : 1 );

	o << nPositives << " (" << (unsigned long)(nPositives*100/n) << "%) successfull\n";
	o << "Besides that " << nCachedPositive << " successfull and " << nCachedNegative
	  << " unsuccessfull subsumption tests were cached\n";
	if ( nSortedNegative )
		o << "Sorted reasoning deals with " << nSortedNegative << " non-subsumptions\n";
	if ( nModuleNegative )
		o << "Modular reasoning deals with " << nModuleNegative << " non-subsumptions\n";
	o << "There were made " << nSearchCalls << " search calls\nThere were made " << nSubCalls
	  << " Sub calls, of which " << nNonTrivialSubCalls << " non-trivial\n";
	o << "Current efficiency (wrt Brute-force) is " << nEntries*(nEntries-1)/n << "\n";

	TaxonomyCreator::print(o);
}

// Baader procedures
void
DLConceptTaxonomy :: searchBaader ( TaxonomyVertex* cur )
{
	// label 'visited'
	pTax->setVisited(cur);

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

	// in case current node is unchecked (no BOTTOM node) -- check it explicitely
	if ( !isValued(cur) )
		setValue ( cur, testSubsumption(cur) );

	// mark labelled leaf node as a parent
	if ( noPosSucc && cur->getValue() )
		pTax->getCurrent()->addNeighbour ( !upDirection, cur );
}

bool
DLConceptTaxonomy :: enhancedSubs1 ( TaxonomyVertex* cur )
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

bool
DLConceptTaxonomy :: testSubsumption ( TaxonomyVertex* cur )
{
	const TConcept* testC = static_cast<const TConcept*>(cur->getPrimer());
	if ( upDirection )
		return testSub ( testC, curConcept() );
	else
		return testSub ( curConcept(), testC );
}

void
DLConceptTaxonomy :: propagateOneCommon ( TaxonomyVertex* node )
{
	// checked if node already was visited this session
	if ( pTax->isVisited(node) )
		return;

	// mark node visited
	pTax->setVisited(node);
	node->setCommon();
	if ( node->correctCommon(nCommon) )
		Common.push_back(node);

	// mark all children
	for ( TaxonomyVertex::iterator p = node->begin(/*upDirection=*/false), p_end = node->end(/*upDirection=*/false); p != p_end; ++p )
		propagateOneCommon(*p);
}

bool DLConceptTaxonomy :: propagateUp ( void )
{
	// including node always have some parents (TOP at least)
	TaxonomyVertex* Current = pTax->getCurrent();
	TaxonomyVertex::iterator p = Current->begin(/*upDirection=*/true), p_end = Current->end(/*upDirection=*/true);
	fpp_assert ( p != p_end );	// there is at least one parent (TOP)

	TaxVertexVec aux;	// aux set for the verteces in ...
	nCommon = 1;	// number of common parents
	clearCommon();

	// define possible successors of the node
	propagateOneCommon(*p);
	pTax->clearVisited();

	for ( ++p; p != p_end; ++p )
	{
		if ( (*p)->noNeighbours(/*upDirection=*/false) )
			return true;
		if ( Common.empty() )
			return true;

		++nCommon;
		// now Aux contain data from previous run
		aux.swap(Common);
		Common.clear();
		propagateOneCommon(*p);
		pTax->clearVisited();

		// clear all non-common nodes (visited on a previous run)
		for ( TaxVertexVec::iterator q = aux.begin(), q_end = aux.end(); q < q_end; ++q )
			(*q)->correctCommon(nCommon);
	}

	return false;
}

/// check if no BU classification is required as C=TOP
bool
DLConceptTaxonomy :: isEqualToTop ( void )
{
	// check this up-front to avoid Sorted check's flaw wrt equals-to-top
	const modelCacheInterface* cache = tBox.initCache ( curConcept(), /*sub=*/true );
	if ( cache->getState() != csInvalid )
		return false;
	// here concept = TOP
	pTax->getCurrent()->addNeighbour ( /*upDirection=*/false, pTax->getTopVertex() );
	return true;
}

/// @return true iff curEntry is classified as a synonym
bool
DLConceptTaxonomy :: classifySynonym ( void )
{
	if ( TaxonomyCreator::classifySynonym() )
		return true;

	if ( curConcept()->isSingleton() )
	{
		TIndividual* curI = (TIndividual*)const_cast<TConcept*>(curConcept());

		if ( unlikely(tBox.isBlockedInd(curI)) )
		{	// check whether current entry is the same as another individual
			TIndividual* syn = tBox.getBlockingInd(curI);
			fpp_assert ( syn->getTaxVertex() != NULL );

 			if ( tBox.isBlockingDet(curI) )
			{	// deterministic merge => curI = syn
 				pTax->addCurrentToSynonym(syn->getTaxVertex());
				return true;
			}
			else	// non-det merge: check whether it is the same
			{
				if ( LLM.isWritable(llTaxTrying) )
					LL << "\nTAX: trying '" << curI->getName() << "' = '" << syn->getName() << "'... ";
				if ( testSubTBox ( curI, syn ) )	// they are actually the same
				{
					pTax->addCurrentToSynonym(syn->getTaxVertex());
					return true;
				}
			}
		}
	}

	return false;
}

void
DLConceptTaxonomy :: checkExtraParents ( void )
{
	inSplitCheck = true;
	TaxonomyVertex::iterator p, p_end;
	TaxonomyVertex* Current = pTax->getCurrent();
	for ( p = Current->begin(/*upDirection=*/true), p_end = Current->end(/*upDirection=*/true); p != p_end; ++p )
		propagateTrueUp(*p);
	Current->clearLinks(/*upDirection=*/true);
	runTopDown();
	// save all indirect parents to remove them later (to avoid iterator invalidation)
	TaxVertexVec vec;
	for ( p = Current->begin(/*upDirection=*/true), p_end = Current->end(/*upDirection=*/true); p != p_end; ++p )
		if ( !isDirectParent(*p) )
			vec.push_back(*p);
	// now it is time to remove links
	for ( TaxVertexVec::iterator q = vec.begin(), q_end = vec.end(); q != q_end; ++q )
	{
		(*q)->removeLink ( /*upDirection=*/false, Current );
		Current->removeLink ( /*upDirection=*/true, *q );
	}

	clearLabels();
	inSplitCheck = false;
}

/// merge vars came from a given SPLIT together
void
DLConceptTaxonomy :: mergeSplitVars ( TSplitVar* split )
{
	typedef std::set<TaxonomyVertex*> TVSet1;

	TVSet1 splitVertices;
	TaxonomyVertex* v = split->C->getTaxVertex();
	bool cIn = ( v != NULL );
	if ( cIn )	// there is C-node in the taxonomy
		splitVertices.insert(v);
	TSplitVar::iterator q = split->begin(), q_end = split->end();
	for ( ; q != q_end; ++q )
		splitVertices.insert(q->C->getTaxVertex());
	// set V to be a node-to-add
	// FIXME!! check later the case whether both TOP and BOT are there
	TaxonomyVertex* bot = pTax->getBottomVertex();
	TaxonomyVertex* top = pTax->getTopVertex();
	TaxonomyVertex* cur = pTax->getCurrent();

	if ( splitVertices.count(bot) > 0 )
		v = bot;
	else if ( splitVertices.count(top) > 0 )
		v = top;
	else
	{
		setCurrentEntry(split->C);
		v = cur;
	}

	if ( v != cur && !cIn )
		v->addSynonym(split->C);
	for ( TVSet1::iterator p = splitVertices.begin(), p_end = splitVertices.end(); p != p_end; ++p )
		mergeVertex ( v, *p, splitVertices );
	if ( v == cur )
	{
		checkExtraParents();
		pTax->finishCurrentNode();
	}
}

/********************************************************\
|* 			Implementation of class TBox				*|
\********************************************************/

void TBox :: createTaxonomy ( bool needIndividual )
{
	bool needConcept = !needIndividual;

	// if there were SAT queries before -- the query (or other) concepts are there. Delete it
	clearQueryConcept();

	// here we sure that ontology is consistent
	// FIXME!! distinguish later between the 1st run and the following runs
	if ( pTax == NULL )	// 1st run
		initTaxonomy();

	DLHeap.setSubOrder();	// init priorities in order to do subsumption tests
	pTaxCreator->setBottomUp(GCIs);
	needConcept |= needIndividual;	// together with concepts
//	else	// not a first run
//		return;	// FIXME!! now we don't perform staged reasoning, so everything is done
/*
	{
		fpp_assert ( needIndividual );
		pTax->deFinalise();
	}
*/
	if ( verboseOutput )
		std::cerr << "Processing query...";

	TsProcTimer locTimer;
	locTimer.Start();

	// calculate number of items to be classified
	unsigned int nItems = 0;

	// fills collections
	arrayCD.clear();
	arrayNoCD.clear();
	arrayNP.clear();

//	if ( needConcept )
		nItems += fillArrays ( c_begin(), c_end() );
//	if ( needIndividual )
		nItems += fillArrays ( i_begin(), i_end() );

	// taxonomy progress
	if ( pMonitor )
	{
		pMonitor->setClassificationStarted(nItems);
		pTaxCreator->setProgressIndicator(pMonitor);
	}

	duringClassification = true;

//	sort ( arrayCD.begin(), arrayCD.end(), TSDepthCompare() );
	classifyConcepts ( arrayCD, true, "completely defined" );
//	sort ( arrayNoCD.begin(), arrayNoCD.end(), TSDepthCompare() );
	classifyConcepts ( arrayNoCD, false, "regular" );
//	sort ( arrayNP.begin(), arrayNP.end(), TSDepthCompare() );
	classifyConcepts ( arrayNP, false, "non-primitive" );

	duringClassification = false;

	pTaxCreator->processSplits();

	if ( pMonitor )
	{
		pMonitor->setFinished();
		setProgressMonitor(NULL);	// no need of PM after classification done
		pTaxCreator->setProgressIndicator(NULL);
	}
	pTax->finalise();

	locTimer.Stop();
	if ( verboseOutput )
		std::cerr << " done in " << locTimer << " seconds\n";

	if ( needConcept && Status < kbClassified )
		Status = kbClassified;
	if ( needIndividual )
		Status = kbRealised;

	if ( verboseOutput/* && needIndividual*/ )
	{
		std::ofstream of("Taxonomy.log");
		pTaxCreator->print(of);
	}
}

void
TBox :: classifyConcepts ( const ConceptVector& collection, bool curCompletelyDefined, const char* type )
{
	// set CD for taxonomy
	pTaxCreator->setCompletelyDefined(curCompletelyDefined);

	if ( LLM.isWritable(llStartCfyConcepts) )
		LL << "\n\n---Start classifying " << type << " concepts";

	unsigned int n = 0;

	for ( ConceptVector::const_iterator q = collection.begin(), q_end = collection.end(); q < q_end; ++q )
		// check if concept is already classified
		if ( !isCancelled() && !(*q)->isClassified () /*&& (*q)->isClassifiable(curCompletelyDefined)*/ )
		{
			classifyEntry(*q);	// need to classify concept
			if ( (*q)->isClassified() )
				++n;
		}

	if ( LLM.isWritable(llStartCfyConcepts) )
		LL << "\n---Done: " << n << " " << type << " concepts classified";
}
