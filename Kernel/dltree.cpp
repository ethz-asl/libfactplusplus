/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2009 by Dmitry Tsarkov

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

#include "dltree.h"

#include <cassert>
#include <iostream>

	/// create inverse of role R
DLTree* createInverse ( DLTree* R )
{
	// check for empty argument
	if ( R == NULL )
		return NULL;

	switch ( R->Element().getToken() )
	{
	case UROLE:	// U- = U
		return R;
	case INV:	// R-- = R
	{
		DLTree* p = R;
		R = R->Left();
		delete p;
		return R;
	}
	case RNAME:	// role name
		return new DLTree ( INV, R );
	default:	// error
		return NULL;
	}
}

	/// create negation of given formula
DLTree* createSNFNot ( DLTree* C )
{
	// check for empty argument (possible in OR processing)
	if ( C == NULL )
		return NULL;
	// \not F = T
	if ( C->Element() == BOTTOM )
		C->Element() = TOP;
	// \not T = F
	else if ( C->Element() == TOP )
		C->Element() = BOTTOM;
	// \not\not C = C
	else if ( C->Element () == NOT )
	{	// delete p; return p->Left
		DLTree* p = C;
		C = C->Left();
		delete p;
	}
	// general case
	else
		C = new DLTree ( NOT, C );

	return C;
}

	/// create conjunction of given formulas
DLTree* createSNFAnd ( DLTree* C, DLTree* D )
{
	// try to simplify conjunction
	if ( C == NULL )	// single element
		return D;
	if ( D == NULL )
		return C;

	if ( C->Element() == TOP ||		// T\and D = D
		 D->Element() == BOTTOM )	// C\and F = F
	{
		deleteTree(C);
		return D;
	}

	if ( D->Element() == TOP ||		// C\and T = C
		 C->Element() == BOTTOM )	// F\and D = F
	{
		deleteTree(D);
		return C;
	}

	// no simplification possible -- return actual conjunction
	return new DLTree ( AND, C, D );
}

static bool
containsC ( DLTree* C, DLTree* D )
{
	switch ( C->Element().getToken() )
	{
	case CNAME:
		return equalTrees ( C, D );
	case AND:
		return containsC ( C->Left(), D ) || containsC ( C->Right(), D );
	default:
		return false;
	}
}

DLTree* createSNFReducedAnd ( DLTree* C, DLTree* D )
{
	if ( C == NULL || D == NULL )
		return createSNFAnd ( C, D );

	if ( D->Element().getToken() == CNAME && containsC ( C, D ) )
	{
		delete D;
		return C;
	}
	else if ( D->Element().getToken() == AND )
	{
		C = createSNFReducedAnd ( C, D->Left() );
		C = createSNFReducedAnd ( C, D->Right() );
		delete D;	// just an AND
		return C;
	}
	else	// can't optimise
		return createSNFAnd ( C, D );
}

	/// create universal restriction of given formulas (\AR.C)
DLTree* createSNFForall ( DLTree* R, DLTree* C )
{
	if ( C->Element() == TOP )	// \AR.T = T
	{
		deleteTree(R);
		return C;
	}
	else	// no simplification possible
		return new DLTree ( FORALL, R, C );
}

	/// create at-least (GE) restriction of given formulas (>= n R.C)
DLTree* createSNFGE ( unsigned int n, DLTree* R, DLTree* C )
{
	if ( n == 0 )
	{		// >= 0 R.C -> T
		deleteTree(R);
		deleteTree(C);
		return new DLTree ( TOP );
	}
	if ( C->Element() == BOTTOM )
	{		// >=n R.F -> F
		deleteTree(R);
		return C;
	}
	else	// >= n R.C -> !<= (n-1) R.C
		return createSNFNot ( createSNFLE ( n-1 , R, C ) );
}

//********************************************************************************************
//**	equalTrees implementation
//********************************************************************************************
bool equalTrees ( const DLTree* t1, const DLTree* t2 )
{
	// empty trees are equal
	if ( t1 == NULL && t2 == NULL )
		return true;

	// empty and non-empty trees are not equal
	if ( t1 == NULL || t2 == NULL )
		return false;

	// non-empty trees are checked recursively
	return ( t1->Element() == t2->Element() ) &&
		   equalTrees ( t1->Left(), t2->Left() ) &&
		   equalTrees ( t1->Right(), t2->Right() );
}

bool isSubTree ( const DLTree* t1, const DLTree* t2 )
{
	if ( t1 == NULL || t1->Element() == TOP )
		return true;
	if ( t2 == NULL )
		return false;
	if ( t1->Element() == AND )
		return isSubTree ( t1->Left(), t2 ) && isSubTree ( t1->Right(), t2 );
	// t1 is a single elem, t2 is a (probably) AND-tree
	if ( t2->Element() == AND )
		return isSubTree ( t1, t2->Left() ) || isSubTree ( t1, t2->Right() );
	// t1 and t2 are non-single elements
	return equalTrees(t1,t2);
}
//********************************************************************************************
//**	OnlySNF realization
//********************************************************************************************
bool isSNF ( const DLTree* t )
{
	if ( t == NULL )
		return true;

	switch ( t -> Element (). getToken () )
	{
	case TOP:
	case BOTTOM:
	case NAME:
	case DATAEXPR:
	case NOT:
	case INV:
	case AND:
	case FORALL:
	case LE:
	case REFLEXIVE:
		return ( isSNF (t->Left()) && isSNF (t->Right()) );
	default:
		return false;
	}
}

//********************************************************************************************
const char* TokenName ( Token t )
{
	switch ( t )
	{
	case TOP:		return "*TOP*";
	case BOTTOM:	return "*BOTTOM*";
	case UROLE:		return "*UROLE*";
	case CNAME:		return "cname";
	case INAME:		return "iname";
	case RNAME:		return "rname";
	case DATAEXPR:	return "dataexpr";
	case CONCEPT:	return "concept";
	case PCONCEPT:	return "primconcept";
	case INV:		return "inv";
	case OR:		return "or";
	case AND:		return "and";
	case NOT:		return "not";
	case EXISTS:	return "some";
	case FORALL:	return "all";
	case GE:	return "at-least";
	case LE:	return "at-most";
	case REFLEXIVE: return "self-ref";
	default:
		std::cerr << "token " << t << "has no name";
					assert ( 0 );
					return NULL;
	};
}

std::ostream& operator << ( std::ostream& o, const DLTree *form )
{
	if ( form == NULL )
		return o;

	const TLexeme& lex = form->Element();
	switch ( lex.getToken() )
	{
	case TOP:
	case BOTTOM:
	case UROLE:
		o << ' ' << TokenName(lex.getToken());
		break;
	case NAME:
	case DATAEXPR:	// FIXME!! later on -- print restrictions
		o << ' ' << lex.getName()->getName();
		break;

	case NOT:
	case INV:
	case REFLEXIVE:
		o << " (" << TokenName (lex.getToken()) << form->Left() << ')';
		break;

	case AND:
	case OR:
	case EXISTS:
	case FORALL:
		o << " (" << TokenName (lex.getToken()) << form->Left() << form->Right() << ')';
		break;

	case GE:
	case LE:
		o << " (" << TokenName (lex.getToken()) << ' ' << lex.getData()
		  << form->Left() << form->Right() << ')';
		break;

	default:
		break;
	}
	return o;
}

//********************************************************************************************
