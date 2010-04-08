/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2010 by Dmitry Tsarkov

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
		DLTree* left = (t == IMPLIES_R) ? getComplexRoleExpression() : getRoleExpression();
		DLTree* right = getRoleExpression();
		bool data = isDataRole(left);

		try
		{
			if ( t == INVERSE )
				Kernel->setInverseRoles ( left, right );
			else if ( t == IMPLIES_R )
			{
				if ( data )
					Kernel->impliesDRoles ( left, right );
				else
					Kernel->impliesORoles ( left, right );
			}
			else
			{	// make N-ary arg
				EManager->openArgList();
				EManager->addArg(left);
				EManager->addArg(right);
				if ( data )
				{
					if ( t == DISJOINT_R )
						Kernel->disjointDRoles();
					else
						Kernel->equalDRoles();
				}
				else
				{
					if ( t == DISJOINT_R )
						Kernel->disjointORoles();
					else
						Kernel->equalORoles();
				}
			}
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
		DLTree* R = getRoleExpression();
		bool data = isDataRole(R);

		try
		{
			switch (t)
			{
			case FUNCTIONAL:
				if ( data )
					Kernel->setDFunctional(R);
				else
					Kernel->setOFunctional(R);
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
				if ( data )
					Kernel->setDRange ( R, getDataExpression() );
				else
					Kernel->setORange ( R, processConceptTree() );
				break;
			case ROLEDOMAIN:
				if ( data )
					Kernel->setDDomain ( R, processConceptTree() );
				else
					Kernel->setODomain ( R, processConceptTree() );
				break;
			default:
				fpp_unreachable();
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
				fpp_unreachable();
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
			Kernel->declare(Name);
		break;

	case CONCEPT:
		Name = getConcept();
		Kernel->equalConcepts ( Name, processConceptTree() );
		break;

	case DEFINDIVIDUAL:		// just register singleton
		Kernel->declare(getSingleton());
		break;

	case INSTANCE:
		Name = getSingleton();
		Kernel->instanceOf ( Name, processConceptTree() );
		break;

	case RELATED:			// command is (Related id1 R id2);
	try {
		DLTree* id1 = getSingleton();
		DLTree* R = getRoleExpression();
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
		else
			Kernel->declare(Name);
		break;

	case PATTR:
		Name = getRole();

		try
		{
			Kernel->setOFunctional(clone(Name));
		}
		catch ( EFaCTPlusPlus ex )
		{
			parseError(ex.what());
		}

		if ( Current == RBRACK )
			Kernel->declare(Name);
		else
			parseRoleArguments(Name);
		break;

	case DATAROLE:		// register data role
		Kernel->declare(getDataRole());
		break;

	default:
		parseError ( "Unrecognised command" );
	}

	MustBeM ( RBRACK );	// skip bracket
}

void DLLispParser :: parseConceptList ( bool singletonsOnly )
{
	EManager->openArgList();

	// continue with all concepts
	while ( Current != RBRACK )
		EManager->addArg ( singletonsOnly ? getSingleton() : processConceptTree() );

	// skip RBRACK
	MustBeM (RBRACK);
}

void DLLispParser :: parseRoleArguments ( DLTree* R )
{
	bool data = isDataRole(R);
	while ( Current != RBRACK )
		if ( scan.isKeyword ("parents") || scan.isKeyword ("supers") )
		{	// followed by a list of parent role names
			NextLex ();
			MustBeM ( LBRACK );

			while ( Current != RBRACK )
			{
				DLTree* S = getRoleExpression();
				try
				{
					if ( data )
						Kernel->impliesDRoles ( clone(R), S );
					else
						Kernel->impliesORoles ( clone(R), S );
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

	// R is not needed anymore
	deleteTree(R);
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
		switch ( scan.getNameKeyword() )
		{	// Top/Bottom or real ID
		case TOP: NextLex(); return EManager->Top();
		case BOTTOM: NextLex(); return EManager->Bottom();
		default: return getConcept();
		}
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
	case GE:
	case LE:
		// hack: id here is a number restriction
		n = scan.GetNumber ();
		NextLex ();
		// hack: throughout
	case FORALL:
	case EXISTS:
		// first argument -- role name
		left = getRoleExpression();
		// second argument -- concept description
		if ( Current == RBRACK )
			right = EManager->Top();	// for (GE n R)
		else
			right = isDataRole(left) ? getDataExpression() : processConceptTree();

		// skip right bracket
		MustBeM ( RBRACK );

		return EManager->ComplexExpression ( T, n, left, right );

	case REFLEXIVE:	// self-reference
		left = getRoleExpression();
		// skip right bracket
		MustBeM(RBRACK);
		return Kernel->SelfReference(left);

	case NOT:
		left = processConceptTree ();
		// skip right bracket
		MustBeM ( RBRACK );
		return EManager->Not(left);

	case AND:
	case OR:	// multiple And's/Or's
		EManager->openArgList();
		do
		{
			EManager->addArg(processConceptTree());
		} while ( Current != RBRACK );

		// list is parsed here
		NextLex();	// skip ')'
		return T == AND ? EManager->And() : EManager->Or();

	case ONEOF:
		parseConceptList(/*singletonsOnly=*/true);
		return EManager->OneOf(/*data=*/false);

	default:	// error
		parseError ( "Unknown concept constructor" );
		return NULL;	// FSCO
	}
}

DLTree* DLLispParser :: getRoleExpression ( void )
{
	if ( Current != LBRACK )
		return getRole();

	NextLex();	// skip '('
	if ( scan.getExpressionKeyword() != INV )
		MustBe ( INV, "only role names and their inverses are allowed as a role expression" );
	NextLex();	// skip INV
	DLTree* ret = EManager->Inverse(getRoleExpression());
	MustBeM(RBRACK);
	return ret;
}

DLTree* DLLispParser :: getComplexRoleExpression ( void )
{
	if ( Current != LBRACK )
		return getRole();

	NextLex();	// skip '('
	Token keyword = scan.getExpressionKeyword();
	NextLex();	// skip keyword
	DLTree* ret = NULL;
	switch ( keyword )
	{
	case INV:	// inverse of a simple role
		ret = EManager->Inverse(getRoleExpression());
		break;
	case RCOMPOSITION:	// role composition expression = list of simple roles
		EManager->openArgList();
		while ( Current != RBRACK )
			EManager->addArg(getRoleExpression());
		ret = EManager->Compose();
		break;
	case PROJINTO:	// role projection operator, parse simple role and concept
		ret = getRoleExpression();
		ret = EManager->ProjectInto ( ret, processConceptTree() );
		break;
	case PROJFROM:	// role projection operator, parse simple role and concept
		ret = getRoleExpression();
		ret = Kernel->ProjectFrom ( ret, processConceptTree() );
		break;
	default:
		MustBe ( INV, "unknown expression in complex role constructor" );
	}

	MustBeM(RBRACK);
	return ret;
}

DLTree*
DLLispParser :: getDataExpression ( void )
{
	// check for TOP/BOTTOM
	if ( Code() == ID )
	{
		switch ( scan.getNameKeyword() )
		{	// Top/Bottom; can not be name
		case TOP: NextLex(); return EManager->Top();
		case BOTTOM: NextLex(); return EManager->Bottom();
		default: parseError ( "Unknown concept constructor" ); return NULL;
		}
	}

	MustBeM(LBRACK);	// always complex expression
	Token T = scan.getExpressionKeyword();
	DLTree *left = NULL, *right = NULL;

	NextLex ();

	switch (T)
	{
	case DTGT:
		left = getDataExpression();
		MustBeM(RBRACK);
		right = Kernel->getDataTypeCenter().getDataExpr(left);
		Kernel->getDataTypeCenter().applyFacet ( right,
			Kernel->getDataTypeCenter().getMinExclusiveFacet(left) );
		return right;
	case DTGE:
		left = getDataExpression();
		MustBeM(RBRACK);
		right = Kernel->getDataTypeCenter().getDataExpr(left);
		Kernel->getDataTypeCenter().applyFacet ( right,
			Kernel->getDataTypeCenter().getMinInclusiveFacet(left) );
		return right;
	case DTLT:
		left = getDataExpression();
		MustBeM(RBRACK);
		right = Kernel->getDataTypeCenter().getDataExpr(left);
		Kernel->getDataTypeCenter().applyFacet ( right,
			Kernel->getDataTypeCenter().getMaxExclusiveFacet(left) );
		return right;
	case DTLE:
		left = getDataExpression();
		MustBeM(RBRACK);
		right = Kernel->getDataTypeCenter().getDataExpr(left);
		Kernel->getDataTypeCenter().applyFacet ( right,
			Kernel->getDataTypeCenter().getMaxInclusiveFacet(left) );
		return right;

	case NOT:
		left = getDataExpression();
		// skip right bracket
		MustBeM ( RBRACK );
		return EManager->Not(left);

	case DONEOF:
	case AND:
	case OR:	// multiple And's/Or's
		EManager->openArgList();
		do
		{
			EManager->addArg(getDataExpression());
		} while ( Current != RBRACK );

		// list is parsed here
		NextLex();	// skip ')'
		return T == AND ? EManager->And() : T == OR ? EManager->Or() : EManager->OneOf(/*data=*/true);

	case STRING:	// expression (string <value>)
	case NUMBER:	// expression (number <value>)
	case REAL:		// expression (real <value>)
	case BOOL:
		left =	(T == STRING) ? Kernel->getDataTypeCenter().getStringType():
				(T == NUMBER) ? Kernel->getDataTypeCenter().getNumberType():
				(T == REAL ) ? Kernel->getDataTypeCenter().getRealType():
				(T == BOOL ) ? Kernel->getDataTypeCenter().getBoolType():
				NULL;	// error, but this can not happens

		if ( Current == RBRACK )	// just datatype
		{
			NextLex();
			return left;
		}

		NextLex();
		return getDTValue(left);

	default:	// error
		parseError ( "Unknown data constructor" );
		return NULL;	// FSCO
	}
}
