/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2011 by Dmitry Tsarkov

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

#include "Taxonomy.h"
#include "globaldef.h"
#include "logging.h"

#include <fstream>

//#define TMP_PRINT_TAXONOMY_INFO

/********************************************************\
|* 			Implementation of class Taxonomy			*|
\********************************************************/
Taxonomy :: ~Taxonomy ( void )
{
	delete Current;
	for ( iterator p = Graph.begin(), p_end = Graph.end(); p < p_end; ++p )
		delete *p;
}

void Taxonomy :: print ( std::ostream& o ) const
{
	o << "Taxonomy consists of " << nEntries << " entries\n";
	o << "            of which " << nCDEntries << " are completely defined\n\n";
	o << "All entries are in format:\n\"entry\" {n: parent_1 ... parent_n} {m: child_1 child_m}\n\n";

	TVSet sorted(itop()+1, end());

	getTopVertex()->print(o);
	for ( TVSet::const_iterator p = sorted.begin(), p_end = sorted.end(); p != p_end; ++p )
		(*p)->print(o);
	getBottomVertex()->print(o);
}

//---------------------------------------------------
// classification part
//---------------------------------------------------

void
Taxonomy :: insertCurrent ( TaxonomyVertex* syn )
{
	if ( willInsertIntoTaxonomy )
	{
		// check if current concept is synonym to someone
		if ( syn != NULL )
		{
			syn->addSynonym(curEntry);

			if ( LLM.isWritable(llTaxInsert) )
				LL << "\nTAX:set " << curEntry->getName() << " equal " << syn->getPrimer()->getName();
		}
		else	// just incorporate it as a special entry and save into Graph
		{
			Current->incorporate(curEntry);
			Graph.push_back(Current);
			// we used the Current so need to create a new one
			Current = new TaxonomyVertex();
		}
	}
	else	// check if node is synonym of existing one and copy EXISTING info to Current
	{
		if ( syn != NULL )	// set synonym to the tax-entry for the checked one
			syn->setHostVertex(curEntry);
		else	// mark a current one to be a tax-entry
			Current->setSample(curEntry);
	}
}

void
Taxonomy :: performClassification ( void )
{
	// do something before classification (tunable)
	preClassificationActions();

	++nEntries;

	if ( LLM.isWritable(llStartCfyEntry) && needLogging() )
		LL << "\n\nTAX: start classifying entry " << curEntry->getName();

	// if no classification needed -- nothing to do
	if ( immediatelyClassified() )
		return;

	// perform main classification
	generalTwoPhaseClassification();

	// create new vertex
	insertCurrent(Current->isSynonymNode());

	// clear all labels
	clearLabels();
}

void Taxonomy :: generalTwoPhaseClassification ( void )
{
	// Top-Down phase

	// setup TD phase (ie, identify parent candidates)
	setupTopDown();

	// run TD phase if necessary (ie, entry is completely defined)
	if ( needTopDown() )
	{
		getTopVertex()->setValued ( true, valueLabel );		// C [= TOP == true
		getBottomVertex()->setValued ( false, valueLabel );	// C [= BOT == false (catched by UNSAT)
		runTopDown();
	}

	clearLabels();

	// Bottom-Up phase

	// run BU if necessary
	if ( needBottomUp() )
	{
		getBottomVertex()->setValued ( true, valueLabel );	// BOT [= C == true
		runBottomUp();
	}

	clearLabels();
}

bool Taxonomy :: classifySynonym ( void )
{
	const ClassifiableEntry* syn = resolveSynonym(curEntry);

	if ( syn == curEntry )
		return false;	// not a synonym

	// update synonym vertex:
	fpp_assert ( syn->getTaxVertex() != NULL );
	insertCurrent(syn->getTaxVertex());

	return true;
}

bool
Taxonomy :: isDirectParent ( TaxonomyVertex* v ) const
{
	for ( TaxonomyVertex::const_iterator q = v->begin(/*upDirection=*/false), q_end = v->end(/*upDirection=*/false); q < q_end; ++q )
		if ( (*q)->isValued(valueLabel) && (*q)->getValue() == true )
		{
#		ifdef WARN_EXTRA_SUBSUMPTION
			std::cout << "\nCTAX!!: Definition (implies '" << curEntry->getName()
					  << "','" << (*p)->getName() << "') is extra because of definition (implies '"
					  << curEntry->getName() << "','" << (*q)->getPrimer()->getName() << "')\n";
#		endif
			return false;
		}
	return true;
}

void Taxonomy :: setNonRedundantCandidates ( void )
{
	if ( LLM.isWritable(llCDConcept) && needLogging() )
	{
		if ( !curEntry->hasToldSubsumers() )
			LL << "\nTAX: TOP";
		LL << " completely defines concept " << curEntry->getName();
	}

	// test if some "told subsumer" is not an immediate TS (ie, not a border element)
	for ( ss_iterator p = told_begin(), p_end = told_end(); p < p_end; ++p )
	{
		TaxonomyVertex* par = (*p)->getTaxVertex();
		if ( par == NULL )	// non-classifiable TS
			continue;
		if ( isDirectParent(par) )
			Current->addNeighbour ( /*upDirection=*/true, par );
	}
}

void Taxonomy :: setToldSubsumers ( void )
{
	if ( LLM.isWritable(llTSList) && needLogging() && !ksStack.top()->s_empty() )
		LL << "\nTAX: told subsumers";

	for ( ss_iterator p = told_begin(), p_end = told_end(); p < p_end; ++p )
	{
		if ( !(*p)->isClassified() )	// non-primitive/non-classifiable concept
			continue;	// safety check

		if ( LLM.isWritable(llTSList) && needLogging() )
			LL << " '" << (*p)->getName() << "'";

		propagateTrueUp((*p)->getTaxVertex());
	}

	if ( !ksStack.top()->p_empty() && LLM.isWritable(llTSList) && needLogging() )
	{
		LL << " and possibly ";

		for ( ss_iterator q = ksStack.top()->p_begin(), q_end = ksStack.top()->p_end(); q < q_end; ++q )
			LL << " '" << (*q)->getName() << "'";
	}
}

void
Taxonomy :: propagateTrueUp ( TaxonomyVertex* node )
{
	// if taxonomy class already checked -- do nothing
	if ( node->isValued(valueLabel) )
	{
		fpp_assert ( node->getValue() );
		return;
	}

	// overwise -- value it...
	node->setValued ( true, valueLabel );

	// ... and value all parents
	for ( iterator p = node->begin(/*upDirection=*/true), p_end = node->end(/*upDirection=*/true); p < p_end; ++p )
		propagateTrueUp(*p);
}

//-----------------------------------------------------------------
//--	DFS-based classification methods
//-----------------------------------------------------------------

ClassifiableEntry*
Taxonomy :: prepareTS ( ClassifiableEntry* cur )
{
	// we just found that TS forms a cycle -- return stop-marker
	if ( waitStack.contains(cur) )
		return cur;

	// starting from the topmost entry
	addTop(cur);
	// true iff CUR is a reason of the cycle
	bool cycleFound = false;
	// for all the told subsumers...
	for ( ss_iterator p = told_begin(), p_end = told_end(); p < p_end; ++p )
		if ( !(*p)->isClassified() )	// need to classify it first
		{
			if ( unlikely((*p)->isNonClassifiable()) )
				continue;
			// prepare TS for *p
			ClassifiableEntry* v = prepareTS(*p);
			// if NULL is returned -- just continue
			if ( v == NULL )
				continue;
			if ( v == cur )	// current cycle is finished, all saved in Syns
			{
				// after classification of CUR we need to mark all the Syns as synonyms
				cycleFound = true;
				// continue to prepare its classification
				continue;
			}
			else
			{
				// arbitrary vertex in a cycle: save in synonyms of a root cause
				Syns.push_back(cur);
				// don't need to classify it
				removeTop();
				// return the cycle cause
				return v;
			}
		}
	// all TS are ready here -- let's classify!
	classifyTop();
	// now if CUR is the reason of cycle mark all SYNs as synonyms
	if ( cycleFound )
	{
		TaxonomyVertex* syn = cur->getTaxVertex();
		for ( std::vector<ClassifiableEntry*>::iterator q = Syns.begin(), q_end = Syns.end(); q != q_end; ++q )
			syn->addSynonym(*q);
		Syns.clear();
	}
	// here the cycle is gone
	return NULL;
}

