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

#ifndef TDLEXPRESSION_H
#define TDLEXPRESSION_H

#include <vector>
#include <string>

// forward declaration for all expression classes: necessary for the visitor pattern
class TDLExpression;

class TDLConceptExpression;

class TDLRoleExpression;
class TDLObjectRoleExpression;

class TDLDataRoleExpression;

class TDLIndividualExpression;

class TDLDataExpression;

/// general visitor for DL expressions
class DLExpressionVisitor
{
public:		// visitor interface
	// concept expressions

	// object role expressions

	// data role expressions

	// data expressions

	// other methods
	virtual ~DLExpressionVisitor ( void ) {}
}; // DLExpressionVisitor


/// base class for the DL expression, which include concept-, (data)role-, individual-, and data ones
class TDLExpression
{
public:		// interface
		/// empty c'tor
	TDLExpression ( void ) {}
		/// empty d'tor: note that no deep delete is necessary as all the elements are RO
	virtual ~TDLExpression ( void ) {}

		/// accept method for the visitor pattern
	virtual void accept ( DLExpressionVisitor& visitor ) = 0;
}; // TDLExpression


//------------------------------------------------------------------
//	concept expressions
//------------------------------------------------------------------


//------------------------------------------------------------------
///	general concept expression
//------------------------------------------------------------------
class TDLConceptExpression: public TDLExpression
{
public:		// interface
		/// empty c'tor
	TDLConceptExpression ( void ) : TDLExpression() {}
		/// empty d'tor
	virtual ~TDLConceptExpression ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) = 0;
}; // TDLConceptExpression


//------------------------------------------------------------------
//	role expressions
//------------------------------------------------------------------


//------------------------------------------------------------------
///	general role expression
//------------------------------------------------------------------
class TDLRoleExpression: public TDLExpression
{
public:		// interface
		/// empty c'tor
	TDLRoleExpression ( void ) : TDLExpression() {}
		/// empty d'tor
	virtual ~TDLRoleExpression ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) = 0;
}; // TDLRoleExpression


//------------------------------------------------------------------
//	object role expressions
//------------------------------------------------------------------


//------------------------------------------------------------------
///	general object role expression
//------------------------------------------------------------------
class TDLObjectRoleExpression: public TDLRoleExpression
{
public:		// interface
		/// empty c'tor
	TDLObjectRoleExpression ( void ) : TDLRoleExpression() {}
		/// empty d'tor
	virtual ~TDLObjectRoleExpression ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) = 0;
}; // TDLObjectRoleExpression


//------------------------------------------------------------------
//	data role expressions
//------------------------------------------------------------------


//------------------------------------------------------------------
///	general data role expression
//------------------------------------------------------------------
class TDLDataRoleExpression: public TDLRoleExpression
{
public:		// interface
		/// empty c'tor
	TDLDataRoleExpression ( void ) : TDLRoleExpression() {}
		/// empty d'tor
	virtual ~TDLDataRoleExpression ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) = 0;
}; // TDLDataRoleExpression


//------------------------------------------------------------------
//	data expressions
//------------------------------------------------------------------


//------------------------------------------------------------------
///	general data expression
//------------------------------------------------------------------
class TDLDataExpression: public TDLExpression
{
public:		// interface
		/// empty c'tor
	TDLDataExpression ( void ) : TDLExpression() {}
		/// empty d'tor
	virtual ~TDLDataExpression ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) = 0;
}; // TDLDataExpression


#endif
