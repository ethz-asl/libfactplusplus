/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2013-2015 by Dmitry Tsarkov

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

// uncomment the following line to debug the locality checker
//#define FPP_DEBUG_EXTENDED_LOCALITY

// debug also the general locality checker
#ifdef FPP_DEBUG_EXTENDED_LOCALITY
#	define FPP_DEBUG_LOCALITY
#endif

#include "GeneralSyntacticLocalityChecker.h"

#ifdef FPP_DEBUG_EXTENDED_LOCALITY
#	include "tExpressionPrinterLISP.h"
#endif

// forward declarations
class UpperBoundDirectEvaluator;
class LowerBoundDirectEvaluator;
class UpperBoundComplementEvaluator;
class LowerBoundComplementEvaluator;

class CardinalityEvaluatorBase: protected SigAccessor, public DLExpressionVisitorEmpty
{
protected:	// members
#ifdef FPP_DEBUG_EXTENDED_LOCALITY
	TLISPExpressionPrinter lp;
	const char* name;
#endif

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

	void dumpValue ( const TDLExpression& expr ATTR_UNUSED )
	{
#	ifdef FPP_DEBUG_EXTENDED_LOCALITY
		std::cout << name << ": value for";
		expr.accept(lp);
		std::cout << " is " << value << std::endl;
#	endif
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
		/// define a special value for concepts that are not in C[C}^{<= n}
	int noUpperValue ( void ) const { return -1; }
		/// define a special value for concepts that are in C[C]^{<= n} for all n
	int anyUpperValue ( void ) const { return 0; }
		/// return all or none values depending on the condition
	int getAllNoneUpper ( bool condition ) const { return condition ? anyUpperValue() : noUpperValue(); }
		/// define a special value for concepts that are not in C[C]^{>= n}
	int noLowerValue ( void ) const { return 0; }
		/// define a special value for concepts that are in C[C]^{>= n} for all n
	int anyLowerValue ( void ) const { return -1; }
		/// return 1 or none values depending on the condition
	int getOneNoneLower ( bool condition ) const { return condition ? 1 : noLowerValue(); }

	bool isBotEquivalent ( const TDLExpression* expr ) { return getUpperBoundDirect(*expr) == 0; }
	bool isTopEquivalent ( const TDLExpression* expr ) { return getUpperBoundComplement(*expr) == 0; }

		/// helper for entities
	virtual int getEntityValue ( const TNamedEntity* entity ) = 0;
		/// helper for All
	virtual int getForallValue ( const TDLRoleExpression* R, const TDLExpression* C ) = 0;
		/// helper for things like >= m R.C
	virtual int getMinValue ( unsigned int m, const TDLRoleExpression* R, const TDLExpression* C ) = 0;
		/// helper for things like <= m R.C
	virtual int getMaxValue ( unsigned int m, const TDLRoleExpression* R, const TDLExpression* C ) = 0;
		/// helper for things like = m R.C
	virtual int getExactValue ( unsigned int m, const TDLRoleExpression* R, const TDLExpression* C ) = 0;

public:		// interface
		/// init c'tor
	CardinalityEvaluatorBase ( const TSignature* s, const char* n ATTR_UNUSED )
		: SigAccessor(s)
#	ifdef FPP_DEBUG_EXTENDED_LOCALITY
		, lp(std::cout)
		, name(n)
#	endif
		, value(0)
		{}
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
		{ value = getMinValue ( 1, expr.getOR(), expr.getC() ); dumpValue(expr); }
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
		// FaCT++ extension: equivalent to R(x,y) and C(x), so copy behaviour from ER.X
	virtual void visit ( const TDLObjectRoleProjectionFrom& expr )
		{ value = getMinValue ( 1, expr.getOR(), expr.getC() ); }
		// FaCT++ extension: equivalent to R(x,y) and C(y), so copy behaviour from ER.X
	virtual void visit ( const TDLObjectRoleProjectionInto& expr )
		{ value = getMinValue ( 1, expr.getOR(), expr.getC() ); }

	// data role expressions
	virtual void visit ( const TDLDataRoleName& expr )
		{ value = getEntityValue(expr.getEntity()); }
}; // CardinalityEvaluatorBase

/// determine how many instances can an expression have;
/// all methods return minimal n such that expr\in C^{<= n}, n >= 0
class UpperBoundDirectEvaluator: public CardinalityEvaluatorBase
{
protected:	// methods
		/// helper for entities TODO: checks only C top-locality, not R
	virtual int getEntityValue ( const TNamedEntity* entity )
		{ return getAllNoneUpper ( botCLocal() && nc(entity) ); }
		/// helper for All
	virtual int getForallValue ( const TDLRoleExpression* R, const TDLExpression* C )
		{ return getAllNoneUpper ( isTopEquivalent(R) && getLowerBoundComplement(C) >= 1 ); }
		/// helper for things like >= m R.C
	virtual int getMinValue ( unsigned int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// m > 0 and...
		if ( m <= 0 )
			return noUpperValue();
		// R = \bot or...
		if ( isBotEquivalent(R) )
			return anyUpperValue();
		// C \in C^{<= m-1}
		int ubD = getUpperBoundDirect(C);
		return getAllNoneUpper ( ubD != noUpperValue() && ubD < (int)m );
	}
		/// helper for things like <= m R.C
	virtual int getMaxValue ( unsigned int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// R = \top and...
		if ( !isTopEquivalent(R) )
			return noUpperValue();
		// C\in C^{>= m+1}
		int lbD = getLowerBoundDirect(C);
		// note bound flip
		return getAllNoneUpper ( lbD != noLowerValue() && lbD > (int)m );
	}
		/// helper for things like = m R.C
	virtual int getExactValue ( unsigned int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// here the maximal value between Mix and Max is an answer. The -1 case will be dealt with automagically
		return std::max ( getMinValue ( m, R, C ), getMaxValue ( m, R, C ) );
	}

		/// helper for And
	template<class C>
	int getAndValue ( const TDLNAryExpression<C>& expr )
	{
		int max = noUpperValue();
		// we are looking for the maximal value here; -1 will be dealt with automagically
		for ( typename TDLNAryExpression<C>::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			int n = getUpperBoundDirect(*p);
			// if an argument is in C^{<= n} for every n, so is a conjunction
			if ( n == anyUpperValue() )
				return anyUpperValue();
			max = std::max ( max, n );
		}
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
			if ( n == noUpperValue() )
				return noUpperValue();
			sum += n;
		}
		return sum;
	}

public:		// interface
		/// init c'tor
	UpperBoundDirectEvaluator ( const TSignature* s ) : CardinalityEvaluatorBase ( s, "UpperBoundDirect" ) {}
		/// empty d'tor
	virtual ~UpperBoundDirectEvaluator ( void ) {}

public:		// visitor implementation
	// make all other visit() methods from the base implementation visible
	using CardinalityEvaluatorBase::visit;
	// concept expressions
	virtual void visit ( const TDLConceptTop& ) { value = noUpperValue(); }
	virtual void visit ( const TDLConceptBottom& ) { value = anyUpperValue(); }
	virtual void visit ( const TDLConceptNot& expr ) { value = getUpperBoundComplement(expr.getC()); }
	virtual void visit ( const TDLConceptAnd& expr ) { value = getAndValue(expr); dumpValue(expr); }
	virtual void visit ( const TDLConceptOr& expr ) { value = getOrValue(expr); }
	virtual void visit ( const TDLConceptOneOf& expr ) { value = (int)expr.size(); }
	virtual void visit ( const TDLConceptObjectSelf& expr ) { value = getAllNoneUpper(isBotEquivalent(expr.getOR())); }
	virtual void visit ( const TDLConceptObjectValue& expr ) { value = getAllNoneUpper(isBotEquivalent(expr.getOR())); }
	virtual void visit ( const TDLConceptDataValue& expr ) { value = getAllNoneUpper(isBotEquivalent(expr.getDR())); }

	// object role expressions
	virtual void visit ( const TDLObjectRoleTop& ) { value = noUpperValue(); }
	virtual void visit ( const TDLObjectRoleBottom& ) { value = anyUpperValue(); }
	virtual void visit ( const TDLObjectRoleInverse& expr ) { value = getUpperBoundDirect(expr.getOR()); }
	virtual void visit ( const TDLObjectRoleChain& expr )
	{
		for ( TDLObjectRoleChain::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( isBotEquivalent(*p) )
			{
				value = anyUpperValue();
				return;
			}
		value = noUpperValue();
	}

	// data role expressions
	virtual void visit ( const TDLDataRoleTop& ) { value = noUpperValue(); }
	virtual void visit ( const TDLDataRoleBottom& ) { value = anyUpperValue(); }

	// data expressions
	virtual void visit ( const TDLDataTop& ) { value = noUpperValue(); }
	virtual void visit ( const TDLDataBottom& ) { value = anyUpperValue(); }
	// FIXME!! not ready
	//virtual void visit ( const TDLDataTypeName& ) { isBotEq = false; }
	// FIXME!! not ready
	//virtual void visit ( const TDLDataTypeRestriction& ) { isBotEq = false; }
	virtual void visit ( const TDLDataValue& ) { value = 1; }
	virtual void visit ( const TDLDataNot& expr ) { value = getUpperBoundComplement(expr.getExpr()); }
	virtual void visit ( const TDLDataAnd& expr ) { value = getAndValue(expr); }
	virtual void visit ( const TDLDataOr& expr ) { value = getOrValue(expr); }
	virtual void visit ( const TDLDataOneOf& expr ) { value = (int)expr.size(); }
}; // UpperBoundDirectEvaluator

/// determine how many instances can a complement of expression have;
/// all methods return minimal n such that expr\in CC^{<= n}, n >= 0
class UpperBoundComplementEvaluator: public CardinalityEvaluatorBase
{
protected:	// methods
		/// helper for entities TODO: checks only C top-locality, not R
	virtual int getEntityValue ( const TNamedEntity* entity )
		{ return getAllNoneUpper ( topCLocal() && nc(entity) ); }
		/// helper for All
	virtual int getForallValue ( const TDLRoleExpression* R, const TDLExpression* C )
		{ return getAllNoneUpper ( isBotEquivalent(R) || getUpperBoundComplement(C) == 0 ); }
		/// helper for things like >= m R.C
	virtual int getMinValue ( unsigned int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// m == 0 or...
		if ( m == 0 )
			return anyUpperValue();
		// R = \top and...
		if ( !isTopEquivalent(R) )
			return noUpperValue();
		// C \in C^{>= m}
		return getAllNoneUpper ( getLowerBoundDirect(C) >= (int)m );
	}
		/// helper for things like <= m R.C
	virtual int getMaxValue ( unsigned int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// R = \bot or...
		if ( isBotEquivalent(R)  )
			return anyUpperValue();
		// C\in C^{<= m}
		int ubD = getUpperBoundDirect(C);
		return getAllNoneUpper ( ubD != noUpperValue() && ubD <= (int)m );
	}
		/// helper for things like = m R.C
	virtual int getExactValue ( unsigned int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// here the minimal value between Mix and Max is an answer. The -1 case will be dealt with automagically
		return std::min ( getMinValue ( m, R, C ), getMaxValue ( m, R, C ) );
	}

		/// helper for And
	template<class C>
	int getAndValue ( const TDLNAryExpression<C>& expr )
	{
		int sum = 0;
		for ( typename TDLNAryExpression<C>::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			int n = getUpperBoundComplement(*p);
			if ( n == noUpperValue() )
				return noUpperValue();
			sum += n;
		}
		return sum;
	}
		/// helper for Or
	template<class C>
	int getOrValue ( const TDLNAryExpression<C>& expr )
	{
		int max = noUpperValue();
		// we are looking for the maximal value here; -1 will be dealt with automagically
		for ( typename TDLNAryExpression<C>::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			int n = getUpperBoundComplement(*p);
			// if an argument is in CC^{<= n} for every n, so is a disjunction
			if ( n == anyUpperValue() )
				return anyUpperValue();
			max = std::max ( max, n );
		}
		return max;
	}

public:		// interface
		/// init c'tor
	UpperBoundComplementEvaluator ( const TSignature* s ) : CardinalityEvaluatorBase ( s, "UpperBoundCompliment" ) {}
		/// empty d'tor
	virtual ~UpperBoundComplementEvaluator ( void ) {}

public:		// visitor interface
	// make all other visit() methods from the base implementation visible
	using CardinalityEvaluatorBase::visit;
	// concept expressions
	virtual void visit ( const TDLConceptTop& ) { value = anyUpperValue(); }
	virtual void visit ( const TDLConceptBottom& ) { value = noUpperValue(); }
	virtual void visit ( const TDLConceptNot& expr ) { value = getUpperBoundDirect(expr.getC()); }
	virtual void visit ( const TDLConceptAnd& expr ) { value = getAndValue(expr); dumpValue(expr); }
	virtual void visit ( const TDLConceptOr& expr ) { value = getOrValue(expr); }
	virtual void visit ( const TDLConceptOneOf& ) { value = noUpperValue(); }
	virtual void visit ( const TDLConceptObjectSelf& expr ) { value = getAllNoneUpper(isTopEquivalent(expr.getOR())); }
	virtual void visit ( const TDLConceptObjectValue& expr ) { value = getAllNoneUpper(isTopEquivalent(expr.getOR())); }
	virtual void visit ( const TDLConceptDataValue& expr ) { value = getAllNoneUpper(isTopEquivalent(expr.getDR())); }

	// object role expressions
	virtual void visit ( const TDLObjectRoleTop& ) { value = anyUpperValue(); }
	virtual void visit ( const TDLObjectRoleBottom& ) { value = noUpperValue(); }
	virtual void visit ( const TDLObjectRoleInverse& expr ) { value = getUpperBoundComplement(expr.getOR()); }
	virtual void visit ( const TDLObjectRoleChain& expr )
	{
		for ( TDLObjectRoleChain::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( !isTopEquivalent(*p) )
			{
				value = noUpperValue();
				return;
			}
		value = anyUpperValue();
	}

	// data role expressions
	virtual void visit ( const TDLDataRoleTop& ) { value = anyUpperValue(); }
	virtual void visit ( const TDLDataRoleBottom& ) { value = noUpperValue(); }

	// data expressions
	virtual void visit ( const TDLDataTop& ) { value = anyUpperValue(); }
	virtual void visit ( const TDLDataBottom& ) { value = noUpperValue(); }
	// negated datatype is a union of all other DTs that are infinite
	virtual void visit ( const TDLDataTypeName& ) { value = noUpperValue(); }
	// negated restriction include negated DT
	virtual void visit ( const TDLDataTypeRestriction& ) { value = noUpperValue(); }
	virtual void visit ( const TDLDataValue& ) { value = noUpperValue(); }
	virtual void visit ( const TDLDataNot& expr ) { value = getUpperBoundDirect(expr.getExpr()); }
	virtual void visit ( const TDLDataAnd& expr ) { value = getAndValue(expr); }
	virtual void visit ( const TDLDataOr& expr ) { value = getOrValue(expr); }
	virtual void visit ( const TDLDataOneOf& ) { value = noUpperValue(); }
}; // UpperBoundComplementEvaluator

/// determine how many instances can an expression have;
/// all methods return maximal n such that expr\in C^{>= n}, n >= 1
class LowerBoundDirectEvaluator: public CardinalityEvaluatorBase
{
protected:	// methods
		/// helper for entities TODO: checks only C top-locality, not R
	virtual int getEntityValue ( const TNamedEntity* entity )
		{ return getOneNoneLower ( topCLocal() && nc(entity) ); }
		/// helper for All
	virtual int getForallValue ( const TDLRoleExpression* R, const TDLExpression* C )
		{ return getOneNoneLower ( isBotEquivalent(R) || getUpperBoundComplement(C) == 0 ); }
		/// helper for things like >= m R.C
	virtual int getMinValue ( unsigned int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// m == 0 or...
		if ( m == 0 )
			return anyLowerValue();
		// R = \top and...
		if ( !isTopEquivalent(R) )
			return noLowerValue();
		// C \in C^{>= m}
		return getLowerBoundDirect(C) >= (int)m ? (int)m : noLowerValue();
	}
		/// helper for things like <= m R.C
	virtual int getMaxValue ( unsigned int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// R = \bot or...
		if ( isBotEquivalent(R) )
			return 1;
		// C\in C^{<= m}
		int ubD = getUpperBoundDirect(C);
		// note bound flip
		return getOneNoneLower ( ubD != noUpperValue() && ubD <= (int)m );
	}
		/// helper for things like = m R.C
	virtual int getExactValue ( unsigned int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		int min = getMinValue ( m, R, C ), max = getMaxValue ( m, R, C );
		// we need to take the lowest value
		if ( min == noLowerValue() || max == noLowerValue() )
			return noLowerValue();
		if ( min == anyLowerValue() )
			return max;
		if ( max == anyLowerValue() )
			return min;
		return std::min ( min, max );
	}

		/// helper for And
		// FIXME!! not done yet
	template<class C>
	int getAndValue ( const TDLNAryExpression<C>& expr )
	{
		// return m - sumK, where
		bool foundC = false;	// true if found a conjunct that is in C^{>=}
		int foundM = 0;
		int mMax = 0, kMax = 0;	// the m- and k- values for the C_j with max m+k
		int sumK = 0;			// sum of all known k
		// 1st pass: check for none-case, deals with deterministic cases
		for ( typename TDLNAryExpression<C>::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			int m = getLowerBoundDirect(*p);		// C_j \in C^{>= m}
			int k = getUpperBoundComplement(*p);	// C_j \in CC^{<= k}
			// note bound flip for K
			// case 0: if both aren't known then we don't know
			if ( m == noLowerValue() && k == noUpperValue() )
				return noLowerValue();
			// if only k exists then add it to k
			if ( m == noLowerValue() )
			{
//				if ( k == getAllValue() )	// we don't have any bound then
//					return getNoneValue();
				sumK += k;
				continue;
			}
			// if only m exists then set it to m
			if ( k == noUpperValue() )
			{
				if ( foundC )	// should not have 2 elements in C
					return noLowerValue();
				foundC = true;
				foundM = m;
				continue;
			}
			// here both k and m are values
			sumK += k;	// count k for the
			if ( k+m > kMax + mMax )
			{
				kMax = k;
				mMax = m;
			}
		}
		// here we know the best candidate for M, and only need to set it up
		if ( foundC )	// found during the deterministic case
		{
			foundM -= sumK;
			return foundM > 0 ? foundM : noLowerValue();
		}
		else	// no deterministic option; choose the best one
		{
			sumK -= kMax;
			mMax -= sumK;
			return mMax > 0 ? mMax : noLowerValue();
		}
	}
		/// helper for Or
	template<class C>
	int getOrValue ( const TDLNAryExpression<C>& expr )
	{
		int max = noLowerValue();
		// we are looking for the maximal value here; ANY need to be special-cased
		for ( typename TDLNAryExpression<C>::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			int n = getLowerBoundDirect(*p);
			if ( n == anyLowerValue() )
				return anyLowerValue();
			max = std::max ( max, n );
		}
		return max;
	}

public:		// interface
		/// init c'tor
	LowerBoundDirectEvaluator ( const TSignature* s ) : CardinalityEvaluatorBase ( s, "LowerBoundDirect" ) {}
		/// empty d'tor
	virtual ~LowerBoundDirectEvaluator ( void ) {}

public:		// visitor interface
	// make all other visit() methods from the base implementation visible
	using CardinalityEvaluatorBase::visit;
	// concept expressions
	virtual void visit ( const TDLConceptTop& ) { value = 1; }
	virtual void visit ( const TDLConceptBottom& ) { value = noLowerValue(); }
	virtual void visit ( const TDLConceptNot& expr ) { value = getLowerBoundComplement(expr.getC()); }
	virtual void visit ( const TDLConceptAnd& expr ) { value = getAndValue(expr); dumpValue(expr); }
	virtual void visit ( const TDLConceptOr& expr ) { value = getOrValue(expr); }
	virtual void visit ( const TDLConceptOneOf& expr ) { value = getOneNoneLower(expr.size() > 0); }
	virtual void visit ( const TDLConceptObjectSelf& expr ) { value = getOneNoneLower(isTopEquivalent(expr.getOR())); }
	// FIXME!! differ from the paper
	virtual void visit ( const TDLConceptObjectValue& expr ) { value = getOneNoneLower(isTopEquivalent(expr.getOR())); }
	virtual void visit ( const TDLConceptDataValue& expr ) { value = getOneNoneLower(isTopEquivalent(expr.getDR())); }

	// object role expressions
	virtual void visit ( const TDLObjectRoleTop& ) { value = anyLowerValue(); }
	virtual void visit ( const TDLObjectRoleBottom& ) { value = noLowerValue(); }
	virtual void visit ( const TDLObjectRoleInverse& expr ) { value = getLowerBoundDirect(expr.getOR()); }
	virtual void visit ( const TDLObjectRoleChain& expr )
	{
		for ( TDLObjectRoleChain::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( !isTopEquivalent(*p) )
			{
				value = noLowerValue();
				return;
			}
		value = anyLowerValue();
	}

	// data role expressions
	virtual void visit ( const TDLDataRoleTop& ) { value = anyLowerValue(); }
	virtual void visit ( const TDLDataRoleBottom& ) { value = noLowerValue(); }

	// data expressions
	virtual void visit ( const TDLDataTop& ) { value = anyLowerValue(); }
	virtual void visit ( const TDLDataBottom& ) { value = noLowerValue(); }
	// negated datatype is a union of all other DTs that are infinite
	virtual void visit ( const TDLDataTypeName& ) { value = noLowerValue(); }
	// negated restriction include negated DT
	virtual void visit ( const TDLDataTypeRestriction& ) { value = noLowerValue(); }
	virtual void visit ( const TDLDataValue& ) { value = noLowerValue(); }
	virtual void visit ( const TDLDataNot& expr ) { value = getLowerBoundComplement(expr.getExpr()); }
	virtual void visit ( const TDLDataAnd& expr ) { value = getAndValue(expr); }
	virtual void visit ( const TDLDataOr& expr ) { value = getOrValue(expr); }
	virtual void visit ( const TDLDataOneOf& expr ) { value = getOneNoneLower(expr.size() > 0); }
}; // LowerBoundDirectEvaluator

/// determine how many instances can an expression have;
/// all methods return maximal n such that expr\in CC^{>= n}, n >= 1
class LowerBoundComplementEvaluator: public CardinalityEvaluatorBase
{
protected:	// methods
		/// helper for entities TODO: checks only C top-locality, not R
	virtual int getEntityValue ( const TNamedEntity* entity )
		{ return getOneNoneLower ( botCLocal() && nc(entity) ); }
		/// helper for All
	virtual int getForallValue ( const TDLRoleExpression* R, const TDLExpression* C )
		{ return getOneNoneLower ( isTopEquivalent(R) && getLowerBoundComplement(C) >= 1 ); }
		/// helper for things like >= m R.C
	virtual int getMinValue ( unsigned int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// m > 0 and...
		if ( m <= 0 )
			return noLowerValue();
		// R = \bot or...
		if ( isBotEquivalent(R) )
			return 1;
		// C \in C^{<= m-1}
		int ubD = getUpperBoundDirect(C);
		// note bound flip
		return getOneNoneLower ( ubD != noUpperValue() && ubD < (int)m );
	}
		/// helper for things like <= m R.C
	virtual int getMaxValue ( unsigned int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// R = \top and...
		if ( !isTopEquivalent(R) )
			return noLowerValue();
		// C\in C^{>= m+1}
		int lbD = getLowerBoundDirect(C);
		if ( lbD != noLowerValue() && lbD > (int)m )
			return (int)m+1;
		else
			return noLowerValue();
	}
		/// helper for things like = m R.C
	virtual int getExactValue ( unsigned int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// here the maximal value between Mix and Max is an answer. The -1 case will be dealt with automagically
		// because both min and max are between 0 and m+1
		return std::max ( getMinValue ( m, R, C ), getMaxValue ( m, R, C ) );
	}

		/// helper for And
	template<class C>
	int getAndValue ( const TDLNAryExpression<C>& expr )
	{
		int max = noLowerValue();
		// we are looking for the maximal value here; ANY need to be special-cased
		for ( typename TDLNAryExpression<C>::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			int n = getLowerBoundComplement(*p);
			if ( n == anyLowerValue() )
				return anyLowerValue();
			max = std::max ( max, n );
		}
		return max;
	}
		/// helper for Or
	template<class C>
	int getOrValue ( const TDLNAryExpression<C>& expr )
	{
		// return m - sumK, where
		bool foundC = false;	// true if found a conjunct that is in C^{>=}
		int foundM = 0;
		int mMax = 0, kMax = 0;	// the m- and k- values for the C_j with max m+k
		int sumK = 0;			// sum of all known k
		// 1st pass: check for none-case, deals with deterministic cases
		for ( typename TDLNAryExpression<C>::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			int m = getLowerBoundComplement(*p);	// C_j \in CC^{>= m}
			int k = getUpperBoundDirect(*p);		// C_j \in C^{<= k}
			// note bound flip for K
			// case 0: if both aren't known then we don't know
			if ( m == noLowerValue() && k == noUpperValue() )
				return noLowerValue();
			// if only k exists then add it to k
			if ( m == noLowerValue() )
			{
//				if ( k == getAllValue() )	// we don't have any bound then
//					return getNoneValue();
				sumK += k;
				continue;
			}
			// if only m exists then set it to m
			if ( k == noUpperValue() )
			{
				if ( foundC )	// should not have 2 elements in C
					return noLowerValue();
				foundC = true;
				foundM = m;
				continue;
			}
			// here both k and m are values
			sumK += k;	// count k for the
			if ( k+m > kMax + mMax )
			{
				kMax = k;
				mMax = m;
			}
		}
		// here we know the best candidate for M, and only need to set it up
		if ( foundC )	// found during the deterministic case
		{
			foundM -= sumK;
			return foundM > 0 ? foundM : noLowerValue();
		}
		else	// no deterministic option; choose the best one
		{
			sumK -= kMax;
			mMax -= sumK;
			return mMax > 0 ? mMax : noLowerValue();
		}
	}

public:		// interface
		/// init c'tor
	LowerBoundComplementEvaluator ( const TSignature* s ) : CardinalityEvaluatorBase ( s, "LowerBoundComplement" ) {}
		/// empty d'tor
	virtual ~LowerBoundComplementEvaluator ( void ) {}

public:		// visitor implementation
	// make all other visit() methods from the base implementation visible
	using CardinalityEvaluatorBase::visit;
	// concept expressions
	virtual void visit ( const TDLConceptTop& ) { value = noLowerValue(); }
	virtual void visit ( const TDLConceptBottom& ) { value = 1; }
	virtual void visit ( const TDLConceptNot& expr ) { value = getLowerBoundDirect(expr.getC()); }
	virtual void visit ( const TDLConceptAnd& expr ) { value = getAndValue(expr); dumpValue(expr); }
	virtual void visit ( const TDLConceptOr& expr ) { value = getOrValue(expr); }
	virtual void visit ( const TDLConceptOneOf& ) { value = noLowerValue(); }
	virtual void visit ( const TDLConceptObjectSelf& expr ) { value = getOneNoneLower(isBotEquivalent(expr.getOR())); }
	virtual void visit ( const TDLConceptObjectValue& expr ) { value = getOneNoneLower(isBotEquivalent(expr.getOR())); }
	virtual void visit ( const TDLConceptDataValue& expr ) { value = getOneNoneLower(isBotEquivalent(expr.getDR())); }

	// object role expressions
	virtual void visit ( const TDLObjectRoleTop& ) { value = noLowerValue(); }
	virtual void visit ( const TDLObjectRoleBottom& ) { value = anyLowerValue(); }
	virtual void visit ( const TDLObjectRoleInverse& expr ) { value = getLowerBoundComplement(expr.getOR()); }
	virtual void visit ( const TDLObjectRoleChain& expr )
	{
		for ( TDLObjectRoleChain::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( isBotEquivalent(*p) )
			{
				value = anyLowerValue();
				return;
			}
		value = noLowerValue();
	}

	// data role expressions
	virtual void visit ( const TDLDataRoleTop& ) { value = noLowerValue(); }
	virtual void visit ( const TDLDataRoleBottom& ) { value = anyLowerValue(); }

	// data expressions
	virtual void visit ( const TDLDataTop& ) { value = noLowerValue(); }
	virtual void visit ( const TDLDataBottom& ) { value = anyLowerValue(); }
	// FIXME!! not ready
	//virtual void visit ( const TDLDataTypeName& ) { isBotEq = false; }
	// FIXME!! not ready
	//virtual void visit ( const TDLDataTypeRestriction& ) { isBotEq = false; }
	virtual void visit ( const TDLDataValue& ) { value = 1; }
	virtual void visit ( const TDLDataNot& expr ) { value = getLowerBoundDirect(expr.getExpr()); }
	virtual void visit ( const TDLDataAnd& expr ) { value = getAndValue(expr); }
	virtual void visit ( const TDLDataOr& expr ) { value = getOrValue(expr); }
	virtual void visit ( const TDLDataOneOf& ) { value = noLowerValue(); }
}; // LowerBoundComplementEvaluator

/// implementation of evaluation
inline int
CardinalityEvaluatorBase :: getUpperBoundDirect ( const TDLExpression& expr ) { return UBD->getValue(expr); }
/// implementation of evaluation
inline int
CardinalityEvaluatorBase :: getUpperBoundComplement ( const TDLExpression& expr ) { return UBC->getValue(expr); }
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
#ifdef FPP_DEBUG_EXTENDED_LOCALITY
	TLISPExpressionPrinter lp;
#endif

protected:	// methods
		/// @return true iff EXPR is top equivalent
	virtual bool isTopEquivalent ( const TDLExpression* expr ) {
#	ifdef FPP_DEBUG_EXTENDED_LOCALITY
		std::cout << "Checking top locality of";
		expr->accept(lp);
		std::cout << "\n";
#	endif
		bool ret = UBC.getUpperBoundComplement(expr) == 0;
#	ifdef FPP_DEBUG_EXTENDED_LOCALITY
		std::cout << "top loc: " << ret << "\n";
#	endif
		return ret;
	}
		/// @return true iff EXPR is bottom equivalent
	virtual bool isBotEquivalent ( const TDLExpression* expr ) {
#	ifdef FPP_DEBUG_EXTENDED_LOCALITY
		std::cout << "Checking bot locality of";
		expr->accept(lp);
		std::cout << "\n";
#	endif
		bool ret = UBD.getUpperBoundDirect(expr) == 0;
#	ifdef FPP_DEBUG_EXTENDED_LOCALITY
		std::cout << "bot loc: " << ret << "\n";
#	endif
		return ret;
	}

public:		// interface
		/// init c'tor
	ExtendedSyntacticLocalityChecker ( const TSignature* s )
		: GeneralSyntacticLocalityChecker(s)
		, UBD(s)
		, LBD(s)
		, UBC(s)
		, LBC(s)
#	ifdef FPP_DEBUG_EXTENDED_LOCALITY
		, lp(std::cout)
#	endif
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
