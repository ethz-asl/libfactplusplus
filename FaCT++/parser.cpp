/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2008 by Dmitry Tsarkov

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

#include "parser.h"

#include <ctype.h>
#include <string>
#include <iomanip>
#include <sstream>

/*********************  DLParser implementation  ***************************/

void DLLispParser :: Parse ( void )
{
	while ( Current != LEXEOF )
		parseCommand ();
}

void DLLispParser :: parseCommand ( void )
{
	MustBeM (LBRACK);
	MustBe(ID);
	Token t = scan.getCommandKeyword();
	NextLex ();

	if ( t == SUBSUMES || t == EQUAL_C )
	{
		DLTree* left = processConceptTree ();
		DLTree* right = processConceptTree ();

		bool fail;

		if ( t == SUBSUMES )
			fail = Kernel->impliesConcepts ( left, right );
		else
			fail = Kernel->equalConcepts ( left, right );

		if ( fail )
			parseError ( "Undefined concept name" );

		MustBeM ( RBRACK );
		return;
	}

	if ( t == IMPLIES_R || t == EQUAL_R || t == DISJOINT_R || t == INVERSE )
	{
		DLTree* left = getRoleExpression(/*allowChain=*/(t == IMPLIES_R));
		DLTree* right = getRoleExpression(/*allowChain=*/false);

		bool fail;

		if ( t == IMPLIES_R )
			fail = Kernel->impliesRoles ( left, right );
		else if ( t == EQUAL_R )
			fail = Kernel->equalRoles ( left, right );
		else if ( t == INVERSE )
		{
			right = createInverse(right);
			fail = Kernel->equalRoles ( left, right );
		}
		else
			fail = Kernel->disjointRoles ( left, right );

		if ( fail )
			parseError ( "Undefined role name in role axiom" );

		// clean role stuff
		deleteTree(left);
		deleteTree(right);

		MustBeM ( RBRACK );
		return;
	}

	if ( t == FUNCTIONAL ||
		 t == TRANSITIVE ||
		 t == REFLEXIVE ||
		 t == IRREFLEXIVE ||
		 t == SYMMETRIC ||
		 t == ANTISYMMETRIC ||
		 t == ROLERANGE ||
		 t == ROLEDOMAIN )
	{
		DLTree* R = getRoleExpression(/*allowChain=*/false);
		bool fail;

		switch (t)
		{
		case FUNCTIONAL:
			fail = Kernel->setFunctional(R);
			break;
		case TRANSITIVE:
			fail = Kernel->setTransitive(R);
			break;
		case REFLEXIVE:
			fail = Kernel->setReflexive(R);
			break;
		case IRREFLEXIVE:
			fail = Kernel->setIrreflexive(R);
			break;
		case SYMMETRIC:
			fail = Kernel->setSymmetric(R);
			break;
		case ANTISYMMETRIC:
			fail = Kernel->setAntiSymmetric(R);
			break;
		case ROLERANGE:
			fail = Kernel->setRange ( R, processConceptTree() );
			break;
		case ROLEDOMAIN:
			fail = Kernel->setDomain ( R, processConceptTree() );
			break;
		default:
			assert (0);
		}

		if ( fail )
			parseError ( "Undefined role name in role property command" );

		delete R;

		MustBeM ( RBRACK );
		return;
	}

	// disjoint-like operator
	if ( t == DISJOINT ||
		 t == DIFFERENT ||
		 t == SAME )
	{
		parseConceptList ( /*singletonsOnly=*/ t != DISJOINT );
		switch (t)
		{
		case DISJOINT:
			if ( Kernel->processDisjoint () )
				parseError ( "Singular Disjoint statement" );
			return;		// already read ')'
		case SAME:
			if ( Kernel->processSame () )
				parseError ( "Individual names expected in 'same' statement" );
			return;		// already read ')'
		case DIFFERENT:
			if ( Kernel->processDifferent () )
				parseError ( "Individual names expected in 'different' statement" );
			return;		// already read ')'
		default:
			assert (0);
		}
	}

	// not GCI -- first argument is a name determined by an axiom
	DLTree* Name = NULL;

	// set up creation mode
	switch (t)
	{
	case PCONCEPT:
		Name = getConcept();
		if ( Current != RBRACK )
			Kernel->impliesConcepts ( Name, processConceptTree() );
		else
			delete Name;
		break;

	case CONCEPT:
		Name = getConcept();
		Kernel->equalConcepts ( Name, processConceptTree() );
		break;

	case DEFINDIVIDUAL:		// just register singleton
		delete getSingleton();
		break;

	case INSTANCE:
		Name = getSingleton();
		Kernel->instanceOf ( Name, processConceptTree() );
		delete Name;
		break;

	case RELATED:			// command is (Related id1 R id2);
	{
		DLTree* id1 = getSingleton();
		DLTree* R = getRoleExpression(/*allowChain=*/false);
		MustBe (ID);	// second indiv.
		DLTree* id2 = getSingleton();
		if ( Kernel->relatedTo ( id1, R, id2 ) )
			parseError ( "Unsupported feature -- relatedTo" );
		deleteTree(id1);
		deleteTree(R);	// clear role tree
		deleteTree(id2);	// pawel.kaplanski@gmail.com
		break;
	}

	case PROLE:
		Name = getRole();
		if ( Current != RBRACK )
			parseRoleArguments(Name);
		else
			delete Name;
		break;

	case PATTR:
		Name = getRole();

		if ( Kernel->setFunctional(Name) )
			parseError ( "Role name already registered" );

		if ( Current != RBRACK )
			parseRoleArguments(Name);
		else
			delete Name;
		break;

	case DATAROLE:		// register data role
		delete getDataRole();
		break;

	case FAIRNESS:
		Name = getConcept();
		if ( Kernel->setFairnessConstraint(Name) )
			parseError ( "Second fairness constraint or not a primitive named concept" );
		deleteTree(Name);
		break;

	default:
		parseError ( "Unrecognised command" );
	}

	MustBeM ( RBRACK );	// skip bracket
}

void DLLispParser :: parseConceptList ( bool singletonsOnly )
{
	Kernel->openArgList();

	// continue with all concepts
	while ( Current != RBRACK )
		Kernel->addArg ( singletonsOnly ? getSingleton() : processConceptTree() );

	// skip RBRACK
	MustBeM (RBRACK);
}

void DLLispParser :: parseRoleArguments ( DLTree* R )
{
	while ( Current != RBRACK )
		if ( scan.isKeyword ("parents") || scan.isKeyword ("supers") )
		{	// followed by a list of parent role names
			NextLex ();
			MustBeM ( LBRACK );

			while ( Current != RBRACK )
			{
				DLTree* S = getRoleExpression(/*allowChain=*/false);
				if ( Kernel->impliesRoles ( R, S ) )
					parseError ( "Role name not registered in :parents definition" );
				deleteTree(S);
			}
			NextLex ();	// skip last RBRACK
		}
		else if ( scan.isKeyword ("transitive") )
		{	// followed by NIL or (usually!) T
			NextLex ();

			if ( Kernel->setTransitive (R) )
				parseError ( "Role name not registered in :transitive definition" );

			NextLex ();	// skip token
		}
		else
			parseError ( "use either :parents or :transitive command in role description" );

	delete R;
}

DLTree* DLLispParser :: processConceptTree ( void )
{
	switch ( Code() )
	{
	case LBRACK:	// complex description
		return processComplexConceptTree ();
	case NUM:		// numbers in concept expressions are constant names
		return getConcept();
	case ID:
	{
		Token T = scan.getNameKeyword();

		if ( T != ID )	// TOP or BOTTOM
		{
			NextLex ();
			return new DLTree (T);
		}
		// else -- concept name
		return getConcept();
	}
	default:	// else -- report syntax error
		MustBe(ID);
		return NULL;
	}
}

DLTree* DLLispParser :: processComplexConceptTree ( void )
{
	MustBeM ( LBRACK );
	Token T = scan.getExpressionKeyword();
	unsigned int n = 0;	// number for >= (<=) expression (or just 0)
	DLTree *left = NULL, *right = NULL;

	NextLex ();

	switch (T)
	{
	case DTGT:
		left = processConceptTree();
		MustBeM(RBRACK);
		right = Kernel->getDataTypeCenter().getDataExpr(left);
		Kernel->getDataTypeCenter().applyFacet ( right,
			Kernel->getDataTypeCenter().getMinExclusiveFacet(left) );
		return right;
	case DTGE:
		left = processConceptTree();
		MustBeM(RBRACK);
		right = Kernel->getDataTypeCenter().getDataExpr(left);
		Kernel->getDataTypeCenter().applyFacet ( right,
			Kernel->getDataTypeCenter().getMinInclusiveFacet(left) );
		return right;
	case DTLT:
		left = processConceptTree();
		MustBeM(RBRACK);
		right = Kernel->getDataTypeCenter().getDataExpr(left);
		Kernel->getDataTypeCenter().applyFacet ( right,
			Kernel->getDataTypeCenter().getMaxExclusiveFacet(left) );
		return right;
	case DTLE:
		left = processConceptTree();
		MustBeM(RBRACK);
		right = Kernel->getDataTypeCenter().getDataExpr(left);
		Kernel->getDataTypeCenter().applyFacet ( right,
			Kernel->getDataTypeCenter().getMaxInclusiveFacet(left) );
		return right;

	case BETWEEN:	// = (x,y)
		left = processConceptTree();	// read x
		right = Kernel->getDataTypeCenter().getDataExpr(left);
		Kernel->getDataTypeCenter().applyFacet ( right,
			Kernel->getDataTypeCenter().getMinExclusiveFacet(left) );
		left = processConceptTree();	// read y
		MustBeM(RBRACK);
		Kernel->getDataTypeCenter().applyFacet ( right,
			Kernel->getDataTypeCenter().getMaxExclusiveFacet(left) );
		return right;
	case INRANGE:	// = [x,y]
		left = processConceptTree();	// read x
		right = Kernel->getDataTypeCenter().getDataExpr(left);
		Kernel->getDataTypeCenter().applyFacet ( right,
			Kernel->getDataTypeCenter().getMinInclusiveFacet(left) );
		left = processConceptTree();	// read y
		MustBeM(RBRACK);
		Kernel->getDataTypeCenter().applyFacet ( right,
			Kernel->getDataTypeCenter().getMaxInclusiveFacet(left) );
		return right;

	case GE:
	case LE:
		// hack: id here is a number restriction
		n = scan.GetNumber ();
		NextLex ();
		// hack: throughout
	case FORALL:
	case EXISTS:
		// first argument -- role name
		left = getRoleExpression(/*allowChain=*/false);
		// second argument -- concept description
		if ( Current == RBRACK )
			right = new DLTree ( TOP );	// for (GE n R)
		else
			right = processConceptTree ();

		// skip right bracket
		MustBeM ( RBRACK );

		return Kernel->ComplexExpression ( T, n, left, right );

	case REFLEXIVE:	// self-reference
		left = getRoleExpression(/*allowChain=*/false);
		// skip right bracket
		MustBeM(RBRACK);
		return Kernel->SelfReference(left);

	case NOT:
		left = processConceptTree ();
		// skip right bracket
		MustBeM ( RBRACK );
		return Kernel->Not(left);

	case AND:
	case OR:	// multiple And's/Or's
		left = processConceptTree ();
		// 1 operand
		if ( Current == RBRACK )
		{
			NextLex ();
			return left;
		}

		// >= 1 operand
		while (1)
		{
			right = processConceptTree ();

			if ( Current == RBRACK )
			{
				NextLex ();
				return Kernel->SimpleExpression ( T, left, right );
			}
			else
				left = Kernel->SimpleExpression ( T, left, right );
		}
		break;

	case ONEOF:
		parseConceptList(/*singletonsOnly=*/true);
		return Kernel->processOneOf ();

	case DONEOF:
		parseConceptList(/*singletonsOnly=*/false);
		return Kernel->processOneOf(/*data=*/true);

	case STRING:	// expression (string <value>)
	case NUMBER:	// expression (number <value>)
	case REAL:		// expression (real <value>)
		left =	(T == STRING) ? Kernel->getDataTypeCenter().getStringType():
				(T == NUMBER) ? Kernel->getDataTypeCenter().getNumberType():
				(T == REAL ) ? Kernel->getDataTypeCenter().getRealType():
				NULL;	// error, but this can not happens

		if ( Current == RBRACK )	// just datatype
		{
			NextLex();
			return left;
		}

		NextLex();
		return getDTValue(left);

	default:	// error
		parseError ( "Unknown concept constructor" );
		return NULL;	// FSCO
	}
}

DLTree* DLLispParser :: getRoleExpression ( bool allowChain )
{
	if ( Current != LBRACK )
		return getRole();

	NextLex();	// skip '('
	if ( scan.getExpressionKeyword() == INV )
	{
		NextLex();	// skip INV
		DLTree* ret = Kernel->Inverse(getRoleExpression(/*allowChain=*/false));
		MustBeM(RBRACK);
		return ret;
	}
	else if ( !allowChain )	// only role name or inverses here
		MustBe ( INV, "only role names and their inverses are allowed as a role expression" );

	// else -- really complex expression = list of simple roles
	DLTree* ret = getRoleExpression(/*allowChain=*/false);
	while ( Current != RBRACK )
		ret = Kernel->Compose ( ret, getRoleExpression(/*allowChain=*/false) );
	MustBeM(RBRACK);
	return ret;
}
