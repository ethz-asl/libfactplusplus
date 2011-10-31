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
		/// vector of cached expressions
	std::vector<const TDLExpression*> Trans;

protected:	// methods
		/// build concept expression by a vertex V
	const TDLConceptExpression* buildCExpr ( const DLVertex& v );
		/// build data expression by a vertex V
	const TDLDataExpression* buildDExpr ( const DLVertex& v );
		/// build expression by a vertex V given the DATA flag
	const TDLExpression* buildExpr ( const DLVertex& v, bool data )
	{
		if ( data )
			return buildDExpr(v);
		else
			return buildCExpr(v);
	}

public:		// interface
		/// init c'tor
	TDag2Interface ( const DLDag& dag, TExpressionManager* manager )
		: Dag(dag)
		, Manager(manager)
		, Trans(dag.size(),NULL)
		{}
		/// empty d'tor: every newly created thing will be destroyed by manager
	~TDag2Interface ( void ) {}

		/// make sure that size of expression cache is the same as the size of a DAG
	void ensureDagSize ( void )
	{
		size_t ds = Dag.size(), ts = Trans.size();
		if ( likely(ds == ts) )
			return;
		Trans.resize(ds);
		if ( ds > ts )
			while ( ts != ds )
				Trans[ts++] = NULL;
	}
		/// get concept expression corresponding index of vertex
	const TDLConceptExpression* getCExpr ( BipolarPointer p )
	{
		if ( isNegative(p) )
			return Manager->Not(getCExpr(inverse(p)));
		if ( Trans[p] == NULL )
			Trans[p] = buildCExpr(Dag[p]);
		return dynamic_cast<const TDLConceptExpression*>(Trans[p]);
	}
		/// get data expression corresponding index of vertex
	const TDLDataExpression* getDExpr ( BipolarPointer p )
	{
		if ( isNegative(p) )
			return Manager->DataNot(getDExpr(inverse(p)));
		if ( Trans[p] == NULL )
			Trans[p] = buildDExpr(Dag[p]);
		return dynamic_cast<const TDLDataExpression*>(Trans[p]);
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
