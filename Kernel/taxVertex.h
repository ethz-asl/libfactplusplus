/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2006 by Dmitry Tsarkov

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

#ifndef _TAXVERTEX_H
#define _TAXVERTEX_H

#include <vector>
#include <iostream>

#include "taxNamEntry.h"
#include "tLabeller.h"

class TaxonomyVertex
{
public:
	// typedefs for vertex structure
	typedef std::vector <TaxonomyVertex*> TaxonomyLink;
	typedef std::vector <const ClassifiableEntry*> EqualNames;
	typedef std::vector <EqualNames> SetOfTaxElements;

	// accessors type
	typedef TaxonomyLink::iterator iterator;
	typedef TaxonomyLink::const_iterator const_iterator;
	typedef EqualNames::const_iterator syn_iterator;

private:
	/// immediate parents and children
	TaxonomyLink Links[2];
protected:
		/// entry corresponding to current tax vertex
	const ClassifiableEntry* sample;
		/// synonyms of the sample entry
	EqualNames synonyms;

	/// indirect access to Links (non-const version)
	TaxonomyLink& neigh ( bool upDirection ) { return Links[!upDirection]; }
	/// indirect access to Links (const version)
	const TaxonomyLink& neigh ( bool upDirection ) const { return Links[!upDirection]; }

protected:	// members
		/// labellers for marking taxonomy
	static TLabeller checkLab, valuedLab;

	// labels for different purposes. all for 2 directions: top-down and bottom-up search

		/// flag if given vertex was checked; connected with checkLab
	TLabeller::LabType theChecked;
		/// flag if given vertex has value; connected with valuedLab
	TLabeller::LabType theValued;
		/// satisfiability value of a valued vertex
	bool checkValue;
		/// number of common parents of a node
	unsigned int common;

public:		// flags interface

	// checked part
	bool isChecked ( void ) const { return checkLab.isLabelled(theChecked); }
	void setChecked ( void ) { checkLab.set(theChecked); }

	// value part
	bool isValued ( void ) const { return valuedLab.isLabelled(theValued); }
	bool getValue ( void ) const { return checkValue; }
	bool setValued ( bool val )
	{
		valuedLab.set(theValued);
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

		/// clear checked flag
	static void clearChecked ( void ) { checkLab.newLab(); }
		/// clear all flags
	static void clearAllLabels ( void ) { clearChecked(); valuedLab.newLab(); }
		/// put initial values on the flags
	void initFlags ( void )
	{
		checkLab.clear(theChecked);
		valuedLab.clear(theValued);
		clearCommon();
	}

	// get info about taxonomy structure

	syn_iterator begin_syn ( void ) const { return synonyms.begin(); }
	syn_iterator end_syn ( void ) const { return synonyms.end(); }

		/// apply Actor to subgraph starting from current node and defined by flags
	template<bool onlyDirect, bool upDirection, class Actor>
	void getRelativesInfoRec ( Actor& actor )
	{
		// recursive applicability checking
		if ( isChecked() )
			return;

		// label node as visited
		setChecked();

		// if current node processed OK and there is no need to continue -- exit
		// if node is NOT processed for some reasons -- go to another level
		if ( actor.apply(*this) && onlyDirect )
			return;

		// apply method to the proper neighbours with proper parameters
		for ( iterator p = begin(upDirection), p_end = end(upDirection); p != p_end; ++p )
			(*p)->getRelativesInfoRec<onlyDirect, upDirection>(actor);
	}

	// propagate different values through label

		/// propagate VALUE to the parents
	void propagateValueUp ( const bool value );
		/// propagate common value to all descendants; save visited nodes in VISITED
	void propagateOne ( TaxonomyLink& visited );

public:
	TaxonomyVertex ( void ) : sample(NULL) { initFlags(); }
	TaxonomyVertex ( const ClassifiableEntry* p )
		: sample(p)
	{
		initFlags();
		const_cast<ClassifiableEntry*>(p)->setTaxVertex (this);
	}
	~TaxonomyVertex ( void ) {}
	void addSynonym ( const ClassifiableEntry* p )
	{
		synonyms.push_back(p);
		const_cast<ClassifiableEntry*>(p)->setTaxVertex (this);
	}
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
	/** Check if vertex has a synonym (ie Up[i]==Down[j]) and moves all
		synonym concepts to a one defined by given flag.
		@return pointer to synonym if could find one; @return NULL otherwise
	*/
	TaxonomyVertex* incorporateSynonym ( bool moveToExisting = true );
		/// Remove link P from neighbours (given by flag). @return true if such link was removed
	bool removeLink ( bool upDirection, TaxonomyVertex* p );
		/// remove latest link (usually to the BOTTOM node)
	void removeLastLink ( bool upDirection ) { neigh(upDirection).resize(neigh(upDirection).size()-1); }

		/** apply Actor to subgraph starting from current node and defined by flags;
			clear the labels afterwards
		*/
	template<bool needCurrent, bool onlyDirect, bool upDirection, class Actor>
	void getRelativesInfo ( Actor& actor )
	{
		if ( needCurrent )
			actor.apply(*this);

		for ( iterator p = begin(upDirection), p_end = end(upDirection); p != p_end; ++p )
			(*p)->getRelativesInfoRec<onlyDirect, upDirection>(actor);

		clearChecked();
	}

		/// print entry name and its synonyms (if any)
	void printSynonyms ( std::ostream& o ) const;
		/// print neighbours of a vertex in given direction
	void printNeighbours ( std::ostream& o, bool upDirection ) const;
		/// print taxonomy vertex in format <equals parents children>
	void print ( std::ostream& o ) const
	{
		printSynonyms(o);
		printNeighbours ( o, true );
		printNeighbours ( o, false );
		o << "\n";
	}
}; // TaxonomyVertex

#endif // _TAXVERTEX_H
