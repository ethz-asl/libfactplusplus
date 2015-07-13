/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2015 by Dmitry Tsarkov

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
|*      Implementation of taxonomy vertex class        *|
\*******************************************************/

#include <algorithm>
#include <ostream>

#include "taxVertex.h"
#include "logging.h"

// removes given pointer from neigh.
bool TaxonomyVertex :: removeLink ( bool upDirection, TaxonomyVertex* p )
{
	// for all neighbours of current vertex...
	for ( iterator q = begin(upDirection), q_end = end(upDirection); q < q_end; ++q )
		if ( *q == p )	// if given neighbour found...
		{
			*q = neigh(upDirection).back();
			neigh(upDirection).pop_back();	// remove last entry (by resizing)
			return true;		// there is at most one link for each node
		}
	return false;	// no such link
}

void TaxonomyVertex :: incorporate ( void )
{
	// setup links
	iterator u, u_end = end(/*upDirection=*/true), d, d_end = end(/*upDirection=*/false);

	// correct links on lower concepts...
	for ( d = begin(/*upDirection=*/false); d != d_end; ++d )
	{
		// remove all down links
		for ( u = begin(/*upDirection=*/true); u != u_end; ++u )
			if ( (*d)->removeLink ( /*upDirection=*/true, *u ) )
				(*u)->removeLink ( /*upDirection=*/false, *d );

		// add new link between v and current
		(*d)->removeLink (/*upDirection=*/true, this);	// safe in general case, crucial for incremental
		(*d)->addNeighbour (/*upDirection=*/true, this);
	}

	// add new link between v and current
	for ( u = begin(/*upDirection=*/true); u != u_end; ++u )
		(*u)->addNeighbour ( /*upDirection=*/false, this );

	CHECK_LL_RETURN(llTaxInsert);

	LL << "\nTAX:inserting '" << getPrimer()->getName() << "' with up = {";

	u = begin(/*upDirection=*/true);
	if ( u != u_end )
	{
		LL << (*u)->getPrimer()->getName();
		for ( ++u; u != u_end; ++u )
			LL << "," << (*u)->getPrimer()->getName();
	}
	LL << "} and down = {";

	d = begin(/*upDirection=*/false);
	if ( d != d_end )
	{
		LL << (*d)->getPrimer()->getName();
		for ( ++d; d != d_end; ++d )
			LL << "," << (*d)->getPrimer()->getName();
	}
	LL << "}";
}

/// remove one half of a given node from a graph
void
TaxonomyVertex :: removeLinks ( bool upDirection )
{
	std::for_each ( begin(upDirection), end(upDirection), [=] (TaxonomyVertex* vertex) {
		vertex->removeLink ( !upDirection, this );
	});

	clearLinks(upDirection);
}

void TaxonomyVertex :: printSynonyms ( std::ostream& o ) const
{
	fpp_assert ( sample != nullptr );

	if ( likely(synonyms.empty()) )
		o << '"' << getPrimer()->getName() << '"';
	else
	{
		o << "(\"" << getPrimer()->getName();
		for ( syn_iterator q = begin_syn(), q_end = end_syn(); q < q_end; ++q )
			o << "\"=\"" << (*q)->getName();
		o << "\")";
	}
}

void TaxonomyVertex :: printNeighbours ( std::ostream& o, bool upDirection ) const
{
	// write number of elements
	o << " {" << neigh(upDirection).size() << ":";

	TVSet sorted ( begin(upDirection), end(upDirection) );
	for  ( auto vertex: sorted)
		o << " \"" << vertex->getPrimer()->getName() << '"';

	o << "}";
}

/// print taxonomy vertex in format <equals parents children>
void
TaxonomyVertex :: print ( std::ostream& o ) const
{
	printSynonyms(o);
	printNeighbours ( o, true );
	printNeighbours ( o, false );
	o << "\n";
}
