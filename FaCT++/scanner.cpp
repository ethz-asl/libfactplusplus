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

//-------------------------------------------------------------------------
//
//  Scanner implementation for FaCT++ program
//
//-------------------------------------------------------------------------
#include "scanner.h"

#include <cctype>		// isalnum
#include <cassert>

// main methods
bool TsScanner :: isLegalIdChar ( char c ) const	//id=[_a..z0-9[].]
{
	return !isspace(c) && c != '(' && c != ')' && c != ';' && c != '|' && !eof(c);
}

// Word must be in a CAPITAL LETTERS
void TsScanner :: FillBuffer ( register char c )
{
	register unsigned int i = 0;
	LexBuff [0] = c;

	for ( i = 1; i < MaxIDLength &&
		  isLegalIdChar ( c = NextChar () );
		  LexBuff [i++] = c )
		(void)NULL;

	LexBuff [i] = 0;

	if ( i == MaxIDLength )
	{
		std::cerr << "Identifier was restricted to " << LexBuff << std::endl;
		for ( ; isLegalIdChar ( c = NextChar () ); )
			(void)NULL;
	}
	// OK or read the end of ID

	PutBack ( c );
}

void TsScanner :: FillNameBuffer ( register char c )
{
	register unsigned int i = 0;
	assert ( c == '|' );

	for ( ; i < MaxIDLength && ( ( c = NextChar () ) != '|' ); LexBuff [i++] = c )
		(void)NULL;

	LexBuff [i] = 0;

	if ( i == MaxIDLength )
	{
		std::cerr << "Identifier was restricted to " << LexBuff << std::endl;
		for ( ; ( c = NextChar () ) != '|'; )
			(void)NULL;
	}
}

Token TsScanner :: GetLex ( void )
{
	register char c;

	while ( !eof( c = NextChar() ) )
	{
		if ( c == ' ' || c == '\t' || c == 13 )
			continue;

		if ( c == '\n' )
		{
			CurLine++;
			continue;
		}

		if ( c == ';' )
		{	// Skip comments
			while ( ( c = NextChar() ) != '\n' && !eof(c) )
				(void)NULL;
			CurLine++;
			continue;
		}

		// one-symbol tokens
		if ( c == '(' )
			return LBRACK;

		if ( c == ')' )
			return RBRACK;

		if ( c == ':' )	// some keyword
		{	// skip colon
			FillBuffer ( NextChar () );
			return ID;
		}

		if ( c == '|' )
		{
			FillNameBuffer ( c );
			return ID;
		}

		if ( isdigit ( c ) )	//number
		{
			FillBuffer ( c );
			return NUM;
		}
		else	// id
		{
			FillBuffer ( c );
			return ID;
		}

		// all alternates was checked => error
		return BAD_LEX;
	}
	// read EOF - end of lex

	return LEXEOF;
}

// recognize TOP, BOTTOM or general ID
Token TsScanner :: getNameKeyword ( void ) const
{
	if ( isKeyword ("*TOP*") || isKeyword ("TOP") )
		return TOP;

	if ( isKeyword ("*BOTTOM*") || isKeyword ("BOTTOM") )
		return BOTTOM;

	if ( isKeyword("*UROLE*") )
		return UROLE;

	// not a keyword just an ID
	return ID;
}

// concept/role constructors; return BAD_LEX in case of error
Token TsScanner :: getExpressionKeyword ( void ) const
{
	if ( isKeyword ("and") )
		return AND;

	if ( isKeyword ("or") )
		return OR;

	if ( isKeyword ("not") )
		return NOT;

	if ( isKeyword ("inv") || isKeyword ("inverse") )
		return INV;

	if ( isKeyword ("some") )
		return EXISTS;

	if ( isKeyword ("all") )
		return FORALL;

	if ( isKeyword ("at-least") || isKeyword ("atleast") )
		return GE;

	if ( isKeyword ("at-most") || isKeyword ("atmost") )
		return LE;

	if ( isKeyword ("one-of") )
		return ONEOF;

	if ( isKeyword ("self-ref") )
		return REFLEXIVE;

	if ( isKeyword ("string") )
		return STRING;

	if ( isKeyword ("number") )
		return NUMBER;

	if ( isKeyword ("real") )
		return REAL;

	if ( isKeyword("gt") )
		return DTGT;

	if ( isKeyword("lt") )
		return DTLT;

	if ( isKeyword("ge") )
		return DTGE;

	if ( isKeyword("le") )
		return DTLE;

	if ( isKeyword("between") )
		return BETWEEN;

	if ( isKeyword("in-range") )
		return INRANGE;

	if ( isKeyword("d-one-of") )
		return DONEOF;

	// not a keyword -- error
	return BAD_LEX;
}

// recognize FaCT++ keywords; return BAD_LEX if not found
Token TsScanner :: getCommandKeyword ( void ) const
{
	// definitions
	if ( isKeyword ("defprimconcept") )
		return PCONCEPT;

	if ( isKeyword ("defconcept") )
		return CONCEPT;

	if ( isKeyword ("defprimrole") )
		return PROLE;

	if ( isKeyword ("defdatarole") )
		return DATAROLE;

	if ( isKeyword ("defprimattribute") )
		return PATTR;

	if ( isKeyword ("defindividual") )
		return DEFINDIVIDUAL;

	// general relations
	if ( isKeyword ("implies") || isKeyword ("implies_c") )
		return SUBSUMES;

	if ( isKeyword ("equal_c") )
		return EQUAL_C;

	if ( isKeyword ("disjoint") || isKeyword ("disjoint_c") )
		return DISJOINT;

	if ( isKeyword ("implies_r") )
		return IMPLIES_R;

	if ( isKeyword ("equal_r") )
		return EQUAL_R;

	if ( isKeyword ("disjoint_r") )
		return DISJOINT_R;

	if ( isKeyword ("inverse") )
		return INVERSE;

	// role stuff
	if ( isKeyword ("functional") )
		return FUNCTIONAL;

	if ( isKeyword ("transitive") )
		return TRANSITIVE;

	if ( isKeyword ("reflexive") )
		return REFLEXIVE;

	if ( isKeyword ("irreflexive") )
		return IRREFLEXIVE;

	if ( isKeyword ("symmetric") )
		return SYMMETRIC;

	if ( isKeyword ("antisymmetric") )
		return ANTISYMMETRIC;

	if ( isKeyword ("range") )
		return ROLERANGE;

	if ( isKeyword ("domain") )
		return ROLEDOMAIN;

	// individual stuff
	if ( isKeyword ("instance") )
		return INSTANCE;

	if ( isKeyword ("related") )
		return RELATED;

	if ( isKeyword ("same") )
		return SAME;

	if ( isKeyword ("different") )
		return DIFFERENT;

	if ( isKeyword ("fairness") )
		return FAIRNESS;

	// not a keyword -- error
	return BAD_LEX;
}
