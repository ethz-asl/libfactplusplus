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

#ifndef ELFAXIOMCHECKER_H
#define ELFAXIOMCHECKER_H

#include "tDLAxiom.h"
#include "ELFExpressionChecker.h"

class ELFAxiomChecker: public DLAxiomVisitor
{
protected:
	ELFExpressionChecker eCh;
	bool value;
	bool v ( const TDLExpression* expr ) { value = eCh.v(expr); return value; }
public:		// visitor interface
	virtual void visit ( const TDLAxiomDeclaration& axiom ) { v(axiom.getDeclaration()); }

	virtual void visit ( const TDLAxiomEquivalentConcepts& axiom )
	{
		value = false;
		for ( TDLAxiomEquivalentConcepts::iterator p = axiom.begin(), p_end = axiom.end(); p != p_end; ++p )
			if ( !v(*p) )
				return;
		value = true;
	}
	virtual void visit ( const TDLAxiomDisjointConcepts& axiom )
	{
		value = false;
		for ( TDLAxiomDisjointConcepts::iterator p = axiom.begin(), p_end = axiom.end(); p != p_end; ++p )
			if ( !v(*p) )
				return;
		value = true;
	}
	virtual void visit ( const TDLAxiomDisjointUnion& axiom ) { value = (axiom.size() > 1); }
	virtual void visit ( const TDLAxiomEquivalentORoles& axiom )
	{
		value = false;
		for ( TDLAxiomEquivalentORoles::iterator p = axiom.begin(), p_end = axiom.end(); p != p_end; ++p )
			if ( !v(*p) )
				return;
		value = true;
	}
	virtual void visit ( const TDLAxiomEquivalentDRoles& axiom ) { value = false; }
	virtual void visit ( const TDLAxiomDisjointORoles& axiom ) { value = false; }
	virtual void visit ( const TDLAxiomDisjointDRoles& axiom ) { value = false; }
	virtual void visit ( const TDLAxiomSameIndividuals& axiom ) { value = false; }
	virtual void visit ( const TDLAxiomDifferentIndividuals& axiom ) { value = false; }
	virtual void visit ( const TDLAxiomFairnessConstraint& axiom ) { value = false; }

	virtual void visit ( const TDLAxiomRoleInverse& axiom ) { value = false; }
	virtual void visit ( const TDLAxiomORoleSubsumption& axiom )
	{
		if ( v(axiom.getSubRole()) )
			v(axiom.getRole());
	}
	virtual void visit ( const TDLAxiomDRoleSubsumption& axiom ) { value = false; }
	// FIXME!! check later
	virtual void visit ( const TDLAxiomORoleDomain& axiom ) { value = false; }
	virtual void visit ( const TDLAxiomDRoleDomain& axiom ) { value = false; }
	// FIXME!! check later
	virtual void visit ( const TDLAxiomORoleRange& axiom ){ value = false; }
	virtual void visit ( const TDLAxiomDRoleRange& axiom ) { value = false; }
	virtual void visit ( const TDLAxiomRoleTransitive& axiom ) { value = true; }
	virtual void visit ( const TDLAxiomRoleReflexive& axiom ) { value = false; }
	virtual void visit ( const TDLAxiomRoleIrreflexive& axiom ) { value = false; }
	virtual void visit ( const TDLAxiomRoleSymmetric& axiom ) { value = false; }
	virtual void visit ( const TDLAxiomRoleAsymmetric& axiom ) { value = false; }
	virtual void visit ( const TDLAxiomORoleFunctional& axiom ) { value = false; }
	virtual void visit ( const TDLAxiomDRoleFunctional& axiom ) { value = false; }
	virtual void visit ( const TDLAxiomRoleInverseFunctional& axiom ) { value = false; }

	virtual void visit ( const TDLAxiomConceptInclusion& axiom )
	{
		if ( v(axiom.getSubC()) )
			v(axiom.getSupC());
	}
	virtual void visit ( const TDLAxiomInstanceOf& axiom ) { value = false; }
	virtual void visit ( const TDLAxiomRelatedTo& axiom ) { value = false; }
	virtual void visit ( const TDLAxiomRelatedToNot& axiom ) { value = false; }
	virtual void visit ( const TDLAxiomValueOf& axiom ) { value = false; }
	virtual void visit ( const TDLAxiomValueOfNot& axiom ) { value = false; }

	virtual void visitOntology ( TOntology& ontology )
	{
		value = true;
		for ( TOntology::iterator p = ontology.begin(), p_end = ontology.end(); value && p != p_end; ++p )
			if ( (*p)->isUsed() )
				(*p)->accept(*this);
	}
	ELFAxiomChecker ( void ) : value(true) {}
	virtual ~ELFAxiomChecker ( void ) {}
	operator bool ( void ) const { return value; }
}; // ELFAxiomChecker

#endif
