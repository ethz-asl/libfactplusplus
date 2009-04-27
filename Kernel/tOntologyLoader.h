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

public:		// visitor interface
	virtual void visit ( TDLAxiomEquivalentConcepts& axiom ) { kb.processEquivalentC(axiom.begin(),axiom.end()); }
	virtual void visit ( TDLAxiomDisjointConcepts& axiom ) { kb.processDisjointC(axiom.begin(),axiom.end()); }
	virtual void visit ( TDLAxiomEquivalentRoles& axiom ) { kb.processEquivalentR(axiom.begin(),axiom.end()); }
	virtual void visit ( TDLAxiomDisjointRoles& axiom ) { kb.processDisjointR(axiom.begin(),axiom.end()); }
	virtual void visit ( TDLAxiomSameIndividuals& axiom ) { kb.processSame(axiom.begin(),axiom.end()); }
	virtual void visit ( TDLAxiomDifferentIndividuals& axiom ) { kb.processDifferent(axiom.begin(),axiom.end()); }
	virtual void visit ( TDLAxiomFairnessConstraint& axiom ) { kb.setFairnessConstraint(axiom.begin(),axiom.end()); }

	virtual void visit ( TDLAxiomRoleSubsumption& axiom )
		{ kb.getRM()->addRoleParent ( axiom.getSubRole(), getRole ( axiom.getRole(), "Role expression expected in Roles Subsumption axiom" ) ); }
	virtual void visit ( TDLAxiomRoleDomain& axiom )
		{ getRole ( axiom.getRole(), "Role expression expected in Role Domain axiom" )->setDomain(clone(axiom.getDomain())); }
	virtual void visit ( TDLAxiomRoleRange& axiom )
		{ getRole ( axiom.getRole(), "Role expression expected in Role Range axiom" )->setRange(clone(axiom.getRange())); }
	virtual void visit ( TDLAxiomRoleTransitive& axiom )
	{
		if ( !isUniversalRole(axiom.getRole()) )	// universal role always transitive
			getRole ( axiom.getRole(), "Role expression expected in Role Transitivity axiom" )->setBothTransitive();
	}
	virtual void visit ( TDLAxiomRoleReflexive& axiom )
	{
		if ( !isUniversalRole(axiom.getRole()) )	// universal role always reflexive
			getRole ( axiom.getRole(), "Role expression expected in Role Reflexivity axiom" )->setBothReflexive();
	}
	virtual void visit ( TDLAxiomRoleIrreflexive& axiom )
	{
		if ( isUniversalRole(axiom.getRole()) )	// KB became inconsistent
			throw EFPPInconsistentKB();
		getRole ( axiom.getRole(), "Role expression expected in Role Irreflexivity axiom" )
			->setDomain(new DLTree(NOT,new DLTree(REFLEXIVE,clone(axiom.getRole()))));
	}
	virtual void visit ( TDLAxiomRoleSymmetric& axiom )
	{
		if ( !isUniversalRole(axiom.getRole()) )
		{
			TRole* invR = getRole ( axiom.getRole(), "Role expression expected in Role Symmetry axiom" )->inverse();
			kb.getRM()->addRoleParent ( axiom.getRole(), invR );
		}
	}
	virtual void visit ( TDLAxiomRoleAntiSymmetric& axiom )
	{
		if ( isUniversalRole(axiom.getRole()) )	// KB became inconsistent
			throw EFPPInconsistentKB();
		TRole* R = getRole ( axiom.getRole(), "Role expression expected in Role AntiSymmetry axiom" );
		kb.getRM()->addDisjointRoles ( R, R->inverse() );
	}
	virtual void visit ( TDLAxiomRoleFunctional& axiom )
	{
		if ( isUniversalRole(axiom.getRole()) )	// KB became inconsistent
			throw EFPPInconsistentKB();
		getRole ( axiom.getRole(), "Role expression expected in Role Functionality axiom" )->setFunctional();
	}

	virtual void visit ( TDLAxiomConceptInclusion& axiom ) { kb.addSubsumeAxiom ( clone(axiom.getSubC()), clone(axiom.getSupC()) ); }
	virtual void visit ( TDLAxiomInstanceOf& axiom )
		{ kb.RegisterInstance ( getIndividual ( axiom.getIndividual(), "Individual expected in Instance axiom" ), clone(axiom.getC()) ); }
	virtual void visit ( TDLAxiomRelatedTo& axiom )
	{
		if ( !isUniversalRole(axiom.getRelation()) )	// nothing to do for universal role
			kb.RegisterIndividualRelation (
				getIndividual ( axiom.getIndividual(), "Individual expected in Related To axiom" ),
				getRole ( axiom.getRelation(), "Role expression expected in Related To axiom" ),
				getIndividual ( axiom.getRelatedIndividual(), "Individual expected in Related To axiom" ) );
	}
	virtual void visit ( TDLAxiomRelatedToNot& axiom )
	{
		if ( isUniversalRole(axiom.getRelation()) )	// inconsistent ontology
			throw EFPPInconsistentKB();
		// make sure everything is consistent
		getIndividual ( axiom.getRelatedIndividual(), "Individual expected in Related To Not axiom" );
		// make an axiom i:AR.\neg{j}
		kb.RegisterInstance (
				getIndividual ( axiom.getIndividual(), "Individual expected in Related To Not axiom" ),
				new DLTree(FORALL,clone(axiom.getRelation()),new DLTree(NOT,clone(axiom.getRelatedIndividual()))) );
	}
	virtual void visit ( TDLAxiomValueOf& axiom )
	{
		if ( isUniversalRole(axiom.getAttribute()) )	// data role can't be universal
			throw EFPPInconsistentKB();
		// make an axiom i:EA.V
		kb.RegisterInstance (
				getIndividual ( axiom.getIndividual(), "Individual expected in Value Of axiom" ),
				new DLTree(EXISTS,clone(axiom.getAttribute()),clone(axiom.getValue())) );
	}
	virtual void visit ( TDLAxiomValueOfNot& axiom )
	{
		if ( isUniversalRole(axiom.getAttribute()) )	// data role can't be universal
			throw EFPPInconsistentKB();
		// make an axiom i:AA.\neg V
		kb.RegisterInstance (
				getIndividual ( axiom.getIndividual(), "Individual expected in Value Of Not axiom" ),
				new DLTree(FORALL,clone(axiom.getAttribute()),new DLTree(NOT,clone(axiom.getValue()))) );
	}

public:		// interface
	TOntologyLoader ( TBox& KB ) : kb(KB) {}
	virtual ~TOntologyLoader ( void ) {}

		/// load ontology to a given KB
	virtual void visitOntology ( TOntology& ontology )
	{
		for ( TOntology::iterator p = ontology.beginUnprocessed(), p_end = ontology.end(); p < p_end; ++p )
		{
			kb.setAxiomId((*p)->getId());
			(*p)->accept(*this);
		}
	}
}; // TOntologyLoader

#endif
