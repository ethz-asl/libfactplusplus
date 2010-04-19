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

	switch (t)
	{
	case SUBSUMES:
	case EQUAL_C:
	{
		TConceptExpr* left = getConceptExpression();
		TConceptExpr* right = getConceptExpression();

		try
		{
			if ( t == SUBSUMES )
				Kernel->impliesConcepts ( left, right );
			else
			{
				EManager->newArgList();
				EManager->addArg(left);
				EManager->addArg(right);
				Kernel->equalConcepts();
			}
		}
		catch ( EFaCTPlusPlus ex )
		{
			parseError(ex.what());
		}
		break;
	}

	case IMPLIES_R:
	case EQUAL_R:
	case DISJOINT_R:
	case INVERSE:
	{
		TRoleExpr* R = (t == IMPLIES_R) ? getComplexRoleExpression() : getRoleExpression();
		if ( isDataRole(R) )
			tellRoleAxiom ( t, dynamic_cast<TDRoleExpr*>(R), getDRoleExpression() );
		else if ( t != IMPLIES_R )
			tellRoleAxiom ( t, dynamic_cast<TORoleExpr*>(R), getORoleExpression() );
		else	// implies_R
			try
			{
				Kernel->impliesORoles ( dynamic_cast<TORoleComplexExpr*>(R), getORoleExpression() );
			}
			catch ( EFaCTPlusPlus ex )
			{
				parseError(ex.what());
			}
		break;
	}

	case FUNCTIONAL:
	case TRANSITIVE:
	case REFLEXIVE:
	case IRREFLEXIVE:
	case SYMMETRIC:
	case ANTISYMMETRIC:
	{
		TRoleExpr* R = getRoleExpression();
		if ( isDataRole(R) )
			tellRoleAxiom ( t, dynamic_cast<TDRoleExpr*>(R), NULL );
		else
			tellRoleAxiom ( t, dynamic_cast<TORoleExpr*>(R), NULL );
		break;
	}

	case ROLERANGE:
	case ROLEDOMAIN:
	{
		TRoleExpr* R = getRoleExpression();
		try
		{
			if ( isDataRole(R) )
			{
				TDRoleExpr* S = dynamic_cast<TDRoleExpr*>(R);
				if ( t == ROLERANGE )
					Kernel->setDRange ( S, getDataExpression() );
				else
					Kernel->setDDomain ( S, getConceptExpression() );
			}
			else
			{
				TORoleExpr* S = dynamic_cast<TORoleExpr*>(R);
				if ( t == ROLERANGE )
					Kernel->setORange ( S, getConceptExpression() );
				else
					Kernel->setODomain ( S, getConceptExpression() );
			}
		}
		catch ( EFaCTPlusPlus ex )
		{
			parseError(ex.what());
		}
		break;
	}

	// disjoint-like operator
	case DISJOINT:
	case FAIRNESS:
	case DIFFERENT:
	case SAME:
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
		break;

	case PCONCEPT:
	{
		TConceptExpr* Name = getConcept();
		if ( Current != RBRACK )
			Kernel->impliesConcepts ( Name, getConceptExpression() );
		else
			Kernel->declare(Name);
		break;
	}

	case CONCEPT:
		EManager->newArgList();
		EManager->addArg(getConcept());
		EManager->addArg(getConceptExpression());
		Kernel->equalConcepts();
		break;

	case DEFINDIVIDUAL:		// just register singleton
		Kernel->declare(getSingleton());
		break;

	case INSTANCE:
	{
		TIndividualExpr* Name = getSingleton();
		Kernel->instanceOf ( Name, getConceptExpression() );
		break;
	}

	case RELATED:			// command is (Related id1 R id2);
		try
		{
			TIndividualExpr* id1 = getSingleton();
			TORoleExpr* R = getORoleExpression();
			MustBe (ID);	// second indiv.
			TIndividualExpr* id2 = getSingleton();
			Kernel->relatedTo ( id1, R, id2 );
		}
		catch ( EFaCTPlusPlus ex )
		{
			parseError(ex.what());
		}
		break;

	case PROLE:
	case PATTR:
	{
		TORoleExpr* Name = getObjectRole();

		if ( t == PATTR )
			try
			{
				Kernel->setOFunctional(Name);
			}
			catch ( EFaCTPlusPlus ex )
			{
				parseError(ex.what());
			}

		if ( Current != RBRACK )
			parseRoleArguments(Name);
		else
			Kernel->declare(Name);
		break;
	}

	case DATAROLE:		// register data role
		Kernel->declare(getDataRole());
		break;

	default:
		parseError ( "Unrecognised command" );
	}

	MustBeM ( RBRACK );	// skip bracket
}

/// generate object role axiom between R and S according to the operation TAG
void
DLLispParser :: tellRoleAxiom ( Token tag, TORoleExpr* R, TORoleExpr* S )
{
	try
	{
		switch(tag)
		{
		case INVERSE:
			Kernel->setInverseRoles ( R, S );
			break;

		case DISJOINT_R:
		case EQUAL_R:
			EManager->newArgList();
			EManager->addArg(R);
			EManager->addArg(S);
			if ( tag == DISJOINT_R )
				Kernel->disjointORoles();
			else
				Kernel->equalORoles();
			break;

		case FUNCTIONAL:
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

		default:
			fpp_unreachable();
		}
	}
	catch ( EFaCTPlusPlus ex )
	{
		parseError(ex.what());
	}
}
/// generate data role axiom between R and S according to the operation TAG
void
DLLispParser :: tellRoleAxiom ( Token tag, TDRoleExpr* R, TDRoleExpr* S )
{
	try
	{
		switch(tag)
		{
		case IMPLIES_R:
			Kernel->impliesDRoles ( R, S );
			break;

		case DISJOINT_R:
		case EQUAL_R:
			EManager->newArgList();
			EManager->addArg(R);
			EManager->addArg(S);
			if ( tag == DISJOINT_R )
				Kernel->disjointDRoles();
			else
				Kernel->equalDRoles();
			break;

		case FUNCTIONAL:
			Kernel->setDFunctional(R);
			break;

		default:
			fpp_unreachable();
		}
	}
	catch ( EFaCTPlusPlus ex )
	{
		parseError(ex.what());
	}
}

void DLLispParser :: parseConceptList ( bool singletonsOnly )
{
	EManager->newArgList();

	// continue with all concepts
	while ( Current != RBRACK )
		if ( singletonsOnly )
			EManager->addArg(getSingleton());
		else
			EManager->addArg(getConceptExpression());

	// skip RBRACK
	MustBeM (RBRACK);
}

void DLLispParser :: parseRoleArguments ( TORoleExpr* R )
{
	while ( Current != RBRACK )
		if ( scan.isKeyword ("parents") || scan.isKeyword ("supers") )
		{	// followed by a list of parent role names
			NextLex ();
			MustBeM ( LBRACK );

			while ( Current != RBRACK )
			{
				try
				{	// only object roles can have arguments
					Kernel->impliesORoles ( R, getORoleExpression() );
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
				Kernel->setTransitive(R);
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

DLLispParser::TConceptExpr*
DLLispParser :: getConceptExpression ( void )
{
	switch ( Code() )
	{
	case LBRACK:	// complex description
		return getComplexConceptExpression ();
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

DLLispParser::TConceptExpr*
DLLispParser :: getComplexConceptExpression ( void )
{
	MustBeM ( LBRACK );
	Token T = scan.getExpressionKeyword();
	unsigned int n = 0;	// number for >= (<=) expression (or just 0)
	TRoleExpr* R = NULL;
	TConceptExpr* C = NULL;

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
		R = getRoleExpression();
		if ( isDataRole(R) )	// data expression
		{
			TDRoleExpr* A = dynamic_cast<TDRoleExpr*>(R);
			TDataExpr* E = NULL;

			// second argument -- data expression
			if ( Current == RBRACK )
				E = EManager->DataTop();	// for (GE n R)
			else
				E = getDataExpression();

			// skip right bracket
			MustBeM(RBRACK);

			if ( T == EXISTS )
				return EManager->Exists ( A, E );
			else if ( T == FORALL )
				return EManager->Forall ( A, E );
			else if ( T == GE )
				return EManager->MinCardinality ( n, A, E );
			else
				return EManager->MaxCardinality ( n, A, E );
		}
		else
		{
			TORoleExpr* S = dynamic_cast<TORoleExpr*>(R);
			TConceptExpr* C = NULL;

			// second argument -- data expression
			if ( Current == RBRACK )
				C = EManager->Top();	// for (GE n R)
			else
				C = getConceptExpression();

			// skip right bracket
			MustBeM(RBRACK);

			if ( T == EXISTS )
				return EManager->Exists ( S, C );
			else if ( T == FORALL )
				return EManager->Forall ( S, C );
			else if ( T == GE )
				return EManager->MinCardinality ( n, S, C );
			else
				return EManager->MaxCardinality ( n, S, C );
		}

	case REFLEXIVE:	// self-reference
		R = getORoleExpression();
		// skip right bracket
		MustBeM(RBRACK);
		return EManager->SelfReference(dynamic_cast<TORoleExpr*>(R));

	case NOT:
		C = getConceptExpression();
		// skip right bracket
		MustBeM(RBRACK);
		return EManager->Not(C);

	case AND:
	case OR:	// multiple And's/Or's
		EManager->newArgList();
		do
		{
			EManager->addArg(getConceptExpression());
		} while ( Current != RBRACK );

		// list is parsed here
		NextLex();	// skip ')'
		return T == AND ? EManager->And() : EManager->Or();

	case ONEOF:
		parseConceptList(/*singletonsOnly=*/true);
		return EManager->OneOf();

	default:	// error
		parseError ( "Unknown concept constructor" );
		return NULL;	// FSCO
	}
}

DLLispParser::TDRoleExpr*
DLLispParser :: getDRoleExpression ( void )
{
	// FIXME!! later on -- add DataRoleTop/bottom
	MustBe ( ID, "Data role name expected" );
	return getDataRole();
}

DLLispParser::TORoleExpr*
DLLispParser :: getORoleExpression ( void )
{
	if ( Current != LBRACK )
		return getObjectRole();

	NextLex();	// skip '('
	if ( scan.getExpressionKeyword() != INV )
		MustBe ( INV, "only role names and their inverses are allowed as a role expression" );
	NextLex();	// skip INV
	TORoleExpr* ret = EManager->Inverse(getORoleExpression());
	MustBeM(RBRACK);
	return ret;
}

DLLispParser::TRoleExpr*
DLLispParser :: getRoleExpression ( void )
{
	if ( Current != LBRACK )
		return getRole();

	NextLex();	// skip '('
	if ( scan.getExpressionKeyword() != INV )
		MustBe ( INV, "only role names and their inverses are allowed as a role expression" );
	NextLex();	// skip INV
	TORoleExpr* ret = EManager->Inverse(getORoleExpression());
	MustBeM(RBRACK);
	return ret;
}

DLLispParser::TRoleExpr*
DLLispParser :: getComplexRoleExpression ( void )
{
	if ( Current != LBRACK )
		return getRole();

	NextLex();	// skip '('
	Token keyword = scan.getExpressionKeyword();
	NextLex();	// skip keyword
	TORoleComplexExpr* ret = NULL;
	TORoleExpr* R;
	switch ( keyword )
	{
	case INV:	// inverse of a simple role
		ret = EManager->Inverse(getORoleExpression());
		break;
	case RCOMPOSITION:	// role composition expression = list of simple roles
		EManager->newArgList();
		while ( Current != RBRACK )
			EManager->addArg(getORoleExpression());
		ret = EManager->Compose();
		break;
	case PROJINTO:	// role projection operator, parse simple role and concept
		R = getORoleExpression();
		ret = EManager->ProjectInto ( R, getConceptExpression() );
		break;
	case PROJFROM:	// role projection operator, parse simple role and concept
		R = getORoleExpression();
		ret = EManager->ProjectFrom ( R, getConceptExpression() );
		break;
	default:
		MustBe ( INV, "unknown expression in complex role constructor" );
	}

	MustBeM(RBRACK);
	return ret;
}

DLLispParser::TDataExpr*
DLLispParser :: getDataExpression ( void )
{
	// check for TOP/BOTTOM
	if ( Code() == ID )
	{
		switch ( scan.getNameKeyword() )
		{	// Top/Bottom; can not be name
		case TOP: NextLex(); return EManager->DataTop();
		case BOTTOM: NextLex(); return EManager->DataBottom();
		default: parseError ( "Unknown concept constructor" ); return NULL;
		}
	}

	MustBeM(LBRACK);	// always complex expression
	Token T = scan.getExpressionKeyword();
	TDataExpr *left = NULL, *right = NULL;

	NextLex ();

	switch (T)
	{
#if 0
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
#endif

	case NOT:
		left = getDataExpression();
		// skip right bracket
		MustBeM ( RBRACK );
		return EManager->DataNot(left);

	case DONEOF:
	case AND:
	case OR:	// multiple And's/Or's
		EManager->newArgList();
		do
		{
			EManager->addArg(getDataExpression());
		} while ( Current != RBRACK );

		// list is parsed here
		NextLex();	// skip ')'
		return T == AND ? EManager->DataAnd() : T == OR ? EManager->DataOr() : EManager->DataOneOf();

#if 0
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
#endif
	default:	// error
		parseError ( "Unknown data constructor" );
		return NULL;	// FSCO
	}
}
