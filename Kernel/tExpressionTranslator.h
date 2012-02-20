/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2010-2012 by Dmitry Tsarkov

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
		/// signature of non-trivial entities; used in semantic locality checkers only
	TSignature* sig;

#define THROW_UNSUPPORTED(name) \
	throw EFaCTPlusPlus("Unsupported expression '" name "' in transformation")

protected:	// methods
		/// create DLTree of given TAG and named ENTRY; set the entry's ENTITY if necessary
	void setEntry ( Token tag, TNamedEntry* entry, const TNamedEntity* entity )
	{
		if ( unlikely ( entry->getEntity() == NULL ) )
			entry->setEntity(entity);
		tree = new DLTree(TLexeme(tag,entry));
	}
		/// @return true iff ENTRY is not in signature
	bool nc ( const TNamedEntity* entity ) const { return unlikely(sig != NULL) && !sig->contains(entity); }

public:		// interface
		/// empty c'tor
	TExpressionTranslator ( TBox& kb ) : tree(NULL), KB(kb), sig(NULL) {}
		/// empty d'tor
	virtual ~TExpressionTranslator ( void ) { deleteTree(tree); }

		/// get (single) access to the tree
	operator DLTree* ( void ) { DLTree* ret = tree; tree = NULL; return ret; }
		/// set internal signature to a given signature S
	void setSignature ( TSignature* s ) { sig = s; }

public:		// visitor interface
	// concept expressions
	virtual void visit ( const TDLConceptTop& expr ATTR_UNUSED ) { tree = createTop(); }
	virtual void visit ( const TDLConceptBottom& expr ATTR_UNUSED ) { tree = createBottom(); }
	virtual void visit ( const TDLConceptName& expr )
	{
		if ( nc(&expr) )
			tree = sig->topCLocal() ? createTop() : createBottom();
		else
			setEntry ( CNAME, KB.getConcept(expr.getName()), &expr );
	}
	virtual void visit ( const TDLConceptNot& expr ) { expr.getC()->accept(*this); tree = createSNFNot(*this); }
	virtual void visit ( const TDLConceptAnd& expr )
	{
		DLTree* acc = createTop();

		for ( TDLConceptAnd::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			(*p)->accept(*this);
			acc = createSNFAnd ( acc, *this );
		}

		tree = acc;
	}
	virtual void visit ( const TDLConceptOr& expr )
	{
		DLTree* acc = createBottom();

		for ( TDLConceptOr::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			(*p)->accept(*this);
			acc = createSNFOr ( acc, *this );
		}

		tree = acc;
	}
	virtual void visit ( const TDLConceptOneOf& expr )
	{
		DLTree* acc = createBottom();

		for ( TDLConceptOneOf::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			(*p)->accept(*this);
			acc = createSNFOr ( acc, *this );
		}

		tree = acc;
	}
	virtual void visit ( const TDLConceptObjectSelf& expr )
	{
		expr.getOR()->accept(*this);
		DLTree* R = *this;
		tree = R->Element().getNE()->isBottom() ? createBottom() : new DLTree ( TLexeme(SELF), R );
	}
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
	virtual void visit ( const TDLConceptDataValue& expr )
	{
		expr.getDR()->accept(*this);
		DLTree* R = *this;
		expr.getExpr()->accept(*this);
		tree = createSNFExists ( R, *this );
	}
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
	virtual void visit ( const TDLIndividualName& expr ) { setEntry ( INAME, KB.getIndividual(expr.getName()), &expr ); }

	// object role expressions
	virtual void visit ( const TDLObjectRoleTop& expr ATTR_UNUSED ) { THROW_UNSUPPORTED("top object role"); }
	virtual void visit ( const TDLObjectRoleBottom& expr ATTR_UNUSED ) { THROW_UNSUPPORTED("bottom object role"); }
	virtual void visit ( const TDLObjectRoleName& expr )
	{
		RoleMaster* RM = KB.getORM();
		TNamedEntry* role;
		if ( nc(&expr) )
			role = sig->topRLocal() ? RM->getTopRole() : RM->getBotRole();
		else
			role = RM->ensureRoleName(expr.getName());
		setEntry ( RNAME, role, &expr );
	}
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
	virtual void visit ( const TDLDataRoleName& expr )
	{
		RoleMaster* RM = KB.getDRM();
		TNamedEntry* role;
		if ( nc(&expr) )
			role = sig->topRLocal() ? RM->getTopRole() : RM->getBotRole();
		else
			role = RM->ensureRoleName(expr.getName());
		setEntry ( DNAME, role, &expr );
	}

	// data expressions
	virtual void visit ( const TDLDataTop& expr ATTR_UNUSED ) { tree = createTop(); }
	virtual void visit ( const TDLDataBottom& expr ATTR_UNUSED ) { tree = createBottom(); }
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
		else if ( isTimeDataType(&expr) )
			tree = DTC.getTimeType();
		else
			THROW_UNSUPPORTED("data type name");
	}
	virtual void visit ( const TDLDataTypeRestriction& expr )
	{
		DLTree* acc = createTop();

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
		DLTree* acc = createTop();

		for ( TDLDataAnd::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			(*p)->accept(*this);
			acc = createSNFAnd ( acc, *this );
		}

		tree = acc;
	}
	virtual void visit ( const TDLDataOr& expr )
	{
		DLTree* acc = createBottom();

		for ( TDLDataOr::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			(*p)->accept(*this);
			acc = createSNFOr ( acc, *this );
		}

		tree = acc;
	}
	virtual void visit ( const TDLDataOneOf& expr )
	{
		DLTree* acc = createBottom();

		for ( TDLDataOneOf::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			(*p)->accept(*this);
			acc = createSNFOr ( acc, *this );
		}

		tree = acc;
	}

	// facets
	virtual void visit ( const TDLFacetMinInclusive& expr )
	{
		expr.getExpr()->accept(*this);
		tree = KB.getDataTypeCenter().getIntervalFacetExpr ( tree, /*min=*/true, /*excl=*/false );
	}
	virtual void visit ( const TDLFacetMinExclusive& expr )
	{
		expr.getExpr()->accept(*this);
		tree = KB.getDataTypeCenter().getIntervalFacetExpr ( tree, /*min=*/true, /*excl=*/true );
	}
	virtual void visit ( const TDLFacetMaxInclusive& expr )
	{
		expr.getExpr()->accept(*this);
		tree = KB.getDataTypeCenter().getIntervalFacetExpr ( tree, /*min=*/false, /*excl=*/false );
	}
	virtual void visit ( const TDLFacetMaxExclusive& expr )
	{
		expr.getExpr()->accept(*this);
		tree = KB.getDataTypeCenter().getIntervalFacetExpr ( tree, /*min=*/false, /*excl=*/true );
	}

#undef THROW_UNSUPPORTED
}; // TExpressionTranslator

#endif
