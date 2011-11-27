/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2011 by Dmitry Tsarkov

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

#ifndef TDAG2INTERFACE_H
#define TDAG2INTERFACE_H

#include "tDLExpression.h"
#include "tExpressionManager.h"
#include "dlDag.h"

/// class to translate DAG entities into the TDL* expressions
class TDag2Interface
{
protected:	// members
		/// DAG to be translated
	const DLDag& Dag;
		/// expression manager
	TExpressionManager* Manager;
		/// vector of cached concept expressions
	std::vector<const TDLConceptExpression*> TransC;
		/// vector of cached data expressions
	std::vector<const TDLDataExpression*> TransD;

protected:	// methods
		/// build concept expression by a vertex V
	const TDLConceptExpression* buildCExpr ( const DLVertex& v );
		/// build data expression by a vertex V
	const TDLDataExpression* buildDExpr ( const DLVertex& v );

public:		// interface
		/// init c'tor
	TDag2Interface ( const DLDag& dag, TExpressionManager* manager )
		: Dag(dag)
		, Manager(manager)
		, TransC(dag.size(),NULL)
		, TransD(dag.size(),NULL)
		{}
		/// empty d'tor: every newly created thing will be destroyed by manager
	~TDag2Interface ( void ) {}

		/// make sure that size of expression cache is the same as the size of a DAG
	void ensureDagSize ( void )
	{
		size_t ds = Dag.size(), ts = TransC.size();
		if ( likely(ds == ts) )
			return;
		TransC.resize(ds);
		TransD.resize(ds);
		if ( unlikely(ds>ts) )
			for ( ; ts != ds; ++ts )
			{
				TransC[ts] = NULL;
				TransD[ts] = NULL;
			}
	}
		/// get concept expression corresponding index of vertex
	const TDLConceptExpression* getCExpr ( BipolarPointer p )
	{
		if ( isNegative(p) )
			return Manager->Not(getCExpr(inverse(p)));
		if ( TransC[p] == NULL )
			TransC[p] = buildCExpr(Dag[p]);
		return TransC[p];
	}
		/// get data expression corresponding index of vertex
	const TDLDataExpression* getDExpr ( BipolarPointer p )
	{
		if ( isNegative(p) )
			return Manager->DataNot(getDExpr(inverse(p)));
		if ( TransD[p] == NULL )
			TransD[p] = buildDExpr(Dag[p]);
		return TransD[p];
	}
		/// get expression corresponding index of vertex given the DATA flag
	const TDLExpression* getExpr ( BipolarPointer p, bool data )
	{
		if ( data )
			return getDExpr(p);
		else
			return getCExpr(p);
	}
}; // TDag2Interface

#endif
