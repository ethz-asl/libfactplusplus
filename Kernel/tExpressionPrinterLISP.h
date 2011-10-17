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

#ifndef TEXPRESSIONPRINTERLISP_H
#define TEXPRESSIONPRINTERLISP_H

#include <iostream>

#include "tDLExpression.h"

class TLISPExpressionPrinter: public DLExpressionVisitor
{
protected:	// members
		/// main stream
	std::ostream& o;
		/// define str-str map
	typedef std::map<std::string, std::string> SSMap;
		/// map between OWL datatype names and FaCT++ ones
	SSMap DTNames;
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
	void printArray ( const TDLNAryExpression<Argument>& expr )
	{
		for ( typename TDLNAryExpression<Argument>::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			(*p)->accept(*this);
	}
		/// datatype helper to get a LISP datatype name by a OWL one
	const char* getDTName ( const char* owlName ) const
	{
		SSMap::const_iterator p = DTNames.find(owlName);
		if ( p != DTNames.end() )	// known name
			return p->second.c_str();
		return owlName;
	}

#define THROW_UNSUPPORTED(name) \
	throw EFaCTPlusPlus("Unsupported expression '" name "' in LISP printer")

public:		// interface
		/// init c'tor
	TLISPExpressionPrinter ( std::ostream& o_ ) : o(o_)
	{
		DTNames["http://www.w3.org/1999/02/22-rdf-syntax-ns#PlainLiteral"] = "string";
		DTNames["http://www.w3.org/2001/XMLSchema#string"] = "string";
		DTNames["http://www.w3.org/2001/XMLSchema#anyURI"] = "string";

		DTNames["http://www.w3.org/2001/XMLSchema#integer"] = "number";
		DTNames["http://www.w3.org/2001/XMLSchema#int"] = "number";

		DTNames["http://www.w3.org/2001/XMLSchema#float"] = "real";
		DTNames["http://www.w3.org/2001/XMLSchema#double"] = "real";
		DTNames["http://www.w3.org/2001/XMLSchema#real"] = "real";
	}
		/// empty d'tor
	virtual ~TLISPExpressionPrinter ( void ) {}

public:		// visitor interface
	// concept expressions
	virtual void visit ( const TDLConceptTop& expr ATTR_UNUSED ) { o << " *TOP*"; }
	virtual void visit ( const TDLConceptBottom& expr ATTR_UNUSED ) { o << " *BOTTOM*"; }
	virtual void visit ( const TDLConceptName& expr ) { o << " " << expr.getName(); }
	virtual void visit ( const TDLConceptNot& expr ) { BR b(o,"not"); expr.getC()->accept(*this); }
	virtual void visit ( const TDLConceptAnd& expr ) { BR b(o,"and"); printArray(expr); }
	virtual void visit ( const TDLConceptOr& expr ) { BR b(o,"or"); printArray(expr); }
	virtual void visit ( const TDLConceptOneOf& expr ) { BR b(o,"one-of"); printArray(expr); }
	virtual void visit ( const TDLConceptObjectSelf& expr ) { BR b(o,"self-ref"); expr.getOR()->accept(*this); }
	virtual void visit ( const TDLConceptObjectValue& expr ) { BR b(o,"some"); expr.getOR()->accept(*this); BR i(o,"one-of"); expr.getI()->accept(*this); }
	virtual void visit ( const TDLConceptObjectExists& expr ) { BR b(o,"some"); expr.getOR()->accept(*this); expr.getC()->accept(*this); }
	virtual void visit ( const TDLConceptObjectForall& expr ) { BR b(o,"all"); expr.getOR()->accept(*this); expr.getC()->accept(*this); }
	virtual void visit ( const TDLConceptObjectMinCardinality& expr )
		{ BR b(o,"atleast"); o << " " << expr.getNumber(); expr.getOR()->accept(*this); expr.getC()->accept(*this); }
	virtual void visit ( const TDLConceptObjectMaxCardinality& expr )
		{ BR b(o,"atmost"); o << " " << expr.getNumber(); expr.getOR()->accept(*this); expr.getC()->accept(*this); }
	virtual void visit ( const TDLConceptObjectExactCardinality& expr )
	{
		BR a(o,"and");
		{ BR b(o,"atleast"); o << " " << expr.getNumber(); expr.getOR()->accept(*this); expr.getC()->accept(*this); }
		{ BR b(o,"atmost"); o << " " << expr.getNumber(); expr.getOR()->accept(*this); expr.getC()->accept(*this); }
	}
	virtual void visit ( const TDLConceptDataValue& expr ) { BR b(o,"some"); expr.getDR()->accept(*this); expr.getExpr()->accept(*this); }
	virtual void visit ( const TDLConceptDataExists& expr ) { BR b(o,"some"); expr.getDR()->accept(*this); expr.getExpr()->accept(*this); }
	virtual void visit ( const TDLConceptDataForall& expr ) { BR b(o,"all"); expr.getDR()->accept(*this); expr.getExpr()->accept(*this); }
	virtual void visit ( const TDLConceptDataMinCardinality& expr )
		{ BR b(o,"atleast"); o << " " << expr.getNumber(); expr.getDR()->accept(*this); expr.getExpr()->accept(*this); }
	virtual void visit ( const TDLConceptDataMaxCardinality& expr )
		{ BR b(o,"atmost"); o << " " << expr.getNumber(); expr.getDR()->accept(*this); expr.getExpr()->accept(*this); }
	virtual void visit ( const TDLConceptDataExactCardinality& expr )
	{
		BR a(o,"and");
		{ BR b(o,"atleast"); o << " " << expr.getNumber(); expr.getDR()->accept(*this); expr.getExpr()->accept(*this); }
		{ BR b(o,"atmost"); o << " " << expr.getNumber(); expr.getDR()->accept(*this); expr.getExpr()->accept(*this); }
	}

	// individual expressions
	virtual void visit ( const TDLIndividualName& expr ) { o << " " << expr.getName(); }

	// object role expressions
	virtual void visit ( const TDLObjectRoleTop& expr ATTR_UNUSED ) { THROW_UNSUPPORTED("top object role"); }
	virtual void visit ( const TDLObjectRoleBottom& expr ATTR_UNUSED ) { THROW_UNSUPPORTED("bottom object role"); }
	virtual void visit ( const TDLObjectRoleName& expr ) { o << " " << expr.getName(); }
	virtual void visit ( const TDLObjectRoleInverse& expr ) { BR b(o,"inv"); expr.getOR()->accept(*this); }
	virtual void visit ( const TDLObjectRoleChain& expr ) { BR b(o,"compose"); printArray(expr); }
	virtual void visit ( const TDLObjectRoleProjectionFrom& expr )
		{ BR b(o,"project_from"); expr.getOR()->accept(*this); expr.getC()->accept(*this); }
	virtual void visit ( const TDLObjectRoleProjectionInto& expr )
		{ BR b(o,"project_into"); expr.getOR()->accept(*this); expr.getC()->accept(*this); }

	// data role expressions
	virtual void visit ( const TDLDataRoleTop& expr ATTR_UNUSED ) { THROW_UNSUPPORTED("top data role");  }
	virtual void visit ( const TDLDataRoleBottom& expr ATTR_UNUSED ) { THROW_UNSUPPORTED("bottom data role"); }
	virtual void visit ( const TDLDataRoleName& expr ) { o << " " << expr.getName(); }

	// data expressions
	virtual void visit ( const TDLDataTop& expr ATTR_UNUSED ) { o << " *TOP*"; }
	virtual void visit ( const TDLDataBottom& expr ATTR_UNUSED ) { o << " *BOTTOM*"; }
	virtual void visit ( const TDLDataTypeName& expr ) { o << " (" << expr.getName() << ")"; }
		// no need to use a type of a restriction here, as all contains in constants
	virtual void visit ( const TDLDataTypeRestriction& expr ) { BR b(o,"and"); printArray(expr); }
	virtual void visit ( const TDLDataValue& expr )
		{ o << " (" << getDTName(getBasicDataType(const_cast<TDLDataTypeExpression*>(expr.getExpr()))->getName()) << " " << expr.getName() << ")"; }
	virtual void visit ( const TDLDataNot& expr ) { BR b(o,"not"); expr.getExpr()->accept(*this); }
	virtual void visit ( const TDLDataAnd& expr ) { BR b(o,"and"); printArray(expr); }
	virtual void visit ( const TDLDataOr& expr ) { BR b(o,"or"); printArray(expr); }
	virtual void visit ( const TDLDataOneOf& expr ) { BR b(o,"d-one-of"); printArray(expr); }

	// facets
	virtual void visit ( const TDLFacetMinInclusive& expr ) { BR b(o,"ge"); expr.getExpr()->accept(*this); }
	virtual void visit ( const TDLFacetMinExclusive& expr ) { BR b(o,"gt"); expr.getExpr()->accept(*this); }
	virtual void visit ( const TDLFacetMaxInclusive& expr ) { BR b(o,"le"); expr.getExpr()->accept(*this); }
	virtual void visit ( const TDLFacetMaxExclusive& expr ) { BR b(o,"lt"); expr.getExpr()->accept(*this); }

#undef THROW_UNSUPPORTED
}; // TLISPExpressionPrinter

#endif
