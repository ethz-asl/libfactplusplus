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
		/// get negation of a concept C
	TDLConceptExpression* getConceptNot ( TDLConceptExpression* C ) { return record(new TDLConceptNot(C)); }
		/// get an n-ary conjunction expression; take the arguments from the last argument list
	TDLConceptExpression* getConceptAnd ( void ) { return record(new TDLConceptAnd(ArgQueue.getLastArgList())); }
		/// get an n-ary disjunction expression; take the arguments from the last argument list
	TDLConceptExpression* getConceptOr ( void ) { return record(new TDLConceptOr(ArgQueue.getLastArgList())); }
		/// get an n-ary one-of expression; take the arguments from the last argument list
	TDLConceptExpression* getConceptOneOf ( void ) { return record(new TDLConceptOneOf(ArgQueue.getLastArgList())); }

		/// get self-reference restriction of an object role R
	TDLConceptExpression* getConceptObjectSelf ( TDLObjectRoleExpression* R ) { return record(new TDLConceptObjectSelf(R)); }
		/// get value restriction wrt an object role R and an individual I
	TDLConceptExpression* getConceptObjectValue ( TDLObjectRoleExpression* R, TDLIndividualExpression* I )
		{ return record(new TDLConceptObjectValue(R,I)); }
		/// get existential restriction wrt an object role R and a concept C
	TDLConceptExpression* getConceptObjectExists ( TDLObjectRoleExpression* R, TDLConceptExpression* C )
		{ return record(new TDLConceptObjectExists(R,C)); }
		/// get universal restriction wrt an object role R and a concept C
	TDLConceptExpression* getConceptObjectForall ( TDLObjectRoleExpression* R, TDLConceptExpression* C )
		{ return record(new TDLConceptObjectForall(R,C)); }
		/// get min cardinality restriction wrt number N, an object role R and a concept C
	TDLConceptExpression* getConceptObjectMinCardinality ( unsigned int n, TDLObjectRoleExpression* R, TDLConceptExpression* C )
		{ return record(new TDLConceptObjectMinCardinality(n,R,C)); }
		/// get max cardinality restriction wrt number N, an object role R and a concept C
	TDLConceptExpression* getConceptObjectMaxCardinality ( unsigned int n, TDLObjectRoleExpression* R, TDLConceptExpression* C )
		{ return record(new TDLConceptObjectMaxCardinality(n,R,C)); }
		/// get exact cardinality restriction wrt number N, an object role R and a concept C
	TDLConceptExpression* getConceptObjectExactCardinality ( unsigned int n, TDLObjectRoleExpression* R, TDLConceptExpression* C )
		{ return record(new TDLConceptObjectExactCardinality(n,R,C)); }

		/// get value restriction wrt a data role R and a data value V
	TDLConceptExpression* getConceptDataValue ( TDLDataRoleExpression* R, TDLDataValue* V )
		{ return record(new TDLConceptDataValue(R,V)); }
		/// get existential restriction wrt a data role R and a data expression E
	TDLConceptExpression* getConceptDataExists ( TDLDataRoleExpression* R, TDLDataExpression* E )
		{ return record(new TDLConceptDataExists(R,E)); }
		/// get universal restriction wrt a data role R and a data expression E
	TDLConceptExpression* getConceptDataForall ( TDLDataRoleExpression* R, TDLDataExpression* E )
		{ return record(new TDLConceptDataForall(R,E)); }
		/// get min cardinality restriction wrt number N, a data role R and a data expression E
	TDLConceptExpression* getConceptDataMinCardinality ( unsigned int n, TDLDataRoleExpression* R, TDLDataExpression* E )
		{ return record(new TDLConceptDataMinCardinality(n,R,E)); }
		/// get max cardinality restriction wrt number N, a data role R and a data expression E
	TDLConceptExpression* getConceptDataMaxCardinality ( unsigned int n, TDLDataRoleExpression* R, TDLDataExpression* E )
		{ return record(new TDLConceptDataMaxCardinality(n,R,E)); }
		/// get exact cardinality restriction wrt number N, a data role R and a data expression E
	TDLConceptExpression* getConceptDataExactCardinality ( unsigned int n, TDLDataRoleExpression* R, TDLDataExpression* E )
		{ return record(new TDLConceptDataExactCardinality(n,R,E)); }

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
		/// get data value with given VALUE and TYPE
	TDLDataExpression* getDataValue ( const std::string& value, TDLDataTypeName* type ) { return record(new TDLDataValue(value,type)); }
}; // TExpressionManager

#endif
