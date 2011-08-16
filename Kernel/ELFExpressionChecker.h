/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2011 by Dmitry Tsarkov

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

#ifndef ELFEXPRESSIONCHECKER_H
#define ELFEXPRESSIONCHECKER_H

#include "tDLExpression.h"

class ELFExpressionChecker: public DLExpressionVisitor
{
protected:	// members
	bool value;

public:
		/// get DLTree corresponding to an expression EXPR
	bool v ( const TDLExpression* expr )
	{
		expr->accept(*this);
		return value;
	}
	// concept expressions
	virtual void visit ( const TDLConceptTop& expr ATTR_UNUSED ) { value = true; }
	virtual void visit ( const TDLConceptBottom& expr ATTR_UNUSED ) { value = true; }
	virtual void visit ( const TDLConceptName& expr ATTR_UNUSED ) { value = true; }
	virtual void visit ( const TDLConceptNot& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLConceptAnd& expr )
	{
		value = false;
		for ( TDLConceptAnd::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( !v(*p) )
				return;
		value = true;
	}
	virtual void visit ( const TDLConceptOr& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLConceptOneOf& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLConceptObjectSelf& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLConceptObjectValue& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLConceptObjectExists& expr )
	{
		value = false;
		// check role
		if ( !v(expr.getOR()) )
			return;
		// check concept
		v(expr.getC());
	}
	virtual void visit ( const TDLConceptObjectForall& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLConceptObjectMinCardinality& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLConceptObjectMaxCardinality& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLConceptObjectExactCardinality& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLConceptDataValue& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLConceptDataExists& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLConceptDataForall& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLConceptDataMinCardinality& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLConceptDataMaxCardinality& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLConceptDataExactCardinality& expr ATTR_UNUSED ) { value = false; }

	// individual expressions
	virtual void visit ( const TDLIndividualName& expr ATTR_UNUSED ) { value = false; }

	// object role expressions
	virtual void visit ( const TDLObjectRoleTop& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLObjectRoleBottom& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLObjectRoleName& expr ATTR_UNUSED ) { value = true; }
	virtual void visit ( const TDLObjectRoleInverse& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLObjectRoleChain& expr )
	{
		value = false;
		for ( TDLObjectRoleChain::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( !v(*p) )
				return;
		value = true;
	}
	virtual void visit ( const TDLObjectRoleProjectionFrom& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLObjectRoleProjectionInto& expr ATTR_UNUSED ) { value = false; }

	// data role expressions
	virtual void visit ( const TDLDataRoleTop& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLDataRoleBottom& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLDataRoleName& expr ATTR_UNUSED ) { value = false; }

	// data expressions
	virtual void visit ( const TDLDataTop& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLDataBottom& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLDataTypeName& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLDataTypeRestriction& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLDataValue& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLDataNot& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLDataAnd& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLDataOr& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLDataOneOf& expr ATTR_UNUSED ) { value = false; }

	// facets
	virtual void visit ( const TDLFacetMinInclusive& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLFacetMinExclusive& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLFacetMaxInclusive& expr ATTR_UNUSED ) { value = false; }
	virtual void visit ( const TDLFacetMaxExclusive& expr ATTR_UNUSED ) { value = false; }

	// other methods
	virtual ~ELFExpressionChecker ( void ) {}
		/// get (single) access to the tree
	operator bool ( void ) const { return value; }
};

#endif
