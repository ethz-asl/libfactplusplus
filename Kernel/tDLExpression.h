/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2010-2011 by Dmitry Tsarkov

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

#include "globaldef.h"
#include "eFaCTPlusPlus.h"
#include "fpp_assert.h"
#include "tNameSet.h"

// forward declaration for all expression classes: necessary for the visitor pattern
class TDLExpression;

class  TDLConceptExpression;
class   TDLConceptTop;
class   TDLConceptBottom;
class   TDLConceptName;
class   TDLConceptNot;
class   TDLConceptAnd;
class   TDLConceptOr;
class   TDLConceptOneOf;
class   TDLConceptObjectRoleExpression;
class    TDLConceptObjectSelf;
class    TDLConceptObjectValue;
class    TDLConceptObjectRCExpression;
class     TDLConceptObjectExists;
class     TDLConceptObjectForall;
class     TDLConceptObjectCardinalityExpression;
class      TDLConceptObjectMinCardinality;
class      TDLConceptObjectMaxCardinality;
class      TDLConceptObjectExactCardinality;
class   TDLConceptDataRoleExpression;
class    TDLConceptDataValue;
class    TDLConceptDataRVExpression;
class     TDLConceptDataExists;
class     TDLConceptDataForall;
class     TDLConceptDataCardinalityExpression;
class      TDLConceptDataMinCardinality;
class      TDLConceptDataMaxCardinality;
class      TDLConceptDataExactCardinality;

class  TDLIndividualExpression;
class   TDLIndividualName;

class  TDLRoleExpression;
class   TDLObjectRoleComplexExpression;
class    TDLObjectRoleExpression;
class     TDLObjectRoleTop;
class     TDLObjectRoleBottom;
class     TDLObjectRoleName;
class     TDLObjectRoleInverse;
class    TDLObjectRoleChain;
class    TDLObjectRoleProjectionFrom;
class    TDLObjectRoleProjectionInto;

class   TDLDataRoleExpression;
class    TDLDataRoleTop;
class    TDLDataRoleBottom;
class    TDLDataRoleName;

class  TDLDataExpression;
class   TDLDataTop;
class   TDLDataBottom;
class   TDLDataTypeExpression;
class    TDLDataTypeName;
class    TDLDataTypeRestriction;
class   TDLDataValue;
class   TDLDataNot;
class   TDLDataAnd;
class   TDLDataOr;
class   TDLDataOneOf;
class   TDLFacetExpression;
class    TDLFacetMinInclusive;
class    TDLFacetMinExclusive;
class    TDLFacetMaxInclusive;
class    TDLFacetMaxExclusive;
//class    TDLFacetLength;
//class    TDLFacetMinLength;
//class    TDLFacetMaxLength;
//class    TDLFacetPattern;
//class    TDLFacetLangRange;

/// general visitor for DL expressions
class DLExpressionVisitor
{
public:		// visitor interface
	// concept expressions
	virtual void visit ( const TDLConceptTop& expr ) = 0;
	virtual void visit ( const TDLConceptBottom& expr ) = 0;
	virtual void visit ( const TDLConceptName& expr ) = 0;
	virtual void visit ( const TDLConceptNot& expr ) = 0;
	virtual void visit ( const TDLConceptAnd& expr ) = 0;
	virtual void visit ( const TDLConceptOr& expr ) = 0;
	virtual void visit ( const TDLConceptOneOf& expr ) = 0;
	virtual void visit ( const TDLConceptObjectSelf& expr ) = 0;
	virtual void visit ( const TDLConceptObjectValue& expr ) = 0;
	virtual void visit ( const TDLConceptObjectExists& expr ) = 0;
	virtual void visit ( const TDLConceptObjectForall& expr ) = 0;
	virtual void visit ( const TDLConceptObjectMinCardinality& expr ) = 0;
	virtual void visit ( const TDLConceptObjectMaxCardinality& expr ) = 0;
	virtual void visit ( const TDLConceptObjectExactCardinality& expr ) = 0;
	virtual void visit ( const TDLConceptDataValue& expr ) = 0;
	virtual void visit ( const TDLConceptDataExists& expr ) = 0;
	virtual void visit ( const TDLConceptDataForall& expr ) = 0;
	virtual void visit ( const TDLConceptDataMinCardinality& expr ) = 0;
	virtual void visit ( const TDLConceptDataMaxCardinality& expr ) = 0;
	virtual void visit ( const TDLConceptDataExactCardinality& expr ) = 0;

	// individual expressions
	virtual void visit ( const TDLIndividualName& expr ) = 0;

	// object role expressions
	virtual void visit ( const TDLObjectRoleTop& expr ) = 0;
	virtual void visit ( const TDLObjectRoleBottom& expr ) = 0;
	virtual void visit ( const TDLObjectRoleName& expr ) = 0;
	virtual void visit ( const TDLObjectRoleInverse& expr ) = 0;
	virtual void visit ( const TDLObjectRoleChain& expr ) = 0;
	virtual void visit ( const TDLObjectRoleProjectionFrom& expr ) = 0;
	virtual void visit ( const TDLObjectRoleProjectionInto& expr ) = 0;

	// data role expressions
	virtual void visit ( const TDLDataRoleTop& expr ) = 0;
	virtual void visit ( const TDLDataRoleBottom& expr ) = 0;
	virtual void visit ( const TDLDataRoleName& expr ) = 0;

	// data expressions
	virtual void visit ( const TDLDataTop& expr ) = 0;
	virtual void visit ( const TDLDataBottom& expr ) = 0;
	virtual void visit ( const TDLDataTypeName& expr ) = 0;
	virtual void visit ( const TDLDataTypeRestriction& expr ) = 0;
	virtual void visit ( const TDLDataValue& expr ) = 0;
	virtual void visit ( const TDLDataNot& expr ) = 0;
	virtual void visit ( const TDLDataAnd& expr ) = 0;
	virtual void visit ( const TDLDataOr& expr ) = 0;
	virtual void visit ( const TDLDataOneOf& expr ) = 0;

	// facets
	virtual void visit ( const TDLFacetMinInclusive& expr ) = 0;
	virtual void visit ( const TDLFacetMinExclusive& expr ) = 0;
	virtual void visit ( const TDLFacetMaxInclusive& expr ) = 0;
	virtual void visit ( const TDLFacetMaxExclusive& expr ) = 0;

	// other methods
	virtual ~DLExpressionVisitor ( void ) {}
}; // DLExpressionVisitor

/// empty visitor for DL expressions implementation
class DLExpressionVisitorEmpty: public DLExpressionVisitor
{
public:		// visitor interface
	// concept expressions
	virtual void visit ( const TDLConceptTop& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLConceptBottom& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLConceptName& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLConceptNot& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLConceptAnd& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLConceptOr& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLConceptOneOf& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLConceptObjectSelf& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLConceptObjectValue& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLConceptObjectExists& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLConceptObjectForall& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLConceptObjectMinCardinality& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLConceptObjectMaxCardinality& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLConceptObjectExactCardinality& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLConceptDataValue& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLConceptDataExists& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLConceptDataForall& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLConceptDataMinCardinality& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLConceptDataMaxCardinality& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLConceptDataExactCardinality& expr ATTR_UNUSED ) {}

	// individual expressions
	virtual void visit ( const TDLIndividualName& expr ATTR_UNUSED ) {}

	// object role expressions
	virtual void visit ( const TDLObjectRoleTop& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLObjectRoleBottom& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLObjectRoleName& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLObjectRoleInverse& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLObjectRoleChain& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLObjectRoleProjectionFrom& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLObjectRoleProjectionInto& expr ATTR_UNUSED ) {}

	// data role expressions
	virtual void visit ( const TDLDataRoleTop& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLDataRoleBottom& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLDataRoleName& expr ATTR_UNUSED ) {}

	// data expressions
	virtual void visit ( const TDLDataTop& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLDataBottom& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLDataTypeName& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLDataTypeRestriction& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLDataValue& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLDataNot& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLDataAnd& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLDataOr& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLDataOneOf& expr ATTR_UNUSED ) {}

	// facets
	virtual void visit ( const TDLFacetMinInclusive& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLFacetMinExclusive& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLFacetMaxInclusive& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLFacetMaxExclusive& expr ATTR_UNUSED ) {}

	// other methods
	virtual ~DLExpressionVisitorEmpty ( void ) {}
}; // DLExpressionVisitorEmpty


/// base class for the DL expression, which include concept-, (data)role-, individual-, and data ones
class TDLExpression
{
public:		// interface
		/// empty c'tor
	TDLExpression ( void ) {}
		/// empty d'tor: note that no deep delete is necessary as all the elements are RO
	virtual ~TDLExpression ( void ) {}

		/// accept method for the visitor pattern
	virtual void accept ( DLExpressionVisitor& visitor ) const = 0;
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
		/// get access to the element itself
	const TNamedEntity* getEntity ( void ) const { return this; }
}; // TNamedEntity

//------------------------------------------------------------------
///	concept argument
//------------------------------------------------------------------
class TConceptArg
{
protected:	// members
		/// concept argument
	const TDLConceptExpression* C;

public:		// interface
		/// init c'tor
	TConceptArg ( const TDLConceptExpression* c ) : C(c) {}
		/// empty d'tor
	virtual ~TConceptArg ( void ) {}

		/// get access to the argument
	const TDLConceptExpression* getC ( void ) const { return C; }
}; // TConceptArg

//------------------------------------------------------------------
///	individual argument
//------------------------------------------------------------------
class TIndividualArg
{
protected:	// members
		/// individual argument
	const TDLIndividualExpression* I;

public:		// interface
		/// init c'tor
	TIndividualArg ( const TDLIndividualExpression* i ) : I(i) {}
		/// empty d'tor
	virtual ~TIndividualArg ( void ) {}

		/// get access to the argument
	const TDLIndividualExpression* getI ( void ) const { return I; }
}; // TIndividualArg

//------------------------------------------------------------------
///	numerical argument
//------------------------------------------------------------------
class TNumberArg
{
protected:	// members
		/// number argument
	unsigned int N;

public:		// interface
		/// init c'tor
	TNumberArg ( unsigned int n ) : N(n) {}
		/// empty d'tor
	virtual ~TNumberArg ( void ) {}

		/// get access to the argument
	unsigned int getNumber ( void ) const { return N; }
}; // TNumberArg

//------------------------------------------------------------------
///	object role argument
//------------------------------------------------------------------
class TObjectRoleArg
{
protected:	// members
		/// object role argument
	const TDLObjectRoleExpression* OR;

public:		// interface
		/// init c'tor
	TObjectRoleArg ( const TDLObjectRoleExpression* oR ) : OR(oR) {}
		/// empty d'tor
	virtual ~TObjectRoleArg ( void ) {}

		/// get access to the argument
	const TDLObjectRoleExpression* getOR ( void ) const { return OR; }
}; // TObjectRoleArg

//------------------------------------------------------------------
///	data role argument
//------------------------------------------------------------------
class TDataRoleArg
{
protected:	// members
		/// data role argument
	const TDLDataRoleExpression* DR;

public:		// interface
		/// init c'tor
	TDataRoleArg ( const TDLDataRoleExpression* dR ) : DR(dR) {}
		/// empty d'tor
	virtual ~TDataRoleArg ( void ) {}

		/// get access to the argument
	const TDLDataRoleExpression* getDR ( void ) const { return DR; }
}; // TDataRoleArg

//------------------------------------------------------------------
///	data expression argument (templated with the exact type)
//------------------------------------------------------------------
template<class TExpression>
class TDataExpressionArg
{
protected:	// members
		/// data expression argument
	const TExpression* Expr;

public:		// interface
		/// init c'tor
	TDataExpressionArg ( const TExpression* expr ) : Expr(expr) {}
		/// empty d'tor
	virtual ~TDataExpressionArg ( void ) {}

		/// get access to the argument
	const TExpression* getExpr ( void ) const { return Expr; }
}; // TDataExpressionArg

//------------------------------------------------------------------
///	general n-argument expression
//------------------------------------------------------------------
template<class Argument>
class TDLNAryExpression
{
public:		// types
		/// base type
	typedef std::vector<const Argument*> ArgumentArray;
		/// RW iterator over base type
	typedef typename ArgumentArray::const_iterator iterator;
		/// input array type
	typedef std::vector<const TDLExpression*> ExpressionArray;
		/// RW input iterator
	typedef ExpressionArray::const_iterator i_iterator;

protected:	// members
		/// set of equivalent concept descriptions
	ArgumentArray Base;
		/// name for excepion depending on class name and direction
	std::string EString;

protected:	// methods
		/// transform general expression into the argument one
	const Argument* transform ( const TDLExpression* arg ) const
	{
		const Argument* p = dynamic_cast<const Argument*>(arg);
		if ( p == NULL )
			throw EFaCTPlusPlus(EString.c_str());
		return p;
	}

public:		// interface
		/// c'tor: build an error string
	TDLNAryExpression ( const char* typeName, const char* className )
	{
		EString = "Expected ";
		EString += typeName;
		EString += " argument in the '";
		EString += className;
		EString += "' expression";
	}
		/// empty d'tor
	virtual ~TDLNAryExpression ( void ) {}

		/// @return true iff the expression has no elements
	bool empty ( void ) const { return Base.empty(); }
		/// @return number of elements
	size_t size ( void ) const { return Base.size(); }

	// add elements to the array

		/// add a single element to the array
	void add ( const TDLExpression* p ) { Base.push_back(transform(p)); }
		/// add a range to the array
	void add ( i_iterator b, i_iterator e )
	{
		for ( ; b != e; ++b )
			add(*b);
	}
		/// add a vector
	void add ( const ExpressionArray& v ) { add ( v.begin(), v.end() ); }

	// access to members

		/// RW begin iterator for array
	iterator begin ( void ) const { return Base.begin(); }
		/// RW end iterator for array
	iterator end ( void ) const { return Base.end(); }
}; // TDLNAryExpression


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
	void accept ( DLExpressionVisitor& visitor ) const = 0;
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
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
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
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLConceptBottom

//------------------------------------------------------------------
///	named concept expression
//------------------------------------------------------------------
class TDLConceptName: public TDLConceptExpression, public TNamedEntity
{
public:		// interface
		/// init c'tor
	TDLConceptName ( const std::string& name ) : TDLConceptExpression(), TNamedEntity(name) {}
		/// empty d'tor
	virtual ~TDLConceptName ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLConceptName

//------------------------------------------------------------------
///	concept NOT expression
//------------------------------------------------------------------
class TDLConceptNot: public TDLConceptExpression, public TConceptArg
{
public:		// interface
		/// init c'tor
	TDLConceptNot ( const TDLConceptExpression* C )
		: TDLConceptExpression()
		, TConceptArg(C)
		{}
		/// empty d'tor
	virtual ~TDLConceptNot ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLConceptNot

//------------------------------------------------------------------
///	concept AND expression
//------------------------------------------------------------------
class TDLConceptAnd: public TDLConceptExpression, public TDLNAryExpression<TDLConceptExpression>
{
public:		// interface
		/// init c'tor: create AND of expressions from the given array
	TDLConceptAnd ( const ExpressionArray& v )
		: TDLConceptExpression()
		, TDLNAryExpression<TDLConceptExpression>("concept expression","AND")
	{
		add(v);
	}
		/// empty d'tor
	virtual ~TDLConceptAnd ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLConceptAnd

//------------------------------------------------------------------
///	concept OR expression
//------------------------------------------------------------------
class TDLConceptOr: public TDLConceptExpression, public TDLNAryExpression<TDLConceptExpression>
{
public:		// interface
		/// init c'tor: create OR of expressions from the given array
	TDLConceptOr ( const ExpressionArray& v )
		: TDLConceptExpression()
		, TDLNAryExpression<TDLConceptExpression>("concept expression","OR")
	{
		add(v);
	}
		/// empty d'tor
	virtual ~TDLConceptOr ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLConceptOr

//------------------------------------------------------------------
///	concept one-of expression
//------------------------------------------------------------------
class TDLConceptOneOf: public TDLConceptExpression, public TDLNAryExpression<TDLIndividualName>
{
public:		// interface
		/// init c'tor: create one-of from individuals in the given array
	TDLConceptOneOf ( const ExpressionArray& v )
		: TDLConceptExpression()
		, TDLNAryExpression<TDLIndividualName>("individual name","OneOf")
	{
		add(v);
	}
		/// empty d'tor
	virtual ~TDLConceptOneOf ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLConceptOneOf

//------------------------------------------------------------------
///	general concept expression that contains an object role
//------------------------------------------------------------------
class TDLConceptObjectRoleExpression: public TDLConceptExpression, public TObjectRoleArg
{
public:		// interface
		/// init c'tor
	TDLConceptObjectRoleExpression ( const TDLObjectRoleExpression* R )
		: TDLConceptExpression()
		, TObjectRoleArg(R)
		{}
		/// empty d'tor
	virtual ~TDLConceptObjectRoleExpression ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const = 0;
}; // TDLConceptObjectRoleExpression

//------------------------------------------------------------------
///	concept self-ref expression
//------------------------------------------------------------------
class TDLConceptObjectSelf: public TDLConceptObjectRoleExpression
{
public:		// interface
		/// init c'tor
	TDLConceptObjectSelf ( const TDLObjectRoleExpression* R )
		: TDLConceptObjectRoleExpression(R)
		{}
		/// empty d'tor
	virtual ~TDLConceptObjectSelf ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLConceptObjectSelf

//------------------------------------------------------------------
///	concept some value restriction expression
//------------------------------------------------------------------
class TDLConceptObjectValue: public TDLConceptObjectRoleExpression, public TIndividualArg
{
public:		// interface
		/// init c'tor
	TDLConceptObjectValue ( const TDLObjectRoleExpression* R, const TDLIndividualExpression* I )
		: TDLConceptObjectRoleExpression(R)
		, TIndividualArg(I)
		{}
		/// empty d'tor
	virtual ~TDLConceptObjectValue ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLConceptObjectValue

//------------------------------------------------------------------
///	general concept expression that contains an object role and a class expression
//------------------------------------------------------------------
class TDLConceptObjectRCExpression: public TDLConceptObjectRoleExpression, public TConceptArg
{
public:		// interface
		/// init c'tor
	TDLConceptObjectRCExpression ( const TDLObjectRoleExpression* R, const TDLConceptExpression* C )
		: TDLConceptObjectRoleExpression(R)
		, TConceptArg(C)
		{}
		/// empty d'tor
	virtual ~TDLConceptObjectRCExpression ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const = 0;
}; // TDLConceptObjectRCExpression

//------------------------------------------------------------------
///	concept object existential restriction expression
//------------------------------------------------------------------
class TDLConceptObjectExists: public TDLConceptObjectRCExpression
{
public:		// interface
		/// init c'tor
	TDLConceptObjectExists ( const TDLObjectRoleExpression* R, const TDLConceptExpression* C )
		: TDLConceptObjectRCExpression(R,C)
		{}
		/// empty d'tor
	virtual ~TDLConceptObjectExists ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLConceptObjectExists

//------------------------------------------------------------------
///	concept object universal restriction expression
//------------------------------------------------------------------
class TDLConceptObjectForall: public TDLConceptObjectRCExpression
{
public:		// interface
		/// init c'tor
	TDLConceptObjectForall ( const TDLObjectRoleExpression* R, const TDLConceptExpression* C )
		: TDLConceptObjectRCExpression(R,C)
		{}
		/// empty d'tor
	virtual ~TDLConceptObjectForall ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLConceptObjectForall

//------------------------------------------------------------------
///	general object role cardinality expression
//------------------------------------------------------------------
class TDLConceptObjectCardinalityExpression: public TDLConceptObjectRCExpression, public TNumberArg
{
public:		// interface
		/// init c'tor
	TDLConceptObjectCardinalityExpression ( unsigned int n, const TDLObjectRoleExpression* R, const TDLConceptExpression* C )
		: TDLConceptObjectRCExpression(R,C)
		, TNumberArg(n)
		{}
		/// empty d'tor
	virtual ~TDLConceptObjectCardinalityExpression ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const = 0;
}; // TDLConceptObjectCardinalityExpression

//------------------------------------------------------------------
///	concept object min cardinality expression
//------------------------------------------------------------------
class TDLConceptObjectMinCardinality: public TDLConceptObjectCardinalityExpression
{
public:		// interface
		/// init c'tor
	TDLConceptObjectMinCardinality ( unsigned int n, const TDLObjectRoleExpression* R, const TDLConceptExpression* C )
		: TDLConceptObjectCardinalityExpression(n,R,C)
		{}
		/// empty d'tor
	virtual ~TDLConceptObjectMinCardinality ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLConceptObjectMinCardinality

//------------------------------------------------------------------
///	concept object max cardinality expression
//------------------------------------------------------------------
class TDLConceptObjectMaxCardinality: public TDLConceptObjectCardinalityExpression
{
public:		// interface
		/// init c'tor
	TDLConceptObjectMaxCardinality ( unsigned int n, const TDLObjectRoleExpression* R, const TDLConceptExpression* C )
		: TDLConceptObjectCardinalityExpression(n,R,C)
		{}
		/// empty d'tor
	virtual ~TDLConceptObjectMaxCardinality ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLConceptObjectMaxCardinality

//------------------------------------------------------------------
///	concept object exact cardinality expression
//------------------------------------------------------------------
class TDLConceptObjectExactCardinality: public TDLConceptObjectCardinalityExpression
{
public:		// interface
		/// init c'tor
	TDLConceptObjectExactCardinality ( unsigned int n, const TDLObjectRoleExpression* R, const TDLConceptExpression* C )
		: TDLConceptObjectCardinalityExpression(n,R,C)
		{}
		/// empty d'tor
	virtual ~TDLConceptObjectExactCardinality ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLConceptObjectExactCardinality

//------------------------------------------------------------------
///	general concept expression that contains an data role
//------------------------------------------------------------------
class TDLConceptDataRoleExpression: public TDLConceptExpression, public TDataRoleArg
{
public:		// interface
		/// init c'tor
	TDLConceptDataRoleExpression ( const TDLDataRoleExpression* R )
		: TDLConceptExpression()
		, TDataRoleArg(R)
		{}
		/// empty d'tor
	virtual ~TDLConceptDataRoleExpression ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const = 0;
}; // TDLConceptDataRoleExpression

//------------------------------------------------------------------
///	concept some value restriction expression
//------------------------------------------------------------------
class TDLConceptDataValue: public TDLConceptDataRoleExpression, public TDataExpressionArg<TDLDataValue>
{
public:		// interface
		/// init c'tor
	TDLConceptDataValue ( const TDLDataRoleExpression* R, const TDLDataValue* V )
		: TDLConceptDataRoleExpression(R)
		, TDataExpressionArg<TDLDataValue>(V)
		{}
		/// empty d'tor
	virtual ~TDLConceptDataValue ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLConceptDataValue

//------------------------------------------------------------------
///	general concept expression that contains an data role and a data expression
//------------------------------------------------------------------
class TDLConceptDataRVExpression: public TDLConceptDataRoleExpression, public TDataExpressionArg<TDLDataExpression>
{
public:		// interface
		/// init c'tor
	TDLConceptDataRVExpression ( const TDLDataRoleExpression* R, const TDLDataExpression* E )
		: TDLConceptDataRoleExpression(R)
		, TDataExpressionArg<TDLDataExpression>(E)
		{}
		/// empty d'tor
	virtual ~TDLConceptDataRVExpression ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const = 0;
}; // TDLConceptDataRVExpression

//------------------------------------------------------------------
///	concept data existential restriction expression
//------------------------------------------------------------------
class TDLConceptDataExists: public TDLConceptDataRVExpression
{
public:		// interface
		/// init c'tor
	TDLConceptDataExists ( const TDLDataRoleExpression* R, const TDLDataExpression* E )
		: TDLConceptDataRVExpression(R,E)
		{}
		/// empty d'tor
	virtual ~TDLConceptDataExists ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLConceptDataExists

//------------------------------------------------------------------
///	concept data universal restriction expression
//------------------------------------------------------------------
class TDLConceptDataForall: public TDLConceptDataRVExpression
{
public:		// interface
		/// init c'tor
	TDLConceptDataForall ( const TDLDataRoleExpression* R, const TDLDataExpression* E )
		: TDLConceptDataRVExpression(R,E)
		{}
		/// empty d'tor
	virtual ~TDLConceptDataForall ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLConceptDataForall

//------------------------------------------------------------------
///	general data role cardinality expression
//------------------------------------------------------------------
class TDLConceptDataCardinalityExpression: public TDLConceptDataRVExpression, public TNumberArg
{
public:		// interface
		/// init c'tor
	TDLConceptDataCardinalityExpression ( unsigned int n, const TDLDataRoleExpression* R, const TDLDataExpression* E )
		: TDLConceptDataRVExpression(R,E)
		, TNumberArg(n)
		{}
		/// empty d'tor
	virtual ~TDLConceptDataCardinalityExpression ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const = 0;
}; // TDLConceptDataCardinalityExpression

//------------------------------------------------------------------
///	concept data min cardinality expression
//------------------------------------------------------------------
class TDLConceptDataMinCardinality: public TDLConceptDataCardinalityExpression
{
public:		// interface
		/// init c'tor
	TDLConceptDataMinCardinality ( unsigned int n, const TDLDataRoleExpression* R, const TDLDataExpression* E )
		: TDLConceptDataCardinalityExpression(n,R,E)
		{}
		/// empty d'tor
	virtual ~TDLConceptDataMinCardinality ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLConceptDataMinCardinality

//------------------------------------------------------------------
///	concept data max cardinality expression
//------------------------------------------------------------------
class TDLConceptDataMaxCardinality: public TDLConceptDataCardinalityExpression
{
public:		// interface
		/// init c'tor
	TDLConceptDataMaxCardinality ( unsigned int n, const TDLDataRoleExpression* R, const TDLDataExpression* E )
		: TDLConceptDataCardinalityExpression(n,R,E)
		{}
		/// empty d'tor
	virtual ~TDLConceptDataMaxCardinality ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLConceptDataMaxCardinality

//------------------------------------------------------------------
///	concept data exact cardinality expression
//------------------------------------------------------------------
class TDLConceptDataExactCardinality: public TDLConceptDataCardinalityExpression
{
public:		// interface
		/// init c'tor
	TDLConceptDataExactCardinality ( unsigned int n, const TDLDataRoleExpression* R, const TDLDataExpression* E )
		: TDLConceptDataCardinalityExpression(n,R,E)
		{}
		/// empty d'tor
	virtual ~TDLConceptDataExactCardinality ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLConceptDataExactCardinality


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
	void accept ( DLExpressionVisitor& visitor ) const = 0;
}; // TDLIndividualExpression

//------------------------------------------------------------------
///	named individual expression
//------------------------------------------------------------------
class TDLIndividualName: public TDLIndividualExpression, public TNamedEntity
{
public:		// interface
		/// init c'tor
	TDLIndividualName ( const std::string& name ) : TDLIndividualExpression(), TNamedEntity(name) {}
		/// empty d'tor
	virtual ~TDLIndividualName ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
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
	void accept ( DLExpressionVisitor& visitor ) const = 0;
}; // TDLRoleExpression


//------------------------------------------------------------------
//	object role expressions
//------------------------------------------------------------------


//------------------------------------------------------------------
///	complex object role expression (general expression, role chain or projection)
//------------------------------------------------------------------
class TDLObjectRoleComplexExpression: public TDLRoleExpression
{
public:		// interface
		/// empty c'tor
	TDLObjectRoleComplexExpression ( void ) : TDLRoleExpression() {}
		/// empty d'tor
	virtual ~TDLObjectRoleComplexExpression ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const = 0;
}; // TDLObjectRoleComplexExpression

//------------------------------------------------------------------
///	general object role expression
//------------------------------------------------------------------
class TDLObjectRoleExpression: public TDLObjectRoleComplexExpression
{
public:		// interface
		/// empty c'tor
	TDLObjectRoleExpression ( void ) : TDLObjectRoleComplexExpression() {}
		/// empty d'tor
	virtual ~TDLObjectRoleExpression ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const = 0;
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
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
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
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLObjectRoleBottom

//------------------------------------------------------------------
///	named object role expression
//------------------------------------------------------------------
class TDLObjectRoleName: public TDLObjectRoleExpression, public TNamedEntity
{
public:		// interface
		/// init c'tor
	TDLObjectRoleName ( const std::string& name ) : TDLObjectRoleExpression(), TNamedEntity(name) {}
		/// empty d'tor
	virtual ~TDLObjectRoleName ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLObjectRoleName

//------------------------------------------------------------------
///	inverse object role expression
//------------------------------------------------------------------
class TDLObjectRoleInverse: public TDLObjectRoleExpression, public TObjectRoleArg
{
public:		// interface
		/// init c'tor
	TDLObjectRoleInverse ( const TDLObjectRoleExpression* R )
		: TDLObjectRoleExpression()
		, TObjectRoleArg(R)
		{}
		/// empty d'tor
	virtual ~TDLObjectRoleInverse ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLObjectRoleInverse

//------------------------------------------------------------------
/// object role chain expression
//------------------------------------------------------------------
class TDLObjectRoleChain: public TDLObjectRoleComplexExpression, public TDLNAryExpression<TDLObjectRoleExpression>
{
public:		// interface
		/// init c'tor: create role chain from given array
	TDLObjectRoleChain ( const ExpressionArray& v )
		: TDLObjectRoleComplexExpression()
		, TDLNAryExpression<TDLObjectRoleExpression>("object role expression","role chain")
	{
		add(v);
	}
		/// empty d'tor
	virtual ~TDLObjectRoleChain ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLObjectRoleChain

//------------------------------------------------------------------
///	object role projection from expression
//------------------------------------------------------------------
class TDLObjectRoleProjectionFrom
	: public TDLObjectRoleComplexExpression
	, public TObjectRoleArg
	, public TConceptArg
{
public:		// interface
		/// init c'tor
	TDLObjectRoleProjectionFrom ( const TDLObjectRoleExpression* R, const TDLConceptExpression* C )
		: TDLObjectRoleComplexExpression()
		, TObjectRoleArg(R)
		, TConceptArg(C)
		{}
		/// empty d'tor
	virtual ~TDLObjectRoleProjectionFrom ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLObjectRoleProjectionFrom

//------------------------------------------------------------------
///	object role projection from expression
//------------------------------------------------------------------
class TDLObjectRoleProjectionInto
	: public TDLObjectRoleComplexExpression
	, public TObjectRoleArg
	, public TConceptArg
{
public:		// interface
		/// init c'tor
	TDLObjectRoleProjectionInto ( const TDLObjectRoleExpression* R, const TDLConceptExpression* C )
		: TDLObjectRoleComplexExpression()
		, TObjectRoleArg(R)
		, TConceptArg(C)
		{}
		/// empty d'tor
	virtual ~TDLObjectRoleProjectionInto ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLObjectRoleProjectionInto


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
	void accept ( DLExpressionVisitor& visitor ) const = 0;
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
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
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
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLDataRoleBottom

//------------------------------------------------------------------
///	named data role expression
//------------------------------------------------------------------
class TDLDataRoleName: public TDLDataRoleExpression, public TNamedEntity
{
public:		// interface
		/// init c'tor
	TDLDataRoleName ( const std::string& name ) : TDLDataRoleExpression(), TNamedEntity(name) {}
		/// empty d'tor
	virtual ~TDLDataRoleName ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
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
	void accept ( DLExpressionVisitor& visitor ) const = 0;
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
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
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
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLDataBottom

//------------------------------------------------------------------
///	general data type expression
//------------------------------------------------------------------
class TDLDataTypeExpression: public TDLDataExpression
{
public:		// interface
		/// empty c'tor
	TDLDataTypeExpression ( void ) : TDLDataExpression() {}
		/// empty d'tor
	virtual ~TDLDataTypeExpression ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const = 0;
}; // TDLDataTypeExpression

//------------------------------------------------------------------
///	restricted data type expression
//------------------------------------------------------------------
class TDLDataTypeRestriction: public TDLDataTypeExpression, public TDataExpressionArg<TDLDataTypeName>, public TDLNAryExpression<TDLFacetExpression>
{
public:		// interface
		/// init c'tor
	TDLDataTypeRestriction ( const TDLDataTypeName* T )
		: TDLDataTypeExpression()
		, TDataExpressionArg<TDLDataTypeName>(T)
		, TDLNAryExpression<TDLFacetExpression>("facet expression","Datatype restriction")
		{}
		/// empty d'tor
	virtual ~TDLDataTypeRestriction ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLDataTypeRestriction

//------------------------------------------------------------------
///	data value expression
//------------------------------------------------------------------
class TDLDataValue: public TDLDataExpression, public TNamedEntity, public TDataExpressionArg<TDLDataTypeExpression>
{
public:		// interface
		/// fake c'tor (to make TNameSet happy); shouldn't be called
	TDLDataValue ( const std::string& value )
		: TDLDataExpression()
		, TNamedEntity(value)
		, TDataExpressionArg<TDLDataTypeExpression>(NULL)
		{ fpp_unreachable(); }
		/// init c'tor
	TDLDataValue ( const std::string& value, const TDLDataTypeExpression* T )
		: TDLDataExpression()
		, TNamedEntity(value)
		, TDataExpressionArg<TDLDataTypeExpression>(T)
		{}
		/// empty d'tor
	virtual ~TDLDataValue ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLDataValue

//------------------------------------------------------------------
///	data NOT expression
//------------------------------------------------------------------
class TDLDataNot: public TDLDataExpression, public TDataExpressionArg<TDLDataExpression>
{
public:		// interface
		/// init c'tor
	TDLDataNot ( const TDLDataExpression* E )
		: TDLDataExpression()
		, TDataExpressionArg<TDLDataExpression>(E)
		{}
		/// empty d'tor
	virtual ~TDLDataNot ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLDataNot

//------------------------------------------------------------------
///	data AND expression
//------------------------------------------------------------------
class TDLDataAnd: public TDLDataExpression, public TDLNAryExpression<TDLDataExpression>
{
public:		// interface
		/// init c'tor: create AND of expressions from the given array
	TDLDataAnd ( const ExpressionArray& v )
		: TDLDataExpression()
		, TDLNAryExpression<TDLDataExpression>("data expression","data AND")
	{
		add(v);
	}
		/// empty d'tor
	virtual ~TDLDataAnd ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLDataAnd

//------------------------------------------------------------------
///	data OR expression
//------------------------------------------------------------------
class TDLDataOr: public TDLDataExpression, public TDLNAryExpression<TDLDataExpression>
{
public:		// interface
		/// init c'tor: create OR of expressions from the given array
	TDLDataOr ( const ExpressionArray& v )
		: TDLDataExpression()
		, TDLNAryExpression<TDLDataExpression>("data expression","data OR")
	{
		add(v);
	}
		/// empty d'tor
	virtual ~TDLDataOr ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLDataOr

//------------------------------------------------------------------
///	data one-of expression
//------------------------------------------------------------------
class TDLDataOneOf: public TDLDataExpression, public TDLNAryExpression<TDLDataValue>
{
public:		// interface
		/// init c'tor: create one-of from individuals in the given array
	TDLDataOneOf ( const ExpressionArray& v )
		: TDLDataExpression()
		, TDLNAryExpression<TDLDataValue>("data value","data OneOf")
	{
		add(v);
	}
		/// empty d'tor
	virtual ~TDLDataOneOf ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLDataOneOf

//------------------------------------------------------------------
///	general data facet expression
//------------------------------------------------------------------
class TDLFacetExpression: public TDLDataExpression, public TDataExpressionArg<TDLDataValue>
{
public:		// interface
		/// init c'tor: create facet from a given value V
	TDLFacetExpression ( const TDLDataValue* V )
		: TDLDataExpression()
		, TDataExpressionArg<TDLDataValue>(V)
		{}
		/// empty d'tor
	virtual ~TDLFacetExpression ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const = 0;
}; // TDLFacetExpression

//------------------------------------------------------------------
///	min-inclusive facet expression
//------------------------------------------------------------------
class TDLFacetMinInclusive: public TDLFacetExpression
{
public:		// interface
		/// init c'tor
	TDLFacetMinInclusive ( const TDLDataValue* V ) : TDLFacetExpression(V) {}
		/// empty d'tor
	virtual ~TDLFacetMinInclusive ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLFacetMinInclusive

//------------------------------------------------------------------
///	min-exclusive facet expression
//------------------------------------------------------------------
class TDLFacetMinExclusive: public TDLFacetExpression
{
public:		// interface
		/// init c'tor
	TDLFacetMinExclusive ( const TDLDataValue* V ) : TDLFacetExpression(V) {}
		/// empty d'tor
	virtual ~TDLFacetMinExclusive ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLFacetMinExclusive

//------------------------------------------------------------------
///	max-inclusive facet expression
//------------------------------------------------------------------
class TDLFacetMaxInclusive: public TDLFacetExpression
{
public:		// interface
		/// init c'tor
	TDLFacetMaxInclusive ( const TDLDataValue* V ) : TDLFacetExpression(V) {}
		/// empty d'tor
	virtual ~TDLFacetMaxInclusive ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLFacetMaxInclusive

//------------------------------------------------------------------
///	max-exclusive facet expression
//------------------------------------------------------------------
class TDLFacetMaxExclusive: public TDLFacetExpression
{
public:		// interface
		/// init c'tor
	TDLFacetMaxExclusive ( const TDLDataValue* V ) : TDLFacetExpression(V) {}
		/// empty d'tor
	virtual ~TDLFacetMaxExclusive ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLFacetMaxExclusive


// data type is defined here as they are more complex than the rest

//------------------------------------------------------------------
/// data type implementation for the DL expressions
//------------------------------------------------------------------
class TDLDataType
{
protected:	// classes
		/// class to create a new entry with a given data type as a parameter
	class DVCreator: public TNameCreator<TDLDataValue>
	{
	protected:	// members
			/// type for all the values
		const TDLDataTypeExpression* type;
	public:		// interface
			/// init c'tor
		DVCreator ( const TDLDataTypeExpression* t ) : type(t) {}
			/// empty d'tor
		virtual ~DVCreator ( void ) {}
			/// create new value of a given type
		virtual TDLDataValue* makeEntry ( const std::string& name ) const { return new TDLDataValue(name,type); }
	}; // DVCreator

protected:	// members
		/// all the values of the datatype
	TNameSet<TDLDataValue> Values;

public:		// interface
		/// empty c'tor
	TDLDataType ( const TDLDataTypeExpression* type ) : Values(new DVCreator(type)) {}
		/// empty d'tor
	~TDLDataType ( void ) {}

		/// get new data value of the given type
	const TDLDataValue* getValue ( const std::string& name ) { return Values.insert(name); }
}; // TDLDataType


//------------------------------------------------------------------
///	named data type expression
//------------------------------------------------------------------
class TDLDataTypeName: public TDLDataTypeExpression, public TDLDataType, public TNamedEntity
{
public:		// interface
		/// init c'tor
	TDLDataTypeName ( const std::string& name ) : TDLDataTypeExpression(), TDLDataType(this), TNamedEntity(name) {}
		/// empty d'tor
	virtual ~TDLDataTypeName ( void ) {}

		/// accept method for the visitor pattern
	void accept ( DLExpressionVisitor& visitor ) const { visitor.visit(*this); }
}; // TDLDataTypeName


#endif
