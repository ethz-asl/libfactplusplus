/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2010 by Dmitry Tsarkov

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

//-------------------------------------------------------------------------
//
//  Grammar for FaCT++
//
//-------------------------------------------------------------------------
#ifndef __GRAMMAR_HPP
#define __GRAMMAR_HPP

// constants for symbols
enum Token {
	BAD_LEX = 50,

	UNUSED,	// never used one

	LEXEOF,

	// symbols
	LBRACK,
	RBRACK,
	AND,
	OR,
	NOT,
	INV,
	RCOMPOSITION,	// role composition
	PROJINTO,		// role projection into
	PROJFROM,		// role projection from

	TOP,
	BOTTOM,
	EXISTS,
	FORALL,
	GE,
//	ATLEAST = GE,
	LE,
//	ATMOST = LE,

	// common metasymbols
	ID,		// should NOT appear in KB -- use *NAME instead
	NUM,
	DATAEXPR,	// any data expression: data value, [constrained] datatype

	// more precise ID's discretion
	CNAME,	// name of a concept
	INAME,	// name of a singleton
	RNAME,	// name of a role
	DNAME,	// name of a data role

	// FaCT commands
	// definitions
	PCONCEPT,
	PROLE,
	PATTR,
	CONCEPT,
	DATAROLE,

	// FaCT++ commands for internal DataTypes
	NUMBER,
	STRING,
	REAL,
	BOOL,

	// datatype operations command names -- used only as an external commands
	DTGT,
	DTLT,
	DTGE,
	DTLE,
	DONEOF,

	// general commands
	SUBSUMES,
	DISJOINT,
	EQUAL_C,

	// new for roles
	INVERSE,
	EQUAL_R,
	IMPLIES_R,
	DISJOINT_R,
	FUNCTIONAL,
	TRANSITIVE,
	REFLEXIVE,
	IRREFLEXIVE,
	SYMMETRIC,
	ANTISYMMETRIC,
	ROLERANGE,
	ROLEDOMAIN,

	// new for individuals
	DEFINDIVIDUAL,
	INSTANCE,
	RELATED,
	ONEOF,
	SAME,
	DIFFERENT,

	// fairness constraints
	FAIRNESS,
};

// some multi-case defines

// any name (like ID)
#define NAME CNAME: case INAME: case RNAME: case DNAME

#endif
