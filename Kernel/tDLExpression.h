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
class TDLConceptTop;
class TDLConceptBottom;

class TDLRoleExpression;
class TDLObjectRoleExpression;
class TDLObjectRoleTop;
class TDLObjectRoleBottom;

class TDLDataRoleExpression;
class TDLDataRoleTop;
class TDLDataRoleBottom;

class TDLIndividualExpression;

class TDLDataExpression;
class TDLDataTop;
class TDLDataBottom;

/// general visitor for DL expressions
class DLExpressionVisitor
{
public:		// visitor interface
	// concept expressions
	virtual void visit ( TDLConceptTop& expr ) = 0;
	virtual void visit ( TDLConceptBottom& expr ) = 0;

	// object role expressions
	virtual void visit ( TDLObjectRoleTop& expr ) = 0;
	virtual void visit ( TDLObjectRoleBottom& expr ) = 0;

	// data role expressions
	virtual void visit ( TDLDataRoleTop& expr ) = 0;
	virtual void visit ( TDLDataRoleBottom& expr ) = 0;

	// data expressions
	virtual void visit ( TDLDataTop& expr ) = 0;
	virtual void visit ( TDLDataBottom& expr ) = 0;

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
///	concept TOP expression
//------------------------------------------------------------------
class TDLConceptTop: public TDLConceptExpression
{
public:		// interface
		/// empty c'tor
	TDLConceptTop ( void ) : TDLConceptExpression() {}
		/// empty d'tor
	virtual ~TDLConceptTop ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) { visitor.visit(*this); }
}; // TDLConceptTop

//------------------------------------------------------------------
///	concept BOTTOM expression
//------------------------------------------------------------------
class TDLConceptBottom: public TDLConceptExpression
{
public:		// interface
		/// empty c'tor
	TDLConceptBottom ( void ) : TDLConceptExpression() {}
		/// empty d'tor
	virtual ~TDLConceptBottom ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) { visitor.visit(*this); }
}; // TDLConceptBottom



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
///	object role TOP expression
//------------------------------------------------------------------
class TDLObjectRoleTop: public TDLObjectRoleExpression
{
public:		// interface
		/// empty c'tor
	TDLObjectRoleTop ( void ) : TDLObjectRoleExpression() {}
		/// empty d'tor
	virtual ~TDLObjectRoleTop ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) { visitor.visit(*this); }
}; // TDLObjectRoleTop

//------------------------------------------------------------------
///	object role BOTTOM expression
//------------------------------------------------------------------
class TDLObjectRoleBottom: public TDLObjectRoleExpression
{
public:		// interface
		/// empty c'tor
	TDLObjectRoleBottom ( void ) : TDLObjectRoleExpression() {}
		/// empty d'tor
	virtual ~TDLObjectRoleBottom ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) { visitor.visit(*this); }
}; // TDLObjectRoleBottom


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
///	data role TOP expression
//------------------------------------------------------------------
class TDLDataRoleTop: public TDLDataRoleExpression
{
public:		// interface
		/// empty c'tor
	TDLDataRoleTop ( void ) : TDLDataRoleExpression() {}
		/// empty d'tor
	virtual ~TDLDataRoleTop ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) { visitor.visit(*this); }
}; // TDLDataRoleTop

//------------------------------------------------------------------
///	data role BOTTOM expression
//------------------------------------------------------------------
class TDLDataRoleBottom: public TDLDataRoleExpression
{
public:		// interface
		/// empty c'tor
	TDLDataRoleBottom ( void ) : TDLDataRoleExpression() {}
		/// empty d'tor
	virtual ~TDLDataRoleBottom ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) { visitor.visit(*this); }
}; // TDLDataRoleBottom


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

//------------------------------------------------------------------
///	data TOP expression
//------------------------------------------------------------------
class TDLDataTop: public TDLDataExpression
{
public:		// interface
		/// empty c'tor
	TDLDataTop ( void ) : TDLDataExpression() {}
		/// empty d'tor
	virtual ~TDLDataTop ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) { visitor.visit(*this); }
}; // TDLDataTop

//------------------------------------------------------------------
///	data BOTTOM expression
//------------------------------------------------------------------
class TDLDataBottom: public TDLDataExpression
{
public:		// interface
		/// empty c'tor
	TDLDataBottom ( void ) : TDLDataExpression() {}
		/// empty d'tor
	virtual ~TDLDataBottom ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) { visitor.visit(*this); }
}; // TDLDataBottom


#endif
