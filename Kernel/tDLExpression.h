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
class TDLConceptName;

class TDLIndividualExpression;
class TDLIndividualName;

class TDLRoleExpression;
class TDLObjectRoleExpression;
class TDLObjectRoleTop;
class TDLObjectRoleBottom;
class TDLObjectRoleName;

class TDLDataRoleExpression;
class TDLDataRoleTop;
class TDLDataRoleBottom;
class TDLDataRoleName;

class TDLDataExpression;
class TDLDataTop;
class TDLDataBottom;
class TDLDataTypeName;

/// general visitor for DL expressions
class DLExpressionVisitor
{
public:		// visitor interface
	// concept expressions
	virtual void visit ( TDLConceptTop& expr ) = 0;
	virtual void visit ( TDLConceptBottom& expr ) = 0;
	virtual void visit ( TDLConceptName& expr ) = 0;

	// individual expressions
	virtual void visit ( TDLIndividualName& expr ) = 0;

	// object role expressions
	virtual void visit ( TDLObjectRoleTop& expr ) = 0;
	virtual void visit ( TDLObjectRoleBottom& expr ) = 0;
	virtual void visit ( TDLObjectRoleName& expr ) = 0;

	// data role expressions
	virtual void visit ( TDLDataRoleTop& expr ) = 0;
	virtual void visit ( TDLDataRoleBottom& expr ) = 0;
	virtual void visit ( TDLDataRoleName& expr ) = 0;

	// data expressions
	virtual void visit ( TDLDataTop& expr ) = 0;
	virtual void visit ( TDLDataBottom& expr ) = 0;
	virtual void visit ( TDLDataTypeName& expr ) = 0;

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
//	helper classes
//------------------------------------------------------------------


//------------------------------------------------------------------
///	named entity
//------------------------------------------------------------------
class TNamedEntity
{
protected:	// members
		/// name of the entity
	std::string Name;

public:		// interface
		/// c'tor: initialise name
	TNamedEntity ( const std::string& name ) : Name(name) {}
		/// empty d'tor
	virtual ~TNamedEntity ( void ) {}

		/// get access to the name
	const char* getName ( void ) const { return Name.c_str(); }
}; // TNamedEntity


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
///	named concept expression
//------------------------------------------------------------------
class TDLConceptName: public TDLConceptExpression, public TNamedEntity
{
public:		// interface
		/// c'tor: init field(s)
	TDLConceptName ( const std::string& name ) : TDLConceptExpression(), TNamedEntity(name) {}
		/// empty d'tor
	virtual ~TDLConceptName ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) { visitor.visit(*this); }
}; // TDLConceptName


//------------------------------------------------------------------
//	individual expressions
//------------------------------------------------------------------


//------------------------------------------------------------------
///	general individual expression
//------------------------------------------------------------------
class TDLIndividualExpression: public TDLExpression
{
public:		// interface
		/// empty c'tor
	TDLIndividualExpression ( void ) : TDLExpression() {}
		/// empty d'tor
	virtual ~TDLIndividualExpression ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) = 0;
}; // TDLIndividualExpression

//------------------------------------------------------------------
///	named individual expression
//------------------------------------------------------------------
class TDLIndividualName: public TDLIndividualExpression, public TNamedEntity
{
public:		// interface
		/// c'tor: init field(s)
	TDLIndividualName ( const std::string& name ) : TDLIndividualExpression(), TNamedEntity(name) {}
		/// empty d'tor
	virtual ~TDLIndividualName ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) { visitor.visit(*this); }
}; // TDLIndividualName


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
///	named object role expression
//------------------------------------------------------------------
class TDLObjectRoleName: public TDLObjectRoleExpression, public TNamedEntity
{
public:		// interface
		/// c'tor: init field(s)
	TDLObjectRoleName ( const std::string& name ) : TDLObjectRoleExpression(), TNamedEntity(name) {}
		/// empty d'tor
	virtual ~TDLObjectRoleName ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) { visitor.visit(*this); }
}; // TDLObjectRoleName


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
///	named data role expression
//------------------------------------------------------------------
class TDLDataRoleName: public TDLDataRoleExpression, public TNamedEntity
{
public:		// interface
		/// c'tor: init field(s)
	TDLDataRoleName ( const std::string& name ) : TDLDataRoleExpression(), TNamedEntity(name) {}
		/// empty d'tor
	virtual ~TDLDataRoleName ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) { visitor.visit(*this); }
}; // TDLDataRoleName


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

//------------------------------------------------------------------
///	named data type expression
//------------------------------------------------------------------
class TDLDataTypeName: public TDLDataExpression, public TNamedEntity
{
public:		// interface
		/// c'tor: init field(s)
	TDLDataTypeName ( const std::string& name ) : TDLDataExpression(), TNamedEntity(name) {}
		/// empty d'tor
	virtual ~TDLDataTypeName ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) { visitor.visit(*this); }
}; // TDLDataTypeName


#endif
