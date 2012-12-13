/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2011-2012 by Dmitry Tsarkov

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

#ifndef SYNLOCCHECKER_H
#define SYNLOCCHECKER_H

#include "LocalityChecker.h"

// forward declarations
class BotEquivalenceEvaluator;
class TopEquivalenceEvaluator;

	/// @return true iff EXPR is a top datatype
inline bool
isTopDataType ( const TDLExpression* expr ) { return dynamic_cast<const TDLDataTop*>(expr) != NULL; }
	/// @return true iff EXPR is a top datatype or a built-in datatype;
inline bool
isTopOrBuiltInDataType ( const TDLExpression* expr )
	{ return isTopDataType(expr) || dynamic_cast<const TDLDataTypeName*>(expr) != NULL; }
	/// @return true iff EXPR is a top datatype or an infinite built-in datatype; FIXME add real/fraction later
inline bool
isTopOrBuiltInInfDataType ( const TDLExpression* expr )
{
	if ( isTopDataType(expr) )
		return true;
	if ( const TDLDataTypeName* namedDT = dynamic_cast<const TDLDataTypeName*>(expr) )
	{
		std::string name = namedDT->getName();
		if ( name == TDataTypeManager::getStrTypeName() || name == TDataTypeManager::getTimeTypeName() )
			return true;
	}
	return false;
}

/// check whether class expressions are equivalent to bottom wrt given locality class
class BotEquivalenceEvaluator: protected SigAccessor, public DLExpressionVisitorEmpty
{
protected:	// members
		/// corresponding top evaluator
	TopEquivalenceEvaluator* TopEval;
		/// keep the value here
	bool isBotEq;

protected:	// methods
		/// check whether the expression is top-equivalent
	bool isTopEquivalent ( const TDLExpression& expr );
		/// convinience helper
	bool isTopEquivalent ( const TDLExpression* expr ) { return isTopEquivalent(*expr); }

	// object role QCRs

		/// @return true iff (>= n R.C) is botEq
	bool isMinBotEquivalent ( unsigned int n, const TDLObjectRoleExpression* R, const TDLConceptExpression* C )
		{ return (n > 0) && (isBotEquivalent(R) || isBotEquivalent(C)); }
		/// @return true iff (<= n R.C) is botEq
	bool isMaxBotEquivalent ( unsigned int n, const TDLObjectRoleExpression* R, const TDLConceptExpression* C )
		{ return (n > 0) && isTopEquivalent(R) && isTopEquivalent(C); }

	// data role QCRs

		/// @return true iff (>= n R.D) is botEq
	bool isMinBotEquivalent ( unsigned int n, const TDLDataRoleExpression* R, const TDLDataExpression* D ATTR_UNUSED )
		{ return (n > 0) && isBotEquivalent(R); }
		/// @return true iff (<= n R.D) is botEq
	bool isMaxBotEquivalent ( unsigned int n, const TDLDataRoleExpression* R, const TDLDataExpression* D )
	{
		return isTopEquivalent(R) &&
			( (n <= 1) ? isTopOrBuiltInDataType(D) : isTopOrBuiltInInfDataType(D) );
	}

public:		// interface
		/// init c'tor
	BotEquivalenceEvaluator ( const TSignature* s ) : SigAccessor(s), isBotEq(false) {}
		/// empty d'tor
	virtual ~BotEquivalenceEvaluator ( void ) {}

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
	virtual void visit ( const TDLConceptTop& expr ATTR_UNUSED ) { isBotEq = false; }
	virtual void visit ( const TDLConceptBottom& expr ATTR_UNUSED ) { isBotEq = true; }
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
		for ( TDLConceptAnd::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( !isBotEquivalent(*p) )	// here isBotEq is false, so just return
				return;
		isBotEq = true;
	}
	virtual void visit ( const TDLConceptOneOf& expr ) { isBotEq = expr.empty(); }
	virtual void visit ( const TDLConceptObjectSelf& expr ) { isBotEq = isBotEquivalent(expr.getOR()); }
	virtual void visit ( const TDLConceptObjectValue& expr ) { isBotEq = isBotEquivalent(expr.getOR()); }
	virtual void visit ( const TDLConceptObjectExists& expr )
		{ isBotEq = isBotEquivalent(expr.getC()) || isBotEquivalent(expr.getOR()); }
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
	virtual void visit ( const TDLConceptDataValue& expr ) { isBotEq = isBotEquivalent(expr.getDR()); }
	virtual void visit ( const TDLConceptDataExists& expr ) { isBotEq = isBotEquivalent(expr.getDR()); }
	virtual void visit ( const TDLConceptDataForall& expr ) { isBotEq = isTopEquivalent(expr.getDR()) && !isTopDataType(expr.getExpr()); }
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
	virtual void visit ( const TDLObjectRoleTop& expr ATTR_UNUSED ) { isBotEq = false; }
	virtual void visit ( const TDLObjectRoleBottom& expr ATTR_UNUSED ) { isBotEq = true; }
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
		{ isBotEq = isBotEquivalent(expr.getC()) || isBotEquivalent(expr.getOR()); }
		// equivalent to R(x,y) and C(y), so copy behaviour from ER.X
	virtual void visit ( const TDLObjectRoleProjectionInto& expr )
		{ isBotEq = isBotEquivalent(expr.getC()) || isBotEquivalent(expr.getOR()); }

	// data role expressions
	virtual void visit ( const TDLDataRoleTop& expr ATTR_UNUSED ) { isBotEq = false; }
	virtual void visit ( const TDLDataRoleBottom& expr ATTR_UNUSED ) { isBotEq = true; }
	virtual void visit ( const TDLDataRoleName& expr ) { isBotEq = !topRLocal() && nc(expr.getEntity()); }
}; // BotEquivalenceEvaluator

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
		/// convinience helper
	bool isBotEquivalent ( const TDLExpression* expr ) { return isBotEquivalent(*expr); }

	// object role QCRs

		/// @return true iff (>= n R.C) is topEq
	bool isMinTopEquivalent ( unsigned int n, const TDLObjectRoleExpression* R, const TDLConceptExpression* C )
		{ return n == 0 || ( (n == 1) && isTopEquivalent(R) && isTopEquivalent(C) ); }
		/// @return true iff (<= n R.C) is topEq
	bool isMaxTopEquivalent ( unsigned int n ATTR_UNUSED, const TDLObjectRoleExpression* R, const TDLConceptExpression* C )
		{ return isBotEquivalent(C) || isBotEquivalent(R); }

	// data role QCRs

		/// @return true iff (>= n R.D) is topEq
	bool isMinTopEquivalent ( unsigned int n, const TDLDataRoleExpression* R, const TDLDataExpression* D )
	{
		return (n == 0) || ( isTopEquivalent(R) &&
				(n == 1) ? isTopOrBuiltInDataType(D) : isTopOrBuiltInInfDataType(D) );
	}
		/// @return true iff (<= n R.D) is topEq
	bool isMaxTopEquivalent ( unsigned int n ATTR_UNUSED, const TDLDataRoleExpression* R, const TDLDataExpression* D ATTR_UNUSED )
		{ return isBotEquivalent(R); }

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
	virtual void visit ( const TDLConceptTop& expr ATTR_UNUSED ) { isTopEq = true; }
	virtual void visit ( const TDLConceptBottom& expr ATTR_UNUSED ) { isTopEq = false; }
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
	virtual void visit ( const TDLConceptOneOf& expr ATTR_UNUSED ) { isTopEq = false; }
	virtual void visit ( const TDLConceptObjectSelf& expr ) { isTopEq = isTopEquivalent(expr.getOR()); }
	virtual void visit ( const TDLConceptObjectValue& expr ) { isTopEq = isTopEquivalent(expr.getOR()); }
	virtual void visit ( const TDLConceptObjectExists& expr )
		{ isTopEq = isTopEquivalent(expr.getOR()) && isTopEquivalent(expr.getC()); }
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
	virtual void visit ( const TDLConceptDataValue& expr ) { isTopEq = isTopEquivalent(expr.getDR()); }
	virtual void visit ( const TDLConceptDataExists& expr ) { isTopEq = isTopEquivalent(expr.getDR()) && isTopOrBuiltInDataType(expr.getExpr()); }
	virtual void visit ( const TDLConceptDataForall& expr ) { isTopEq = isTopDataType(expr.getExpr()) || isBotEquivalent(expr.getDR()); }
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
	virtual void visit ( const TDLObjectRoleTop& expr ATTR_UNUSED ) { isTopEq = true; }
	virtual void visit ( const TDLObjectRoleBottom& expr ATTR_UNUSED ) { isTopEq = false; }
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
		{ isTopEq = isTopEquivalent(expr.getOR()) && isTopEquivalent(expr.getC()); }
		// equivalent to R(x,y) and C(y), so copy behaviour from ER.X
	virtual void visit ( const TDLObjectRoleProjectionInto& expr )
		{ isTopEq = isTopEquivalent(expr.getOR()) && isTopEquivalent(expr.getC()); }

	// data role expressions
	virtual void visit ( const TDLDataRoleTop& expr ATTR_UNUSED ) { isTopEq = true; }
	virtual void visit ( const TDLDataRoleBottom& expr ATTR_UNUSED ) { isTopEq = false; }
	virtual void visit ( const TDLDataRoleName& expr ) { isTopEq = topRLocal() && nc(expr.getEntity()); }
}; // TopEquivalenceEvaluator

inline bool
BotEquivalenceEvaluator :: isTopEquivalent ( const TDLExpression& expr )
{
	return TopEval->isTopEquivalent(expr);
}


/// syntactic locality checker for DL axioms
class SyntacticLocalityChecker: public LocalityChecker
{
protected:	// members
		/// top evaluator
	TopEquivalenceEvaluator TopEval;
		/// bottom evaluator
	BotEquivalenceEvaluator BotEval;

protected:	// methods
		/// @return true iff EXPR is top equivalent
	bool isTopEquivalent ( const TDLExpression* expr ) { return TopEval.isTopEquivalent(*expr); }
		/// @return true iff EXPR is bottom equivalent
	bool isBotEquivalent ( const TDLExpression* expr ) { return BotEval.isBotEquivalent(*expr); }
		/// @return true iff role expression in equivalent to const wrt locality
	bool isREquivalent ( const TDLExpression* expr ) { return topRLocal() ? isTopEquivalent(expr) : isBotEquivalent(expr); }

public:		// interface
		/// init c'tor
	SyntacticLocalityChecker ( const TSignature* s )
		: LocalityChecker(s)
		, TopEval(s)
		, BotEval(s)
	{
		TopEval.setBotEval(&BotEval);
		BotEval.setTopEval(&TopEval);
	}
		/// empty d'tor
	virtual ~SyntacticLocalityChecker ( void ) {}

public:		// visitor interface
	virtual void visit ( const TDLAxiomDeclaration& axiom ATTR_UNUSED ) { isLocal = true; }

	virtual void visit ( const TDLAxiomEquivalentConcepts& axiom )
	{
		// 1 element => local
		if ( axiom.size() == 1 )
		{
			isLocal = true;
			return;
		}

		// axiom is local iff all the classes are either top- or bot-local
		isLocal = false;
		TDLAxiomEquivalentConcepts::iterator p = axiom.begin(), p_end = axiom.end();
		if ( isBotEquivalent(*p) )
		{
			for ( ++p; p != p_end; ++p )
				if ( !isBotEquivalent(*p) )
					return;
		}
		else
		{
			if ( !isTopEquivalent(*p) )
				return;
			for ( ++p; p != p_end; ++p )
				if ( !isTopEquivalent(*p) )
					return;
		}

		isLocal = true;
	}
	virtual void visit ( const TDLAxiomDisjointConcepts& axiom )
	{
		// local iff at most 1 concept is not bot-equiv
		bool hasNBE = false;
		isLocal = true;
		for ( TDLAxiomDisjointConcepts::iterator p = axiom.begin(), p_end = axiom.end(); p != p_end; ++p )
			if ( !isBotEquivalent(*p) )
			{
				if ( hasNBE )
				{
					isLocal = false;
					break;
				}
				else
					hasNBE = true;
			}
	}
	virtual void visit ( const TDLAxiomDisjointUnion& axiom )
	{
		isLocal = false;
		bool topLoc = topCLocal();
		if ( !(topLoc ? isTopEquivalent(axiom.getC()) : isBotEquivalent(axiom.getC())) )
			return;
		bool topEqDesc = false;
		for ( TDLAxiomDisjointUnion::iterator p = axiom.begin(), p_end = axiom.end(); p != p_end; ++p )
			if ( !isBotEquivalent(*p) )
			{
				if ( !topLoc )
					return;	// non-local straight away
				if ( isTopEquivalent(*p) )
				{
					if ( topEqDesc )
						return;	// 2nd top in there -- non-local
					else
						topEqDesc = true;
				}
				else
					return;	// non-local
			}

		isLocal = true;
	}
	virtual void visit ( const TDLAxiomEquivalentORoles& axiom )
	{
		isLocal = true;
		if ( axiom.size() <= 1 )
			return;
		for ( TDLAxiomEquivalentORoles::iterator p = axiom.begin(), p_end = axiom.end(); p != p_end; ++p )
			if ( !isREquivalent(*p) )
			{
				isLocal = false;
				break;
			}
	}
	virtual void visit ( const TDLAxiomEquivalentDRoles& axiom )
	{
		isLocal = true;
		if ( axiom.size() <= 1 )
			return;
		for ( TDLAxiomEquivalentDRoles::iterator p = axiom.begin(), p_end = axiom.end(); p != p_end; ++p )
			if ( !isREquivalent(*p) )
			{
				isLocal = false;
				break;
			}
	}
	virtual void visit ( const TDLAxiomDisjointORoles& axiom )
	{
		isLocal = false;
		if ( topRLocal() )
			return;

		bool hasNBE = false;
		for ( TDLAxiomDisjointORoles::iterator p = axiom.begin(), p_end = axiom.end(); p != p_end; ++p )
			if ( !isREquivalent(*p) )
			{
				if ( hasNBE )
					return;	// false here
				else
					hasNBE = true;
			}

		isLocal = true;
	}
	virtual void visit ( const TDLAxiomDisjointDRoles& axiom )
	{
		isLocal = false;
		if ( topRLocal() )
			return;

		bool hasNBE = false;
		for ( TDLAxiomDisjointDRoles::iterator p = axiom.begin(), p_end = axiom.end(); p != p_end; ++p )
			if ( !isREquivalent(*p) )
			{
				if ( hasNBE )
					return;	// false here
				else
					hasNBE = true;
			}

		isLocal = true;
	}
	virtual void visit ( const TDLAxiomSameIndividuals& axiom ATTR_UNUSED ) { isLocal = false; }
	virtual void visit ( const TDLAxiomDifferentIndividuals& axiom ATTR_UNUSED ) { isLocal = false; }
		/// there is no such axiom in OWL API, but I hope nobody would use Fairness here
	virtual void visit ( const TDLAxiomFairnessConstraint& axiom ATTR_UNUSED ) { isLocal = true; }

	virtual void visit ( const TDLAxiomRoleInverse& axiom ) { isLocal = isREquivalent(axiom.getRole()) && isREquivalent(axiom.getInvRole()); }
	virtual void visit ( const TDLAxiomORoleSubsumption& axiom ) { isLocal = isTopEquivalent(axiom.getRole()) || isBotEquivalent(axiom.getSubRole()); }
	virtual void visit ( const TDLAxiomDRoleSubsumption& axiom ) { isLocal = isTopEquivalent(axiom.getRole()) || isBotEquivalent(axiom.getSubRole()); }
	virtual void visit ( const TDLAxiomORoleDomain& axiom )
		{ isLocal = isTopEquivalent(axiom.getDomain()) || isBotEquivalent(axiom.getRole()); }
	virtual void visit ( const TDLAxiomDRoleDomain& axiom )
		{ isLocal = isTopEquivalent(axiom.getDomain()) || isBotEquivalent(axiom.getRole()); }
	virtual void visit ( const TDLAxiomORoleRange& axiom )
		{ isLocal = isTopEquivalent(axiom.getRange()) || isBotEquivalent(axiom.getRole()); }
	virtual void visit ( const TDLAxiomDRoleRange& axiom )
		{ isLocal = isTopDataType(axiom.getRange()) || isBotEquivalent(axiom.getRole()); }
	virtual void visit ( const TDLAxiomRoleTransitive& axiom ) { isLocal = isREquivalent(axiom.getRole()); }
		/// as BotRole is irreflexive, the only local axiom is topEquivalent(R)
	virtual void visit ( const TDLAxiomRoleReflexive& axiom ) { isLocal = isTopEquivalent(axiom.getRole()); }
	virtual void visit ( const TDLAxiomRoleIrreflexive& axiom ) { isLocal = isBotEquivalent(axiom.getRole()); }
	virtual void visit ( const TDLAxiomRoleSymmetric& axiom ) { isLocal = isREquivalent(axiom.getRole()); }
	virtual void visit ( const TDLAxiomRoleAsymmetric& axiom ) { isLocal = isBotEquivalent(axiom.getRole()); }
	virtual void visit ( const TDLAxiomORoleFunctional& axiom ) { isLocal = isBotEquivalent(axiom.getRole()); }
	virtual void visit ( const TDLAxiomDRoleFunctional& axiom ) { isLocal = isBotEquivalent(axiom.getRole()); }
	virtual void visit ( const TDLAxiomRoleInverseFunctional& axiom ) { isLocal = isBotEquivalent(axiom.getRole()); }

	virtual void visit ( const TDLAxiomConceptInclusion& axiom ) { isLocal = isBotEquivalent(axiom.getSubC()) || isTopEquivalent(axiom.getSupC()); }
	virtual void visit ( const TDLAxiomInstanceOf& axiom ) { isLocal = isTopEquivalent(axiom.getC()); }
	virtual void visit ( const TDLAxiomRelatedTo& axiom ) { isLocal = isTopEquivalent(axiom.getRelation()); }
	virtual void visit ( const TDLAxiomRelatedToNot& axiom ) { isLocal = isBotEquivalent(axiom.getRelation()); }
	virtual void visit ( const TDLAxiomValueOf& axiom ) { isLocal = isTopEquivalent(axiom.getAttribute()); }
	virtual void visit ( const TDLAxiomValueOfNot& axiom ) { isLocal = isBotEquivalent(axiom.getAttribute()); }
}; // SyntacticLocalityChecker

#endif
