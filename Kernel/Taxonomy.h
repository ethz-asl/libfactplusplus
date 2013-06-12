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

#ifndef TAXONOMY_H
#define TAXONOMY_H

// taxonomy graph for DL

#include "taxVertex.h"

class Taxonomy
{
protected:	// FORNOW: friend declarations
	friend class TaxonomyCreator;
	friend class DLConceptTaxonomy;

public:		// typedefs
		/// type for a vector of TaxVertex
	typedef std::vector<TaxonomyVertex*> TaxVertexVec;
		/// iterator on the set of vertex
	typedef TaxVertexVec::iterator iterator;
		/// const_iterator on the set of vertex
	typedef TaxVertexVec::const_iterator const_iterator;

protected:	// members
		/// array of taxonomy vertices
	TaxVertexVec Graph;

		/// labeller for marking nodes as checked
	TLabeller visitedLabel;
		/// aux vertex to be included to taxonomy
	TaxonomyVertex* Current;
		/// vertex with parent Top and child Bot, represents the fresh entity
	TaxonomyVertex FreshNode;

		/// behaviour flag: if true, insert temporary vertex into taxonomy
	bool willInsertIntoTaxonomy;

private:	// no copy
		/// no copy c'tor
	Taxonomy ( const Taxonomy& );
		/// no assignment
	Taxonomy& operator = ( const Taxonomy& );

protected:	// methods

	//-----------------------------------------------------------------
	//--	General classification support
	//-----------------------------------------------------------------

		/// make the only parent -- top
	void setParentTop ( void ) { Current->addNeighbour ( /*upDirection=*/true, getTopVertex() ); }
		/// make the only child -- bottom
	void setChildBottom ( void ) { Current->addNeighbour ( /*upDirection=*/false, getBottomVertex() ); }
		/// add current entry to a synonym SYN
	void addCurrentToSynonym ( TaxonomyVertex* syn );
		/// insert current node to a taxonomy (if not in query more)
	void insertCurrentNode ( void );
		/// remove node from the taxonomy; assume no references to the node
	void removeNode ( TaxonomyVertex* node ) { node->setInUse(false); }
		/// @return true if taxonomy works in a query mode (no need to insert query vertex)
	bool queryMode ( void ) const { return !willInsertIntoTaxonomy; }

		/// apply ACTOR to subgraph starting from NODE as defined by flags
	template<bool onlyDirect, bool upDirection, class Actor>
	void getRelativesInfoRec ( TaxonomyVertex* node, Actor& actor )
	{
		// recursive applicability checking
		if ( isVisited(node) )
			return;

		// label node as visited
		setVisited(node);

		// if current node processed OK and there is no need to continue -- exit
		// if node is NOT processed for some reasons -- go to another level
		if ( actor.apply(*node) && onlyDirect )
			return;

		// apply method to the proper neighbours with proper parameters
		for ( TaxonomyVertex::iterator p = node->begin(upDirection), p_end = node->end(upDirection); p != p_end; ++p )
			getRelativesInfoRec<onlyDirect, upDirection> ( *p, actor );
	}

public:		// interface
		/// init c'tor
	Taxonomy ( const ClassifiableEntry* pTop, const ClassifiableEntry* pBottom )
		: Current(new TaxonomyVertex())
		, willInsertIntoTaxonomy (true)
	{
		Graph.push_back (new TaxonomyVertex(pBottom));	// bottom
		Graph.push_back (new TaxonomyVertex(pTop));		// top
		// set up fresh node
		FreshNode.addNeighbour ( /*upDirection=*/true, getTopVertex() );
		FreshNode.addNeighbour ( /*upDirection=*/false, getBottomVertex() );
	}
		/// d'tor
	~Taxonomy ( void );

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
		/// get RW access to current
	TaxonomyVertex* getCurrent ( void ) { return Current; }
		/// get RO access to current
	const TaxonomyVertex* getCurrent ( void ) const { return Current; }

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

		for ( TaxonomyVertex::iterator p = node->begin(upDirection), p_end = node->end(upDirection); p != p_end; ++p )
			getRelativesInfoRec<onlyDirect, upDirection> ( *p, actor );

		clearVisited();
	}

	//------------------------------------------------------------------------------
	//--	classification support
	//------------------------------------------------------------------------------

		/// set node NODE as checked within taxonomy
	void setVisited ( TaxonomyVertex* node ) const { node->setChecked(visitedLabel); }
		/// check whether NODE is checked within taxonomy
	bool isVisited ( TaxonomyVertex* node ) const { return node->isChecked(visitedLabel); }
		/// @return true if current entry is a synonym of an already classified one
	bool processSynonym ( void );
		/// insert current node either directly or as a synonym
	void finishCurrentNode ( void )
	{
		TaxonomyVertex* syn = Current->getSynonymNode();
		if ( syn )
			addCurrentToSynonym(syn);
		else
			insertCurrentNode();
	}

	//------------------------------------------------------------------------------
	//--	classification interface
	//------------------------------------------------------------------------------

		/// clear the CHECKED label from all the taxonomy vertex
	void clearVisited ( void ) { visitedLabel.newLabel(); }

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
			  p != p_end; ++p )
			(*p)->removeLink ( !upDirection, bot );
		bot->clearLinks(upDirection);
		willInsertIntoTaxonomy = true;	// it's possible again to add entries
	}

	// taxonomy info access

		/// print taxonomy info to a stream
	void print ( std::ostream& o ) const;

	// save/load interface; implementation is in SaveLoad.cpp

		/// save entry
	void Save ( std::ostream& o ) const;
		/// load entry
	void Load ( std::istream& i );
}; // Taxonomy

#endif // TAXONOMY_H
