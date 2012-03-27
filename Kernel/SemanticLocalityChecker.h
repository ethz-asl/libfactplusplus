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

#ifndef SEMLOCCHECKER_H
#define SEMLOCCHECKER_H

#include "tSignature.h"
#include "Kernel.h"

/// semantic locality checker for DL axioms
class SemanticLocalityChecker: public DLAxiomVisitor
{
protected:	// members
		/// Reasoner to detect the tautology
	ReasoningKernel Kernel;
		/// Expression manager of a kernel
	TExpressionManager* pEM;
		/// signature to keep
	const TSignature* sig;
		/// remember the axiom locality value here
	bool isLocal;

public:		// interface
		/// init c'tor
	SemanticLocalityChecker ( const TSignature* s ) : sig(s), isLocal(true)
	{
		pEM = Kernel.getExpressionManager();
		// for tests we will need TB names to be from the OWL 2 namespace
		pEM->setTopBottomRoles(
			"http://www.w3.org/2002/07/owl#topObjectProperty",
			"http://www.w3.org/2002/07/owl#bottomObjectProperty",
			"http://www.w3.org/2002/07/owl#topDataProperty",
			"http://www.w3.org/2002/07/owl#bottomDataProperty");
	}
		/// empty d'tor
	virtual ~SemanticLocalityChecker ( void ) {}

	// set fields

		/// @return true iff an AXIOM is local wrt defined policy
	bool local ( const TDLAxiom* axiom )
	{
		axiom->accept(*this);
		return isLocal;
	}
		/// init kernel with the ontology signature
	void setOntologySig ( const TSignature& s )
	{
		Kernel.clearKB();
		// register all the objects in the ontology signature
		for ( TSignature::iterator p = s.begin(), p_end = s.end(); p != p_end; ++p )
			Kernel.declare(dynamic_cast<const TDLExpression*>(*p));
		// prepare the reasoner to check tautologies
		Kernel.realiseKB();
		// after TBox appears there, set signature to translate
		Kernel.setSignature(sig);
	}
		/// load ontology to a given KB
	virtual void visitOntology ( TOntology& ontology )
	{
		for ( TOntology::iterator p = ontology.begin(), p_end = ontology.end(); p < p_end; ++p )
			if ( likely((*p)->isUsed()) )
				(*p)->accept(*this);
	}

public:		// visitor interface
	virtual void visit ( const TDLAxiomDeclaration& axiom ATTR_UNUSED ) { isLocal = true; }

	virtual void visit ( const TDLAxiomEquivalentConcepts& axiom )
	{
		isLocal = false;
		TDLAxiomEquivalentConcepts::iterator p = axiom.begin(), p_end = axiom.end();
		const TDLConceptExpression* C = *p;
		while (  ++p != p_end )
			if ( !Kernel.isEquivalent ( C, *p ) )
				return;
		isLocal = true;
	}
	virtual void visit ( const TDLAxiomDisjointConcepts& axiom )
	{
		isLocal = false;
		for ( TDLAxiomDisjointConcepts::iterator p = axiom.begin(), q, p_end = axiom.end(); p != p_end; ++p )
			for ( q = p+1; q != p_end; ++q )
				if ( !Kernel.isDisjoint ( *p, *q ) )
					return;
		isLocal = true;
	}
		/// FIXME!! fornow
	virtual void visit ( const TDLAxiomDisjointUnion& axiom ATTR_UNUSED )
	{
		isLocal = true;
	}
	virtual void visit ( const TDLAxiomEquivalentORoles& axiom )
	{
		isLocal = false;
		TDLAxiomEquivalentORoles::iterator p = axiom.begin(), p_end = axiom.end();
		const TDLObjectRoleExpression* R = *p;
		while (  ++p != p_end )
			if ( !(Kernel.isSubRoles ( R, *p ) && Kernel.isSubRoles ( *p, R )) )
				return;
		isLocal = true;
	}
		// tautology if all the subsumptions Ri [= Rj holds
	virtual void visit ( const TDLAxiomEquivalentDRoles& axiom )
	{
		isLocal = false;
		TDLAxiomEquivalentDRoles::iterator p = axiom.begin(), p_end = axiom.end();
		const TDLDataRoleExpression* R = *p;
		while (  ++p != p_end )
			if ( !(Kernel.isSubRoles ( R, *p ) && Kernel.isSubRoles ( *p, R )) )
				return;
		isLocal = true;
	}
	virtual void visit ( const TDLAxiomDisjointORoles& axiom )
	{
		pEM->newArgList();
		for ( TDLAxiomDisjointORoles::iterator p = axiom.begin(), p_end = axiom.end(); p != p_end; ++p )
			pEM->addArg(*p);
		isLocal = Kernel.isDisjointRoles();
	}
	virtual void visit ( const TDLAxiomDisjointDRoles& axiom )
	{
		pEM->newArgList();
		for ( TDLAxiomDisjointDRoles::iterator p = axiom.begin(), p_end = axiom.end(); p != p_end; ++p )
			pEM->addArg(*p);
		isLocal = Kernel.isDisjointRoles();
	}
		// never local
	virtual void visit ( const TDLAxiomSameIndividuals& axiom ATTR_UNUSED ) { isLocal = false; }
		// never local
	virtual void visit ( const TDLAxiomDifferentIndividuals& axiom ATTR_UNUSED ) { isLocal = false; }
		/// there is no such axiom in OWL API, but I hope nobody would use Fairness here
	virtual void visit ( const TDLAxiomFairnessConstraint& axiom ATTR_UNUSED ) { isLocal = true; }

		// R = inverse(S) is tautology iff R [= S- and S [= R-
	virtual void visit ( const TDLAxiomRoleInverse& axiom )
	{
		isLocal = Kernel.isSubRoles ( axiom.getRole(), pEM->Inverse(axiom.getInvRole()) ) &&
				Kernel.isSubRoles ( axiom.getInvRole(), pEM->Inverse(axiom.getRole()) );
	}
	virtual void visit ( const TDLAxiomORoleSubsumption& axiom )
	{
		// check whether the LHS is a role chain
		const TDLObjectRoleChain* Chain = dynamic_cast<const TDLObjectRoleChain*>(axiom.getSubRole());
		if ( Chain != NULL )
		{
			pEM->newArgList();
			for ( TDLObjectRoleChain::iterator p = Chain->begin(), p_end = Chain->end(); p != p_end; ++p )
				pEM->addArg(*p);
			isLocal = Kernel.isSubChain(axiom.getRole());
			return;
		}
		// check whether the LHS is a plain rle or inverse
		const TDLObjectRoleExpression* Sub = dynamic_cast<const TDLObjectRoleExpression*>(axiom.getSubRole());
		if ( Sub != NULL )
		{
			isLocal = Kernel.isSubRoles ( Sub, axiom.getRole() );
			return;
		}
		// here we have a projection expression. FIXME!! for now
		isLocal = true;
	}
	virtual void visit ( const TDLAxiomDRoleSubsumption& axiom ) { isLocal = Kernel.isSubRoles ( axiom.getSubRole(), axiom.getRole() ); }
		// Domain(R) = C is tautology iff ER.Top [= C
	virtual void visit ( const TDLAxiomORoleDomain& axiom ) { isLocal = Kernel.isSubsumedBy ( pEM->Exists ( axiom.getRole(), pEM->Top() ), axiom.getDomain() ); }
	virtual void visit ( const TDLAxiomDRoleDomain& axiom ) { isLocal = Kernel.isSubsumedBy ( pEM->Exists ( axiom.getRole(), pEM->DataTop() ), axiom.getDomain() ); }
		// Range(R) = C is tautology iff ER.~C is unsatisfiable
	virtual void visit ( const TDLAxiomORoleRange& axiom ) { isLocal = !Kernel.isSatisfiable ( pEM->Exists ( axiom.getRole(), pEM->Not(axiom.getRange()) ) ); }
	virtual void visit ( const TDLAxiomDRoleRange& axiom ) { isLocal = !Kernel.isSatisfiable ( pEM->Exists ( axiom.getRole(), pEM->DataNot(axiom.getRange()) ) ); }
	virtual void visit ( const TDLAxiomRoleTransitive& axiom ) { isLocal = Kernel.isTransitive(axiom.getRole()); }
	virtual void visit ( const TDLAxiomRoleReflexive& axiom ) { isLocal = Kernel.isReflexive(axiom.getRole()); }
	virtual void visit ( const TDLAxiomRoleIrreflexive& axiom ) { isLocal = Kernel.isIrreflexive(axiom.getRole()); }
	virtual void visit ( const TDLAxiomRoleSymmetric& axiom ) { isLocal = Kernel.isSymmetric(axiom.getRole()); }
	virtual void visit ( const TDLAxiomRoleAsymmetric& axiom ) { isLocal = Kernel.isAsymmetric(axiom.getRole()); }
	virtual void visit ( const TDLAxiomORoleFunctional& axiom ) { isLocal = Kernel.isFunctional(axiom.getRole()); }
	virtual void visit ( const TDLAxiomDRoleFunctional& axiom ) { isLocal = Kernel.isFunctional(axiom.getRole()); }
	virtual void visit ( const TDLAxiomRoleInverseFunctional& axiom ) { isLocal = Kernel.isInverseFunctional(axiom.getRole()); }

	virtual void visit ( const TDLAxiomConceptInclusion& axiom ) { isLocal = Kernel.isSubsumedBy ( axiom.getSubC(), axiom.getSupC() ); }
		// for top locality, this might be local
	virtual void visit ( const TDLAxiomInstanceOf& axiom ) { isLocal = Kernel.isInstance ( axiom.getIndividual(), axiom.getC() ); }
		// R(i,j) holds if {i} [= \ER.{j}
	virtual void visit ( const TDLAxiomRelatedTo& axiom ) { isLocal = Kernel.isSubsumedBy ( pEM->OneOf(axiom.getIndividual()), pEM->Value ( axiom.getRelation(), axiom.getRelatedIndividual() ) ); }
		///!R(i,j) holds if {i} [= \AR.!{j}=!\ER.{j}
	virtual void visit ( const TDLAxiomRelatedToNot& axiom ) { isLocal = Kernel.isSubsumedBy ( pEM->OneOf(axiom.getIndividual()), pEM->Not ( pEM->Value ( axiom.getRelation(), axiom.getRelatedIndividual() ) ) ); }
		// R(i,v) holds if {i} [= \ER.{v}
	virtual void visit ( const TDLAxiomValueOf& axiom ) { isLocal = Kernel.isSubsumedBy ( pEM->OneOf(axiom.getIndividual()), pEM->Value ( axiom.getAttribute(), axiom.getValue() ) ); }
		// !R(i,v) holds if {i} [= !\ER.{v}
	virtual void visit ( const TDLAxiomValueOfNot& axiom ) { isLocal = Kernel.isSubsumedBy ( pEM->OneOf(axiom.getIndividual()), pEM->Not ( pEM->Value ( axiom.getAttribute(), axiom.getValue() ) ) ); }
}; // SemanticLocalityChecker

#endif
