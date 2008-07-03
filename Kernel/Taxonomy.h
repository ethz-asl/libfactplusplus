/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2008 by Dmitry Tsarkov

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef _TAXONOMY_H
#define _TAXONOMY_H

// taxonomy graph for DL
#include <iostream>

#include "taxVertex.h"
#include "SearchableStack.h"

class Taxonomy
{
protected:	// internal typedefs
		/// type for a TaxLink from TaxVertex
	typedef TaxonomyVertex::TaxonomyLink TaxonomyLink;
		/// type for taxonomy internal representation
	typedef TaxonomyLink SetOfVertex;

public:		// typedefs
		/// iterator on the set of vertex
	typedef SetOfVertex::iterator iterator;
		/// const_iterator on the set of vertex
	typedef SetOfVertex::const_iterator const_iterator;

protected:	// members
	/// array of taxonomy verteces
	SetOfVertex Graph;

	/// aux. vertex to be included to taxonomy
	TaxonomyVertex* Current;
		/// pointer to currently classified entry
	const ClassifiableEntry* curEntry;

	/// optimisation flag: if entry is completely defined by it's told subsumers, no other classification required
	bool useCompletelyDefined;

	/// behaviour flag: if true, insert temporary vertex into taxonomy
	bool willInsertIntoTaxonomy;

		/// behaviour flag: if true, delete temporary vertex
	bool deleteCurrent;

	/// number of tested entryes
	unsigned int nEntries;
		/// number of completely-defined entries
	unsigned long nCDEntries;

	/// stack for Taxonomy creating (if necessary)
	SearchableStack <ClassifiableEntry*> waitStack;

private:	// no copy
		/// no copy c'tor
	Taxonomy ( const Taxonomy& );
		/// no assignment
	Taxonomy& operator = ( const Taxonomy& );

protected:	// methods
	/// initialise aux entry with given concept p
	void setCurrentEntry ( const ClassifiableEntry* p )
	{
		if ( deleteCurrent )
			delete Current;

		Current = new TaxonomyVertex (p);
		curEntry = p;
	}

	//-----------------------------------------------------------------
	//--	General classification support
	//-----------------------------------------------------------------

		/// make the only parent -- top
	void setParentTop ( void ) { Current->addNeighbour ( /*upDirection=*/true, getTop() ); }
		/// make the only child -- bottom
	void setChildBottom ( void ) { Current->addNeighbour ( /*upDirection=*/false, getBottom() ); }
		/// return 1 if current entry is classified as a synonym of already classified one
	bool classifySynonym ( void );

		/// set up Told Subsumers for the current entry
	void setToldSubsumers ( void );
		/// add non-redundant candidates for the current entry
	void setNonRedundantCandidates ( void );

	/// insert Current entry into Graph, correcting all links
	void insertEntry ( void );

	//-----------------------------------------------------------------
	//--	Tunable methods (depending on taxonomy type)
	//-----------------------------------------------------------------

		/// check if no classification needed (synonym, orphan, unsatisfiable)
	virtual bool immediatelyClassified ( void ) { return classifySynonym(); }
		/// setup TD phase (ie, identify/set parent candidates)
	virtual void setupTopDown ( void );
		/// check if it is possible to skip TD phase
	virtual bool needTopDown ( void ) const { return false; }
		/// explicitely run TD phase
	virtual void runTopDown ( void ) {}
		/// setup BU phase (ie, identify/set children candidates)
	virtual void setupBottomUp ( void ) {}
		/// check if it is possible to skip BU phase
	virtual bool needBottomUp ( void ) const { return false; }
		/// explicitely run BU phase
	virtual void runBottomUp ( void ) {}

		/// actions that to be done BEFORE entry will be classified
	virtual void preClassificationActions ( ClassifiableEntry* cur ATTR_UNUSED ) {}
		/// actions that to be done AFTER entry will be classified
	virtual void postClassificationActions ( ClassifiableEntry* cur ATTR_UNUSED ) {}

	//-----------------------------------------------------------------
	//--	General classification methods
	//-----------------------------------------------------------------

		/// Common pre- and post-action to setup 2-phase algo
	void performClassification ( ClassifiableEntry* p );
		/// fills parents and children of Current using tunable general approach
	void generalTwoPhaseClassification ( void );
		/// do all necessary action for the classification of given entry
	void doClassification ( ClassifiableEntry* p )
	{
		preClassificationActions(p);
		performClassification(p);
		postClassificationActions(p);
	}

	//-----------------------------------------------------------------
	//--	DFS-based classification
	//-----------------------------------------------------------------

		/// check if told subsumer P have to be classified during current session
	virtual bool needToldClassification ( ClassifiableEntry* p ATTR_UNUSED ) const { return true; }
		/// ensure that all TS of top entries are classified. @return true if cycle detected.
	bool checkToldSubsumers ( void );
		/// classify top entry in the stack
	void classifyTop ( void );
		/// deal with a TS cycle.
	void classifyCycle ( void );

		/// check if it is necessary to log taxonomy action
	virtual bool needLogging ( void ) const { return false; }

public:		// interface
	Taxonomy ( const ClassifiableEntry* pTop, const ClassifiableEntry* pBottom )
		: Current (NULL)
		, curEntry(NULL)
		, useCompletelyDefined (false)
		, willInsertIntoTaxonomy (true)
		, deleteCurrent (false)
		, nEntries (0)
		, nCDEntries (0)
	{
		Graph.push_back (new TaxonomyVertex(pBottom));	// bottom
		Graph.push_back (new TaxonomyVertex(pTop));		// top
	}

	virtual ~Taxonomy ( void );

	//------------------------------------------------------------------------------
	//--	Access to taxonomy entries
	//------------------------------------------------------------------------------

	iterator begin ( void ) { return Graph.begin(); }
	iterator end ( void ) { return Graph.end(); }
		/// iterator for the bottom of the taxonomy
	iterator ibottom ( void ) { return begin(); }
		/// iterator for the Top of the taxonomy
	iterator itop ( void ) { return begin()+1; }

	const_iterator begin ( void ) const { return Graph.begin(); }
	const_iterator end ( void ) const { return Graph.end(); }
		/// iterator for the bottom of the taxonomy
	const_iterator ibottom ( void ) const { return begin(); }
		/// iterator for the Top of the taxonomy
	const_iterator itop ( void ) const { return begin()+1; }

	/// special access to TOP of taxonomy
	TaxonomyVertex* getTop ( void ) const { return *itop(); }
	/// special access to BOTTOM of taxonomy
	TaxonomyVertex* getBottom ( void ) const { return *ibottom(); }

	//------------------------------------------------------------------------------
	//--	classification interface
	//------------------------------------------------------------------------------

		/// classify given entry: general method is by DFS
	void classifyEntry ( ClassifiableEntry* p );
		/// clear all labels from Taxonomy verteces
	void clearLabels ( void ) { TaxonomyVertex::clearAllLabels(); }

	// flags interface

		/// set Completely Defined flag
	void setCompletelyDefined ( bool use ) { useCompletelyDefined = use; }
		/// set Insert Into Taxonomy flag
	void setInsertIntoTaxonomy ( bool use )
	{
		willInsertIntoTaxonomy = use;
		deleteCurrent = !use;	// current is just temporary
	}
		/// call this method after taxonomy is built
	void finalise ( void )
	{	// create links from leaf concepts to bottom
		const bool upDirection = false;
		for ( iterator p = itop(), p_end = end(); p < p_end; ++p )
			if ( (*p)->noNeighbours(upDirection) )
			{
				(*p)->addNeighbour ( upDirection, getBottom() );
				getBottom()->addNeighbour ( !upDirection, *p );
			}
	}
		/// unlink the bottom from the taxonomy
	void deFinalise ( void )
	{
		const bool upDirection = true;
		TaxonomyVertex* bot = getBottom();
		for ( TaxonomyVertex::iterator
				p = bot->begin(upDirection),
				p_end = bot->end(upDirection);
			  p < p_end; ++p )
			(*p)->removeLink ( !upDirection, bot );
		bot->clearLinks(upDirection);
	}

	// taxonomy info access

		/// print taxonomy info to a stream
	virtual void print ( std::ostream& o ) const;

	// save/load interface; implementation is in SaveLoad.cpp

		/// save entry
	void Save ( std::ostream& o ) const;
		/// load entry
	void Load ( std::istream& i );
}; // Taxonomy

inline void Taxonomy :: setupTopDown ( void )
{
	++nCDEntries;
	setToldSubsumers();
	setNonRedundantCandidates();
}

#endif // _TAXONOMY_H
