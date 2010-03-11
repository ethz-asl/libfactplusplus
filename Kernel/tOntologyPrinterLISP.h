/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2009-2010 by Dmitry Tsarkov

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

#ifndef TONTOLOGYPRINTERLISP_H
#define TONTOLOGYPRINTERLISP_H

#include <iostream>

class TLISPOntologyPrinter: public DLAxiomVisitor
{
protected:	// members
	std::ostream& o;

protected:	// methods
		/// helper to print several expressions in a row
	template<class Iterator>
	void print ( Iterator beg, Iterator end )
	{
		for ( ; beg < end; ++beg )
			o << *beg;
	}
		/// helper to print name (w/o one-of) of the individual
	void pn ( const DLTree* p ) { o << ' ' << p->Element().getName(); }

public:		// visitor interface
	virtual void visit ( TDLAxiomDeclaration& axiom )
	{
		const DLTree* decl = axiom.getDeclaration();
		o << "(def";
		switch ( decl->Element().getToken() )
		{
		case CNAME:
			o << "primconcept";
			break;
		case INAME:
			o << "individual";
			break;
		case RNAME:
			o << "primrole";
			break;
		case DNAME:
			o << "datarole";
			break;
		default:
			break;
		}
		o << decl << ")\n";
	}

	virtual void visit ( TDLAxiomEquivalentConcepts& axiom ) { o << "(equal_c"; print ( axiom.begin(), axiom.end() ); o << ")\n"; }
	virtual void visit ( TDLAxiomDisjointConcepts& axiom ) { o << "(disjoint_c"; print ( axiom.begin(), axiom.end() ); o << ")\n"; }
	virtual void visit ( TDLAxiomEquivalentRoles& axiom ) { o << "(equal_r"; print ( axiom.begin(), axiom.end() ); o << ")\n"; }
	virtual void visit ( TDLAxiomDisjointRoles& axiom ) { o << "(disjoint_r"; print ( axiom.begin(), axiom.end() ); o << ")\n"; }
	virtual void visit ( TDLAxiomSameIndividuals& axiom ) { o << "(same"; print ( axiom.begin(), axiom.end() ); o << ")\n"; }
	virtual void visit ( TDLAxiomDifferentIndividuals& axiom ) { o << "(different"; print ( axiom.begin(), axiom.end() ); o << ")\n"; }
	virtual void visit ( TDLAxiomFairnessConstraint& axiom ) { o << "(fairness"; print ( axiom.begin(), axiom.end() ); o << ")\n"; }

	virtual void visit ( TDLAxiomRoleSubsumption& axiom ) { o << "(implies_r" << axiom.getSubRole() << axiom.getRole() << ")\n"; }
	virtual void visit ( TDLAxiomRoleDomain& axiom ) { o << "(domain" << axiom.getRole() << axiom.getDomain() << ")\n"; }
	virtual void visit ( TDLAxiomRoleRange& axiom ) { o << "(range" << axiom.getRole() << axiom.getRange() << ")\n"; }
	virtual void visit ( TDLAxiomRoleTransitive& axiom ) { o << "(transitive" << axiom.getRole() << ")\n"; }
	virtual void visit ( TDLAxiomRoleReflexive& axiom ) { o << "(reflexive" << axiom.getRole() << ")\n"; }
	virtual void visit ( TDLAxiomRoleIrreflexive& axiom ) { o << "(irreflexive" << axiom.getRole() << ")\n"; }
	virtual void visit ( TDLAxiomRoleSymmetric& axiom ) { o << "(symmetric" << axiom.getRole() << ")\n"; }
	virtual void visit ( TDLAxiomRoleAntiSymmetric& axiom ) { o << "(antisymmetric" << axiom.getRole() << ")\n"; }
	virtual void visit ( TDLAxiomRoleFunctional& axiom ) { o << "(functional" << axiom.getRole() << ")\n"; }

	virtual void visit ( TDLAxiomConceptInclusion& axiom ) { o << "(implies_c" << axiom.getSubC() << axiom.getSupC() << ")\n"; }
	virtual void visit ( TDLAxiomInstanceOf& axiom ) { o << "(instance"; pn(axiom.getIndividual()); o << axiom.getC() << ")\n"; }
	virtual void visit ( TDLAxiomRelatedTo& axiom )
		{ o << "(related"; pn(axiom.getIndividual()); o << axiom.getRelation(); pn(axiom.getRelatedIndividual()); o << ")\n"; }
	virtual void visit ( TDLAxiomRelatedToNot& axiom )
		{ o << "(instance"; pn(axiom.getIndividual()); o << " (all" << axiom.getRelation() << "(not" << axiom.getRelatedIndividual() << ")))\n"; }
	virtual void visit ( TDLAxiomValueOf& axiom )
		{ o << "(instance"; pn(axiom.getIndividual()); o << " (some" << axiom.getAttribute() << axiom.getValue() << "))\n"; }
	virtual void visit ( TDLAxiomValueOfNot& axiom )
		{ o << "(instance"; pn(axiom.getIndividual()); o << " (all" << axiom.getAttribute() << "(not " << axiom.getValue() << ")))\n"; }

public:		// interface
	TLISPOntologyPrinter ( std::ostream& o_ ) : o(o_) {}
	virtual ~TLISPOntologyPrinter ( void ) {}
	void recordDataRole ( const char* name ) { o << "(defdatarole " << name << ")\n"; }

	virtual void visitOntology ( class TOntology& ontology )
	{
		for ( TOntology::iterator p = ontology.begin(), p_end = ontology.end(); p < p_end; ++p )
			(*p)->accept(*this);
		o << std::endl;
	}
}; // TLISPOntologyPrinter

#endif
