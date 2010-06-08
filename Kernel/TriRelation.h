/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2009-2010 by Dmitry Tsarkov

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

#ifndef TRIRELATION_H
#define TRIRELATION_H

#include <vector>
#include <iostream>

#include "globaldef.h"

// comment this out when TriRelation would need delete edges
#define FPP_NO_DELETE_EDGE

// uncomment the following line to debug tri-relation
//#define FPP_DEBUG_TRI_RELATION

/// class for the binary relation, it's transitive closure and its transitive reduction
class TriRelation
{
public:		// typedefs
		/// typedef for the vertex
	typedef unsigned int Vertex;
		/// bit vector
	typedef std::vector<bool> bvec;
		/// bit matrix
	typedef std::vector<bvec> bMatrix;
		/// int vector
	typedef std::vector<Vertex> nvec;
		/// int matrix
	typedef std::vector<nvec> nMatrix;
		/// int sets: for every index represents an array of verteces
	typedef std::vector<nvec> nSet;

protected:	// members
		/// adjacency matrix for the basic relation R
	bMatrix M;
		/// adjacency matrix for R's transitive closure R*
	bMatrix MStar;
		/// adjacency matrix for R's transitive reduction R-
	bMatrix MMinus;
#ifndef FPP_NO_DELETE_EDGE
		/// R's input matrix
	nMatrix N;
		/// successor's relation for R
	nSet S;
		/// predecessor's relation for R
	nSet P;
#endif
		/// set of verteces of the condencend graph V^c: V_c[v] == true iff v\in V^c
	bvec V_c;
		/// map between verteces and SCC representors (leaders)
	nvec L;
		/// successor's relation for G^c
	nSet S_c;
		/// predecessor's relation for G^c
	nSet P_c;
		/// component vectors (maps leader to the set of its component)
	nSet C_c;
		/// weighted adjacency matrix of the condenced graph
	nMatrix M_c;
#ifndef FPP_NO_DELETE_EDGE
		/// array of links from the vertex landing in the same component
	nvec e_c;
#endif
		/// stack for the red nodes
	nvec redStack;
		/// number of verteces
	Vertex last;

protected:	// methods
		/// maps vertex X to its component
	nvec C ( Vertex x ) { return C_c[L[x]]; }
		/// add an edge from X to Y in the main relation; adjust S and P
	void setM ( Vertex x, Vertex y )
	{
		M[x][y] = true;
		MStar[x][y] = true;
#	ifndef FPP_NO_DELETE_EDGE
		S[x].push_back(y);
		P[y].push_back(x);
#	endif
	}
		/// add condenced node X to a set of condenced nodes S
	void addCondenced ( nvec& s, Vertex x )
	{
		for ( nvec::iterator p = s.begin(), p_end = s.end(); p < p_end; ++p )
			if ( x == *p )
				return;
		s.push_back(x);
	}
		/// add an edge between X and Y in the condenced relation M_c; adjust S^c and P^c
	void setMc ( Vertex x, Vertex y )
	{
		++M_c[x][y];
		addCondenced ( S_c[x], y );
		addCondenced ( P_c[y], x );
	}
		/// add an edge from X to Y in the same SCC
	void adjustEc ( Vertex x ATTR_UNUSED, Vertex y ATTR_UNUSED )
	{
#	ifndef FPP_NO_DELETE_EDGE
		++e_c[y];
#	endif
//		S[x].push_back(y);
	}
		/// init all the graph structures assuming N verteces and no edges
	void init ( unsigned int n );

	// insert support

		/// insert a non-existing edge between X and Y; @return true iff a new cycle appears
	bool insertNew ( Vertex x, Vertex y );
		/// ADAPT helper to the insert procedure
	void adapt ( Vertex k );
		/// JOIN COMPONENTS helper to the insert procedure: C(J) will have new members
	void joinComponents ( Vertex j );

public:		// interface
		/// c'tor for a graph with N nodes
	TriRelation ( unsigned int n = 1 ) { init(n); }
		/// empty d'tor
	~TriRelation ( void ) {}

		/// add a vertex to a graph; @return index of a new vertex
	Vertex newVertex ( void );
		/// insert an edge between verteces X and Y; @return true iff a new cycle appears
	bool insert ( Vertex x, Vertex y )
	{
		if ( !M[x][y] )
			return insertNew(x,y);
#	ifdef FPP_DEBUG_TRI_RELATION
		std::cerr << "Insert known R(" << x << "," << y << ")\n";
#	endif
		return false;
	}

		/// @return true iff R(X,Y) holds
	bool R ( Vertex x, Vertex y ) const { return M[x][y]; }
		/// @return true iff R*(X,Y) holds
	bool RStar ( Vertex x, Vertex y ) const { return MStar[x][y]; }
		/// @return true iff R-(X,Y) holds
	bool RMinus ( Vertex x, Vertex y ) const { return MMinus[L[x]][L[y]]; }
		/// @return number of verteces in a graph
	Vertex size ( void ) const { return last; }
}; // TriRelation

#endif
