/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2010 by Dmitry Tsarkov

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

#ifndef TSUBSUMPTIONMAP_H
#define TSUBSUMPTIONMAP_H

#include "TriRelation.h"
#include "tIndividual.h"

class TSubsumptionMap
{
protected: 	// typedefs
		/// vertex type
	typedef TriRelation::Vertex Vertex;
		/// boolean vector
	typedef TriRelation::bvec bvec;
		/// boolean matrix
	typedef TriRelation::bMatrix bMatrix;

protected:	// members
		/// set of known subsumptions
	TriRelation K;
		/// set of possible subsumptions
	TriRelation::bMatrix P;
		/// reduction of P wrt K
	TriRelation::bMatrix P_K;

		/// index of a TOP vertex
	static const Vertex TopVertex = 0;
		/// index of a BOTTOM vertex
	static const Vertex BotVertex = 1;

protected:	// methods
		/// add vertex to the map
	Vertex addVertex ( void );
		/// add known subsumption between X and Y; @return true iff cycle was found
	bool addKnoSub ( Vertex x, Vertex y );
		/// add known non-subsumption between X and Y
	void addNonSub ( Vertex x, Vertex y );

public:		// interface
		/// c'tor: create Top/Bottom
	TSubsumptionMap ( void );
		/// c'tor: create Top/Bottom and N verteces
	TSubsumptionMap ( unsigned int n );
		/// empty d'tor
	~TSubsumptionMap ( void ) {}

		/// register new classifiable entry in the map
	void registerEntry ( ClassifiableEntry* p )
	{
		Vertex v = addVertex();
		addKnoSub ( BotVertex, v );
		addKnoSub ( v, TopVertex );
		if ( dynamic_cast<TIndividual*>(p) != NULL )
			addNonSub ( v, BotVertex );
		p->setIndex(v);
	}
		/// size of the matrix
	unsigned int size ( void ) const { return K.size(); }

		/// add known subsumption between P and Q
	bool addKnoSub ( const ClassifiableEntry* p, const ClassifiableEntry* q )
		{ return addKnoSub ( p->index(), q->index() ); }
		/// add known non-subsumption between P and Q
	void addNonSub ( const ClassifiableEntry* p, const ClassifiableEntry* q )
		{ addNonSub ( p->index(), q->index() ); }

		/// @return true iff subsumption between P and Q is possible
	bool isSubPossible ( const ClassifiableEntry* p, const ClassifiableEntry* q ) const { return P_K[p->index()][q->index()]; }
		/// @return true iff subsumption between P and Q is known
	bool isSubKnown ( const ClassifiableEntry* p, const ClassifiableEntry* q ) const { return K.RStar ( p->index(), q->index() ); }
}; // TSubsumptionMap

#endif
