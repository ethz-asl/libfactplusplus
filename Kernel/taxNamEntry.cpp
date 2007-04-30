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

#include "taxNamEntry.h"

/// add told subsumers as no duplication can happens
void ClassifiableEntry :: includeParent ( ClassifiableEntry* parent )
{
	assert ( parent != NULL );

	// resolve synonyms
	ClassifiableEntry* ent = ( parent->isSynonym() ? parent->getSynonym() : parent );

	// node can not be its own parent
	if ( ent == this )
		return;

	// check if such entry already exists
	for ( linkSet::iterator p = getTold().begin(); p != getTold().end(); ++p )
		if ( *p == ent )
			return;

	// doesn't exists => add it
	addParent (ent);
}

/// add all parents of given entry (with removing duplications)
void ClassifiableEntry :: addAllParents ( ClassifiableEntry* parent )
{
	assert ( parent != NULL );

	for ( linkSet::iterator p = parent->getTold().begin(); p != parent->getTold().end(); ++p )
		includeParent (*p);
}

/// if two synonyms are in 'told' list, merge them
void ClassifiableEntry :: removeSynonymsFromParents ( void )
{
	linkSet copy(getTold());
	getTold().clear();

	for ( linkSet::iterator p = copy.begin(), p_end = copy.end(); p < p_end; ++p )
		includeParent(*p);	// will resolve synonyms automatically
}
