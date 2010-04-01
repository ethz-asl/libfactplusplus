/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2010 by Dmitry Tsarkov

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

#ifndef TEXPRESSIONMANAGER_H
#define TEXPRESSIONMANAGER_H

#include "tDLExpression.h"
#include "tNameSet.h"
#include "tNAryQueue.h"

/// manager to work with all DL expressions in the kernel
class TExpressionManager
{
protected:	// members
		/// nameset for concepts
	TNameSet<TDLConceptName> NS_C;
		/// nameset for individuals
	TNameSet<TDLIndividualName> NS_I;
		/// nameset for object roles
	TNameSet<TDLObjectRoleName> NS_OR;
		/// nameset for data roles
	TNameSet<TDLDataRoleName> NS_DR;
		/// nameset for data types
	TNameSet<TDLDataTypeName> NS_DT;

		/// n-ary queue for arguments
	TNAryQueue<TDLExpression> ArgQueue;

		/// TOP concept
	TDLConceptTop* CTop;
		/// BOTTOM concept
	TDLConceptBottom* CBottom;
		/// TOP object role
	TDLObjectRoleTop* ORTop;
		/// BOTTOM object role
	TDLObjectRoleBottom* ORBottom;
		/// TOP data role
	TDLDataRoleTop* DRTop;
		/// BOTTOM data role
	TDLDataRoleBottom* DRBottom;
		/// TOP data element
	TDLDataTop* DTop;
		/// BOTTOM data element
	TDLDataBottom* DBottom;

		/// record all the references
	std::vector<TDLExpression*> RefRecorder;

protected:	// methods
		/// record the reference; @return the argument
	template<class T>
	T* record ( T* arg ) { RefRecorder.push_back(arg); return arg; }

public:		// interface
		/// empty c'tor
	TExpressionManager ( void );
		/// d'tor
	~TExpressionManager ( void );

		/// clear the ontology
	void clear ( void );

	// argument lists

		/// opens new argument list
	void newArgList ( void ) { ArgQueue.openArgList(); }
		/// add argument ARG to the current argument list
	void addArg ( TDLExpression* arg ) { ArgQueue.addArg(arg); }

	// create expressions methods

	// concepts

		/// get TOP concept
	TDLConceptExpression* getConceptTop ( void ) const { return CTop; }
		/// get BOTTOM concept
	TDLConceptExpression* getConceptBottom ( void ) const { return CBottom; }
		/// get named concept
	TDLConceptExpression* getConcept ( const std::string& name ) { return NS_C.insert(name); }

	// individuals

		/// get named individual
	TDLIndividualExpression* getIndividual ( const std::string& name ) { return NS_I.insert(name); }

	// object roles

		/// get TOP object role
	TDLObjectRoleExpression* getObjectRoleTop ( void ) const { return ORTop; }
		/// get BOTTOM object role
	TDLObjectRoleExpression* getObjectRoleBottom ( void ) const { return ORBottom; }
		/// get named object role
	TDLObjectRoleExpression* getObjectRole ( const std::string& name ) { return NS_OR.insert(name); }
		/// get an inverse of a given object role expression R
	TDLObjectRoleExpression* getObjectRoleInverse ( TDLObjectRoleExpression* R ) { return record(new TDLObjectRoleInverse(R)); }
		/// get a role chain corresponding to R1 o ... o Rn; take the arguments from the last argument list
	TDLObjectRoleComplexExpression* getObjectRoleChain ( void ) { return record(new TDLObjectRoleChain(ArgQueue.getLastArgList())); }
		/// get a expression corresponding to R projected from C
	TDLObjectRoleComplexExpression* getObjectRoleProjectionFrom ( TDLObjectRoleExpression* R, TDLConceptExpression* C )
		{ return record(new TDLObjectRoleProjectionFrom(R,C)); }
		/// get a expression corresponding to R projected into C
	TDLObjectRoleComplexExpression* getObjectRoleProjectionInto ( TDLObjectRoleExpression* R, TDLConceptExpression* C )
		{ return record(new TDLObjectRoleProjectionInto(R,C)); }

	// data roles

		/// get TOP data role
	TDLDataRoleExpression* getDataRoleTop ( void ) const { return DRTop; }
		/// get BOTTOM data role
	TDLDataRoleExpression* getDataRoleBottom ( void ) const { return DRBottom; }
		/// get named data role
	TDLDataRoleExpression* getDataRole ( const std::string& name ) { return NS_DR.insert(name); }

	// data expressions

		/// get TOP data element
	TDLDataExpression* getDataTop ( void ) const { return DTop; }
		/// get BOTTOM data element
	TDLDataExpression* getDataBottom ( void ) const { return DBottom; }
		/// get named data type
	TDLDataExpression* getDataType ( const std::string& name ) { return NS_DT.insert(name); }
}; // TExpressionManager

#endif
