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
	int isBotEquivalent ( const TDLExpression* expr ) { return getUpperBoundDirect(*expr); }
	int isTopEquivalent ( const TDLExpression* expr ) { return getLowerBoundDirect(*expr); }

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
}; // CardinalityEvaluatorBase

/// determine how many instances can an expression have
class UpperBoundDirectEvaluator: public CardinalityEvaluatorBase
{
protected:	// methods
		/// helper for things like >= m R.C
	int getMinUpperBoundDirect ( int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// m > 0 and...
		if ( m <= 0 )
			return -1;
		// C \in C^{<= m-1} or R = \bot
		int ubC = getUpperBoundDirect(C);
		if ( ubC != -1 && ubC < m )
			return 0;
		return isBotEquivalent(R);
	}
		/// helper for things like <= m R.C
	int getMaxUpperBoundDirect ( int m, const TDLRoleExpression* R, const TDLExpression* C )
	{
		// R = \top and C\in C^{>= m+1}
		if ( isTopEquivalent(R) == -1 )
			return -1;
		int lbC = getLowerBoundDirect(C);
		if ( lbC == -1 || lbC > m )
			return 0;
		else
			return -1;
	}

public:		// interface
		/// init c'tor
	UpperBoundDirectEvaluator ( const TSignature* s ) : CardinalityEvaluatorBase(s) {}
		/// empty d'tor
	virtual ~UpperBoundDirectEvaluator ( void ) {}

public:		// visitor implementation
	// concept expressions
	virtual void visit ( const TDLConceptTop& ) { value = -1; }
	virtual void visit ( const TDLConceptBottom& ) { value = 0; }
	virtual void visit ( const TDLConceptName& expr ) { value = botCLocal() && nc(expr.getEntity()) ? 0 : -1; }
	virtual void visit ( const TDLConceptNot& expr ) { value = getUpperBoundComplement(expr.getC()); }
	virtual void visit ( const TDLConceptAnd& expr )
	{
		int min = -1, n;
		for ( TDLConceptAnd::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			n = getUpperBoundDirect(*p);
			if ( n != -1 )
				min = min == -1 ? n : std::min ( min, n );
		}
		value = min;
	}
	virtual void visit ( const TDLConceptOr& expr )
	{
		int sum = 0, n;
		for ( TDLConceptOr::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			n = getUpperBoundDirect(*p);
			if ( n == -1 )
			{
				value = -1;
				return;
			}
			sum += n;
		}
		value = n;
	}
	virtual void visit ( const TDLConceptOneOf& expr ) { value = expr.size(); }
	virtual void visit ( const TDLConceptObjectSelf& expr ) { value = isBotEquivalent(expr.getOR()); }
	virtual void visit ( const TDLConceptObjectValue& expr ) { value = isBotEquivalent(expr.getOR()); }
	virtual void visit ( const TDLConceptObjectExists& expr )
		{ value = getMinUpperBoundDirect ( 1, expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectForall& expr )
		{ value = isTopEquivalent(expr.getOR()) && getLowerBoundComplement(expr.getC()) >= 1 ? 0 : -1; }
	virtual void visit ( const TDLConceptObjectMinCardinality& expr )
		{ value = getMinUpperBoundDirect ( expr.getNumber(), expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectMaxCardinality& expr )
		{ value = getMaxUpperBoundDirect ( expr.getNumber(), expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectExactCardinality& expr )
	{
		int m = expr.getNumber();
		const TDLObjectRoleExpression* R = expr.getOR();
		const TDLConceptExpression* C = expr.getC();

		// here the maximal value between Mix and Max is an answer. The -1 case will be dealt with automagically
		value = std::max ( getMinUpperBoundDirect ( m, R, C ), getMaxUpperBoundDirect ( m, R, C ) );
	}
	virtual void visit ( const TDLConceptDataValue& expr ) { value = isBotEquivalent(expr.getDR()); }
	virtual void visit ( const TDLConceptDataExists& expr )
		{ value = getMinUpperBoundDirect ( 1, expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataForall& expr )
		{ value = isTopEquivalent(expr.getDR()) && getLowerBoundComplement(expr.getExpr()) >= 1 ? 0 : -1; }
	virtual void visit ( const TDLConceptDataMinCardinality& expr )
		{ value = getMinUpperBoundDirect ( expr.getNumber(), expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataMaxCardinality& expr )
		{ value = getMaxUpperBoundDirect ( expr.getNumber(), expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataExactCardinality& expr )
	{
		int m = expr.getNumber();
		const TDLDataRoleExpression* R = expr.getDR();
		const TDLDataExpression* D = expr.getExpr();

		// here the maximal value between Mix and Max is an answer. The -1 case will be dealt with automagically
		value = std::max ( getMinUpperBoundDirect ( m, R, D ), getMaxUpperBoundDirect ( m, R, D ) );
	}

	// object role expressions
	virtual void visit ( const TDLObjectRoleTop& ) { value = -1; }
	virtual void visit ( const TDLObjectRoleBottom& ) { value = 0; }
	virtual void visit ( const TDLObjectRoleName& expr ) { value = botRLocal() && nc(expr.getEntity()) ? 0 : -1; }
	virtual void visit ( const TDLObjectRoleInverse& expr ) { value = getUpperBoundDirect(expr.getOR()); }
	virtual void visit ( const TDLObjectRoleChain& expr )
	{
		for ( TDLObjectRoleChain::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( isBotEquivalent(*p) )
			{
				value = 0;
				return;
			}
		value = -1;
	}
		// equivalent to R(x,y) and C(x), so copy behaviour from ER.X
	virtual void visit ( const TDLObjectRoleProjectionFrom& expr )
		{ value = getMinUpperBoundDirect ( 1, expr.getOR(), expr.getC() ); }
		// equivalent to R(x,y) and C(y), so copy behaviour from ER.X
	virtual void visit ( const TDLObjectRoleProjectionInto& expr )
		{ value = getMinUpperBoundDirect ( 1, expr.getOR(), expr.getC() ); }

	// data role expressions
	virtual void visit ( const TDLDataRoleTop& ) { value = -1; }
	virtual void visit ( const TDLDataRoleBottom& ) { value = 0; }
	virtual void visit ( const TDLDataRoleName& expr ) { value = botRLocal() && nc(expr.getEntity()) ? 0 : -1; }

	// data expressions
	virtual void visit ( const TDLDataTop& ) { value = -1; }
	virtual void visit ( const TDLDataBottom& ) { value = 0; }
	// FIXME!! not ready
	//virtual void visit ( const TDLDataTypeName& ) { isBotEq = false; }
	// FIXME!! not ready
	//virtual void visit ( const TDLDataTypeRestriction& ) { isBotEq = false; }
	virtual void visit ( const TDLDataValue& ) { value = 1;; }
	virtual void visit ( const TDLDataNot& expr ) { value = getUpperBoundComplement(expr.getExpr()); }
	virtual void visit ( const TDLDataAnd& expr )
	{
		int min = -1, n;
		for ( TDLDataAnd::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			n = getUpperBoundDirect(*p);
			if ( n != -1 )
				min = min == -1 ? n : std::min ( min, n );
		}
		value = min;
	}
	virtual void visit ( const TDLDataOr& expr )
	{
		int sum = 0, n;
		for ( TDLDataOr::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
		{
			n = getUpperBoundDirect(*p);
			if ( n == -1 )
			{
				value = -1;
				return;
			}
			sum += n;
		}
		value = n;
	}
	virtual void visit ( const TDLDataOneOf& expr ) { value = expr.size(); }
}; // UpperBoundDirectEvaluator

class UpperBoundComplementEvaluator: public CardinalityEvaluatorBase
{
public:		// interface
		/// init c'tor
	UpperBoundComplementEvaluator ( const TSignature* s ) : CardinalityEvaluatorBase(s) {}
		/// empty d'tor
	virtual ~UpperBoundComplementEvaluator ( void ) {}
}; // UpperBoundComplementEvaluator

class LowerBoundDirectEvaluator: public CardinalityEvaluatorBase
{
public:		// interface
		/// init c'tor
	LowerBoundDirectEvaluator ( const TSignature* s ) : CardinalityEvaluatorBase(s) {}
		/// empty d'tor
	virtual ~LowerBoundDirectEvaluator ( void ) {}
}; // LowerBoundDirectEvaluator

class LowerBoundComplementEvaluator: public CardinalityEvaluatorBase
{
public:		// interface
		/// init c'tor
	LowerBoundComplementEvaluator ( const TSignature* s ) : CardinalityEvaluatorBase(s) {}
		/// empty d'tor
	virtual ~LowerBoundComplementEvaluator ( void ) {}
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

#if 0
/// determine how many instances can an expression have
class UpperBoundDirectEvaluator: public CardinalityEvaluatorBase
{
protected:	// members
		/// corresponding complement evaluator
	UpperBoundComplementEvaluator* UpperCEval;

protected:	// methods
		/// check whether the expression is top-equivalent
	bool isTopEquivalent ( const TDLExpression& expr );
		/// convenience helper
	bool isTopEquivalent ( const TDLExpression* expr ) { return isTopEquivalent(*expr); }

	// non-empty Concept/Data expression

		/// @return true iff C^I is non-empty
	bool isBotDistinct ( const TDLExpression* C )
	{
		// TOP is non-empty
		if ( isTopEquivalent(C) )
			return true;
		// built-in DT are non-empty
		if ( dynamic_cast<const TDLDataTypeName*>(C) )
			return true;
		// FIXME!! that's it for now
		return false;
	}

	// cardinality of a concept/data expression interpretation

		/// @return true if #C^I > n
	bool isCardLargerThan ( const TDLExpression* C, unsigned int n )
	{
		if ( n == 0 )	// non-empty is enough
			return isBotDistinct(C);
		if ( const TDLDataTypeName* namedDT = dynamic_cast<const TDLDataTypeName*>(C) )
		{	// string/time are infinite DT
			std::string name = namedDT->getName();
			if ( name == TDataTypeManager::getStrTypeName() || name == TDataTypeManager::getTimeTypeName() )
				return true;
		}
		// FIXME!! try to be more precise
		return false;
	}

	// QCRs

		/// @return true iff (>= n R.C) is botEq
	bool isMinBotEquivalent ( unsigned int n, const TDLRoleExpression* R, const TDLExpression* C )
		{ return (n > 0) && (isBotEquivalent(R) || isBotEquivalent(C)); }
		/// @return true iff (<= n R.C) is botEq
	bool isMaxBotEquivalent ( unsigned int n, const TDLRoleExpression* R, const TDLExpression* C )
		{ return isTopEquivalent(R) && isCardLargerThan ( C, n ); }

public:		// interface
		/// init c'tor
	UpperBoundComplementEvaluator ( const TSignature* s ) : CardinalityEvaluatorBase {}
		/// empty d'tor
	virtual ~UpperBoundComplementEvaluator ( void ) {}

	// set fields

		/// set the corresponding top evaluator
	void setTopEval ( TopEquivalenceEvaluator* eval ) { TopEval = eval; }
		/// @return true iff an EXPRession is equivalent to bottom wrt defined policy
	bool isBotEquivalent ( const TDLExpression& expr )
	{
		expr.accept(*this);
		return isBotEq;
	}
		/// @return true iff an EXPRession is equivalent to bottom wrt defined policy
	bool isBotEquivalent ( const TDLExpression* expr ) { return isBotEquivalent(*expr); }

public:		// visitor interface
	// concept expressions
	virtual void visit ( const TDLConceptTop& ) { isBotEq = false; }
	virtual void visit ( const TDLConceptBottom& ) { isBotEq = true; }
	virtual void visit ( const TDLConceptName& expr ) { isBotEq = !topCLocal() && nc(expr.getEntity()); }
	virtual void visit ( const TDLConceptNot& expr ) { isBotEq = isTopEquivalent(expr.getC()); }
	virtual void visit ( const TDLConceptAnd& expr )
	{
		for ( TDLConceptAnd::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( isBotEquivalent(*p) )	// here isBotEq is true, so just return
				return;
		isBotEq = false;
	}
	virtual void visit ( const TDLConceptOr& expr )
	{
		for ( TDLConceptOr::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( !isBotEquivalent(*p) )	// here isBotEq is false, so just return
				return;
		isBotEq = true;
	}
	virtual void visit ( const TDLConceptOneOf& expr ) { isBotEq = expr.empty(); }
	virtual void visit ( const TDLConceptObjectSelf& expr ) { isBotEq = isBotEquivalent(expr.getOR()); }
	virtual void visit ( const TDLConceptObjectValue& expr ) { isBotEq = isBotEquivalent(expr.getOR()); }
	virtual void visit ( const TDLConceptObjectExists& expr )
		{ isBotEq = isMinBotEquivalent ( 1, expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectForall& expr )
		{ isBotEq = isTopEquivalent(expr.getOR()) && isBotEquivalent(expr.getC()); }
	virtual void visit ( const TDLConceptObjectMinCardinality& expr )
		{ isBotEq = isMinBotEquivalent ( expr.getNumber(), expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectMaxCardinality& expr )
		{ isBotEq = isMaxBotEquivalent ( expr.getNumber(), expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectExactCardinality& expr )
	{
		unsigned int n = expr.getNumber();
		const TDLObjectRoleExpression* R = expr.getOR();
		const TDLConceptExpression* C = expr.getC();
		isBotEq = isMinBotEquivalent ( n, R, C ) || isMaxBotEquivalent ( n, R, C );
	}
	virtual void visit ( const TDLConceptDataValue& expr )
		{ isBotEq = isBotEquivalent(expr.getDR()); }
	virtual void visit ( const TDLConceptDataExists& expr )
		{ isBotEq = isMinBotEquivalent ( 1, expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataForall& expr )
		{ isBotEq = isTopEquivalent(expr.getDR()) && !isTopEquivalent(expr.getExpr()); }
	virtual void visit ( const TDLConceptDataMinCardinality& expr )
		{ isBotEq = isMinBotEquivalent ( expr.getNumber(), expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataMaxCardinality& expr )
		{ isBotEq = isMaxBotEquivalent ( expr.getNumber(), expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataExactCardinality& expr )
	{
		unsigned int n = expr.getNumber();
		const TDLDataRoleExpression* R = expr.getDR();
		const TDLDataExpression* D = expr.getExpr();
		isBotEq = isMinBotEquivalent ( n, R, D ) || isMaxBotEquivalent ( n, R, D );
	}

	// object role expressions
	virtual void visit ( const TDLObjectRoleTop& ) { isBotEq = false; }
	virtual void visit ( const TDLObjectRoleBottom& ) { isBotEq = true; }
	virtual void visit ( const TDLObjectRoleName& expr ) { isBotEq = !topRLocal() && nc(expr.getEntity()); }
	virtual void visit ( const TDLObjectRoleInverse& expr ) { isBotEq = isBotEquivalent(expr.getOR()); }
	virtual void visit ( const TDLObjectRoleChain& expr )
	{
		isBotEq = true;
		for ( TDLObjectRoleChain::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( isBotEquivalent(*p) )	// isBotEq is true here
				return;
		isBotEq = false;
	}
		// equivalent to R(x,y) and C(x), so copy behaviour from ER.X
	virtual void visit ( const TDLObjectRoleProjectionFrom& expr )
		{ isBotEq = isMinBotEquivalent ( 1, expr.getOR(), expr.getC() ); }
		// equivalent to R(x,y) and C(y), so copy behaviour from ER.X
	virtual void visit ( const TDLObjectRoleProjectionInto& expr )
		{ isBotEq = isMinBotEquivalent ( 1, expr.getOR(), expr.getC() ); }

	// data role expressions
	virtual void visit ( const TDLDataRoleTop& ) { isBotEq = false; }
	virtual void visit ( const TDLDataRoleBottom& ) { isBotEq = true; }
	virtual void visit ( const TDLDataRoleName& expr ) { isBotEq = !topRLocal() && nc(expr.getEntity()); }

	// data expressions
	virtual void visit ( const TDLDataTop& ) { isBotEq = false; }
	virtual void visit ( const TDLDataBottom& ) { isBotEq = true; }
	virtual void visit ( const TDLDataTypeName& ) { isBotEq = false; }
	virtual void visit ( const TDLDataTypeRestriction& ) { isBotEq = false; }
	virtual void visit ( const TDLDataValue& ) { isBotEq = false; }
	virtual void visit ( const TDLDataNot& expr ) { isBotEq = isTopEquivalent(expr.getExpr()); }
	virtual void visit ( const TDLDataAnd& expr )
	{
		for ( TDLDataAnd::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( isBotEquivalent(*p) )	// here isBotEq is true, so just return
				return;
		isBotEq = false;
	}
	virtual void visit ( const TDLDataOr& expr )
	{
		for ( TDLDataOr::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( !isBotEquivalent(*p) )	// here isBotEq is false, so just return
				return;
		isBotEq = true;
	}
	virtual void visit ( const TDLDataOneOf& expr ) { isBotEq = expr.empty(); }
}; // UpperBoundComplementEvaluator

/// check whether class expressions are equivalent to top wrt given locality class
class TopEquivalenceEvaluator: protected SigAccessor, public DLExpressionVisitorEmpty
{
protected:	// members
		/// corresponding bottom evaluator
	BotEquivalenceEvaluator* BotEval;
		/// keep the value here
	bool isTopEq;

protected:	// methods
		/// check whether the expression is top-equivalent
	bool isBotEquivalent ( const TDLExpression& expr ) { return BotEval->isBotEquivalent(expr); }
		/// convenience helper
	bool isBotEquivalent ( const TDLExpression* expr ) { return isBotEquivalent(*expr); }

	// non-empty Concept/Data expression

		/// @return true iff C^I is non-empty
	bool isBotDistinct ( const TDLExpression* C )
	{
		// TOP is non-empty
		if ( isTopEquivalent(C) )
			return true;
		// built-in DT are non-empty
		if ( dynamic_cast<const TDLDataTypeName*>(C) )
			return true;
		// FIXME!! that's it for now
		return false;
	}

	// cardinality of a concept/data expression interpretation

		/// @return true if #C^I > n
	bool isCardLargerThan ( const TDLExpression* C, unsigned int n )
	{
		if ( n == 0 )	// non-empty is enough
			return isBotDistinct(C);
		if ( dynamic_cast<const TDLDataExpression*>(C) && isTopEquivalent(C) )
			return true;
		if ( const TDLDataTypeName* namedDT = dynamic_cast<const TDLDataTypeName*>(C) )
		{	// string/time are infinite DT
			std::string name = namedDT->getName();
			if ( name == TDataTypeManager::getStrTypeName() || name == TDataTypeManager::getTimeTypeName() )
				return true;
		}
		// FIXME!! try to be more precise
		return false;
	}

	// QCRs

		/// @return true iff (>= n R.C) is topEq
	bool isMinTopEquivalent ( unsigned int n, const TDLRoleExpression* R, const TDLExpression* C )
		{ return (n == 0) || ( isTopEquivalent(R) && isCardLargerThan ( C, n-1 ) ); }
		/// @return true iff (<= n R.C) is topEq
	bool isMaxTopEquivalent ( unsigned int n ATTR_UNUSED, const TDLRoleExpression* R, const TDLExpression* C )
		{ return isBotEquivalent(R) || isBotEquivalent(C); }

public:		// interface
		/// init c'tor
	TopEquivalenceEvaluator ( const TSignature* s ) : SigAccessor(s), isTopEq(false) {}
		/// empty d'tor
	virtual ~TopEquivalenceEvaluator ( void ) {}

	// set fields

		/// set the corresponding bottom evaluator
	void setBotEval ( BotEquivalenceEvaluator* eval ) { BotEval = eval; }
		/// @return true iff an EXPRession is equivalent to top wrt defined policy
	bool isTopEquivalent ( const TDLExpression& expr )
	{
		expr.accept(*this);
		return isTopEq;
	}
		/// @return true iff an EXPRession is equivalent to top wrt defined policy
	bool isTopEquivalent ( const TDLExpression* expr ) { return isTopEquivalent(*expr); }

public:		// visitor interface
	// concept expressions
	virtual void visit ( const TDLConceptTop& ) { isTopEq = true; }
	virtual void visit ( const TDLConceptBottom& ) { isTopEq = false; }
	virtual void visit ( const TDLConceptName& expr ) { isTopEq = topCLocal() && nc(expr.getEntity()); }
	virtual void visit ( const TDLConceptNot& expr ) { isTopEq = isBotEquivalent(expr.getC()); }
	virtual void visit ( const TDLConceptAnd& expr )
	{
		for ( TDLConceptAnd::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( !isTopEquivalent(*p) )	// here isTopEq is false, so just return
				return;
		isTopEq = true;
	}
	virtual void visit ( const TDLConceptOr& expr )
	{
		for ( TDLConceptOr::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( isTopEquivalent(*p) )	// here isTopEq is true, so just return
				return;
		isTopEq = false;
	}
	virtual void visit ( const TDLConceptOneOf& ) { isTopEq = false; }
	virtual void visit ( const TDLConceptObjectSelf& expr ) { isTopEq = isTopEquivalent(expr.getOR()); }
	virtual void visit ( const TDLConceptObjectValue& expr ) { isTopEq = isTopEquivalent(expr.getOR()); }
	virtual void visit ( const TDLConceptObjectExists& expr )
		{ isTopEq = isMinTopEquivalent ( 1, expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectForall& expr )
		{ isTopEq = isTopEquivalent(expr.getC()) || isBotEquivalent(expr.getOR()); }
	virtual void visit ( const TDLConceptObjectMinCardinality& expr )
		{ isTopEq = isMinTopEquivalent ( expr.getNumber(), expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectMaxCardinality& expr )
		{ isTopEq = isMaxTopEquivalent ( expr.getNumber(), expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectExactCardinality& expr )
	{
		unsigned int n = expr.getNumber();
		const TDLObjectRoleExpression* R = expr.getOR();
		const TDLConceptExpression* C = expr.getC();
		isTopEq = isMinTopEquivalent ( n, R, C ) && isMaxTopEquivalent ( n, R, C );
	}
	virtual void visit ( const TDLConceptDataValue& expr )
		{ isTopEq = isTopEquivalent(expr.getDR()); }
	virtual void visit ( const TDLConceptDataExists& expr )
		{ isTopEq = isMinTopEquivalent ( 1, expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataForall& expr ) { isTopEq = isTopEquivalent(expr.getExpr()) || isBotEquivalent(expr.getDR()); }
	virtual void visit ( const TDLConceptDataMinCardinality& expr )
		{ isTopEq = isMinTopEquivalent ( expr.getNumber(), expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataMaxCardinality& expr )
		{ isTopEq = isMaxTopEquivalent ( expr.getNumber(), expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataExactCardinality& expr )
	{
		unsigned int n = expr.getNumber();
		const TDLDataRoleExpression* R = expr.getDR();
		const TDLDataExpression* D = expr.getExpr();
		isTopEq = isMinTopEquivalent ( n, R, D ) && isMaxTopEquivalent ( n, R, D );
	}

	// object role expressions
	virtual void visit ( const TDLObjectRoleTop& ) { isTopEq = true; }
	virtual void visit ( const TDLObjectRoleBottom& ) { isTopEq = false; }
	virtual void visit ( const TDLObjectRoleName& expr ) { isTopEq = topRLocal() && nc(expr.getEntity()); }
	virtual void visit ( const TDLObjectRoleInverse& expr ) { isTopEq = isTopEquivalent(expr.getOR()); }
	virtual void visit ( const TDLObjectRoleChain& expr )
	{
		isTopEq = false;
		for ( TDLObjectRoleChain::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( !isTopEquivalent(*p) )	// isTopEq is false here
				return;
		isTopEq = true;
	}
		// equivalent to R(x,y) and C(x), so copy behaviour from ER.X
	virtual void visit ( const TDLObjectRoleProjectionFrom& expr )
		{ isTopEq = isMinTopEquivalent ( 1, expr.getOR(), expr.getC() ); }
		// equivalent to R(x,y) and C(y), so copy behaviour from ER.X
	virtual void visit ( const TDLObjectRoleProjectionInto& expr )
		{ isTopEq = isMinTopEquivalent ( 1, expr.getOR(), expr.getC() ); }

	// data role expressions
	virtual void visit ( const TDLDataRoleTop& ) { isTopEq = true; }
	virtual void visit ( const TDLDataRoleBottom& ) { isTopEq = false; }
	virtual void visit ( const TDLDataRoleName& expr ) { isTopEq = topRLocal() && nc(expr.getEntity()); }

	// data expressions
	virtual void visit ( const TDLDataTop& ) { isTopEq = true; }
	virtual void visit ( const TDLDataBottom& ) { isTopEq = false; }
	virtual void visit ( const TDLDataTypeName& ) { isTopEq = false; }
	virtual void visit ( const TDLDataTypeRestriction& ) { isTopEq = false; }
	virtual void visit ( const TDLDataValue& ) { isTopEq = false; }
	virtual void visit ( const TDLDataNot& expr ) { isTopEq = isBotEquivalent(expr.getExpr()); }
	virtual void visit ( const TDLDataAnd& expr )
	{
		for ( TDLDataAnd::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( !isTopEquivalent(*p) )	// here isTopEq is false, so just return
				return;
		isTopEq = true;
	}
	virtual void visit ( const TDLDataOr& expr )
	{
		for ( TDLDataOr::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( isTopEquivalent(*p) )	// here isTopEq is true, so just return
				return;
		isTopEq = false;
	}
	virtual void visit ( const TDLDataOneOf& ) { isTopEq = false; }
}; // TopEquivalenceEvaluator

inline bool
BotEquivalenceEvaluator :: isTopEquivalent ( const TDLExpression& expr )
{
	return TopEval->isTopEquivalent(expr);
}

#endif

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
