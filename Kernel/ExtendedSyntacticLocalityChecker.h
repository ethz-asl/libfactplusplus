/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2013 by Dmitry Tsarkov

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

#ifndef EXTENDEDSYNLOCCHECKER_H
#define EXTENDEDSYNLOCCHECKER_H

#include "GeneralSyntacticLocalityChecker.h"

// forward declarations
class UpperBoundDirectEvaluator;
class LowerBoundDirectEvaluator;
class UpperBoundComplementEvaluator;
class LowerBoundComplementEvaluator;

class CardinalityEvaluatorBase: protected SigAccessor, public DLExpressionVisitorEmpty
{
protected:	// members
	UpperBoundDirectEvaluator* UBD;
	LowerBoundDirectEvaluator* LBD;
	UpperBoundComplementEvaluator* UBC;
	LowerBoundComplementEvaluator* LBC;

		/// keep the value here
	int value;

protected:	// methods to
		/// main method to use
	int getValue ( const TDLExpression& expr )
	{
		expr.accept(*this);
		return value;
	}

		/// implementation of evaluation
	int getUpperBoundDirect ( const TDLExpression& expr );
		/// implementation of evaluation
	int getUpperBoundComplement ( const TDLExpression& expr );
		/// implementation of evaluation
	int getLowerBoundDirect ( const TDLExpression& expr );
		/// implementation of evaluation
	int getLowerBoundComplement ( const TDLExpression& expr );

protected:	// visitor helpers
	bool isBotEquivalent ( const TDLExpression* expr ) { return getUpperBoundDirect(*expr) == 0; }
	bool isTopEquivalent ( const TDLExpression* expr ) { return getUpperBoundComplement(*expr) == 0; }

		/// helper for entities
	virtual int getEntityValue ( const TNamedEntity* entity ) = 0;
		/// helper for All
	virtual int getForallValue ( const TDLRoleExpression* R, const TDLExpression* C ) = 0;
		/// helper for things like >= m R.C
	virtual int getMinValue ( int m, const TDLRoleExpression* R, const TDLExpression* C ) = 0;
		/// helper for things like <= m R.C
	virtual int getMaxValue ( int m, const TDLRoleExpression* R, const TDLExpression* C ) = 0;
		/// helper for things like = m R.C
	virtual int getExactValue ( int m, const TDLRoleExpression* R, const TDLExpression* C ) = 0;

public:		// interface
		/// init c'tor
	CardinalityEvaluatorBase ( const TSignature* s ) : SigAccessor(s), value(0) {}
		/// empty d'tor
	virtual ~CardinalityEvaluatorBase ( void ) {}

		/// set all other evaluators
	void setEvaluators ( UpperBoundDirectEvaluator* pUD, LowerBoundDirectEvaluator* pLD, UpperBoundComplementEvaluator* pUC, LowerBoundComplementEvaluator* pLC )
	{
		UBD = pUD;
		LBD = pLD;
		UBC = pUC;
		LBC = pLC;
		fpp_assert ( (void*)UBD == this || (void*)LBD == this || (void*)UBC == this || (void*)LBC == this );
	}

		/// implementation of evaluation
	int getUpperBoundDirect ( const TDLExpression* expr ) { return getUpperBoundDirect(*expr); }
		/// implementation of evaluation
	int getUpperBoundComplement ( const TDLExpression* expr ) { return getUpperBoundComplement(*expr); }
		/// implementation of evaluation
	int getLowerBoundDirect ( const TDLExpression* expr ) { return getLowerBoundDirect(*expr); }
		/// implementation of evaluation
	int getLowerBoundComplement ( const TDLExpression* expr ) { return getLowerBoundComplement(*expr); }

public:		// visitor implementation: common cases
	// concept expressions
	virtual void visit ( const TDLConceptName& expr )
		{ value = getEntityValue(expr.getEntity()); }
	virtual void visit ( const TDLConceptObjectExists& expr )
		{ value = getMinValue ( 1, expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectForall& expr )
		{ value = getForallValue ( expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectMinCardinality& expr )
		{ value = getMinValue ( expr.getNumber(), expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectMaxCardinality& expr )
		{ value = getMaxValue ( expr.getNumber(), expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectExactCardinality& expr )
		{ value = getExactValue ( expr.getNumber(), expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptDataExists& expr )
		{ value = getMinValue ( 1, expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataForall& expr )
		{ value = getForallValue ( expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataMinCardinality& expr )
		{ value = getMinValue ( expr.getNumber(), expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataMaxCardinality& expr )
		{ value = getMaxValue ( expr.getNumber(), expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataExactCardinality& expr )
		{ value = getExactValue ( expr.getNumber(), expr.getDR(), expr.getExpr() ); }

	// object role expressions
	virtual void visit ( const TDLObjectRoleName& expr )
		{ value = getEntityValue(expr.getEntity()); }
		// equivalent to R(x,y) and C(x), so copy behaviour from ER.X
	virtual void visit ( const TDLObjectRoleProjectionFrom& expr )
		{ value = getMinValue ( 1, expr.getOR(), expr.getC() ); }
		// equivalent to R(x,y) and C(y), so copy behaviour from ER.X
	virtual void visit ( const TDLObjectRoleProjectionInto& expr )
		{ value = getMinValue ( 1, expr.getOR(), expr.getC() ); }

	// data role expressions
	virtual void visit ( const TDLDataRoleName& expr )
		{ value = getEntityValue(expr.getEntity()); }
}; // CardinalityEvaluatorBase

/// determine how many instances can an expression have
class UpperBoundDirectEvaluator: public CardinalityEvaluatorBase
{
protected:	// methods
		/// define a special value for concepts that are not in C^{<= n}
	const int getNoneValue ( void ) const { return -1; }
		/// define a special value for concepts that are in C^{<= n} for all n
	const int getAllValue ( void ) const { return 0; }

		/// helper for entities TODO: checks only C top-locality, not R
	virtual int getEntityValue ( const TNamedEntity* entity )
		{ return botCLocal() && nc(entity) ? getAllValue() : getNoneValue(); }
		/// helper for All
	virtual int getForallValue ( const TDLRoleExpression* R, const TDLExpression* C )
	{
		if ( isTopEquivalent(R) && getLowerBoundComplement(C) >= 1 )
			return getAllValue();
		else
			return getNoneValue();
	}
		/// helper for things like >= m R.C
	virtual int getMinValue ( int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// m > 0 and...
		if ( m <= 0 )
			return getNoneValue();
		// R = \bot or...
		if ( isBotEquivalent(R) )
			return getAllValue();
		// C \in C^{<= m-1}
		int ubC = getUpperBoundDirect(C);
		if ( ubC != getNoneValue() && ubC < m )
			return getAllValue();
		else
			return getNoneValue();
	}
		/// helper for things like <= m R.C
	virtual int getMaxValue ( int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// R = \top and...
		if ( !isTopEquivalent(R) )
			return getNoneValue();
		// C\in C^{>= m+1}
		int lbC = getLowerBoundDirect(C);
		if ( lbC != getNoneValue() && lbC > m )
			return getAllValue();
		else
			return getNoneValue();
	}
		/// helper for things like = m R.C
	virtual int getExactValue ( int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// here the maximal value between Mix and Max is an answer. The -1 case will be dealt with automagically
		return std::max ( getMinValue ( m, R, C ), getMaxValue ( m, R, C ) );
	}

		/// helper for And
	template<class C>
	int getAndValue ( const TDLNAryExpression<C>& expr )
	{
		int max = getNoneValue();
		// we are looking for the maximal value here; -1 will be dealt with automagically
		for ( typename TDLNAryExpression<C>::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			max = std::max ( max, getUpperBoundDirect(*p) );
		return max;
	}
		/// helper for Or
	template<class C>
	int getOrValue ( const TDLNAryExpression<C>& expr )
	{
		int sum = 0, n;
		for ( typename TDLNAryExpression<C>::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			n = getUpperBoundDirect(*p);
			if ( n == getNoneValue() )
				return getNoneValue();
			sum += n;
		}
		return sum;
	}

public:		// interface
		/// init c'tor
	UpperBoundDirectEvaluator ( const TSignature* s ) : CardinalityEvaluatorBase(s) {}
		/// empty d'tor
	virtual ~UpperBoundDirectEvaluator ( void ) {}

public:		// visitor implementation
	// concept expressions
	virtual void visit ( const TDLConceptTop& ) { value = getNoneValue(); }
	virtual void visit ( const TDLConceptBottom& ) { value = getAllValue(); }
	virtual void visit ( const TDLConceptNot& expr ) { value = getUpperBoundComplement(expr.getC()); }
	virtual void visit ( const TDLConceptAnd& expr ) { value = getAndValue(expr); }
	virtual void visit ( const TDLConceptOr& expr ) { value = getOrValue(expr); }
	virtual void visit ( const TDLConceptOneOf& expr ) { value = expr.size(); }
	virtual void visit ( const TDLConceptObjectSelf& expr ) { value = isBotEquivalent(expr.getOR()) ? getAllValue() : getNoneValue(); }
	virtual void visit ( const TDLConceptObjectValue& expr ) { value = isBotEquivalent(expr.getOR()) ? getAllValue() : getNoneValue(); }
	virtual void visit ( const TDLConceptDataValue& expr ) { value = isBotEquivalent(expr.getDR()) ? getAllValue() : getNoneValue(); }

	// object role expressions
	virtual void visit ( const TDLObjectRoleTop& ) { value = getNoneValue(); }
	virtual void visit ( const TDLObjectRoleBottom& ) { value = getAllValue(); }
	virtual void visit ( const TDLObjectRoleInverse& expr ) { value = getUpperBoundDirect(expr.getOR()); }
	virtual void visit ( const TDLObjectRoleChain& expr )
	{
		for ( TDLObjectRoleChain::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( isBotEquivalent(*p) )
			{
				value = getAllValue();
				return;
			}
		value = getNoneValue();
	}

	// data role expressions
	virtual void visit ( const TDLDataRoleTop& ) { value = getNoneValue(); }
	virtual void visit ( const TDLDataRoleBottom& ) { value = getAllValue(); }

	// data expressions
	virtual void visit ( const TDLDataTop& ) { value = getNoneValue(); }
	virtual void visit ( const TDLDataBottom& ) { value = getAllValue(); }
	// FIXME!! not ready
	//virtual void visit ( const TDLDataTypeName& ) { isBotEq = false; }
	// FIXME!! not ready
	//virtual void visit ( const TDLDataTypeRestriction& ) { isBotEq = false; }
	virtual void visit ( const TDLDataValue& ) { value = 1; }
	virtual void visit ( const TDLDataNot& expr ) { value = getUpperBoundComplement(expr.getExpr()); }
	virtual void visit ( const TDLDataAnd& expr ) { value = getAndValue(expr); }
	virtual void visit ( const TDLDataOr& expr ) { value = getOrValue(expr); }
	virtual void visit ( const TDLDataOneOf& expr ) { value = expr.size(); }
}; // UpperBoundDirectEvaluator

class UpperBoundComplementEvaluator: public CardinalityEvaluatorBase
{
protected:	// methods
		/// define a special value for concepts that are not in C^{<= n}
	const int getNoneValue ( void ) const { return -1; }
		/// define a special value for concepts that are in C^{<= n} for all n
	const int getAllValue ( void ) const { return 0; }

		/// helper for entities TODO: checks only C top-locality, not R
	virtual int getEntityValue ( const TNamedEntity* entity )
		{ return topCLocal() && nc(entity) ? getAllValue() : getNoneValue(); }
		/// helper for All
	virtual int getForallValue ( const TDLRoleExpression* R, const TDLExpression* C )
	{
		if ( isBotEquivalent(R) || getUpperBoundComplement(C) == 0 )
			return getAllValue();
		else
			return getNoneValue();
	}
		/// helper for things like >= m R.C
	int getMinValue ( int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// m == 0 or...
		if ( m == 0 )
			return getAllValue();
		// R = \top and...
		if ( !isTopEquivalent(R) )
			return getNoneValue();
		// C \in C^{>= m}
		return getLowerBoundDirect(C) >= m ? getAllValue() : getNoneValue();
	}
		/// helper for things like <= m R.C
	int getMaxValue ( int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// R = \bot or...
		if ( isBotEquivalent(R)  )
			return getAllValue();
		// C\in C^{<= m}
		int lbC = getUpperBoundDirect(C);
		if ( lbC != getNoneValue() && lbC <= m )
			return getAllValue();
		else
			return getNoneValue();
	}
		/// helper for things like = m R.C
	virtual int getExactValue ( int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// here the minimal value between Mix and Max is an answer. The -1 case will be dealt with automagically
		return std::min ( getMinValue ( m, R, C ), getMaxValue ( m, R, C ) );
	}

		/// helper for And
	template<class C>
	int getAndValue ( const TDLNAryExpression<C>& expr )
	{
		int sum = 0, n;
		for ( typename TDLNAryExpression<C>::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			n = getUpperBoundDirect(*p);
			if ( n == getNoneValue() )
				return getNoneValue();
			sum += n;
		}
		return sum;
	}
		/// helper for Or
	template<class C>
	int getOrValue ( const TDLNAryExpression<C>& expr )
	{
		int max = getNoneValue();
		// we are looking for the maximal value here; -1 will be dealt with automagically
		for ( typename TDLNAryExpression<C>::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			max = std::max ( max, getUpperBoundDirect(*p) );
		return max;
	}

public:		// interface
		/// init c'tor
	UpperBoundComplementEvaluator ( const TSignature* s ) : CardinalityEvaluatorBase(s) {}
		/// empty d'tor
	virtual ~UpperBoundComplementEvaluator ( void ) {}

public:		// visitor interface
	// concept expressions
	virtual void visit ( const TDLConceptTop& ) { value = getAllValue(); }
	virtual void visit ( const TDLConceptBottom& ) { value = getNoneValue(); }
	virtual void visit ( const TDLConceptNot& expr ) { value = getUpperBoundDirect(expr.getC()); }
	virtual void visit ( const TDLConceptAnd& expr ) { value = getAndValue(expr); }
	virtual void visit ( const TDLConceptOr& expr ) { value = getOrValue(expr); }
	virtual void visit ( const TDLConceptOneOf& ) { value = getNoneValue(); }
	virtual void visit ( const TDLConceptObjectSelf& expr ) { value = isTopEquivalent(expr.getOR()) ? getAllValue() : getNoneValue(); }
	virtual void visit ( const TDLConceptObjectValue& expr ) { value = isTopEquivalent(expr.getOR()) ? getAllValue() : getNoneValue(); }
	virtual void visit ( const TDLConceptDataValue& expr ) { value = isTopEquivalent(expr.getDR()) ? getAllValue() : getNoneValue(); }

	// object role expressions
	virtual void visit ( const TDLObjectRoleTop& ) { value = getAllValue(); }
	virtual void visit ( const TDLObjectRoleBottom& ) { value = getNoneValue(); }
	virtual void visit ( const TDLObjectRoleInverse& expr ) { value = getUpperBoundComplement(expr.getOR()); }
	virtual void visit ( const TDLObjectRoleChain& expr )
	{
		for ( TDLObjectRoleChain::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( !isTopEquivalent(*p) )
			{
				value = getNoneValue();
				return;
			}
		value = getAllValue();
	}

	// data role expressions
	virtual void visit ( const TDLDataRoleTop& ) { value = getAllValue(); }
	virtual void visit ( const TDLDataRoleBottom& ) { value = getNoneValue(); }

	// data expressions
	virtual void visit ( const TDLDataTop& ) { value = getAllValue(); }
	virtual void visit ( const TDLDataBottom& ) { value = getNoneValue(); }
	// negated datatype is a union of all other DTs that are infinite
	virtual void visit ( const TDLDataTypeName& ) { value = getNoneValue(); }
	// negated restriction include negated DT
	virtual void visit ( const TDLDataTypeRestriction& ) { value = getNoneValue(); }
	virtual void visit ( const TDLDataValue& ) { value = getNoneValue(); }
	virtual void visit ( const TDLDataNot& expr ) { value = getUpperBoundDirect(expr.getExpr()); }
	virtual void visit ( const TDLDataAnd& expr ) { value = getAndValue(expr); }
	virtual void visit ( const TDLDataOr& expr ) { value = getOrValue(expr); }
	virtual void visit ( const TDLDataOneOf& ) { value = getNoneValue(); }
}; // UpperBoundComplementEvaluator

class LowerBoundDirectEvaluator: public CardinalityEvaluatorBase
{
protected:	// methods
		/// define a special value for concepts that are not in C^{>= n}
	const int getNoneValue ( void ) const { return 0; }
		/// define a special value for concepts that are in C^{>= n} for all n
	const int getAllValue ( void ) const { return -1; }

		/// helper for entities TODO: checks only C top-locality, not R
	virtual int getEntityValue ( const TNamedEntity* entity )
		{ return topCLocal() && nc(entity) ? 1 : getNoneValue(); }
		/// helper for All
	virtual int getForallValue ( const TDLRoleExpression* R, const TDLExpression* C )
	{
		if ( isBotEquivalent(R) || getUpperBoundComplement(C) == 0 )
			return 1;
		else
			return getNoneValue();
	}
		/// helper for things like >= m R.C
	int getMinValue ( int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// m == 0 or...
		if ( m == 0 )
			return getAllValue();
		// R = \top and...
		if ( !isTopEquivalent(R) )
			return getNoneValue();
		// C \in C^{>= m}
		return getLowerBoundDirect(C) >= m ? m : getNoneValue();
	}
		/// helper for things like <= m R.C
	int getMaxValue ( int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// R = \bot or...
		if ( isBotEquivalent(R) )
			return 1;
		// C\in C^{<= m}
		int lbC = getUpperBoundDirect(C);
		if ( lbC != getNoneValue() && lbC <= m )
			return 1;
		else
			return getNoneValue();
	}
		/// helper for things like = m R.C
	virtual int getExactValue ( int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		int min = getMinValue ( m, R, C ), max = getMaxValue ( m, R, C );
		// we need to take the lowest value
		if ( min == getNoneValue() || max == getNoneValue() )
			return getNoneValue();
		if ( min == getAllValue() )
			return max;
		if ( max == getAllValue() )
			return min;
		return std::min ( min, max );
	}

		/// helper for And
		// FIXME!! not done yet
	template<class C>
	int getAndValue ( const TDLNAryExpression<C>& expr )
	{
		int sum = 0, n;
		for ( typename TDLNAryExpression<C>::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			n = getUpperBoundDirect(*p);
			if ( n == getNoneValue() )
				return getNoneValue();
			sum += n;
		}
		return sum;
	}
		/// helper for Or
	template<class C>
	int getOrValue ( const TDLNAryExpression<C>& expr )
	{
		int max = getNoneValue();
		// we are looking for the maximal value here; -1 will be dealt with automagically
		for ( typename TDLNAryExpression<C>::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			max = std::max ( max, getUpperBoundDirect(*p) );
		return max;
	}

public:		// interface
		/// init c'tor
	LowerBoundDirectEvaluator ( const TSignature* s ) : CardinalityEvaluatorBase(s) {}
		/// empty d'tor
	virtual ~LowerBoundDirectEvaluator ( void ) {}

public:		// visitor interface
	// concept expressions
	virtual void visit ( const TDLConceptTop& ) { value = 1; }
	virtual void visit ( const TDLConceptBottom& ) { value = getNoneValue(); }
	virtual void visit ( const TDLConceptNot& expr ) { value = getLowerBoundComplement(expr.getC()); }
	virtual void visit ( const TDLConceptAnd& expr ) { value = getAndValue(expr); }
	virtual void visit ( const TDLConceptOr& expr ) { value = getOrValue(expr); }
	virtual void visit ( const TDLConceptOneOf& expr ) { value = expr.size() > 0 ? 1 : getNoneValue(); }
	virtual void visit ( const TDLConceptObjectSelf& expr ) { value = isTopEquivalent(expr.getOR()) ? 1 : getNoneValue(); }
	// FIXME!! differ from the paper
	virtual void visit ( const TDLConceptObjectValue& expr ) { value = isTopEquivalent(expr.getOR()) ? 1 : getNoneValue(); }
	virtual void visit ( const TDLConceptDataValue& expr ) { value = isTopEquivalent(expr.getDR()) ? 1 : getNoneValue(); }

	// object role expressions
	virtual void visit ( const TDLObjectRoleTop& ) { value = getAllValue(); }
	virtual void visit ( const TDLObjectRoleBottom& ) { value = getNoneValue(); }
	virtual void visit ( const TDLObjectRoleInverse& expr ) { value = getLowerBoundDirect(expr.getOR()); }
	virtual void visit ( const TDLObjectRoleChain& expr )
	{
		for ( TDLObjectRoleChain::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( !isTopEquivalent(*p) )
			{
				value = getNoneValue();
				return;
			}
		value = getAllValue();
	}

	// data role expressions
	virtual void visit ( const TDLDataRoleTop& ) { value = getAllValue(); }
	virtual void visit ( const TDLDataRoleBottom& ) { value = getNoneValue(); }

	// data expressions
	virtual void visit ( const TDLDataTop& ) { value = getAllValue(); }
	virtual void visit ( const TDLDataBottom& ) { value = getNoneValue(); }
	// negated datatype is a union of all other DTs that are infinite
	virtual void visit ( const TDLDataTypeName& ) { value = getNoneValue(); }
	// negated restriction include negated DT
	virtual void visit ( const TDLDataTypeRestriction& ) { value = getNoneValue(); }
	virtual void visit ( const TDLDataValue& ) { value = getNoneValue(); }
	virtual void visit ( const TDLDataNot& expr ) { value = getLowerBoundComplement(expr.getExpr()); }
	virtual void visit ( const TDLDataAnd& expr ) { value = getAndValue(expr); }
	virtual void visit ( const TDLDataOr& expr ) { value = getOrValue(expr); }
	virtual void visit ( const TDLDataOneOf& expr ) { value = expr.size() > 0 ? 1 : getNoneValue(); }
}; // LowerBoundDirectEvaluator

class LowerBoundComplementEvaluator: public CardinalityEvaluatorBase
{
protected:	// methods
		/// define a special value for concepts that are not in C^{>= n}
	const int getNoneValue ( void ) const { return 0; }
		/// define a special value for concepts that are in C^{>= n} for all n
	const int getAllValue ( void ) const { return -1; }

		/// helper for entities TODO: checks only C top-locality, not R
	virtual int getEntityValue ( const TNamedEntity* entity )
		{ return botCLocal() && nc(entity) ? 1 : getNoneValue(); }
		/// helper for All
	virtual int getForallValue ( const TDLRoleExpression* R, const TDLExpression* C )
	{
		if ( isTopEquivalent(R) && getLowerBoundComplement(C) >= 1 )
			return 1;
		else
			return getNoneValue();
	}
		/// helper for things like >= m R.C
	virtual int getMinValue ( int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// m > 0 and...
		if ( m <= 0 )
			return getNoneValue();
		// R = \bot or...
		if ( isBotEquivalent(R) )
			return 1;
		// C \in C^{<= m-1}
		int ubC = getUpperBoundDirect(C);
		if ( ubC != getNoneValue() && ubC < m )
			return 1;
		else
			return getNoneValue();
	}
		/// helper for things like <= m R.C
	virtual int getMaxValue ( int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// R = \top and...
		if ( !isTopEquivalent(R) )
			return getNoneValue();
		// C\in C^{>= m+1}
		int lbC = getLowerBoundDirect(C);
		if ( lbC != getNoneValue() && lbC > m )
			return m+1;
		else
			return getNoneValue();
	}
		/// helper for things like = m R.C
	virtual int getExactValue ( int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// here the maximal value between Mix and Max is an answer. The -1 case will be dealt with automagically
		// because both min and max are between 0 and m+1
		return std::max ( getMinValue ( m, R, C ), getMaxValue ( m, R, C ) );
	}

		/// helper for And
	template<class C>
	int getAndValue ( const TDLNAryExpression<C>& expr )
	{
		int max = getNoneValue();
		// we are looking for the maximal value here; -1 will be dealt with automagically
		for ( typename TDLNAryExpression<C>::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			max = std::max ( max, getUpperBoundDirect(*p) );
		return max;
	}
		/// helper for Or
	template<class C>
	int getOrValue ( const TDLNAryExpression<C>& expr )
	{
		int sum = 0, n;
		for ( typename TDLNAryExpression<C>::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			n = getUpperBoundDirect(*p);
			if ( n == getNoneValue() )
				return getNoneValue();
			sum += n;
		}
		return sum;
	}

public:		// interface
		/// init c'tor
	LowerBoundComplementEvaluator ( const TSignature* s ) : CardinalityEvaluatorBase(s) {}
		/// empty d'tor
	virtual ~LowerBoundComplementEvaluator ( void ) {}

public:		// visitor implementation
	// concept expressions
	virtual void visit ( const TDLConceptTop& ) { value = getNoneValue(); }
	virtual void visit ( const TDLConceptBottom& ) { value = 1; }
	virtual void visit ( const TDLConceptNot& expr ) { value = getLowerBoundDirect(expr.getC()); }
	virtual void visit ( const TDLConceptAnd& expr ) { value = getAndValue(expr); }
	virtual void visit ( const TDLConceptOr& expr ) { value = getOrValue(expr); }
	virtual void visit ( const TDLConceptOneOf& ) { value = getNoneValue(); }
	virtual void visit ( const TDLConceptObjectSelf& expr ) { value = isBotEquivalent(expr.getOR()) ? 1 : getNoneValue(); }
	virtual void visit ( const TDLConceptObjectValue& expr ) { value = isBotEquivalent(expr.getOR()) ? 1 : getNoneValue(); }
	virtual void visit ( const TDLConceptDataValue& expr ) { value = isBotEquivalent(expr.getDR()) ? 1 : getNoneValue(); }

	// object role expressions
	virtual void visit ( const TDLObjectRoleTop& ) { value = getNoneValue(); }
	virtual void visit ( const TDLObjectRoleBottom& ) { value = getAllValue(); }
	virtual void visit ( const TDLObjectRoleInverse& expr ) { value = getLowerBoundComplement(expr.getOR()); }
	virtual void visit ( const TDLObjectRoleChain& expr )
	{
		for ( TDLObjectRoleChain::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( isBotEquivalent(*p) )
			{
				value = getAllValue();
				return;
			}
		value = getNoneValue();
	}

	// data role expressions
	virtual void visit ( const TDLDataRoleTop& ) { value = getNoneValue(); }
	virtual void visit ( const TDLDataRoleBottom& ) { value = getAllValue(); }

	// data expressions
	virtual void visit ( const TDLDataTop& ) { value = getNoneValue(); }
	virtual void visit ( const TDLDataBottom& ) { value = getAllValue(); }
	// FIXME!! not ready
	//virtual void visit ( const TDLDataTypeName& ) { isBotEq = false; }
	// FIXME!! not ready
	//virtual void visit ( const TDLDataTypeRestriction& ) { isBotEq = false; }
	virtual void visit ( const TDLDataValue& ) { value = 1; }
	virtual void visit ( const TDLDataNot& expr ) { value = getLowerBoundDirect(expr.getExpr()); }
	virtual void visit ( const TDLDataAnd& expr ) { value = getAndValue(expr); }
	virtual void visit ( const TDLDataOr& expr ) { value = getOrValue(expr); }
	virtual void visit ( const TDLDataOneOf& ) { value = getNoneValue(); }
}; // LowerBoundComplementEvaluator

/// implementation of evaluation
inline int
CardinalityEvaluatorBase :: getUpperBoundDirect ( const TDLExpression& expr ) { return UBD->getValue(expr); }
/// implementation of evaluation
inline int
CardinalityEvaluatorBase :: getUpperBoundComplement ( const TDLExpression& expr ) { return UBD->getValue(expr); }
/// implementation of evaluation
inline int
CardinalityEvaluatorBase :: getLowerBoundDirect ( const TDLExpression& expr ) { return LBD->getValue(expr); }
/// implementation of evaluation
inline int
CardinalityEvaluatorBase :: getLowerBoundComplement ( const TDLExpression& expr ) { return LBC->getValue(expr); }

/// syntactic locality checker for DL axioms
class ExtendedSyntacticLocalityChecker: public GeneralSyntacticLocalityChecker
{
protected:	// members
	UpperBoundDirectEvaluator UBD;
	LowerBoundDirectEvaluator LBD;
	UpperBoundComplementEvaluator UBC;
	LowerBoundComplementEvaluator LBC;

protected:	// methods
		/// @return true iff EXPR is top equivalent
	virtual bool isTopEquivalent ( const TDLExpression* expr ) { return UBC.getUpperBoundComplement(expr) == 0; }
		/// @return true iff EXPR is bottom equivalent
	virtual bool isBotEquivalent ( const TDLExpression* expr ) { return UBD.getUpperBoundDirect(expr) == 0; }
		/// @return true iff role expression in equivalent to const wrt locality
	bool isREquivalent ( const TDLExpression* expr ) { return topRLocal() ? isTopEquivalent(expr) : isBotEquivalent(expr); }

public:		// interface
		/// init c'tor
	ExtendedSyntacticLocalityChecker ( const TSignature* s )
		: GeneralSyntacticLocalityChecker(s)
		, UBD(s)
		, LBD(s)
		, UBC(s)
		, LBC(s)
	{
		UBD.setEvaluators ( &UBD, &LBD, &UBC, &LBC );
		LBD.setEvaluators ( &UBD, &LBD, &UBC, &LBC );
		UBC.setEvaluators ( &UBD, &LBD, &UBC, &LBC );
		LBC.setEvaluators ( &UBD, &LBD, &UBC, &LBC );
	}
		/// empty d'tor
	virtual ~ExtendedSyntacticLocalityChecker ( void ) {}
}; // ExtendedSyntacticLocalityChecker

#endif
