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

#include "TriRelation.h"

void
TriRelation :: init ( unsigned int n )
{
	// empty vecs
	bvec bZero ( n, false );
	nvec nZero ( n, 0 );

	// create all matrices
	M.insert ( M.end(), n, bZero );
	MStar.insert ( MStar.end(), n, bZero );
	MMinus.insert ( MMinus.end(), n, bZero );
	N.insert ( N.end(), n, nZero );
	M_c.insert ( M_c.end(), n, nZero );

	// every node is in it's own component
	V_c.insert ( V_c.end(), n, true );
	e_c.insert ( e_c.end(), n, 0 );

	C_c.resize(n);
	for ( Vertex i = 0; i < n; ++i )
	{
		L.push_back(i);
		C_c[i].push_back(i);
	}

	// empty successors/predecessors
	S.resize(n);
	P.resize(n);
	S_c.resize(n);
	P_c.resize(n);

	last = n;
}

TriRelation::Vertex
TriRelation :: newVertex ( void )
{
	// make existing arrays bigger
	for ( unsigned int i = 0; i < last; ++i )
	{
		M[i].push_back(false);
		MStar[i].push_back(false);
		MMinus[i].push_back(false);
		N[i].push_back(0);
		M_c[i].push_back(0);
	}

	// every node is in it's own component
	V_c.push_back(true);
	e_c.push_back(0);

	C_c.resize(last+1);
	L.push_back(last);
	C_c.back().push_back(last);

	++last;

	// empty vecs
	bvec bZero ( last, false );
	nvec nZero ( last, 0 );

	// reserve space for new vertex
	M.push_back(bZero);
	MStar.push_back(bZero);
	MMinus.push_back(bZero);
	N.push_back(nZero);
	M_c.push_back(nZero);

	// empty successors/predecessors
	S.resize(last);
	P.resize(last);
	S_c.resize(last);
	P_c.resize(last);

	return last-1;
}

bool
TriRelation :: insertNew ( Vertex i, Vertex j )
{
	setM(i,j);
	Vertex Li = L[i], Lj = L[j];

	// the link is in the same component
	if ( Li == Lj )
	{
		adjustEc(i,j);
		for ( Vertex k = 0; k < size(); ++k )
			if ( RStar(k,i) )
				++N[k][j];
		return false;
	}
	// the link is between components
	bool newCycle = RStar(j,i);
	setMc(Li,Lj);
	// check all the condenced verteces k with a path to i
	for ( Vertex k = 0; k < size(); ++k )
		if ( V_c[k] && RStar(k,i) )
		{
			for ( nvec::iterator p = C_c[k].begin(), p_end = C_c[k].end(); p < p_end; ++p )
				++N[k][j];
			if ( RStar(k,j) )
			{	// there is path from k to j as well
				if ( (k != Li) && (k != Lj) )
					MMinus[k][Lj] = 0;
			}
			else
			{
				if ( k == Li )
					MMinus[Li][Lj] = 1;
				redStack.push_back(Lj);
				adapt(k);
			}
		}

	// join components if a new cycle happen
	if ( newCycle )
		joinComponents(j);
	return newCycle;
}

void
TriRelation :: adapt ( Vertex k )
{
	nvec::iterator ki, mi, li, ki_end = C_c[k].end(), mi_end, li_end;
	do
	{
		/// L is the red leader
		Vertex l = redStack.back();
		// unmark l
		redStack.pop_back();
		li_end = C_c[l].end();

		// there is a path between K and L
		for ( ki = C_c[k].begin(); ki < ki_end; ++ki )
			for ( li = C_c[l].begin(); li < li_end; ++li )
				MStar[*ki][*li] = true;

		// update input information
		for ( li = C_c[l].begin(); li < li_end; ++li )
			for ( mi = S[*li].begin(), mi_end = S[*li].end(); mi < mi_end; ++mi )
				for ( ki = C_c[k].begin(); ki < ki_end; ++ki )
					++N[*ki][*mi];

		// check all the successors of L
		for ( mi = S_c[l].begin(), mi_end = S_c[l].end(); mi < mi_end; ++mi )
			if ( MStar[k][*mi] )
				MMinus[k][*mi] = false;
			else
				redStack.push_back(*mi);

	} while ( !redStack.empty() );
}

void
TriRelation :: joinComponents ( Vertex j )
{
	nvec::iterator ki, mi, li, ki_end, mi_end, li_end;

	const Vertex n = size();	// record size
	nvec La;	// \Lambda
	Vertex l, l0 = n;	// \lambda_0

	// build \Lambda, lambda0
	for ( l = 0; l < n; ++l )
		if ( MStar[l][j] && MStar[j][l] )
		{
			if ( l0 > l )	// here we init l_0
				l0 = l;			// line 3
			La.push_back(l);	// line 2
		}

	// Make M-[k,m] = 0 for k, m \in V_c s.t. either k or m is in La:
	for ( li = La.begin(), li_end = La.end(); li != li_end; ++li )
		if ( V_c[*li] )
			for ( l = 0; l < n; ++l )
				if ( V_c[l] )
				{
					MMinus[l][*li] = false;	// line 4 part 1
					MMinus[*li][l] = false;	// line 4 part 2
				}

	// line 5:
	// clear M_c
	M_c.clear();
	nvec nZero ( n, 0 );
	M_c.insert ( M_c.end(), n, nZero );
	// clear P_c
	P_c.clear();
	P_c.resize(n);
	// clear S_c
	S_c.clear();
	S_c.resize(n);
	// clear e_c
	e_c.clear();
	e_c.insert ( e_c.end(), n, 0 );

	// Make M-[k,k+1] = 0 for k, m \in C(l) where l\in V_c and La:
	for ( li = La.begin(); li != li_end; ++li )
		if ( V_c[*li] )	// line 6
		{
			nvec& Ck = C_c[*li];
			for ( ki = Ck.begin(), ki_end = Ck.end(); ki+1 != ki_end; ++ki )
				MMinus[*ki][*(ki+1)] = false;	// line 7
			// here ki points to the last element of Ck
			MMinus[*ki][*(Ck.begin())] = false;	// line 7
		}

	// mark leader for Lambda
	for ( li = La.begin(); li != li_end; ++li )
		L[*li] = l0;	// line 10

	// copy Lambda as a C_c of l0
	C_c[l0] = La;		// line 11

	// adjust M_c and e_c by enumerating all edges
	for ( Vertex i = 0; i < n; ++i )
	{
		Vertex Li = L[i];
		for ( Vertex j = 0; j < n; ++j )
			if ( M[i][j] )
			{
				Vertex Lj = L[j];
				if ( Li == Lj )	// same component
					adjustEc(i,j);	// line 12
				else
					setMc(Li,Lj);	// line 12
			}
	}

	// adjust V_c
	for ( li = La.begin(); li != li_end; ++li )
		V_c[*li] = false;	// line 13
	V_c[l0] = true;			// line 13

	// adjust M- for the component Lambda
	for ( li = La.begin(); li+1 != li_end; ++li )
		MMinus[*li][*(li+1)] = true;	// line 14
	// here li points to the last element of Lambda
	MMinus[*li][*(La.begin())] = true;	// line 15

	// adjust M-(l0,*) and M-(*,l0)
	for ( l = 0; l < n; ++l )
	{
		Vertex Li = L[l];
		if ( M_c[l0][Li] )			// line 20
			MMinus[l0][Li] = true;	// line 21
		if ( M_c[Li][l0] )			// line 26
			MMinus[Li][l0] = true;	// line 27
	}
}
