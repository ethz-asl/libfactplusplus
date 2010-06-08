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

#include "tSubsumptionMap.h"

TSubsumptionMap :: TSubsumptionMap ( void )
	: K(2)
{
	P.resize(2);
	P_K.resize(2);
	P[0].push_back(true);
	P[0].push_back(true);
	P[1].push_back(true);
	P[1].push_back(true);
	P_K[0].push_back(true);
	P_K[0].push_back(true);
	P_K[1].push_back(true);
	P_K[1].push_back(true);

	addKnoSub ( TopVertex, TopVertex );
	addKnoSub ( BotVertex, BotVertex );
	addKnoSub ( BotVertex, TopVertex );
}

TSubsumptionMap :: TSubsumptionMap ( unsigned int n )
	: K(n+2)
{
	n += 2;	// top/bottom

	bvec bTrue ( n, true );

	P.insert ( P.end(), n, bTrue );
	P_K.insert ( P_K.end(), n, bTrue );

	// init subsumptions Bot [= C [= Top, C [= C
	for ( Vertex v = 0; v < n; ++v )
	{
		addKnoSub ( v, v );
		addKnoSub ( BotVertex, v );
		addKnoSub ( v, TopVertex );
	}
}

TSubsumptionMap::Vertex
TSubsumptionMap :: addVertex ( void )
{
	// add new vertex to K
	Vertex ret = K.newVertex();
	// add new vertex to existing rows of P, P_K
	for ( unsigned int i = 0; i < ret; ++i )
	{
		P[i].push_back(true);
		P_K[i].push_back(true);
	}
	// add new tautology vec to P, P_K
	bvec bTrue ( ret+1, true );
	P.push_back(bTrue);
	P_K.push_back(bTrue);

	// subsumption relation is reflexive
	addKnoSub ( ret, ret );

	return ret;
}

void
TSubsumptionMap :: addNonSub ( Vertex x, Vertex y )
{
	fpp_assert ( x < size() );
	fpp_assert ( y < size() );

	P[x][y] = false;

	if ( P_K[x][y] == false )
		return;

	// adjust [P]K (Prune-possibles, lines 2-4)
	P_K[x][y] = false;
	for ( Vertex i = 0; i < K.size(); ++i )
		if ( K.R(x,i) )
			for ( Vertex j = 0; j < K.size(); ++j )
				if ( K.R(j,y) )
					P_K[i][j] = false;
}

bool
TSubsumptionMap :: addKnoSub ( Vertex x, Vertex y )
{
	fpp_assert ( x < size() );
	fpp_assert ( y < size() );

	if ( K.R(x,y) )
		return false;	// no new cycles here

	bool ret = K.insert(x,y);

	// adjust [P]K (Prune-possibles, lines 6-13)
	Vertex i, j;
	for ( i = 0; i < K.size(); ++i )
		if ( P_K[y][i] )	// <u,v> in [P]K
			for ( j = 0; j < K.size(); ++j )
				if ( K.R(i,j) && !P[x][j] )
				{
					P_K[y][i] = false;
					break;
				}

	for ( i = 0; i < K.size(); ++i )
		if ( P_K[i][x] )	// <u,v> in [P]K
			for ( j = 0; j < K.size(); ++j )
				if ( K.R(j,i) && !P[j][y] )
				{
					P_K[i][x] = false;
					break;
				}

	if ( ret )
		std::cerr << "Cycle found with " << x << " and " << y << "\n";
	return ret;
}
