/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2009 by Dmitry Tsarkov

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
	TBox& kb;

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
		/// ensure that the concept expression E has its named entities linked to the KB ones
	void ensureNames ( const DLTree* E )
	{
		if ( E == NULL )
			return;
		switch ( E->Element().getToken() )
		{
		case CNAME:
			if ( E->Element().getNE() == NULL )
				const_cast<DLTree*>(E)->Element().setNE(kb.getConcept(E->Element().getName()));
			break;
		case INAME:
			if ( E->Element().getNE() == NULL )
				const_cast<DLTree*>(E)->Element().setNE(kb.getIndividual(E->Element().getName()));
			break;
		case RNAME:
			if ( E->Element().getNE() == NULL )
				const_cast<DLTree*>(E)->Element().setNE ( kb.getRM()->ensureRoleName ( E->Element().getName(), /*isDataRole=*/false ) );
			break;
		case DNAME:
			if ( E->Element().getNE() == NULL )
				const_cast<DLTree*>(E)->Element().setNE ( kb.getRM()->ensureRoleName ( E->Element().getName(), /*isDataRole=*/true ) );
			break;
		default:
			ensureNames(E->Left());
			ensureNames(E->Right());
			break;
		};
	}
		/// ensure names in the [begin,end) interval
	template<class Iterator>
	void ensureNames ( Iterator begin, Iterator end )
	{
		for ( ; begin != end; ++begin )
			ensureNames(*begin);
	}

public:		// visitor interface
	virtual void visit ( TDLAxiomDeclaration& axiom ) { ensureNames(axiom.getDeclaration()); }

	// n-ary axioms

	virtual void visit ( TDLAxiomEquivalentConcepts& axiom )
	{
		ensureNames(axiom.begin(),axiom.end());
		kb.processEquivalentC(axiom.begin(),axiom.end());
	}
	virtual void visit ( TDLAxiomDisjointConcepts& axiom )
	{
		ensureNames(axiom.begin(),axiom.end());
		kb.processDisjointC(axiom.begin(),axiom.end());
	}
	virtual void visit ( TDLAxiomEquivalentRoles& axiom )
	{
		ensureNames(axiom.begin(),axiom.end());
		kb.processEquivalentR(axiom.begin(),axiom.end());
	}
	virtual void visit ( TDLAxiomDisjointRoles& axiom )
	{
		ensureNames(axiom.begin(),axiom.end());
		kb.processDisjointR(axiom.begin(),axiom.end());
	}
	virtual void visit ( TDLAxiomSameIndividuals& axiom )
	{
		ensureNames(axiom.begin(),axiom.end());
		kb.processSame(axiom.begin(),axiom.end());
	}
	virtual void visit ( TDLAxiomDifferentIndividuals& axiom )
	{
		ensureNames(axiom.begin(),axiom.end());
		kb.processDifferent(axiom.begin(),axiom.end());
	}
	virtual void visit ( TDLAxiomFairnessConstraint& axiom )
	{
		ensureNames(axiom.begin(),axiom.end());
		kb.setFairnessConstraint(axiom.begin(),axiom.end());
	}

	// role axioms

	virtual void visit ( TDLAxiomRoleSubsumption& axiom )
	{
		ensureNames(axiom.getRole());
		ensureNames(axiom.getSubRole());
		kb.getRM()->addRoleParent ( axiom.getSubRole(), getRole ( axiom.getRole(), "Role expression expected in Roles Subsumption axiom" ) );
	}
	virtual void visit ( TDLAxiomRoleDomain& axiom )
	{
		ensureNames(axiom.getRole());
		ensureNames(axiom.getDomain());
		getRole ( axiom.getRole(), "Role expression expected in Role Domain axiom" )->setDomain(clone(axiom.getDomain()));
	}
	virtual void visit ( TDLAxiomRoleRange& axiom )
	{
		ensureNames(axiom.getRole());
		ensureNames(axiom.getRange());
		getRole ( axiom.getRole(), "Role expression expected in Role Range axiom" )->setRange(clone(axiom.getRange()));
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
			kb.getRM()->addRoleParent ( axiom.getRole(), invR );
		}
	}
	virtual void visit ( TDLAxiomRoleAntiSymmetric& axiom )
	{
		ensureNames(axiom.getRole());
		if ( isUniversalRole(axiom.getRole()) )	// KB became inconsistent
			throw EFPPInconsistentKB();
		TRole* R = getRole ( axiom.getRole(), "Role expression expected in Role AntiSymmetry axiom" );
		kb.getRM()->addDisjointRoles ( R, R->inverse() );
	}
	virtual void visit ( TDLAxiomRoleFunctional& axiom )
	{
		ensureNames(axiom.getRole());
		if ( isUniversalRole(axiom.getRole()) )	// KB became inconsistent
			throw EFPPInconsistentKB();
		getRole ( axiom.getRole(), "Role expression expected in Role Functionality axiom" )->setFunctional();
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
		kb.RegisterInstance ( getIndividual ( axiom.getIndividual(), "Individual expected in Instance axiom" ), clone(axiom.getC()) );
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
		getIndividual ( axiom.getRelatedIndividual(), "Individual expected in Related To Not axiom" );
		// make an axiom i:AR.\neg{j}
		kb.RegisterInstance (
				getIndividual ( axiom.getIndividual(), "Individual expected in Related To Not axiom" ),
				createSNFForall ( clone(axiom.getRelation()), createSNFNot(clone(axiom.getRelatedIndividual())) ) );
	}
	virtual void visit ( TDLAxiomValueOf& axiom )
	{
		ensureNames(axiom.getIndividual());
		ensureNames(axiom.getAttribute());
		// FIXME!! think about ensuring the value
		if ( isUniversalRole(axiom.getAttribute()) )	// data role can't be universal
			throw EFPPInconsistentKB();
		// make an axiom i:EA.V
		kb.RegisterInstance (
				getIndividual ( axiom.getIndividual(), "Individual expected in Value Of axiom" ),
				createSNFExists ( clone(axiom.getAttribute()), clone(axiom.getValue())) );
	}
	virtual void visit ( TDLAxiomValueOfNot& axiom )
	{
		ensureNames(axiom.getIndividual());
		ensureNames(axiom.getAttribute());
		// FIXME!! think about ensuring the value
		if ( isUniversalRole(axiom.getAttribute()) )	// data role can't be universal
			throw EFPPInconsistentKB();
		// make an axiom i:AA.\neg V
		kb.RegisterInstance (
				getIndividual ( axiom.getIndividual(), "Individual expected in Value Of Not axiom" ),
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
