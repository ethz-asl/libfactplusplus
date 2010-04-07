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

#ifndef TEXPRESSIONPRINTERLISP_H
#define TEXPRESSIONPRINTERLISP_H

#include <iostream>

#include "tDLExpression.h"

class TLISPExpressionPrinter: public DLExpressionVisitor
{
protected:	// members
		/// main stream
	std::ostream& o;
		/// helper class for brackets
	class BR
	{
	protected:
		std::ostream& o;
	public:
		BR ( std::ostream& o_, const char* command ) : o(o_) { o << " (" << command; }
		~BR () { o << ")"; }
	}; // BR
protected:	// methods
		/// array helper
	template <class Argument>
	void printArray ( TDLNAryExpression<Argument> expr )
	{
		for ( typename TDLNAryExpression<Argument>::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			(*p)->accept(*this);
	}

#define THROW_UNSUPPORTED(name) \
	throw EFaCTPlusPlus("Unsupported expression '" name "' in LISP printer")

public:		// interface
		/// init c'tor
	TLISPExpressionPrinter ( std::ostream& o_ ) : o(o_) {}
		/// empty d'tor
	virtual ~TLISPExpressionPrinter ( void ) {}

public:		// visitor interface
	// concept expressions
	virtual void visit ( TDLConceptTop& expr ATTR_UNUSED ) { o << " *TOP*"; }
	virtual void visit ( TDLConceptBottom& expr ATTR_UNUSED ) { o << " *BOTTOM*"; }
	virtual void visit ( TDLConceptName& expr ) { o << " " << expr.getName(); }
	virtual void visit ( TDLConceptNot& expr ) { BR b(o,"not"); expr.getC()->accept(*this); }
	virtual void visit ( TDLConceptAnd& expr ) { BR b(o,"and"); printArray(expr); }
	virtual void visit ( TDLConceptOr& expr ) { BR b(o,"or"); printArray(expr); }
	virtual void visit ( TDLConceptOneOf& expr ) { BR b(o,"one-of"); printArray(expr); }
	virtual void visit ( TDLConceptObjectSelf& expr ) { BR b(o,"self-ref"); expr.getOR()->accept(*this); }
	virtual void visit ( TDLConceptObjectValue& expr ) { BR b(o,"some"); expr.getOR()->accept(*this); BR i(o,"one-of"); expr.getI()->accept(*this); }
	virtual void visit ( TDLConceptObjectExists& expr ) { BR b(o,"some"); expr.getOR()->accept(*this); expr.getC()->accept(*this); }
	virtual void visit ( TDLConceptObjectForall& expr ) { BR b(o,"all"); expr.getOR()->accept(*this); expr.getC()->accept(*this); }
	virtual void visit ( TDLConceptObjectMinCardinality& expr )
		{ BR b(o,"atleast"); o << " " << expr.getNumber(); expr.getOR()->accept(*this); expr.getC()->accept(*this); }
	virtual void visit ( TDLConceptObjectMaxCardinality& expr )
		{ BR b(o,"atmost"); o << " " << expr.getNumber(); expr.getOR()->accept(*this); expr.getC()->accept(*this); }
	virtual void visit ( TDLConceptObjectExactCardinality& expr )
	{
		BR a(o,"and");
		{ BR b(o,"atleast"); o << " " << expr.getNumber(); expr.getOR()->accept(*this); expr.getC()->accept(*this); }
		{ BR b(o,"atmost"); o << " " << expr.getNumber(); expr.getOR()->accept(*this); expr.getC()->accept(*this); }
	}
	virtual void visit ( TDLConceptDataValue& expr ) { BR b(o,"some"); expr.getDR()->accept(*this); expr.getExpr()->accept(*this); }
	virtual void visit ( TDLConceptDataExists& expr ) { BR b(o,"some"); expr.getDR()->accept(*this); expr.getExpr()->accept(*this); }
	virtual void visit ( TDLConceptDataForall& expr ) { BR b(o,"all"); expr.getDR()->accept(*this); expr.getExpr()->accept(*this); }
	virtual void visit ( TDLConceptDataMinCardinality& expr )
		{ BR b(o,"atleast"); o << " " << expr.getNumber(); expr.getDR()->accept(*this); expr.getExpr()->accept(*this); }
	virtual void visit ( TDLConceptDataMaxCardinality& expr )
		{ BR b(o,"atmost"); o << " " << expr.getNumber(); expr.getDR()->accept(*this); expr.getExpr()->accept(*this); }
	virtual void visit ( TDLConceptDataExactCardinality& expr )
	{
		BR a(o,"and");
		{ BR b(o,"atleast"); o << " " << expr.getNumber(); expr.getDR()->accept(*this); expr.getExpr()->accept(*this); }
		{ BR b(o,"atmost"); o << " " << expr.getNumber(); expr.getDR()->accept(*this); expr.getExpr()->accept(*this); }
	}

	// individual expressions
	virtual void visit ( TDLIndividualName& expr ) { o << " " << expr.getName(); }

	// object role expressions
	virtual void visit ( TDLObjectRoleTop& expr ATTR_UNUSED ) { THROW_UNSUPPORTED("top object role"); }
	virtual void visit ( TDLObjectRoleBottom& expr ATTR_UNUSED ) { THROW_UNSUPPORTED("bottom object role"); }
	virtual void visit ( TDLObjectRoleName& expr ) { o << " " << expr.getName(); }
	virtual void visit ( TDLObjectRoleInverse& expr ) { BR b(o,"inv"); expr.getOR()->accept(*this); }
	virtual void visit ( TDLObjectRoleChain& expr ) { BR b(o,"compose"); printArray(expr); }
	virtual void visit ( TDLObjectRoleProjectionFrom& expr )
		{ BR b(o,"project_from"); expr.getOR()->accept(*this); expr.getC()->accept(*this); }
	virtual void visit ( TDLObjectRoleProjectionInto& expr )
		{ BR b(o,"project_into"); expr.getOR()->accept(*this); expr.getC()->accept(*this); }

	// data role expressions
	virtual void visit ( TDLDataRoleTop& expr ATTR_UNUSED ) { THROW_UNSUPPORTED("top data role");  }
	virtual void visit ( TDLDataRoleBottom& expr ATTR_UNUSED ) { THROW_UNSUPPORTED("bottom data role"); }
	virtual void visit ( TDLDataRoleName& expr ) { o << " " << expr.getName(); }

	// data expressions
	virtual void visit ( TDLDataTop& expr ATTR_UNUSED ) { o << " *TOP*"; }
	virtual void visit ( TDLDataBottom& expr ATTR_UNUSED ) { o << " *BOTTOM*"; }
	virtual void visit ( TDLDataTypeName& expr ) { o << " (" << expr.getName() << ")"; }
	virtual void visit ( TDLDataValue& expr ) { o << " (" << expr.getExpr()->getName() << " " << expr.getName() << ")"; }

#undef THROW_UNSUPPORTED
}; // TLISPExpressionPrinter

#endif
