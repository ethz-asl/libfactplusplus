/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2007 by Dmitry Tsarkov

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

// This file contains methods for creating DAG representation of KB

#include "dlTBox.h"

void TBox :: buildDAG ( void )
{
	for ( c_const_iterator pc = c_begin(); pc != c_end(); ++pc )
		concept2dag(*pc);
	for ( i_const_iterator pi = i_begin(); pi != i_end(); ++pi )
		concept2dag(*pi);

	// build all GCIs
	DLTree* GCI = Axioms.getGCI();
	T_G = tree2dag(GCI);
	deleteTree(GCI);
}

/// register data expression in the DAG
BipolarPointer TBox :: addDataExprToHeap ( TDataEntry* p )
{
	if ( isValid(p->getBP()) )	// already registered value
		return p->getBP();

	// determine the type of an entry
	DagTag dt = p->isBasicDataType() ? dtDataType : p->isDataValue() ? dtDataValue : dtDataExpr;
	BipolarPointer hostBP = bpTOP;

	// register host type first (if any)
	if ( p->getType() != NULL )
		hostBP = addDataExprToHeap(const_cast<TDataEntry*>(p->getType()));

	// create new DAG entry for the data value
	DLVertex* ver = new DLVertex ( dt, hostBP );
	ver->setConcept(p);
	p->setBP(DLHeap.directAdd(ver));

	return p->getBP();
}

void TBox :: addConceptToHeap ( TConcept* pConcept )
{
	// choose proper tag by concept
	DagTag tag = pConcept->isPrimitive() ?
		(pConcept->isSingleton() ? dtPSingleton : dtPConcept):
		(pConcept->isSingleton() ? dtNSingleton : dtNConcept);

	// new concept's addition
	DLVertex* ver = new DLVertex(tag);
	ver->setConcept(pConcept);
	pConcept->pName = DLHeap.directAdd(ver);

	BipolarPointer desc = bpTOP;

	// translate body of a concept
	if ( pConcept->Description != NULL )	// complex concept
		desc = tree2dag(pConcept->Description);
	else			// only primivive concepts here
		assert ( pConcept->isPrimitive() );

	// update concept's entry
	pConcept->pBody = desc;
	ver->setChild(desc);
}

BipolarPointer TBox :: tree2dag ( const DLTree* t )
{
	if ( t == NULL )
		return bpINVALID;	// invalid value

	const TLexeme& cur = t->Element();

	switch ( cur.getToken() )
	{
	case BOTTOM:	// it is just !top
		return bpBOTTOM;
	case TOP:		// the 1st node
		return bpTOP;
	case DATAEXPR:	// data-related expression
		return addDataExprToHeap ( static_cast<TDataEntry*>(cur.getName()) );
	case CNAME:		// concept name
		return concept2dag(getConcept(cur.getName()));
	case INAME:		// individual name
		return concept2dag(getIndividual(cur.getName()));

	case NOT:
		return inverse ( tree2dag ( t->Left() ) );

	case AND:
		return and2dag(t);

	case FORALL:
		return forall2dag ( role2dag(t->Left()), tree2dag(t->Right()) );

	case REFLEXIVE:
		return reflexive2dag(role2dag(t->Left()));

	case LE:
		return atmost2dag ( cur.getData(), role2dag(t->Left()), tree2dag(t->Right()) );

	default:
		assert ( isSNF(t) );	// safety check
		assert (0);				// extra safety check ;)
		return bpINVALID;
	}
}

/// fills AND-like vertex V with an AND-like expression T; process result
BipolarPointer
TBox :: and2dag ( DLVertex* v, const DLTree* t )
{
	BipolarPointer ret = bpBOTTOM;

	if ( fillANDVertex ( v, t ) )
	{	// clash found
		// sorts are broken now (see bTR11)
		useSortedReasoning = false;
		delete v;
	}
	else	// AND vertex
		switch ( v->end() - v->begin() )
		{
		case 0:	// and(TOP) = TOP
			delete v;
			return bpTOP;
		case 1:	// and(C) = C
			ret = *v->begin();
			delete v;
			break;
		default:
			ret = DLHeap.add(v);
			break;
		}

	return ret;
}

BipolarPointer TBox :: forall2dag ( const TRole* R, BipolarPointer C )
{
	if ( R->isDataRole() )
		return dataForall2dag(R,C);

	// create \all R.C == \all R{0}.C
	BipolarPointer ret = DLHeap.add ( new DLVertex ( dtForall, 0, R, C ) );

	if ( R->isSimple() )	// don't care about the rest
		return ret;

	// check if the concept is not last
	if ( !DLHeap.isLast(ret) )
		return ret;		// all sub-roles were added before

	// have appropriate concepts for all the automata states
	for ( unsigned int i = 1; i < R->getAutomaton().size(); ++i )
		DLHeap.directAddAndCache ( new DLVertex ( dtForall, i, R, C ) );

	return ret;
}

BipolarPointer TBox :: atmost2dag ( unsigned int n, const TRole* R, BipolarPointer C )
{
	// input check: only simple roles are allowed in the (non-trivial) NR
	if ( !R->isSimple() )
		throw EFPPNonSimpleRole(R->getName());

	if ( R->isDataRole() )
		return dataAtMost2dag(n,R,C);

	BipolarPointer ret = DLHeap.add ( new DLVertex ( dtLE, n, R, C ) );

	// check if the concept is not last
	if ( !DLHeap.isLast(ret) )
		return ret;		// all elements were added before

	// create entries for the transitive sub-roles
	for ( unsigned int m = n-1; m > 0; --m )
		DLHeap.directAddAndCache ( new DLVertex ( dtLE, m, R, C ) );

	return ret;
}

bool TBox :: fillANDVertex ( DLVertex* v, const DLTree* t )
{
	if ( t->Element().getToken() == AND )
		return fillANDVertex ( v, t->Left() ) || fillANDVertex ( v, t->Right() );
	else
		return v->addChild ( tree2dag(t) );
}
