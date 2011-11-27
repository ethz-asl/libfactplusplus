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

#ifndef TAXONOMY_H
#define TAXONOMY_H

// taxonomy graph for DL

#include "taxVertex.h"
#include "SearchableStack.h"

class Taxonomy
{
protected:	// internal typedefs
		/// type for a TaxLink from TaxVertex
	typedef TaxonomyVertex::TaxonomyLink TaxonomyLink;
		/// type for taxonomy internal representation
	typedef TaxonomyLink SetOfVertex;
		/// set of the subsumers
	typedef ClassifiableEntry::linkSet SubsumerSet;
		/// SS RW iterator
	typedef SubsumerSet::iterator ss_iterator;

		/// abstract class to represent the known subsumers of a concept
	class KnownSubsumers
	{
	public:		// interface
			/// empty c'tor
		KnownSubsumers ( void ) {}
			/// empty  d'tor
		virtual ~KnownSubsumers ( void ) {}

		// iterators

			/// begin of the Sure subsumers interval
		virtual ss_iterator s_begin ( void ) = 0;
			/// end of the Sure subsumers interval
		virtual ss_iterator s_end ( void ) = 0;
			/// begin of the Possible subsumers interval
		virtual ss_iterator p_begin ( void ) = 0;
			/// end of the Possible subsumers interval
		virtual ss_iterator p_end ( void ) = 0;

		// flags

			/// whether there are no sure subsumers
		bool s_empty ( void ) { return s_begin() == s_end(); }
			/// whether there are no possible subsumers
		bool p_empty ( void ) { return p_begin() == p_end(); }
			/// @return true iff CE is the possible subsumer
		virtual bool isPossibleSub ( const ClassifiableEntry* ce ATTR_UNUSED ) const { return true; }
	}; // KnownSubsumers

		/// class to represent the TS's
	class ToldSubsumers: public KnownSubsumers
	{
	protected:		// members
			/// two iterators for the TS of a concept
		ss_iterator beg, end;

	public:		// interface
			/// c'tor
		ToldSubsumers ( ss_iterator b, ss_iterator e ) : beg(b), end(e) {}
			/// d'tor
		virtual ~ToldSubsumers ( void ) {}

		// iterators

			/// begin of the Sure subsumers interval
		virtual ss_iterator s_begin ( void ) { return beg; }
			/// end of the Sure subsumers interval
		virtual ss_iterator s_end ( void ) { return end; }
			/// begin of the Possible subsumers interval
		virtual ss_iterator p_begin ( void ) { return end; }
			/// end of the Possible subsumers interval
		virtual ss_iterator p_end ( void ) { return end; }
	}; // ToldSubsumers

public:		// typedefs
		/// iterator on the set of vertex
	typedef SetOfVertex::iterator iterator;
		/// const_iterator on the set of vertex
	typedef SetOfVertex::const_iterator const_iterator;

protected:	// members
		/// array of taxonomy vertices
	SetOfVertex Graph;
		/// aux array to keep synonyms found during TS classification
	std::vector<ClassifiableEntry*> Syns;

		/// labellers for marking taxonomy
	TLabeller checkLabel, valueLabel;
		/// aux vertex to be included to taxonomy
	TaxonomyVertex* Current;
		/// vertex with parent Top and child Bot, represents the fresh entity
	TaxonomyVertex FreshNode;
		/// pointer to currently classified entry
	const ClassifiableEntry* curEntry;

		/// number of tested entries
	unsigned int nEntries;
		/// number of completely-defined entries
	unsigned long nCDEntries;

		/// optimisation flag: if entry is completely defined by it's told subsumers, no other classification required
	bool useCompletelyDefined;

		/// behaviour flag: if true, insert temporary vertex into taxonomy
	bool willInsertIntoTaxonomy;

		/// stack for Taxonomy creation
	SearchableStack <ClassifiableEntry*> waitStack;
		/// told subsumers corresponding to a given entry
	SearchableStack <KnownSubsumers*> ksStack;

private:	// no copy
		/// no copy c'tor
	Taxonomy ( const Taxonomy& );
		/// no assignment
	Taxonomy& operator = ( const Taxonomy& );

protected:	// methods
		/// initialise aux entry with given concept p
	void setCurrentEntry ( const ClassifiableEntry* p )
	{
		Current->clear();
		curEntry = p;
	}

	//-----------------------------------------------------------------
	//--	General classification support
	//-----------------------------------------------------------------

		/// make the only parent -- top
	void setParentTop ( void ) { Current->addNeighbour ( /*upDirection=*/true, getTopVertex() ); }
		/// make the only child -- bottom
	void setChildBottom ( void ) { Current->addNeighbour ( /*upDirection=*/false, getBottomVertex() ); }
		/// return 1 if current entry is classified as a synonym of already classified one
	virtual bool classifySynonym ( void );

		/// set up Told Subsumers for the current entry
	void setToldSubsumers ( void );
		/// add non-redundant candidates for the current entry
	void setNonRedundantCandidates ( void );

	//-----------------------------------------------------------------
	//--	Tunable methods (depending on taxonomy type)
	//-----------------------------------------------------------------

		/// check if no classification needed (synonym, orphan, unsatisfiable)
	virtual bool immediatelyClassified ( void ) { return classifySynonym(); }
		/// setup TD phase (ie, identify/set parent candidates)
	void setupTopDown ( void )
	{
		setToldSubsumers();
		if ( !needTopDown() )
		{
			++nCDEntries;
			setNonRedundantCandidates();
		}
	}
		/// check if it is possible to skip TD phase
	virtual bool needTopDown ( void ) const { return false; }
		/// explicitly run TD phase
	virtual void runTopDown ( void ) {}
		/// check if it is possible to skip BU phase
	virtual bool needBottomUp ( void ) const { return false; }
		/// explicitly run BU phase
	virtual void runBottomUp ( void ) {}

		/// actions that to be done BEFORE entry will be classified
	virtual void preClassificationActions ( void ) {}

	//-----------------------------------------------------------------
	//--	General classification methods
	//-----------------------------------------------------------------

		/// Common pre- and post-action to setup 2-phase algo
	void performClassification ( void );
		/// fills parents and children of Current using tunable general approach
	void generalTwoPhaseClassification ( void );
		/// insert current node into taxonomy wrt flag willIIT; SYN is synonym (if any)
	void insertCurrent ( TaxonomyVertex* syn );
		/// remove node from the taxonomy; assume no references to the node
	void removeNode ( TaxonomyVertex* node )
	{
		for ( iterator p = begin(), p_end = end(); p != p_end; ++p )
			if ( (*p) == node )
			{
				*p = Graph.back();
				Graph.pop_back();
				delete node;
				return;
			}
	}
		/// @return true if V is a direct parent of current wrt labels
	bool isDirectParent ( TaxonomyVertex* v ) const;

	//-----------------------------------------------------------------
	//--	DFS-based classification
	//-----------------------------------------------------------------

		/// prepare known subsumers for given entry if necessary
	virtual KnownSubsumers* buildKnownSubsumers ( ClassifiableEntry* p )
		{ return new ToldSubsumers(p->told_begin(), p->told_end()); }
		/// add top entry together with its known subsumers
	void addTop ( ClassifiableEntry* p )
	{
		waitStack.push(p);
		ksStack.push(buildKnownSubsumers(p));
	}
		/// remove top entry
	void removeTop ( void )
	{
		waitStack.pop();
		delete ksStack.top();
		ksStack.pop();
	}
		/// ensure that all TS of the top entry are classified. @return the reason of cycle or NULL.
	ClassifiableEntry* prepareTS ( ClassifiableEntry* cur );
		/// classify top entry of the stack
	void classifyTop ( void )
	{
		fpp_assert ( !waitStack.empty() );	// sanity check
		setCurrentEntry(waitStack.top());
		performClassification();
		removeTop();
	}
		/// propagate the TRUE value of the KS subsumption up the taxonomy
	void propagateTrueUp ( TaxonomyVertex* node );

	ss_iterator told_begin ( void ) { return ksStack.top()->s_begin(); }
	ss_iterator told_end ( void ) { return ksStack.top()->s_end(); }

		/// check if it is necessary to log taxonomy action
	virtual bool needLogging ( void ) const { return false; }

		/// apply ACTOR to subgraph starting from NODE as defined by flags
	template<bool onlyDirect, bool upDirection, class Actor>
	void getRelativesInfoRec ( TaxonomyVertex* node, Actor& actor )
	{
		// recursive applicability checking
		if ( node->isChecked(checkLabel) )
			return;

		// label node as visited
		node->setChecked(checkLabel);

		// if current node processed OK and there is no need to continue -- exit
		// if node is NOT processed for some reasons -- go to another level
		if ( actor.apply(*node) && onlyDirect )
			return;

		// apply method to the proper neighbours with proper parameters
		for ( iterator p = node->begin(upDirection), p_end = node->end(upDirection); p != p_end; ++p )
			getRelativesInfoRec<onlyDirect, upDirection> ( *p, actor );
	}

public:		// interface
		/// init c'tor
	Taxonomy ( const ClassifiableEntry* pTop, const ClassifiableEntry* pBottom )
		: Current(new TaxonomyVertex())
		, curEntry(NULL)
		, nEntries(0)
		, nCDEntries(0)
		, useCompletelyDefined (false)
		, willInsertIntoTaxonomy (true)
	{
		Graph.push_back (new TaxonomyVertex(pBottom));	// bottom
		Graph.push_back (new TaxonomyVertex(pTop));		// top
		// set up fresh node
		FreshNode.addNeighbour ( /*upDirection=*/true, getTopVertex() );
		FreshNode.addNeighbour ( /*upDirection=*/false, getBottomVertex() );
	}
		/// d'tor
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
	TaxonomyVertex* getTopVertex ( void ) const { return *itop(); }
		/// special access to BOTTOM of taxonomy
	TaxonomyVertex* getBottomVertex ( void ) const { return *ibottom(); }
		/// get node for fresh entity E
	TaxonomyVertex* getFreshVertex ( const ClassifiableEntry* e ) { FreshNode.setSample(e,false); return &FreshNode; }

		/// apply ACTOR to subgraph starting from NODE as defined by flags;
	template<bool needCurrent, bool onlyDirect, bool upDirection, class Actor>
	void getRelativesInfo ( TaxonomyVertex* node, Actor& actor )
	{
		// if current node processed OK and there is no need to continue -- exit
		// this is the helper to the case like getDomain():
		//   if there is a named concept that represent's a domain -- that's what we need
		if ( needCurrent )
			if ( actor.apply(*node) && onlyDirect )
				return;

		for ( iterator p = node->begin(upDirection), p_end = node->end(upDirection); p != p_end; ++p )
			getRelativesInfoRec<onlyDirect, upDirection> ( *p, actor );

		clearCheckedLabel();
	}

	//------------------------------------------------------------------------------
	//--	classification interface
	//------------------------------------------------------------------------------

		/// classify given entry: general method is by DFS
	void classifyEntry ( ClassifiableEntry* p )
	{
		fpp_assert ( waitStack.empty() );	// sanity check

		// don't classify artificial concepts
		if ( p->isNonClassifiable() )
			return;
		prepareTS(p);
	}
		/// clear the CHECKED label from all the taxonomy vertex
	void clearCheckedLabel ( void ) { checkLabel.newLabel(); }
 		/// clear all labels from Taxonomy vertices
	void clearLabels ( void ) { checkLabel.newLabel(); valueLabel.newLabel(); }

	// flags interface

		/// set Completely Defined flag
	void setCompletelyDefined ( bool use ) { useCompletelyDefined = use; }
		/// call this method after taxonomy is built
	void finalise ( void )
	{	// create links from leaf concepts to bottom
		const bool upDirection = false;
		for ( iterator p = itop(), p_end = end(); p < p_end; ++p )
			if ( likely((*p)->isInUse()) && (*p)->noNeighbours(upDirection) )
			{
				(*p)->addNeighbour ( upDirection, getBottomVertex() );
				getBottomVertex()->addNeighbour ( !upDirection, *p );
			}
		willInsertIntoTaxonomy = false;	// after finalisation one shouldn't add new entries to taxonomy
	}
		/// unlink the bottom from the taxonomy
	void deFinalise ( void )
	{
		const bool upDirection = true;
		TaxonomyVertex* bot = getBottomVertex();
		for ( TaxonomyVertex::iterator
				p = bot->begin(upDirection),
				p_end = bot->end(upDirection);
			  p < p_end; ++p )
			(*p)->removeLink ( !upDirection, bot );
		bot->clearLinks(upDirection);
		willInsertIntoTaxonomy = true;	// it's possible again to add entries
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

#endif // TAXONOMY_H
