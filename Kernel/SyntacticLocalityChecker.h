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

#ifndef SYNLOCCHECKER_H
#define SYNLOCCHECKER_H

#include "tSignature.h"

// forward declarations
class BotEquivalenceEvaluator;
class TopEquivalenceEvaluator;

/// helper class to set signature and locality class
class SigAccessor
{
protected:	// members
		/// signature of a module
	const TSignature* sig;

public:		// interface
		/// init c'tor
	SigAccessor ( const TSignature* s ) : sig(s) {}
		/// empty d'tor
	virtual ~SigAccessor ( void ) {}

		/// @return true iff concepts are treated as TOPs
	bool topCLocal ( void ) const { return sig->topCLocal(); }
		/// @return true iff roles are treated as TOPs
	bool topRLocal ( void ) const { return sig->topRLocal(); }
		/// @return true iff SIGnature contains given expression
	bool co ( const TDLExpression* expr ) const { return sig->contains(expr); }
		/// @return true iff SIGnature does NOT contain given expression
	bool nc ( const TNamedEntity* expr ) const { return !sig->contains(expr); }
		/// @return true iff SIGnature does NOT contain given expression
	bool nc ( const TDLExpression* expr ) const { return !sig->contains(expr); }

		/// @return true iff EXPR is a top datatype
	static bool isTopDT ( const TDLExpression* expr ) { return dynamic_cast<const TDLDataTop*>(expr) != NULL; }
		/// @return true iff EXPR is a top datatype or a built-in datatype; FIXME for now -- just top
	static bool isTopOrBuiltInDT ( const TDLExpression* expr ) { return isTopDT(expr); }
		/// @return true iff EXPR is a top datatype or an infinite built-in datatype; FIXME for now -- just top
	static bool isTopOrBuiltInInfDT ( const TDLExpression* expr ) { return isTopDT(expr); }
}; // SigAccessor

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

public:		// interface
		/// init c'tor
	BotEquivalenceEvaluator ( const TSignature* s ) : SigAccessor(s) {}
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
	virtual void visit ( const TDLConceptObjectSelf& expr ) { isBotEq = !topRLocal() && nc(expr.getOR()); }
	virtual void visit ( const TDLConceptObjectValue& expr ) { isBotEq = !topRLocal() && nc(expr.getOR()); }
	virtual void visit ( const TDLConceptObjectExists& expr )
	{
		isBotEq = isBotEquivalent(expr.getC());
		if ( !topRLocal() )
			isBotEq |= nc(expr.getOR());
	}
	virtual void visit ( const TDLConceptObjectForall& expr ) { isBotEq = topRLocal() && nc(expr.getOR()) && isBotEquivalent(expr.getC()); }
	virtual void visit ( const TDLConceptObjectMinCardinality& expr )
		{ isBotEq = expr.getNumber() > 0 && (isBotEquivalent(expr.getC()) || (!topRLocal() && nc(expr.getOR())) ); }
	virtual void visit ( const TDLConceptObjectMaxCardinality& expr )
		{ isBotEq = topRLocal() && (expr.getNumber() > 0) && nc(expr.getOR()) && isTopEquivalent(expr.getC()); }
	virtual void visit ( const TDLConceptObjectExactCardinality& expr )
	{
		isBotEq = expr.getNumber() > 0 &&
			( isBotEquivalent(expr.getC()) || ( nc(expr.getOR()) && (topRLocal() ? isTopEquivalent(expr.getC()) : true) ) );
	}
	virtual void visit ( const TDLConceptDataValue& expr ) { isBotEq = !topRLocal() && nc(expr.getDR()); }
	virtual void visit ( const TDLConceptDataExists& expr ) { isBotEq = !topRLocal() && nc(expr.getDR()); }
	virtual void visit ( const TDLConceptDataForall& expr ) { isBotEq = topRLocal() && nc(expr.getDR()) && !isTopDT(expr.getExpr()); }
	virtual void visit ( const TDLConceptDataMinCardinality& expr )
		{ isBotEq = !topRLocal() && (expr.getNumber() > 0) && nc(expr.getDR()); }
	virtual void visit ( const TDLConceptDataMaxCardinality& expr )
	{
		isBotEq = topRLocal() && nc(expr.getDR()) &&
			( expr.getNumber() <= 1 ? isTopOrBuiltInDT(expr.getExpr()) : isTopOrBuiltInInfDT(expr.getExpr()) );
	}
	virtual void visit ( const TDLConceptDataExactCardinality& expr )
	{
		isBotEq = nc(expr.getDR()) && ( topRLocal()
			? ( expr.getNumber() == 0
				? isTopOrBuiltInDT(expr.getExpr())
				: isTopOrBuiltInInfDT(expr.getExpr()) )
			: expr.getNumber() > 0 );
	}
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

public:		// interface
		/// init c'tor
	TopEquivalenceEvaluator ( const TSignature* s ) : SigAccessor(s) {}
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
	virtual void visit ( const TDLConceptObjectSelf& expr ) { isTopEq = topRLocal() && nc(expr.getOR()); }
	virtual void visit ( const TDLConceptObjectValue& expr ) { isTopEq = topRLocal() && nc(expr.getOR()); }
	virtual void visit ( const TDLConceptObjectExists& expr )
		{ isTopEq = topRLocal() && nc(expr.getOR()) && isTopEquivalent(expr.getC()); }
	virtual void visit ( const TDLConceptObjectForall& expr )
		{ isTopEq = isTopEquivalent(expr.getC()) || (!topRLocal() && nc(expr.getOR())); }
	virtual void visit ( const TDLConceptObjectMinCardinality& expr )
		{ isTopEq = expr.getNumber() == 0 || (topRLocal() && nc(expr.getOR()) && isTopEquivalent(expr.getC())); }
	virtual void visit ( const TDLConceptObjectMaxCardinality& expr )
		{ isTopEq = isBotEquivalent(expr.getC()) || (!topRLocal() && nc(expr.getOR())); }
	virtual void visit ( const TDLConceptObjectExactCardinality& expr )
	{
		isTopEq = expr.getNumber() == 0 &&
			( isBotEquivalent(expr.getC()) || (!topRLocal() && nc(expr.getOR())) );
	}
	virtual void visit ( const TDLConceptDataValue& expr ) { isTopEq = topRLocal() && nc(expr.getDR()); }
	virtual void visit ( const TDLConceptDataExists& expr ) { isTopEq = topRLocal() && nc(expr.getDR()) && isTopOrBuiltInDT(expr.getExpr()); }
	virtual void visit ( const TDLConceptDataForall& expr ) { isTopEq = isTopDT(expr.getExpr()) || (!topRLocal() && nc(expr.getDR())); }
	virtual void visit ( const TDLConceptDataMinCardinality& expr )
	{
		isTopEq = expr.getNumber() == 0;
		if ( topRLocal() )
			isTopEq |= nc(expr.getDR()) && ( expr.getNumber() == 1 ? isTopOrBuiltInDT(expr.getExpr()) : isTopOrBuiltInInfDT(expr.getExpr()) );
	}
	virtual void visit ( const TDLConceptDataMaxCardinality& expr )
		{ isTopEq = !topRLocal() && nc(expr.getDR()); }
	virtual void visit ( const TDLConceptDataExactCardinality& expr )
		{ isTopEq = !topRLocal() && (expr.getNumber() == 0) && nc(expr.getDR()); }
}; // TopEquivalenceEvaluator

inline bool
BotEquivalenceEvaluator :: isTopEquivalent ( const TDLExpression& expr )
{
	return TopEval->isTopEquivalent(expr);
}


/// syntactic locality checker for DL axioms
class SyntacticLocalityChecker: protected SigAccessor, public DLAxiomVisitor
{
protected:	// members
		/// top evaluator
	TopEquivalenceEvaluator TopEval;
		/// bottom evaluator
	BotEquivalenceEvaluator BotEval;
		/// remember the axiom locality value here
	bool isLocal;

protected:	// methods
		/// @return true iff EXPR is top equivalent
	bool isTopEquivalent ( const TDLExpression* expr ) { return TopEval.isTopEquivalent(*expr); }
		/// @return true iff EXPR is bottom equivalent
	bool isBotEquivalent ( const TDLExpression* expr ) { return BotEval.isBotEquivalent(*expr); }

public:		// interface
		/// init c'tor
	SyntacticLocalityChecker ( const TSignature* s )
		: SigAccessor(s)
		, TopEval(s)
		, BotEval(s)
	{
		TopEval.setBotEval(&BotEval);
		BotEval.setTopEval(&TopEval);
	}
		/// empty d'tor
	virtual ~SyntacticLocalityChecker ( void ) {}

	// set fields

		/// @return true iff an AXIOM is local wrt defined policy
	bool local ( const TDLAxiom* axiom )
	{
		axiom->accept(*this);
		return isLocal;
	}
		/// load ontology to a given KB
	virtual void visitOntology ( TOntology& ontology )
	{
		for ( TOntology::iterator p = ontology.begin(), p_end = ontology.end(); p < p_end; ++p )
			if ( (*p)->isUsed() )
			{
				(*p)->accept(*this);
			}
	}

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
		if ( co(axiom.getC()) )
			return;
		bool topEqDesc = false;
		for ( TDLAxiomDisjointUnion::iterator p = axiom.begin(), p_end = axiom.end(); p != p_end; ++p )
			if ( !isBotEquivalent(*p) )
			{
				if ( !topCLocal() )
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
			if ( co(*p) )
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
			if ( co(*p) )
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
			if ( co(*p) )
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
			if ( co(*p) )
			{
				if ( hasNBE )
					return;	// false here
				else
					hasNBE = true;
			}

		isLocal = true;
	}
	virtual void visit ( const TDLAxiomSameIndividuals& axiom ATTR_UNUSED ) { isLocal = true; }
	virtual void visit ( const TDLAxiomDifferentIndividuals& axiom ATTR_UNUSED ) { isLocal = true; }
		/// there is no such axiom in OWL API, but I hope nobody would use Fairness here
	virtual void visit ( const TDLAxiomFairnessConstraint& axiom ATTR_UNUSED ) { isLocal = true; }

	virtual void visit ( const TDLAxiomRoleInverse& axiom ) { isLocal = nc(axiom.getRole()) && nc(axiom.getInvRole()); }
	virtual void visit ( const TDLAxiomORoleSubsumption& axiom ) { isLocal = nc(topRLocal() ? axiom.getRole() : axiom.getSubRole()); }
	virtual void visit ( const TDLAxiomDRoleSubsumption& axiom ) { isLocal = nc(topRLocal() ? axiom.getRole() : axiom.getSubRole()); }
	virtual void visit ( const TDLAxiomORoleDomain& axiom )
	{
		isLocal = isTopEquivalent(axiom.getDomain());
		if ( !topRLocal() )
			isLocal |= nc(axiom.getRole());
	}
	virtual void visit ( const TDLAxiomDRoleDomain& axiom )
	{
		isLocal = isTopEquivalent(axiom.getDomain());
		if ( !topRLocal() )
			isLocal |= nc(axiom.getRole());
	}
	virtual void visit ( const TDLAxiomORoleRange& axiom )
	{
		isLocal = isTopEquivalent(axiom.getRange());
		if ( !topRLocal() )
			isLocal |= nc(axiom.getRole());
	}
	virtual void visit ( const TDLAxiomDRoleRange& axiom )
	{
		isLocal = isTopDT(axiom.getRange());
		if ( !topRLocal() )
			isLocal |= nc(axiom.getRole());
	}
	virtual void visit ( const TDLAxiomRoleTransitive& axiom ) { isLocal = nc(axiom.getRole()); }
	virtual void visit ( const TDLAxiomRoleReflexive& axiom ) { isLocal = nc(axiom.getRole()); }
	virtual void visit ( const TDLAxiomRoleIrreflexive& axiom ATTR_UNUSED ) { isLocal = !topRLocal(); }
	virtual void visit ( const TDLAxiomRoleSymmetric& axiom ) { isLocal = nc(axiom.getRole()); }
	virtual void visit ( const TDLAxiomRoleAsymmetric& axiom ATTR_UNUSED ) { isLocal = !topRLocal(); }
	virtual void visit ( const TDLAxiomORoleFunctional& axiom ) { isLocal = !topRLocal() && nc(axiom.getRole()); }
	virtual void visit ( const TDLAxiomDRoleFunctional& axiom ) { isLocal = !topRLocal() && nc(axiom.getRole()); }
	virtual void visit ( const TDLAxiomRoleInverseFunctional& axiom ) { isLocal = !topRLocal() && nc(axiom.getRole()); }

	virtual void visit ( const TDLAxiomConceptInclusion& axiom ) { isLocal = isBotEquivalent(axiom.getSubC()) || isTopEquivalent(axiom.getSupC()); }
	virtual void visit ( const TDLAxiomInstanceOf& axiom ) { isLocal = isTopEquivalent(axiom.getC()); }
	virtual void visit ( const TDLAxiomRelatedTo& axiom ) { isLocal = topRLocal() && nc(axiom.getRelation()); }
	virtual void visit ( const TDLAxiomRelatedToNot& axiom ) { isLocal = !topRLocal() && nc(axiom.getRelation()); }
	virtual void visit ( const TDLAxiomValueOf& axiom ) { isLocal = topRLocal() && nc(axiom.getAttribute()); }
	virtual void visit ( const TDLAxiomValueOfNot& axiom ) { isLocal = !topRLocal() && nc(axiom.getAttribute()); }
}; // SyntacticLocalityChecker

#endif
