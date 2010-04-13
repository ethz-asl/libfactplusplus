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
#include "dlTBox.h"

class TExpressionTranslator: public DLExpressionVisitor
{
protected:	// members
		/// tree corresponding to a processing expression
	DLTree* tree;

	TNameSet<TTreeNamedEntry>	// for now
		NS_C,
		NS_I,
		NS_OR,
		NS_DR;

#define THROW_UNSUPPORTED(name) \
	throw EFaCTPlusPlus("Unsupported expression '" name "' in transformation")

public:		// interface
		/// empty c'tor
	TExpressionTranslator ( void ) : tree(NULL) {}
		/// empty d'tor
	~TExpressionTranslator ( void ) { deleteTree(tree); }

		/// get (single) access to the tree
	operator DLTree* ( void ) { DLTree* ret = tree; tree = NULL; return ret; }

public:		// visitor interface
	// concept expressions
	virtual void visit ( TDLConceptTop& expr ATTR_UNUSED ) { tree = new DLTree(TOP); }
	virtual void visit ( TDLConceptBottom& expr ATTR_UNUSED ) { tree = new DLTree(BOTTOM); }
	virtual void visit ( TDLConceptName& expr ) { tree = new DLTree(TLexeme(CNAME,NS_C.insert(expr.getName()))); }
	virtual void visit ( TDLConceptNot& expr ) { expr.getC()->accept(*this); tree = createSNFNot(*this); }
	virtual void visit ( TDLConceptAnd& expr )
	{
		DLTree* acc = new DLTree(TOP);

		for ( TDLConceptAnd::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			(*p)->accept(*this);
			acc = createSNFAnd ( acc, *this );
		}

		tree = acc;
	}
	virtual void visit ( TDLConceptOr& expr )
	{
		DLTree* acc = new DLTree(BOTTOM);

		for ( TDLConceptOr::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			(*p)->accept(*this);
			acc = createSNFOr ( acc, *this );
		}

		tree = acc;
	}
	virtual void visit ( TDLConceptOneOf& expr )
	{
		DLTree* acc = new DLTree(BOTTOM);

		for ( TDLConceptOneOf::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			(*p)->accept(*this);
			acc = createSNFOr ( acc, *this );
		}

		tree = acc;
	}
	virtual void visit ( TDLConceptObjectSelf& expr ) { expr.getOR()->accept(*this); tree = new DLTree ( REFLEXIVE, *this ); }
	virtual void visit ( TDLConceptObjectValue& expr )
	{
		expr.getOR()->accept(*this);
		DLTree* R = *this;
		expr.getI()->accept(*this);
		tree = createSNFExists ( R, *this );
	}
	virtual void visit ( TDLConceptObjectExists& expr )
	{
		expr.getOR()->accept(*this);
		DLTree* R = *this;
		expr.getC()->accept(*this);
		tree = createSNFExists ( R, *this );
	}
	virtual void visit ( TDLConceptObjectForall& expr )
	{
		expr.getOR()->accept(*this);
		DLTree* R = *this;
		expr.getC()->accept(*this);
		tree = createSNFForall ( R, *this );
	}
	virtual void visit ( TDLConceptObjectMinCardinality& expr )
	{
		expr.getOR()->accept(*this);
		DLTree* R = *this;
		expr.getC()->accept(*this);
		tree = createSNFGE ( expr.getNumber(), R, *this );
	}
	virtual void visit ( TDLConceptObjectMaxCardinality& expr )
	{
		expr.getOR()->accept(*this);
		DLTree* R = *this;
		expr.getC()->accept(*this);
		tree = createSNFLE ( expr.getNumber(), R, *this );
	}
	virtual void visit ( TDLConceptObjectExactCardinality& expr )
	{
		expr.getOR()->accept(*this);
		DLTree* R = *this;
		expr.getC()->accept(*this);
		DLTree* C = *this;
		DLTree* LE = createSNFLE ( expr.getNumber(), clone(R), clone(C) );
		DLTree* GE = createSNFGE ( expr.getNumber(), R, C );
		tree = createSNFAnd ( GE, LE );
	}
	virtual void visit ( TDLConceptDataValue& expr ATTR_UNUSED ) { THROW_UNSUPPORTED("data value"); }
	virtual void visit ( TDLConceptDataExists& expr )
	{
		expr.getDR()->accept(*this);
		DLTree* R = *this;
		expr.getExpr()->accept(*this);
		tree = createSNFExists ( R, *this );
	}
	virtual void visit ( TDLConceptDataForall& expr )
	{
		expr.getDR()->accept(*this);
		DLTree* R = *this;
		expr.getExpr()->accept(*this);
		tree = createSNFForall ( R, *this );
	}
	virtual void visit ( TDLConceptDataMinCardinality& expr )
	{
		expr.getDR()->accept(*this);
		DLTree* R = *this;
		expr.getExpr()->accept(*this);
		tree = createSNFGE ( expr.getNumber(), R, *this );
	}
	virtual void visit ( TDLConceptDataMaxCardinality& expr )
	{
		expr.getDR()->accept(*this);
		DLTree* R = *this;
		expr.getExpr()->accept(*this);
		tree = createSNFLE ( expr.getNumber(), R, *this );
	}
	virtual void visit ( TDLConceptDataExactCardinality& expr )
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
	virtual void visit ( TDLIndividualName& expr ) { tree = new DLTree(TLexeme(INAME,NS_I.insert(expr.getName()))); }

	// object role expressions
	virtual void visit ( TDLObjectRoleTop& expr ATTR_UNUSED ) { THROW_UNSUPPORTED("top object role"); }
	virtual void visit ( TDLObjectRoleBottom& expr ATTR_UNUSED ) { THROW_UNSUPPORTED("bottom object role"); }
	virtual void visit ( TDLObjectRoleName& expr ) { tree = new DLTree(TLexeme(RNAME,NS_OR.insert(expr.getName()))); }
	virtual void visit ( TDLObjectRoleInverse& expr ) { expr.getOR()->accept(*this); tree = createInverse(*this); }
	virtual void visit ( TDLObjectRoleChain& expr )
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
	virtual void visit ( TDLObjectRoleProjectionFrom& expr )
	{
		expr.getOR()->accept(*this);
		DLTree* R = *this;
		expr.getC()->accept(*this);
		DLTree* C = *this;
		tree = new DLTree ( TLexeme(PROJFROM), R, C );
	}
	virtual void visit ( TDLObjectRoleProjectionInto& expr )
	{
		expr.getOR()->accept(*this);
		DLTree* R = *this;
		expr.getC()->accept(*this);
		DLTree* C = *this;
		tree = new DLTree ( TLexeme(PROJINTO), R, C );
	}

	// data role expressions
	virtual void visit ( TDLDataRoleTop& expr ATTR_UNUSED ) { THROW_UNSUPPORTED("top data role");  }
	virtual void visit ( TDLDataRoleBottom& expr ATTR_UNUSED ) { THROW_UNSUPPORTED("bottom data role"); }
	virtual void visit ( TDLDataRoleName& expr ) { tree = new DLTree(TLexeme(DNAME,NS_DR.insert(expr.getName()))); }

	// data expressions
	virtual void visit ( TDLDataTop& expr ATTR_UNUSED ) { tree = new DLTree(TOP); }
	virtual void visit ( TDLDataBottom& expr ATTR_UNUSED ) { tree = new DLTree(BOTTOM); }
	virtual void visit ( TDLDataTypeName& expr ) {}
	virtual void visit ( TDLDataValue& expr ) {}

#undef THROW_UNSUPPORTED
}; // TExpressionTranslator

#endif
