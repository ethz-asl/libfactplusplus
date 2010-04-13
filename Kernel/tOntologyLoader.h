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
#include "dlTBox.h"

class TOntologyLoader: public DLAxiomVisitor
{
protected:	// members
		/// KB to load the ontology
	TBox& kb;
		/// temporary vector for arguments of TBox n-ary axioms
	std::vector<DLTree*> ArgList;

protected:	// methods
		/// get role by the DLTree; throw exception if unable
	static TRole* getRole ( const DLTree* r, const char* reason )
	{
		try { return resolveRole(r); }
		catch ( EFaCTPlusPlus e ) { throw EFaCTPlusPlus(reason); }
	}
		/// get an individual be the DLTree; throw exception if unable
	TIndividual* getIndividual ( const DLTree* I, const char* reason ) const
	{
		if ( !kb.isIndividual(I) )
			throw EFaCTPlusPlus(reason);
		return static_cast<TIndividual*>(I->Element().getNE());
	}
		/// ensure that the expression E has its named entities linked to the KB ones
	void ensureNames ( const DLTree* E )
	{
		fpp_assert ( E != NULL );	// FORNOW
	}
		/// prepare arguments for the [begin,end) interval
	template<class Iterator>
	void prepareArgList ( Iterator begin, Iterator end )
	{
		ArgList.clear();
		for ( ; begin != end; ++begin )
		{
			ensureNames(*begin);
			ArgList.push_back(*begin);
		}
	}

public:		// visitor interface
	virtual void visit ( TDLAxiomDeclaration& axiom ) { ensureNames(axiom.getDeclaration()); }

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
		kb.getRM(R)->addRoleParent ( axiom.getSubRole(), R );
	}
	virtual void visit ( TDLAxiomDRoleSubsumption& axiom )
	{
		ensureNames(axiom.getRole());
		ensureNames(axiom.getSubRole());
		TRole* R = getRole ( axiom.getRole(), "Role expression expected in Data Roles Subsumption axiom" );
		kb.getRM(R)->addRoleParent ( axiom.getSubRole(), R );
	}
	virtual void visit ( TDLAxiomORoleDomain& axiom )
	{
		ensureNames(axiom.getRole());
		ensureNames(axiom.getDomain());
		getRole ( axiom.getRole(), "Role expression expected in Object Role Domain axiom" )->setDomain(clone(axiom.getDomain()));
	}
	virtual void visit ( TDLAxiomDRoleDomain& axiom )
	{
		ensureNames(axiom.getRole());
		ensureNames(axiom.getDomain());
		getRole ( axiom.getRole(), "Role expression expected in Data Role Domain axiom" )->setDomain(clone(axiom.getDomain()));
	}
	virtual void visit ( TDLAxiomORoleRange& axiom )
	{
		ensureNames(axiom.getRole());
		ensureNames(axiom.getRange());
		getRole ( axiom.getRole(), "Role expression expected in Object Role Range axiom" )->setRange(clone(axiom.getRange()));
	}
	virtual void visit ( TDLAxiomDRoleRange& axiom )
	{
		ensureNames(axiom.getRole());
		ensureNames(axiom.getRange());
		getRole ( axiom.getRole(), "Role expression expected in Data Role Range axiom" )->setRange(clone(axiom.getRange()));
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
			->setDomain(createSNFNot(new DLTree(REFLEXIVE,clone(axiom.getRole()))));
	}
	virtual void visit ( TDLAxiomRoleSymmetric& axiom )
	{
		ensureNames(axiom.getRole());
		if ( !isUniversalRole(axiom.getRole()) )
		{
			TRole* invR = getRole ( axiom.getRole(), "Role expression expected in Role Symmetry axiom" )->inverse();
			kb.getRM(invR)->addRoleParent ( axiom.getRole(), invR );
		}
	}
	virtual void visit ( TDLAxiomRoleAntiSymmetric& axiom )
	{
		ensureNames(axiom.getRole());
		if ( isUniversalRole(axiom.getRole()) )	// KB became inconsistent
			throw EFPPInconsistentKB();
		TRole* R = getRole ( axiom.getRole(), "Role expression expected in Role AntiSymmetry axiom" );
		kb.getRM(R)->addDisjointRoles ( R, R->inverse() );
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
		kb.addSubsumeAxiom ( clone(axiom.getSubC()), clone(axiom.getSupC()) );
	}
	virtual void visit ( TDLAxiomInstanceOf& axiom )
	{
		ensureNames(axiom.getIndividual());
		ensureNames(axiom.getC());
		getIndividual ( axiom.getIndividual(), "Individual expected in Instance axiom" );
		kb.addSubsumeAxiom ( clone(axiom.getIndividual()), clone(axiom.getC()) );
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
				clone(axiom.getIndividual()),
				createSNFForall ( clone(axiom.getRelation()), createSNFNot(clone(axiom.getRelatedIndividual())) ) );
	}
	virtual void visit ( TDLAxiomValueOf& axiom )
	{
		ensureNames(axiom.getIndividual());
		ensureNames(axiom.getAttribute());
		getIndividual ( axiom.getIndividual(), "Individual expected in Value Of axiom" );
		// FIXME!! think about ensuring the value
		if ( isUniversalRole(axiom.getAttribute()) )	// data role can't be universal
			throw EFPPInconsistentKB();
		// make an axiom i:EA.V
		kb.addSubsumeAxiom (
				clone(axiom.getIndividual()),
				createSNFExists ( clone(axiom.getAttribute()), clone(axiom.getValue())) );
	}
	virtual void visit ( TDLAxiomValueOfNot& axiom )
	{
		ensureNames(axiom.getIndividual());
		ensureNames(axiom.getAttribute());
		getIndividual ( axiom.getIndividual(), "Individual expected in Value Of Not axiom" );
		// FIXME!! think about ensuring the value
		if ( isUniversalRole(axiom.getAttribute()) )	// data role can't be universal
			throw EFPPInconsistentKB();
		// make an axiom i:AA.\neg V
		kb.addSubsumeAxiom (
				clone(axiom.getIndividual()),
				createSNFForall ( clone(axiom.getAttribute()), createSNFNot(clone(axiom.getValue()))) );
	}

public:		// interface
	TOntologyLoader ( TBox& KB ) : kb(KB) {}
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
