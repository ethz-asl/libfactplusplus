/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2009 by Dmitry Tsarkov

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

		try
		{
			if ( t == SUBSUMES )
				Kernel->impliesConcepts ( left, right );
			else
				Kernel->equalConcepts ( left, right );
		}
		catch ( EFaCTPlusPlus ex )
		{
			parseError(ex.what());
		}

		MustBeM ( RBRACK );
		return;
	}

	if ( t == IMPLIES_R || t == EQUAL_R || t == DISJOINT_R || t == INVERSE )
	{
		DLTree* left = getRoleExpression(/*allowChain=*/(t == IMPLIES_R));
		DLTree* right = getRoleExpression(/*allowChain=*/false);

		try
		{
			if ( t == IMPLIES_R )
				Kernel->impliesRoles ( left, right );
			else if ( t == EQUAL_R )
				Kernel->equalRoles ( left, right );
			else if ( t == INVERSE )
			{
				right = createInverse(right);
				Kernel->equalRoles ( left, right );
			}
			else
				Kernel->disjointRoles ( left, right );
		}
		catch ( EFaCTPlusPlus ex )
		{
			parseError(ex.what());
		}

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

		try
		{
			switch (t)
			{
			case FUNCTIONAL:
				Kernel->setFunctional(R);
				break;
			case TRANSITIVE:
				Kernel->setTransitive(R);
				break;
			case REFLEXIVE:
				Kernel->setReflexive(R);
				break;
			case IRREFLEXIVE:
				Kernel->setIrreflexive(R);
				break;
			case SYMMETRIC:
				Kernel->setSymmetric(R);
				break;
			case ANTISYMMETRIC:
				Kernel->setAntiSymmetric(R);
				break;
			case ROLERANGE:
				Kernel->setRange ( R, processConceptTree() );
				break;
			case ROLEDOMAIN:
				Kernel->setDomain ( R, processConceptTree() );
				break;
			default:
				assert (0);
			}
		}
		catch ( EFaCTPlusPlus ex )
		{
			parseError(ex.what());
		}

		MustBeM ( RBRACK );
		return;
	}

	// disjoint-like operator
	if ( t == DISJOINT ||
		 t == FAIRNESS ||
		 t == DIFFERENT ||
		 t == SAME )
	{
		parseConceptList ( /*singletonsOnly=*/ (t != DISJOINT) && (t != FAIRNESS) );
		try
		{
			switch (t)
			{
			case DISJOINT:
				Kernel->disjointConcepts();
				return;		// already read ')'
			case FAIRNESS:
				Kernel->setFairnessConstraint();
				return;		// already read ')'
			case SAME:
				Kernel->processSame();
				return;		// already read ')'
			case DIFFERENT:
				Kernel->processDifferent();
				return;		// already read ')'
			default:
				assert (0);
			}
		}
		catch ( EFaCTPlusPlus ex )
		{
			parseError(ex.what());
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
		break;

	case RELATED:			// command is (Related id1 R id2);
	try {
		DLTree* id1 = getSingleton();
		DLTree* R = getRoleExpression(/*allowChain=*/false);
		MustBe (ID);	// second indiv.
		DLTree* id2 = getSingleton();
		Kernel->relatedTo ( id1, R, id2 );
		break;
	}
	catch ( EFaCTPlusPlus ex )
	{
		parseError(ex.what());
	}

	case PROLE:
		Name = getRole();
		if ( Current != RBRACK )
			parseRoleArguments(Name);
		deleteTree(Name);
		break;

	case PATTR:
		Name = getRole();

		try
		{
			Kernel->setFunctional(clone(Name));
		}
		catch ( EFaCTPlusPlus ex )
		{
			parseError(ex.what());
		}

		if ( Current != RBRACK )
			parseRoleArguments(Name);
		deleteTree(Name);
		break;

	case DATAROLE:		// register data role
		delete getDataRole();
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
				try
				{
					Kernel->impliesRoles ( clone(R), S );
				}
				catch ( EFaCTPlusPlus ex )
				{
					parseError(ex.what());
				}
			}
			NextLex ();	// skip last RBRACK
		}
		else if ( scan.isKeyword ("transitive") )
		{	// followed by NIL or (usually!) T
			NextLex ();

			try
			{
				Kernel->setTransitive(clone(R));
			}
			catch ( EFaCTPlusPlus ex )
			{
				parseError(ex.what());
			}

			NextLex ();	// skip token
		}
		else
			parseError ( "use either :parents or :transitive command in role description" );
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
