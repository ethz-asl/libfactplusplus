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

#ifndef TAXNAMENTRY_H
#define TAXNAMENTRY_H

#include <vector>

#include "fpp_assert.h"
#include "tNamedEntry.h"
#include "globaldef.h"

class TaxonomyVertex;

class ClassifiableEntry : public TNamedEntry
{
public:		// type definitions
		/// type for the set of told subsumers
	typedef std::vector<ClassifiableEntry*> linkSet;
		/// told subsumers RW iterator
	typedef linkSet::iterator iterator;
		/// told subsumers RO iterator
	typedef linkSet::const_iterator const_iterator;

protected:	// members
		/// link to taxonomy entry for current entry
	TaxonomyVertex* taxVertex;
		/// links to 'told subsumers' (entryes that are direct super-entries for current)
	linkSet toldSubsumers;
		/// pointer to synonym (entry which contains whole information the same as current)
	ClassifiableEntry* pSynonym;
		/// index as a vertex in the SubsumptionMap/model cache
	unsigned int Index;

private:	// no copy
		/// no copy c'tor
	ClassifiableEntry ( const ClassifiableEntry& );
		/// no assignment
	ClassifiableEntry& operator = ( const ClassifiableEntry& );

public:		// interface
		/// C'tor
	ClassifiableEntry ( const std::string& name )
		: TNamedEntry ( name )
		, taxVertex (NULL)
		, pSynonym (NULL)
		, Index(0)
		{}
		/// D'tor
	~ClassifiableEntry ( void ) {}

	// taxonomy entry access

		/// is current entry classified
	bool isClassified ( void ) const { return ( taxVertex != NULL ); }
		/// set up given entry
	void setTaxVertex ( TaxonomyVertex* vertex ) { taxVertex = vertex; }
		/// get taxonomy vertex of the entry
	TaxonomyVertex* getTaxVertex ( void ) const { return taxVertex; }

	// completely defined interface

		/// register a Completely Defined flag
	FPP_ADD_FLAG(CompletelyDefined,0x2);
		/// register a non-classifiable flag
	FPP_ADD_FLAG(NonClassifiable,0x4);

	// told subsumers interface

		/// begin (RO) of told subsumers
	const_iterator told_begin ( void ) const { return toldSubsumers.begin(); }
		/// end (RO) of told subsumers
	const_iterator told_end ( void ) const { return toldSubsumers.end(); }
		/// begin (RW) of told subsumers
	iterator told_begin ( void ) { return toldSubsumers.begin(); }
		/// end (RW) of told subsumers
	iterator told_end ( void ) { return toldSubsumers.end(); }

		/// check whether entry ihas any TS
	bool hasToldSubsumers ( void ) const { return !toldSubsumers.empty(); }
		/// add told subsumer of entry (duplications possible)
	void addParent ( ClassifiableEntry* parent ) { toldSubsumers.push_back (parent); }
		/// add told subsumer if doesn't already recorded
	void addParentIfNew ( ClassifiableEntry* parent );

		/// add all parents (with duplicates) from the range to current node
	template<class Iterator>
	void addParents ( Iterator begin, Iterator end )
	{
		for ( Iterator p = begin; p < end; ++p )
			addParentIfNew(*p);
	}

	// index interface

		/// get the index value
	unsigned int index ( void ) const { return Index; }
		/// set the index value
	void setIndex ( unsigned int ind ) { Index = ind; }

	// synonym interface

		/// check if current entry is a synonym
	bool isSynonym ( void ) const { return (pSynonym != NULL); }
		/// get synonym of current entry
	ClassifiableEntry* getSynonym ( void ) const { return pSynonym; }
		/// make sure that synonym's representative is not a synonym itself
	void canonicaliseSynonym ( void )
	{
		if ( isSynonym() )
			while ( pSynonym->isSynonym() )
				pSynonym = pSynonym->getSynonym();
	}
		/// add entry's synonym
	void setSynonym ( ClassifiableEntry* syn )
	{
		fpp_assert ( pSynonym == NULL );	// do it only once
		pSynonym = syn;
		canonicaliseSynonym();
	}

		/// if two synonyms are in 'told' list, merge them
	void removeSynonymsFromParents ( void )
	{
		linkSet copy;
		copy.swap(toldSubsumers);
		addParents ( copy.begin(), copy.end() );
	}
}; // ClassifiableEntry

/// general RW resolving synonym operator
template<class T>
inline T*
resolveSynonym ( T* p )
{
	return !p ? NULL : p->isSynonym() ? resolveSynonym(static_cast<T*>(p->getSynonym())) : p;
}

/// general RW resolving synonym operator
template<class T>
inline const T*
resolveSynonym ( const T* p )
{
	return !p ? NULL : p->isSynonym() ? resolveSynonym(static_cast<const T*>(p->getSynonym())) : p;
}

inline void
ClassifiableEntry :: addParentIfNew ( ClassifiableEntry* parent )
{
	// resolve synonyms
	parent = resolveSynonym(parent);

	// node can not be its own parent
	if ( parent == this )
		return;

	// check if such entry already exists
	for ( iterator p = told_begin(); p != told_end(); ++p )
		if ( parent == *p )
			return;

	addParent(parent);
}

#endif // TAXNAMENTRY_H
