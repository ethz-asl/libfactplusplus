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

#ifndef _TAXNAMENTRY_H
#define _TAXNAMENTRY_H

#include <cassert>
#include <vector>

#include "tNamedEntry.h"
#include "globaldef.h"

class TaxonomyVertex;

class ClassifiableEntry : public TNamedEntry
{
public:		// type definitions
	typedef std::vector<ClassifiableEntry*> linkSet;

protected:	// members
		/// link to taxonomy entry for current entry
	TaxonomyVertex* taxVertex;
		/// links to 'told subsumers' (entryes that are direct super-entries for current)
	linkSet toldSubsumers;
		/// pointer to synonym (entry which contains whole information the same as current)
	ClassifiableEntry* pSynonym;

protected:	// methods
		/// add told subsumers as no duplication can happens
	void includeParent ( ClassifiableEntry* parent );

public:		// interface
		/// C'tor
	ClassifiableEntry ( const std::string& name )
		: TNamedEntry ( name )
		, taxVertex (NULL)
		, pSynonym (NULL)
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
		/// register a Not-a-Told-Subsumer flag
	FPP_ADD_FLAG(NaTS,0x4);

	// told subsumers interface

		/// add told subsumer of entry (duplications possible)
	void addParent ( ClassifiableEntry* parent ) { toldSubsumers.push_back (parent); }
		/// add all parents of given entry (with removing duplications)
	void addAllParents ( ClassifiableEntry* parent );
		/// get access to told subsumers (non-const version)
	linkSet& getTold ( void ) { return toldSubsumers; }
		/// get access to told subsumers (const version)
	const linkSet& getTold ( void ) const { return toldSubsumers; }

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
		if ( syn == NULL || pSynonym == NULL )	// clear/create link to synonym
		{
			pSynonym = syn;
			canonicaliseSynonym();
		}
		else if ( syn != getSynonym() )	// safety check
				assert (0);	// FIXME!! check this later on
	}

		/// if two synonyms are in 'told' list, merge them
	void removeSynonymsFromParents ( void );
}; // ClassifiableEntry

#endif // _TAXNAMENTRY_H
