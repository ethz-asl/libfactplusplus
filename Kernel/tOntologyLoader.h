/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2010 by Dmitry Tsarkov

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

#ifndef TONTOLOGYLOADER_H
#define TONTOLOGYLOADER_H

#include "tOntology.h"
#include "tExpressionTranslator.h"
#include "dlTBox.h"

class TOntologyLoader: public DLAxiomVisitor
{
protected:	// members
		/// KB to load the ontology
	TBox& kb;
		/// Transforms TDLExpression hierarchy to the DLTree*
	TExpressionTranslator ETrans;
		/// temporary vector for arguments of TBox n-ary axioms
	std::vector<DLTree*> ArgList;

protected:	// methods
		/// get DLTree corresponding to an expression EXPR
	DLTree* e ( const TDLExpression* expr )
	{
		expr->accept(ETrans);
		return ETrans;
	}
		/// get role by the DLTree; throw exception if unable
	TRole* getRole ( const TDLRoleExpression* r, const char* reason )
	{
		try
		{
			return resolveRole(TreeDeleter(e(r)));
		}
		catch ( EFaCTPlusPlus e ) { throw EFaCTPlusPlus(reason); }
	}
		/// get an individual be the DLTree; throw exception if unable
	TIndividual* getIndividual ( const TDLIndividualExpression* I, const char* reason )
	{
		try
		{
			TreeDeleter i = e(I);
			if ( i == NULL )
				throw EFaCTPlusPlus(reason);
			return static_cast<TIndividual*>(kb.getCI(i));
		}
		catch(...)
		{
			throw EFaCTPlusPlus(reason);
		}
	}
		/// ensure that the expression EXPR has its named entities linked to the KB ones
	void ensureNames ( const TDLExpression* Expr )
	{
		fpp_assert ( Expr != NULL );	// FORNOW
	}
		/// prepare arguments for the [begin,end) interval
	template<class Iterator>
	void prepareArgList ( Iterator begin, Iterator end )
	{
		ArgList.clear();
		for ( ; begin != end; ++begin )
		{
			ensureNames(*begin);
			ArgList.push_back(e(*begin));
		}
	}

public:		// visitor interface
	virtual void visit ( TDLAxiomDeclaration& axiom )
	{
		ensureNames(axiom.getDeclaration());
		TreeDeleter(e(axiom.getDeclaration()));	// register names in the KB
	}

	// n-ary axioms

	virtual void visit ( TDLAxiomEquivalentConcepts& axiom )
	{
		prepareArgList(axiom.begin(),axiom.end());
		kb.processEquivalentC(ArgList.begin(),ArgList.end());
	}
	virtual void visit ( TDLAxiomDisjointConcepts& axiom )
	{
		prepareArgList(axiom.begin(),axiom.end());
		kb.processDisjointC(ArgList.begin(),ArgList.end());
	}
	virtual void visit ( TDLAxiomEquivalentORoles& axiom )
	{
		prepareArgList(axiom.begin(),axiom.end());
		kb.processEquivalentR(ArgList.begin(),ArgList.end());
	}
	virtual void visit ( TDLAxiomEquivalentDRoles& axiom )
	{
		prepareArgList(axiom.begin(),axiom.end());
		kb.processEquivalentR(ArgList.begin(),ArgList.end());
	}
	virtual void visit ( TDLAxiomDisjointORoles& axiom )
	{
		prepareArgList(axiom.begin(),axiom.end());
		kb.processDisjointR(ArgList.begin(),ArgList.end());
	}
	virtual void visit ( TDLAxiomDisjointDRoles& axiom )
	{
		prepareArgList(axiom.begin(),axiom.end());
		kb.processDisjointR(ArgList.begin(),ArgList.end());
	}
	virtual void visit ( TDLAxiomSameIndividuals& axiom )
	{
		prepareArgList(axiom.begin(),axiom.end());
		kb.processSame(ArgList.begin(),ArgList.end());
	}
	virtual void visit ( TDLAxiomDifferentIndividuals& axiom )
	{
		prepareArgList(axiom.begin(),axiom.end());
		kb.processDifferent(ArgList.begin(),ArgList.end());
	}
	virtual void visit ( TDLAxiomFairnessConstraint& axiom )
	{
		prepareArgList(axiom.begin(),axiom.end());
		kb.setFairnessConstraint(ArgList.begin(),ArgList.end());
	}

	// role axioms

	virtual void visit ( TDLAxiomRoleInverse& axiom )
	{
		ensureNames(axiom.getRole());
		ensureNames(axiom.getInvRole());
		TRole* R = getRole ( axiom.getRole(), "Role expression expected in Role Inverse axiom" );
		TRole* iR = getRole ( axiom.getInvRole(), "Role expression expected in Role Inverse axiom" );
		kb.getRM(R)->addRoleSynonym ( iR->inverse(), R );
	}
	virtual void visit ( TDLAxiomORoleSubsumption& axiom )
	{
		ensureNames(axiom.getRole());
		ensureNames(axiom.getSubRole());
		TRole* R = getRole ( axiom.getRole(), "Role expression expected in Object Roles Subsumption axiom" );
		kb.getRM(R)->addRoleParent ( e(axiom.getSubRole()), R );
	}
	virtual void visit ( TDLAxiomDRoleSubsumption& axiom )
	{
		ensureNames(axiom.getRole());
		ensureNames(axiom.getSubRole());
		TRole* R = getRole ( axiom.getRole(), "Role expression expected in Data Roles Subsumption axiom" );
		TRole* S = getRole ( axiom.getSubRole(), "Role expression expected in Data Roles Subsumption axiom" );
		kb.getDRM()->addRoleParent ( S, R );
	}
	virtual void visit ( TDLAxiomORoleDomain& axiom )
	{
		ensureNames(axiom.getRole());
		ensureNames(axiom.getDomain());
		getRole ( axiom.getRole(), "Role expression expected in Object Role Domain axiom" )->setDomain(e(axiom.getDomain()));
	}
	virtual void visit ( TDLAxiomDRoleDomain& axiom )
	{
		ensureNames(axiom.getRole());
		ensureNames(axiom.getDomain());
		getRole ( axiom.getRole(), "Role expression expected in Data Role Domain axiom" )->setDomain(e(axiom.getDomain()));
	}
	virtual void visit ( TDLAxiomORoleRange& axiom )
	{
		ensureNames(axiom.getRole());
		ensureNames(axiom.getRange());
		getRole ( axiom.getRole(), "Role expression expected in Object Role Range axiom" )->setRange(e(axiom.getRange()));
	}
	virtual void visit ( TDLAxiomDRoleRange& axiom )
	{
		ensureNames(axiom.getRole());
		ensureNames(axiom.getRange());
		getRole ( axiom.getRole(), "Role expression expected in Data Role Range axiom" )->setRange(e(axiom.getRange()));
	}
	virtual void visit ( TDLAxiomRoleTransitive& axiom )
	{
		ensureNames(axiom.getRole());
		if ( !isUniversalRole(axiom.getRole()) )	// universal role always transitive
			getRole ( axiom.getRole(), "Role expression expected in Role Transitivity axiom" )->setBothTransitive();
	}
	virtual void visit ( TDLAxiomRoleReflexive& axiom )
	{
		ensureNames(axiom.getRole());
		if ( !isUniversalRole(axiom.getRole()) )	// universal role always reflexive
			getRole ( axiom.getRole(), "Role expression expected in Role Reflexivity axiom" )->setBothReflexive();
	}
	virtual void visit ( TDLAxiomRoleIrreflexive& axiom )
	{
		ensureNames(axiom.getRole());
		if ( isUniversalRole(axiom.getRole()) )	// KB became inconsistent
			throw EFPPInconsistentKB();
		getRole ( axiom.getRole(), "Role expression expected in Role Irreflexivity axiom" )
			->setDomain(createSNFNot(new DLTree(REFLEXIVE,e(axiom.getRole()))));
	}
	virtual void visit ( TDLAxiomRoleSymmetric& axiom )
	{
		ensureNames(axiom.getRole());
		if ( !isUniversalRole(axiom.getRole()) )
		{
			TRole* R = getRole ( axiom.getRole(), "Role expression expected in Role Symmetry axiom" );
			kb.getORM()->addRoleParent ( R, R->inverse() );
		}
	}
	virtual void visit ( TDLAxiomRoleAntiSymmetric& axiom )
	{
		ensureNames(axiom.getRole());
		if ( isUniversalRole(axiom.getRole()) )	// KB became inconsistent
			throw EFPPInconsistentKB();
		TRole* R = getRole ( axiom.getRole(), "Role expression expected in Role AntiSymmetry axiom" );
		kb.getORM()->addDisjointRoles ( R, R->inverse() );
	}
	virtual void visit ( TDLAxiomORoleFunctional& axiom )
	{
		ensureNames(axiom.getRole());
		if ( isUniversalRole(axiom.getRole()) )	// KB became inconsistent
			throw EFPPInconsistentKB();
		getRole ( axiom.getRole(), "Role expression expected in Object Role Functionality axiom" )->setFunctional();
	}
	virtual void visit ( TDLAxiomDRoleFunctional& axiom )
	{
		ensureNames(axiom.getRole());
		if ( isUniversalRole(axiom.getRole()) )	// KB became inconsistent
			throw EFPPInconsistentKB();
		getRole ( axiom.getRole(), "Role expression expected in Data Role Functionality axiom" )->setFunctional();
	}
	virtual void visit ( TDLAxiomRoleInverseFunctional& axiom )
	{
		ensureNames(axiom.getRole());
		if ( isUniversalRole(axiom.getRole()) )	// KB became inconsistent
			throw EFPPInconsistentKB();
		getRole ( axiom.getRole(), "Role expression expected in Role Inverse Functionality axiom" )->inverse()->setFunctional();
	}

	// concept/individual axioms

	virtual void visit ( TDLAxiomConceptInclusion& axiom )
	{
		ensureNames(axiom.getSubC());
		ensureNames(axiom.getSupC());
		DLTree* C = e(axiom.getSubC());
		DLTree* D = e(axiom.getSupC());
		kb.addSubsumeAxiom ( C, D );
	}
	virtual void visit ( TDLAxiomInstanceOf& axiom )
	{
		ensureNames(axiom.getIndividual());
		ensureNames(axiom.getC());
		getIndividual ( axiom.getIndividual(), "Individual expected in Instance axiom" );
		DLTree* I = e(axiom.getIndividual());
		DLTree* C = e(axiom.getC());
		kb.addSubsumeAxiom ( I, C );
	}
	virtual void visit ( TDLAxiomRelatedTo& axiom )
	{
		ensureNames(axiom.getIndividual());
		ensureNames(axiom.getRelation());
		ensureNames(axiom.getRelatedIndividual());
		if ( !isUniversalRole(axiom.getRelation()) )	// nothing to do for universal role
			kb.RegisterIndividualRelation (
				getIndividual ( axiom.getIndividual(), "Individual expected in Related To axiom" ),
				getRole ( axiom.getRelation(), "Role expression expected in Related To axiom" ),
				getIndividual ( axiom.getRelatedIndividual(), "Individual expected in Related To axiom" ) );
	}
	virtual void visit ( TDLAxiomRelatedToNot& axiom )
	{
		ensureNames(axiom.getIndividual());
		ensureNames(axiom.getRelation());
		ensureNames(axiom.getRelatedIndividual());
		if ( isUniversalRole(axiom.getRelation()) )	// inconsistent ontology
			throw EFPPInconsistentKB();
		// make sure everything is consistent
		getIndividual ( axiom.getIndividual(), "Individual expected in Related To Not axiom" ),
		getIndividual ( axiom.getRelatedIndividual(), "Individual expected in Related To Not axiom" );
		// make an axiom i:AR.\neg{j}
		kb.addSubsumeAxiom (
				e(axiom.getIndividual()),
				createSNFForall ( e(axiom.getRelation()), createSNFNot(e(axiom.getRelatedIndividual())) ) );
	}
	virtual void visit ( TDLAxiomValueOf& axiom )
	{
		ensureNames(axiom.getIndividual());
		ensureNames(axiom.getAttribute());
		getIndividual ( axiom.getIndividual(), "Individual expected in Value Of axiom" );
		// FIXME!! think about ensuring the value
		// make an axiom i:EA.V
		kb.addSubsumeAxiom (
				e(axiom.getIndividual()),
				createSNFExists ( e(axiom.getAttribute()), e(axiom.getValue())) );
	}
	virtual void visit ( TDLAxiomValueOfNot& axiom )
	{
		ensureNames(axiom.getIndividual());
		ensureNames(axiom.getAttribute());
		getIndividual ( axiom.getIndividual(), "Individual expected in Value Of Not axiom" );
		// FIXME!! think about ensuring the value
		if ( isUniversalRole(axiom.getAttribute()) )	// inconsistent ontology
			throw EFPPInconsistentKB();
		// make an axiom i:AA.\neg V
		kb.addSubsumeAxiom (
				e(axiom.getIndividual()),
				createSNFForall ( e(axiom.getAttribute()), createSNFNot(e(axiom.getValue()))) );
	}

public:		// interface
	TOntologyLoader ( TBox& KB ) : kb(KB), ETrans(KB) {}
	virtual ~TOntologyLoader ( void ) {}

		/// load ontology to a given KB
	virtual void visitOntology ( TOntology& ontology )
	{
		for ( TOntology::iterator p = ontology.begin(), p_end = ontology.end(); p < p_end; ++p )
			if ( (*p)->isUsed() )
			{
				kb.setAxiomId((*p)->getId());
				(*p)->accept(*this);
			}
	}
}; // TOntologyLoader

#endif
