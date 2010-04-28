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

#ifndef TEXPRESSIONTRANSLATOR_H
#define TEXPRESSIONTRANSLATOR_H

#include "tDLExpression.h"
#include "tDataTypeManager.h"
#include "dlTBox.h"

class TExpressionTranslator: public DLExpressionVisitor
{
protected:	// members
		/// tree corresponding to a processing expression
	DLTree* tree;
		/// TBox to get access to the named entities
	TBox& KB;

#define THROW_UNSUPPORTED(name) \
	throw EFaCTPlusPlus("Unsupported expression '" name "' in transformation")

public:		// interface
		/// empty c'tor
	TExpressionTranslator ( TBox& kb ) : tree(NULL), KB(kb) {}
		/// empty d'tor
	~TExpressionTranslator ( void ) { deleteTree(tree); }

		/// get (single) access to the tree
	operator DLTree* ( void ) { DLTree* ret = tree; tree = NULL; return ret; }

public:		// visitor interface
	// concept expressions
	virtual void visit ( const TDLConceptTop& expr ATTR_UNUSED ) { tree = new DLTree(TOP); }
	virtual void visit ( const TDLConceptBottom& expr ATTR_UNUSED ) { tree = new DLTree(BOTTOM); }
	virtual void visit ( const TDLConceptName& expr ) { tree = new DLTree(TLexeme(CNAME,KB.getConcept(expr.getName()))); }
	virtual void visit ( const TDLConceptNot& expr ) { expr.getC()->accept(*this); tree = createSNFNot(*this); }
	virtual void visit ( const TDLConceptAnd& expr )
	{
		DLTree* acc = new DLTree(TOP);

		for ( TDLConceptAnd::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			(*p)->accept(*this);
			acc = createSNFAnd ( acc, *this );
		}

		tree = acc;
	}
	virtual void visit ( const TDLConceptOr& expr )
	{
		DLTree* acc = new DLTree(BOTTOM);

		for ( TDLConceptOr::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			(*p)->accept(*this);
			acc = createSNFOr ( acc, *this );
		}

		tree = acc;
	}
	virtual void visit ( const TDLConceptOneOf& expr )
	{
		DLTree* acc = new DLTree(BOTTOM);

		for ( TDLConceptOneOf::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			(*p)->accept(*this);
			acc = createSNFOr ( acc, *this );
		}

		tree = acc;
	}
	virtual void visit ( const TDLConceptObjectSelf& expr ) { expr.getOR()->accept(*this); tree = new DLTree ( REFLEXIVE, *this ); }
	virtual void visit ( const TDLConceptObjectValue& expr )
	{
		expr.getOR()->accept(*this);
		DLTree* R = *this;
		expr.getI()->accept(*this);
		tree = createSNFExists ( R, *this );
	}
	virtual void visit ( const TDLConceptObjectExists& expr )
	{
		expr.getOR()->accept(*this);
		DLTree* R = *this;
		expr.getC()->accept(*this);
		tree = createSNFExists ( R, *this );
	}
	virtual void visit ( const TDLConceptObjectForall& expr )
	{
		expr.getOR()->accept(*this);
		DLTree* R = *this;
		expr.getC()->accept(*this);
		tree = createSNFForall ( R, *this );
	}
	virtual void visit ( const TDLConceptObjectMinCardinality& expr )
	{
		expr.getOR()->accept(*this);
		DLTree* R = *this;
		expr.getC()->accept(*this);
		tree = createSNFGE ( expr.getNumber(), R, *this );
	}
	virtual void visit ( const TDLConceptObjectMaxCardinality& expr )
	{
		expr.getOR()->accept(*this);
		DLTree* R = *this;
		expr.getC()->accept(*this);
		tree = createSNFLE ( expr.getNumber(), R, *this );
	}
	virtual void visit ( const TDLConceptObjectExactCardinality& expr )
	{
		expr.getOR()->accept(*this);
		DLTree* R = *this;
		expr.getC()->accept(*this);
		DLTree* C = *this;
		DLTree* LE = createSNFLE ( expr.getNumber(), clone(R), clone(C) );
		DLTree* GE = createSNFGE ( expr.getNumber(), R, C );
		tree = createSNFAnd ( GE, LE );
	}
	virtual void visit ( const TDLConceptDataValue& expr ATTR_UNUSED ) { THROW_UNSUPPORTED("data value"); }
	virtual void visit ( const TDLConceptDataExists& expr )
	{
		expr.getDR()->accept(*this);
		DLTree* R = *this;
		expr.getExpr()->accept(*this);
		tree = createSNFExists ( R, *this );
	}
	virtual void visit ( const TDLConceptDataForall& expr )
	{
		expr.getDR()->accept(*this);
		DLTree* R = *this;
		expr.getExpr()->accept(*this);
		tree = createSNFForall ( R, *this );
	}
	virtual void visit ( const TDLConceptDataMinCardinality& expr )
	{
		expr.getDR()->accept(*this);
		DLTree* R = *this;
		expr.getExpr()->accept(*this);
		tree = createSNFGE ( expr.getNumber(), R, *this );
	}
	virtual void visit ( const TDLConceptDataMaxCardinality& expr )
	{
		expr.getDR()->accept(*this);
		DLTree* R = *this;
		expr.getExpr()->accept(*this);
		tree = createSNFLE ( expr.getNumber(), R, *this );
	}
	virtual void visit ( const TDLConceptDataExactCardinality& expr )
	{
		expr.getDR()->accept(*this);
		DLTree* R = *this;
		expr.getExpr()->accept(*this);
		DLTree* C = *this;
		DLTree* LE = createSNFLE ( expr.getNumber(), clone(R), clone(C) );
		DLTree* GE = createSNFGE ( expr.getNumber(), R, C );
		tree = createSNFAnd ( GE, LE );
	}

	// individual expressions
	virtual void visit ( const TDLIndividualName& expr ) { tree = new DLTree(TLexeme(INAME,KB.getIndividual(expr.getName()))); }

	// object role expressions
	virtual void visit ( const TDLObjectRoleTop& expr ATTR_UNUSED ) { THROW_UNSUPPORTED("top object role"); }
	virtual void visit ( const TDLObjectRoleBottom& expr ATTR_UNUSED ) { THROW_UNSUPPORTED("bottom object role"); }
	virtual void visit ( const TDLObjectRoleName& expr ) { tree = new DLTree(TLexeme(RNAME,KB.getORM()->ensureRoleName(expr.getName()))); }
	virtual void visit ( const TDLObjectRoleInverse& expr ) { expr.getOR()->accept(*this); tree = createInverse(*this); }
	virtual void visit ( const TDLObjectRoleChain& expr )
	{
		TDLObjectRoleChain::iterator p = expr.begin(), p_end = expr.end();
		if ( p == p_end )
			THROW_UNSUPPORTED("empty role chain");

		(*p)->accept(*this);
		DLTree* acc = *this;

		while ( ++p != p_end )
		{
			(*p)->accept(*this);
			acc = new DLTree ( TLexeme(RCOMPOSITION), acc, *this );
		}

		tree = acc;
	}
	virtual void visit ( const TDLObjectRoleProjectionFrom& expr )
	{
		expr.getOR()->accept(*this);
		DLTree* R = *this;
		expr.getC()->accept(*this);
		DLTree* C = *this;
		tree = new DLTree ( TLexeme(PROJFROM), R, C );
	}
	virtual void visit ( const TDLObjectRoleProjectionInto& expr )
	{
		expr.getOR()->accept(*this);
		DLTree* R = *this;
		expr.getC()->accept(*this);
		DLTree* C = *this;
		tree = new DLTree ( TLexeme(PROJINTO), R, C );
	}

	// data role expressions
	virtual void visit ( const TDLDataRoleTop& expr ATTR_UNUSED ) { THROW_UNSUPPORTED("top data role");  }
	virtual void visit ( const TDLDataRoleBottom& expr ATTR_UNUSED ) { THROW_UNSUPPORTED("bottom data role"); }
	virtual void visit ( const TDLDataRoleName& expr ) { tree = new DLTree(TLexeme(DNAME,KB.getDRM()->ensureRoleName(expr.getName()))); }

	// data expressions
	virtual void visit ( const TDLDataTop& expr ATTR_UNUSED ) { tree = new DLTree(TOP); }
	virtual void visit ( const TDLDataBottom& expr ATTR_UNUSED ) { tree = new DLTree(BOTTOM); }
	virtual void visit ( const TDLDataTypeName& expr )
	{
		DataTypeCenter& DTC = KB.getDataTypeCenter();
		if ( isStrDataType(&expr) )
			tree = DTC.getStringType();
		else if ( isIntDataType(&expr) )
			tree = DTC.getNumberType();
		else if ( isRealDataType(&expr) )
			tree = DTC.getRealType();
		else if ( isBoolDataType(&expr) )
			tree = DTC.getBoolType();	// get-by-name("bool")??
		else
			THROW_UNSUPPORTED("data type name");
	}
	virtual void visit ( const TDLDataTypeRestriction& expr )
	{
		DLTree* acc = new DLTree(TOP);

		for ( TDLDataTypeRestriction::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			(*p)->accept(*this);
			acc = createSNFAnd ( acc, *this );
		}

		tree = acc;
	}
	virtual void visit ( const TDLDataValue& expr )
	{
		expr.getExpr()->accept(*this);	// process type
		DLTree* type = *this;
		tree = KB.getDataTypeCenter().getDataValue(expr.getName(),type);
		deleteTree(type);
	}
	virtual void visit ( const TDLDataNot& expr ) { expr.getExpr()->accept(*this); tree = createSNFNot(*this); }
	virtual void visit ( const TDLDataAnd& expr )
	{
		DLTree* acc = new DLTree(TOP);

		for ( TDLDataAnd::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			(*p)->accept(*this);
			acc = createSNFAnd ( acc, *this );
		}

		tree = acc;
	}
	virtual void visit ( const TDLDataOr& expr )
	{
		DLTree* acc = new DLTree(BOTTOM);

		for ( TDLDataOr::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			(*p)->accept(*this);
			acc = createSNFOr ( acc, *this );
		}

		tree = acc;
	}
	virtual void visit ( const TDLDataOneOf& expr )
	{
		DLTree* acc = new DLTree(BOTTOM);

		for ( TDLDataOneOf::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			(*p)->accept(*this);
			acc = createSNFOr ( acc, *this );
		}

		tree = acc;
	}

#undef THROW_UNSUPPORTED
}; // TExpressionTranslator

#endif
