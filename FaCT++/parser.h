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

#ifndef PARSER_H
#define PARSER_H

#include "comparser.h"
#include "dltree.h"
#include "Kernel.h"

/// class for parsing LISP-like ontologies
class DLLispParser: public CommonParser
{
protected:	// members
		/// Kernel to be filled
	ReasoningKernel* Kernel;
		/// expression manager to be used
	TExpressionManager* EManager;
		/// set of known data role names
	std::set<std::string> DataRoles;

protected:	// methods
		/// error by given exception
	void errorByException ( const EFPPCantRegName& ex ) const { parseError(ex.what()); }

		/// @return concept-like Id of just scanned name
	DLTree* getConcept ( void )
	{
		DLTree* ret = EManager->Concept(scan.GetName());
		NextLex();
		return ret;
	}
		/// @return singleton Id of just scanned name
	DLTree* getSingleton ( void )
	{
		DLTree* ret = EManager->Individual(scan.GetName());
		NextLex();
		return ret;
	}
		/// @return role-like Id of just scanned name
	DLTree* getRole ( void )
	{
		DLTree* ret;
		if ( DataRoles.find(scan.GetName()) != DataRoles.end() )
			ret = EManager->DataRole(scan.GetName());	// found data role
		else	// object role
			ret = EManager->ObjectRole(scan.GetName());
		NextLex();
		return ret;
	}
		/// @return role-like Id of just scanned name
	DLTree* getDataRole ( void )
	{
		DataRoles.insert(scan.GetName());
		DLTree* ret = EManager->DataRole(scan.GetName());
		NextLex();
		return ret;
	}
		/// @return role-like Id of just scanned name
	DLTree* getObjectRole ( void )
	{
		DLTree* ret = EManager->ObjectRole(scan.GetName());
		NextLex();
		return ret;
	}
		/// @return datavalue of a data type TYPE with an Id of a just scanned name
	DLTree* getDTValue ( DLTree* type )
	{
		DLTree* ret = Kernel->getDataTypeCenter().getDataValue(scan.GetName(),type);
		NextLex();
		deleteTree(type);
		return ret;
	}

		/// check whether expression R is data role
	bool isDataRole ( DLTree* R ) const { return R->Element().getToken() == DNAME; }
		/// get role expression, ie role or its inverse
	DLTree* getRoleExpression ( void );
		/// get object role expression, ie object role, OR constant or their inverse
	DLTree* getORoleExpression ( void );
		/// get data role expression, ie data role or DR constant
	DLTree* getDRoleExpression ( void );
		/// get simple role expression or role projection or chain
	DLTree* getComplexRoleExpression ( void );
		/// parse simple DL command
	void parseCommand ( void );
		/// parse role arguments if defprimrole command
	void parseRoleArguments ( DLTree* role );
		/// parse list of concept expressions (in disjoint-like commands)
	void parseConceptList ( bool singletonsOnly );
		/// get concept-like expression for simple variants
	DLTree* getConceptExpression ( void );
		/// get concept-like expression for complex constructors
	DLTree* getComplexConceptExpression ( void );
		/// get data expression
	DLTree* getDataExpression ( void );

public:		// interface
		/// the only c'tor
	DLLispParser ( std::istream* in, ReasoningKernel* kernel )
		: CommonParser (in)
		, Kernel (kernel)
		, EManager(kernel->getExpressionManager())
	{
	}
		/// empty d'tor
	~DLLispParser ( void ) {}

		/// main parsing method
	void Parse ( void );
};	// DLLispParser

#endif
