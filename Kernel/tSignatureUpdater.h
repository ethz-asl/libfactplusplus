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

#ifndef TSIGNATUREUPDATER_H
#define TSIGNATUREUPDATER_H

#include "tDLExpression.h"
#include "tSignature.h"

/// update the signature by adding all signature elements from the expression
class TExpressionSignatureUpdater: public DLExpressionVisitor
{
protected:	// members
		/// Signature to be filled
	TSignature& sig;

protected:	// methods
		/// helper for concept arguments
	void vC ( const TConceptArg& expr ) { expr.getC()->accept(*this); }
		/// helper for individual arguments
	void vI ( const TIndividualArg& expr ) { expr.getI()->accept(*this); }
		/// helper for object role arguments
	void vOR ( const TObjectRoleArg& expr ) { expr.getOR()->accept(*this); }
		/// helper for object role arguments
	void vDR ( const TDataRoleArg& expr ) { expr.getDR()->accept(*this); }
		/// helper for the named entity
	void vE ( const TNamedEntity& e ) { sig.add(e.getEntity()); }
		/// array helper
	template <class Argument>
	void processArray ( const TDLNAryExpression<Argument>& expr )
	{
		for ( typename TDLNAryExpression<Argument>::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			(*p)->accept(*this);
	}

public:		// interface
		/// init c'tor
	TExpressionSignatureUpdater ( TSignature& s ) : sig(s) {}
		/// empty d'tor
	virtual ~TExpressionSignatureUpdater ( void ) {}

public:		// visitor interface
	// concept expressions
	virtual void visit ( const TDLConceptTop& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLConceptBottom& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLConceptName& expr ) { vE(expr); }
	virtual void visit ( const TDLConceptNot& expr ) { vC(expr); }
	virtual void visit ( const TDLConceptAnd& expr ) { processArray(expr); }
	virtual void visit ( const TDLConceptOr& expr ) { processArray(expr); }
	virtual void visit ( const TDLConceptOneOf& expr ) { processArray(expr); }
	virtual void visit ( const TDLConceptObjectSelf& expr ) { vOR(expr); }
	virtual void visit ( const TDLConceptObjectValue& expr ) { vOR(expr); vI(expr); }
	virtual void visit ( const TDLConceptObjectExists& expr ) { vOR(expr); vC(expr); }
	virtual void visit ( const TDLConceptObjectForall& expr ) { vOR(expr); vC(expr); }
	virtual void visit ( const TDLConceptObjectMinCardinality& expr ) { vOR(expr); vC(expr); }
	virtual void visit ( const TDLConceptObjectMaxCardinality& expr ) { vOR(expr); vC(expr); }
	virtual void visit ( const TDLConceptObjectExactCardinality& expr ) { vOR(expr); vC(expr); }
	virtual void visit ( const TDLConceptDataValue& expr ) { vDR(expr); }
	virtual void visit ( const TDLConceptDataExists& expr ) { vDR(expr); }
	virtual void visit ( const TDLConceptDataForall& expr ) { vDR(expr); }
	virtual void visit ( const TDLConceptDataMinCardinality& expr ) { vDR(expr); }
	virtual void visit ( const TDLConceptDataMaxCardinality& expr ) { vDR(expr); }
	virtual void visit ( const TDLConceptDataExactCardinality& expr ) { vDR(expr); }

	// individual expressions
	virtual void visit ( const TDLIndividualName& expr ) { vE(expr); }

	// object role expressions
	virtual void visit ( const TDLObjectRoleTop& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLObjectRoleBottom& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLObjectRoleName& expr ) { vE(expr); }
	virtual void visit ( const TDLObjectRoleInverse& expr ) { vOR(expr); }
	virtual void visit ( const TDLObjectRoleChain& expr ) { processArray(expr); }
	virtual void visit ( const TDLObjectRoleProjectionFrom& expr ) { vOR(expr); vC(expr); }
	virtual void visit ( const TDLObjectRoleProjectionInto& expr ) { vOR(expr); vC(expr); }

	// data role expressions
	virtual void visit ( const TDLDataRoleTop& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLDataRoleBottom& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLDataRoleName& expr ) { vE(expr); }

	// data expressions
	virtual void visit ( const TDLDataTop& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLDataBottom& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLDataTypeName& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLDataTypeRestriction& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLDataValue& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLDataNot& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLDataAnd& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLDataOr& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLDataOneOf& expr ATTR_UNUSED ) {}

	// facets
	virtual void visit ( const TDLFacetMinInclusive& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLFacetMinExclusive& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLFacetMaxInclusive& expr ATTR_UNUSED ) {}
	virtual void visit ( const TDLFacetMaxExclusive& expr ATTR_UNUSED ) {}
}; // TExpressionSignatureUpdater

/// update signature by adding the signature of a given axiom to it
class TSignatureUpdater: public DLAxiomVisitor
{
protected:	// members
		/// helper with expressions
	TExpressionSignatureUpdater Updater;

protected:	// methods
		/// helper for the expression processing
	void v ( const TDLExpression* E ) { E->accept(Updater); }
		/// helper for the [begin,end) interval
	template<class Iterator>
	void v ( Iterator begin, Iterator end )
	{
		for ( ; begin != end; ++begin )
			v(*begin);
	}

public:		// visitor interface
	virtual void visit ( const TDLAxiomDeclaration& axiom ) { v(axiom.getDeclaration()); }

	virtual void visit ( const TDLAxiomEquivalentConcepts& axiom ) { v ( axiom.begin(), axiom.end() ); }
	virtual void visit ( const TDLAxiomDisjointConcepts& axiom ) { v ( axiom.begin(), axiom.end() ); }
	virtual void visit ( const TDLAxiomDisjointUnion& axiom ) { v(axiom.getC()); v ( axiom.begin(), axiom.end() ); }
	virtual void visit ( const TDLAxiomEquivalentORoles& axiom ) { v ( axiom.begin(), axiom.end() ); }
	virtual void visit ( const TDLAxiomEquivalentDRoles& axiom ) { v ( axiom.begin(), axiom.end() ); }
	virtual void visit ( const TDLAxiomDisjointORoles& axiom ) { v ( axiom.begin(), axiom.end() ); }
	virtual void visit ( const TDLAxiomDisjointDRoles& axiom ) { v ( axiom.begin(), axiom.end() ); }
	virtual void visit ( const TDLAxiomSameIndividuals& axiom ) { v ( axiom.begin(), axiom.end() ); }
	virtual void visit ( const TDLAxiomDifferentIndividuals& axiom ) { v ( axiom.begin(), axiom.end() ); }
	virtual void visit ( const TDLAxiomFairnessConstraint& axiom ) { v ( axiom.begin(), axiom.end() ); }

	virtual void visit ( const TDLAxiomRoleInverse& axiom ) { v(axiom.getRole()); v(axiom.getInvRole()); }
	virtual void visit ( const TDLAxiomORoleSubsumption& axiom ) { v(axiom.getRole()); v(axiom.getSubRole()); }
	virtual void visit ( const TDLAxiomDRoleSubsumption& axiom ) { v(axiom.getRole()); v(axiom.getSubRole()); }
	virtual void visit ( const TDLAxiomORoleDomain& axiom ) { v(axiom.getRole()); v(axiom.getDomain()); }
	virtual void visit ( const TDLAxiomDRoleDomain& axiom ) { v(axiom.getRole()); v(axiom.getDomain()); }
	virtual void visit ( const TDLAxiomORoleRange& axiom ) { v(axiom.getRole()); v(axiom.getRange()); }
	virtual void visit ( const TDLAxiomDRoleRange& axiom ) { v(axiom.getRole()); v(axiom.getRange()); }
	virtual void visit ( const TDLAxiomRoleTransitive& axiom ) { v(axiom.getRole()); }
	virtual void visit ( const TDLAxiomRoleReflexive& axiom ) { v(axiom.getRole()); }
	virtual void visit ( const TDLAxiomRoleIrreflexive& axiom ) { v(axiom.getRole()); }
	virtual void visit ( const TDLAxiomRoleSymmetric& axiom ) { v(axiom.getRole()); }
	virtual void visit ( const TDLAxiomRoleAsymmetric& axiom ) { v(axiom.getRole()); }
	virtual void visit ( const TDLAxiomORoleFunctional& axiom ) { v(axiom.getRole()); }
	virtual void visit ( const TDLAxiomDRoleFunctional& axiom ) { v(axiom.getRole()); }
	virtual void visit ( const TDLAxiomRoleInverseFunctional& axiom ) { v(axiom.getRole()); }

	virtual void visit ( const TDLAxiomConceptInclusion& axiom ) { v(axiom.getSubC()); v(axiom.getSupC()); }
	virtual void visit ( const TDLAxiomInstanceOf& axiom ) { v(axiom.getIndividual()); v(axiom.getC()); }
	virtual void visit ( const TDLAxiomRelatedTo& axiom ) { v(axiom.getIndividual()); v(axiom.getRelation()); v(axiom.getRelatedIndividual()); }
	virtual void visit ( const TDLAxiomRelatedToNot& axiom ) { v(axiom.getIndividual()); v(axiom.getRelation()); v(axiom.getRelatedIndividual()); }
	virtual void visit ( const TDLAxiomValueOf& axiom ) { v(axiom.getIndividual()); v(axiom.getAttribute()); }
	virtual void visit ( const TDLAxiomValueOfNot& axiom ) { v(axiom.getIndividual()); v(axiom.getAttribute()); }

public:		// interface
		/// init c'tor
	TSignatureUpdater ( TSignature& sig ) : Updater(sig) {}
		/// empty d'tor
	virtual ~TSignatureUpdater ( void ) {}

		/// load ontology to a given KB
	virtual void visitOntology ( TOntology& ontology )
	{
		for ( TOntology::iterator p = ontology.begin(), p_end = ontology.end(); p < p_end; ++p )
			if ( (*p)->isUsed() )
			{
				(*p)->accept(*this);
			}
	}
}; // TSignatureUpdater

#endif
