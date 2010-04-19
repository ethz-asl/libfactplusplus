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

#include "tExpressionPrinterLISP.h"

class TLISPOntologyPrinter: public DLAxiomVisitor
{
protected:	// members
		/// main stream
	std::ostream& o;
		/// printer for the expressions
	TLISPExpressionPrinter LEP;

protected:	// methods
		/// helper to print several expressions in a row
	template<class Iterator>
	void print ( Iterator beg, Iterator end )
	{
		for ( ; beg < end; ++beg )
			(*beg)->accept(LEP);
	}
		/// helper to print name (w/o one-of) of the individual
	void pn ( const DLTree* p ) { o << ' ' << p->Element().getName(); }
		/// helper to print a string
	TLISPOntologyPrinter& operator << ( const char* str ) { o << str; return *this; }
		/// helper to print an expression
	TLISPOntologyPrinter& operator << ( const TDLExpression* expr ) { expr->accept(LEP); return *this; }

public:		// visitor interface
	virtual void visit ( TDLAxiomDeclaration& axiom )
	{
		const TDLExpression* decl = axiom.getDeclaration();
		bool cname = dynamic_cast<const TDLConceptName*>(decl) != NULL;
		bool iname = dynamic_cast<const TDLIndividualName*>(decl) != NULL;
		bool rname = dynamic_cast<const TDLObjectRoleName*>(decl) != NULL;
		bool dname = dynamic_cast<const TDLDataRoleName*>(decl) != NULL;

		// do not print TOP/BOT
		if ( !cname && !iname && !rname && !dname )
			return;

		*this << "(def" <<
			(cname ? "primconcept" :
			 iname ? "individual" :
			 rname ? "primrole" :
				    "datarole")
		<< decl << ")\n";
	}

	virtual void visit ( TDLAxiomEquivalentConcepts& axiom ) { o << "(equal_c"; print ( axiom.begin(), axiom.end() ); o << ")\n"; }
	virtual void visit ( TDLAxiomDisjointConcepts& axiom ) { o << "(disjoint_c"; print ( axiom.begin(), axiom.end() ); o << ")\n"; }
	virtual void visit ( TDLAxiomEquivalentORoles& axiom ) { o << "(equal_r"; print ( axiom.begin(), axiom.end() ); o << ")\n"; }
	virtual void visit ( TDLAxiomEquivalentDRoles& axiom ) { o << "(equal_r"; print ( axiom.begin(), axiom.end() ); o << ")\n"; }
	virtual void visit ( TDLAxiomDisjointORoles& axiom ) { o << "(disjoint_r"; print ( axiom.begin(), axiom.end() ); o << ")\n"; }
	virtual void visit ( TDLAxiomDisjointDRoles& axiom ) { o << "(disjoint_r"; print ( axiom.begin(), axiom.end() ); o << ")\n"; }
	virtual void visit ( TDLAxiomSameIndividuals& axiom ) { o << "(same"; print ( axiom.begin(), axiom.end() ); o << ")\n"; }
	virtual void visit ( TDLAxiomDifferentIndividuals& axiom ) { o << "(different"; print ( axiom.begin(), axiom.end() ); o << ")\n"; }
	virtual void visit ( TDLAxiomFairnessConstraint& axiom ) { o << "(fairness"; print ( axiom.begin(), axiom.end() ); o << ")\n"; }

	virtual void visit ( TDLAxiomRoleInverse& axiom ) { *this << "(equal_r" << axiom.getRole() << " (inv" << axiom.getInvRole() << "))\n"; }
	virtual void visit ( TDLAxiomORoleSubsumption& axiom ) { *this << "(implies_r" << axiom.getSubRole() << axiom.getRole() << ")\n"; }
	virtual void visit ( TDLAxiomDRoleSubsumption& axiom ) { *this << "(implies_r" << axiom.getSubRole() << axiom.getRole() << ")\n"; }
	virtual void visit ( TDLAxiomORoleDomain& axiom ) { *this << "(domain" << axiom.getRole() << axiom.getDomain() << ")\n"; }
	virtual void visit ( TDLAxiomDRoleDomain& axiom ) { *this << "(domain" << axiom.getRole() << axiom.getDomain() << ")\n"; }
	virtual void visit ( TDLAxiomORoleRange& axiom ) { *this << "(range" << axiom.getRole() << axiom.getRange() << ")\n"; }
	virtual void visit ( TDLAxiomDRoleRange& axiom ) { *this << "(range" << axiom.getRole() << axiom.getRange() << ")\n"; }
	virtual void visit ( TDLAxiomRoleTransitive& axiom ) { *this << "(transitive" << axiom.getRole() << ")\n"; }
	virtual void visit ( TDLAxiomRoleReflexive& axiom ) { *this << "(reflexive" << axiom.getRole() << ")\n"; }
	virtual void visit ( TDLAxiomRoleIrreflexive& axiom ) { *this << "(irreflexive" << axiom.getRole() << ")\n"; }
	virtual void visit ( TDLAxiomRoleSymmetric& axiom ) { *this << "(symmetric" << axiom.getRole() << ")\n"; }
	virtual void visit ( TDLAxiomRoleAntiSymmetric& axiom ) { *this << "(antisymmetric" << axiom.getRole() << ")\n"; }
	virtual void visit ( TDLAxiomORoleFunctional& axiom ) { *this << "(functional" << axiom.getRole() << ")\n"; }
	virtual void visit ( TDLAxiomDRoleFunctional& axiom ) { *this << "(functional" << axiom.getRole() << ")\n"; }
	virtual void visit ( TDLAxiomRoleInverseFunctional& axiom ) { *this << "(functional (inv" << axiom.getRole() << "))\n"; }

	virtual void visit ( TDLAxiomConceptInclusion& axiom ) { *this << "(implies_c" << axiom.getSubC() << axiom.getSupC() << ")\n"; }
	virtual void visit ( TDLAxiomInstanceOf& axiom ) { *this << "(instance" << axiom.getIndividual()  << axiom.getC() << ")\n"; }
	virtual void visit ( TDLAxiomRelatedTo& axiom )
		{ *this << "(related" << axiom.getIndividual() << axiom.getRelation() << axiom.getRelatedIndividual() << ")\n"; }
	virtual void visit ( TDLAxiomRelatedToNot& axiom )
		{ *this<< "(instance" << axiom.getIndividual() << " (all" << axiom.getRelation() << "(not" << axiom.getRelatedIndividual() << ")))\n"; }
	virtual void visit ( TDLAxiomValueOf& axiom )
		{ *this << "(instance" << axiom.getIndividual() << " (some" << axiom.getAttribute() << axiom.getValue() << "))\n"; }
	virtual void visit ( TDLAxiomValueOfNot& axiom )
		{ *this << "(instance" << axiom.getIndividual() << " (all" << axiom.getAttribute() << "(not " << axiom.getValue() << ")))\n"; }

public:		// interface
	TLISPOntologyPrinter ( std::ostream& o_ ) : o(o_), LEP(o_) {}
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
