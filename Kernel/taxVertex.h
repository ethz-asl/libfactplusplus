/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2012 by Dmitry Tsarkov

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

#ifndef TAXVERTEX_H
#define TAXVERTEX_H

#include <set>
#include <vector>
#include <iostream>
#include <string.h>

#include "taxNamEntry.h"
#include "tLabeller.h"

class TaxonomyVertex
{
public:		// typedefs
	// typedefs for vertex structure
	typedef std::vector <TaxonomyVertex*> TaxonomyLink;
	typedef std::vector <const ClassifiableEntry*> EqualNames;

	// accessors type
	typedef TaxonomyLink::iterator iterator;
	typedef TaxonomyLink::const_iterator const_iterator;
	typedef EqualNames::const_iterator syn_iterator;

private:	// members
		/// immediate parents and children
	TaxonomyLink Links[2];

protected:	// members
		/// entry corresponding to current tax vertex
	const ClassifiableEntry* sample;
		/// synonyms of the sample entry
	EqualNames synonyms;

	// labels for different purposes. all for 2 directions: top-down and bottom-up search

		/// flag if given vertex was checked; connected with checkLab
	TLabeller::LabelType theChecked;
		/// flag if given vertex has value; connected with valuedLab
	TLabeller::LabelType theValued;
		/// number of common parents of a node
	unsigned int common;
		/// satisfiability value of a valued vertex
	bool checkValue;
		/// flag to check whether the vertex is in use
	bool inUse;

private:	// no copy
		/// no copy c'tor
	TaxonomyVertex ( const TaxonomyVertex& );
		/// no assignment
	TaxonomyVertex& operator = ( const TaxonomyVertex& );

protected:	// methods
		/// indirect RW access to Links
	TaxonomyLink& neigh ( bool upDirection ) { return Links[!upDirection]; }
		/// indirect RO access to Links
	const TaxonomyLink& neigh ( bool upDirection ) const { return Links[!upDirection]; }

		/// print entry name and its synonyms (if any)
	void printSynonyms ( std::ostream& o ) const;
		/// print neighbours of a vertex in given direction
	void printNeighbours ( std::ostream& o, bool upDirection ) const;

public:		// flags interface

	// checked part
	bool isChecked ( const TLabeller& checkLab ) const { return checkLab.isLabelled(theChecked); }
	void setChecked ( const TLabeller& checkLab ) { checkLab.set(theChecked); }

	// value part
	bool isValued ( const TLabeller& valueLab ) const { return valueLab.isLabelled(theValued); }
	bool getValue ( void ) const { return checkValue; }
	bool setValued ( bool val, const TLabeller& valueLab )
	{
		valueLab.set(theValued);
		checkValue = val;
		return val;
	}

	// common part
	bool isCommon ( void ) const { return common != 0; }
	void setCommon ( void ) { ++common; }
	void clearCommon ( void ) { common = 0; }
		/// keep COMMON flag iff both flags are set; @return true if it is the case
	bool correctCommon ( unsigned int n )
	{
		if ( common == n )
			return true;
		clearCommon();
		return false;
	}

		/// put initial values on the flags
	void initFlags ( void )
	{
		TLabeller::clear(theChecked);
		TLabeller::clear(theValued);
		clearCommon();
	}

	// get info about taxonomy structure

	syn_iterator begin_syn ( void ) const { return synonyms.begin(); }
	syn_iterator end_syn ( void ) const { return synonyms.end(); }

		/// mark vertex as the one corresponding to a given ENTRY
	void setHostVertex ( const ClassifiableEntry* entry ) { const_cast<ClassifiableEntry*>(entry)->setTaxVertex(this); }
		/// set sample to ENTRY
	void setSample ( const ClassifiableEntry* entry, bool linkBack = true )
	{
		sample = entry;
		if ( likely(linkBack) )
			setHostVertex(entry);
	}

public:
		/// empty c'tor
	TaxonomyVertex ( void )
		: sample(NULL)
		, inUse(true)
	{
		initFlags();
	}
		/// init c'tor; use it only for Top/Bot initialisations
	TaxonomyVertex ( const ClassifiableEntry* p )
		: inUse(true)
	{
		setSample(p);
		initFlags();
	}
		/// empty d'tor
	~TaxonomyVertex ( void ) {}

		/// add P as a synonym to curent vertex
	void addSynonym ( const ClassifiableEntry* p )
	{
		synonyms.push_back(p);
		setHostVertex(p);
	}
		/// clears the vertex
	void clear ( void )
	{
		Links[0].clear();
		Links[1].clear();
		sample = NULL;
		initFlags();
	}
		/// get RO access to the primer
	const ClassifiableEntry* getPrimer ( void ) const { return sample; }

		/// add link in given direction to vertex
	void addNeighbour ( bool upDirection, TaxonomyVertex* p ) { neigh(upDirection).push_back(p); }
		/// check if vertex has no neighbours in given direction
	bool noNeighbours ( bool upDirection ) const { return neigh(upDirection).empty(); }

	// iterator access to parents/children

	iterator begin ( bool upDirection ) { return neigh(upDirection).begin(); }
	iterator end ( bool upDirection ) { return neigh(upDirection).end(); }

	const_iterator begin ( bool upDirection ) const { return neigh(upDirection).begin(); }
	const_iterator end ( bool upDirection ) const { return neigh(upDirection).end(); }

	/** Adds vertex to existing graph. For every Up, Down such that (Up->Down)
		creates couple of links (Up->this), (this->Down). Don't work with synonyms!!!
	*/
	void incorporate ( void );
		/// @return v if node represents a synonym (v=Up[i]==Down[j]); @return NULL otherwise
	TaxonomyVertex* getSynonymNode ( void )
	{
		// try to find Vertex such that Vertex\in Up and Vertex\in Down
		for ( iterator q = begin(true), q_end = end(true); q < q_end; ++q )
			for ( iterator r = begin(false), r_end = end(false); r < r_end; ++r )
				if ( *q == *r )	// found such vertex
					return *q;
		return NULL;
	}
		/// Remove link P from neighbours (given by flag). @return true if such link was removed
	bool removeLink ( bool upDirection, TaxonomyVertex* p );
		/// remove latest link (usually to the BOTTOM node)
	void removeLastLink ( bool upDirection ) { neigh(upDirection).resize(neigh(upDirection).size()-1); }
		/// clear all links in a given direction
	void clearLinks ( bool upDirection ) { neigh(upDirection).clear(); }
		/// merge NODE which is independent to THIS
	void mergeIndepNode ( TaxonomyVertex* node, const std::set<TaxonomyVertex*>& excludes, const ClassifiableEntry* curEntry );

	// usage methods

		/// @return true iff the node is in use
	bool isInUse ( void ) const { return inUse; }
		/// set the inUse value of the node
	void setInUse ( bool value ) { inUse = value; }

	// output methods

		/// print taxonomy vertex in format <equals parents children>
	void print ( std::ostream& o ) const
	{
		printSynonyms(o);
		printNeighbours ( o, true );
		printNeighbours ( o, false );
		o << "\n";
	}

	// save/load interface; implementation is in SaveLoad.cpp

		/// save label of the entry
	void SaveLabel ( std::ostream& o ) const;
		/// load label of the entry
	void LoadLabel ( std::istream& i );
		/// save neighbours of the entry
	void SaveNeighbours ( std::ostream& o ) const;
		/// load neighbours of the entry
	void LoadNeighbours ( std::istream& i );
}; // TaxonomyVertex

/// structure to sort tax vertices
struct TaxVertexLess
{
	bool operator()(const TaxonomyVertex* s1, const TaxonomyVertex* s2) const
		{ return strcmp(s1->getPrimer()->getName(), s2->getPrimer()->getName()) < 0; }
};

/// sorted vertices set
typedef std::set<const TaxonomyVertex*, TaxVertexLess> TVSet;

#endif // TAXVERTEX_H
