/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2008-2009 by Dmitry Tsarkov

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

#ifndef TDLAXIOM_H
#define TDLAXIOM_H

#include "dltree.h"

// forward declaration for all axiom classes: necessary for the visitor pattern
class TDLAxiomDeclaration;

class TDLAxiomEquivalentConcepts;
class TDLAxiomDisjointConcepts;
class TDLAxiomEquivalentRoles;
class TDLAxiomDisjointRoles;
class TDLAxiomSameIndividuals;
class TDLAxiomDifferentIndividuals;
class TDLAxiomFairnessConstraint;

class TDLAxiomRoleSubsumption;
class TDLAxiomRoleDomain;
class TDLAxiomRoleRange;
class TDLAxiomRoleTransitive;
class TDLAxiomRoleReflexive;
class TDLAxiomRoleIrreflexive;
class TDLAxiomRoleSymmetric;
class TDLAxiomRoleAntiSymmetric;
class TDLAxiomRoleFunctional;

class TDLAxiomConceptInclusion;
class TDLAxiomInstanceOf;
class TDLAxiomRelatedTo;
class TDLAxiomRelatedToNot;
class TDLAxiomValueOf;
class TDLAxiomValueOfNot;

class TOntology;

/// general visitor for DL axioms
class DLAxiomVisitor
{
public:		// visitor interface
	virtual void visit ( TDLAxiomDeclaration& axiom ) = 0;

	virtual void visit ( TDLAxiomEquivalentConcepts& axiom ) = 0;
	virtual void visit ( TDLAxiomDisjointConcepts& axiom ) = 0;
	virtual void visit ( TDLAxiomEquivalentRoles& axiom ) = 0;
	virtual void visit ( TDLAxiomDisjointRoles& axiom ) = 0;
	virtual void visit ( TDLAxiomSameIndividuals& axiom ) = 0;
	virtual void visit ( TDLAxiomDifferentIndividuals& axiom ) = 0;
	virtual void visit ( TDLAxiomFairnessConstraint& axiom ) = 0;

	virtual void visit ( TDLAxiomRoleSubsumption& axiom ) = 0;
	virtual void visit ( TDLAxiomRoleDomain& axiom ) = 0;
	virtual void visit ( TDLAxiomRoleRange& axiom ) = 0;
	virtual void visit ( TDLAxiomRoleTransitive& axiom ) = 0;
	virtual void visit ( TDLAxiomRoleReflexive& axiom ) = 0;
	virtual void visit ( TDLAxiomRoleIrreflexive& axiom ) = 0;
	virtual void visit ( TDLAxiomRoleSymmetric& axiom ) = 0;
	virtual void visit ( TDLAxiomRoleAntiSymmetric& axiom ) = 0;
	virtual void visit ( TDLAxiomRoleFunctional& axiom ) = 0;

	virtual void visit ( TDLAxiomConceptInclusion& axiom ) = 0;
	virtual void visit ( TDLAxiomInstanceOf& axiom ) = 0;
	virtual void visit ( TDLAxiomRelatedTo& axiom ) = 0;
	virtual void visit ( TDLAxiomRelatedToNot& axiom ) = 0;
	virtual void visit ( TDLAxiomValueOf& axiom ) = 0;
	virtual void visit ( TDLAxiomValueOfNot& axiom ) = 0;

	virtual void visitOntology ( class TOntology& ontology ) = 0;
	virtual ~DLAxiomVisitor ( void ) {}
}; // DLAxiomVisitor

/// base class for the DL axiom, which include T-, A- and RBox ones
class TDLAxiom
{
protected:	// members
		/// id of the axiom
	unsigned int id;
		/// flag to show whether it is used (to support retraction)
	bool used;

public:
		/// empty c'tor
	TDLAxiom ( void ) : used(true) {}
		/// empty d'tor
	virtual ~TDLAxiom ( void ) {}

	// id management

		/// set the id
	void setId ( unsigned int Id ) { id = Id; }
		/// get the id
	unsigned int getId ( void ) const { return id; }

	// used management

		/// set the used flag
	void setUsed ( bool Used ) { used = Used; }
		/// get the value of the used flag
	bool isUsed ( void ) const { return used; }

		/// accept method for the visitor pattern
	virtual void accept ( DLAxiomVisitor& visitor ) = 0;
}; // TDLAxiom

//------------------------------------------------------------------
///	general declaration axiom
//------------------------------------------------------------------
class TDLAxiomDeclaration: public TDLAxiom
{
protected:	// members
	DLTree* D;

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomDeclaration ( DLTree* d ) : TDLAxiom(), D(d) {}
		/// d'tor
	virtual ~TDLAxiomDeclaration ( void ) { deleteTree(D); }

		/// access
	DLTree* getDeclaration ( void ) { return D; }
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }
}; // TDLAxiomIndividual

//------------------------------------------------------------------
//	n-ary axioms
//------------------------------------------------------------------

//------------------------------------------------------------------
///	general n-argument axiom
//------------------------------------------------------------------
class TDLAxiomNAry: public TDLAxiom
{
public:		// types
		/// base type
	typedef std::vector<DLTree*> ExpressionArray;
		/// RW iterator over base type
	typedef ExpressionArray::iterator iterator;

protected:	// members
		/// set of equivalent concept descriptions
	ExpressionArray Base;

public:		// interface
		/// c'tor: create n-ary axiom for n = 2
	TDLAxiomNAry ( DLTree* C, DLTree* D )
		: TDLAxiom()
	{
		Base.push_back(C);
		Base.push_back(D);
	}
		/// c'tor: create n-ary axiom
	TDLAxiomNAry ( const ExpressionArray& v )
		: TDLAxiom()
		, Base(v)
		{}
		/// d'tor
	virtual ~TDLAxiomNAry ( void )
	{
		for ( iterator p = Base.begin(), p_end = Base.end(); p < p_end; ++p )
			deleteTree(*p);
	}

	// access to members

		/// RW begin iterator for array
	iterator begin ( void ) { return Base.begin(); }
		/// RW end iterator for array
	iterator end ( void ) { return Base.end(); }
}; // TDLAxiomNAry


//------------------------------------------------------------------
///	Concept equivalence axiom
//------------------------------------------------------------------
class TDLAxiomEquivalentConcepts: public TDLAxiomNAry
{
public:		// interface
		/// c'tor: create an axiom for C = D
	TDLAxiomEquivalentConcepts ( DLTree* C, DLTree* D ) : TDLAxiomNAry(C,D) {}
		/// c'tor: create an axiom for C1 = ... = Cn
	TDLAxiomEquivalentConcepts ( const ExpressionArray& v ) : TDLAxiomNAry(v) {}
		/// d'tor
	virtual ~TDLAxiomEquivalentConcepts ( void ) {}
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }
}; // TDLAxiomEquivalentConcepts

//------------------------------------------------------------------
///	Concept disjointness axiom
//------------------------------------------------------------------
class TDLAxiomDisjointConcepts: public TDLAxiomNAry
{
public:		// interface
		/// c'tor: create an axiom for C = D
	TDLAxiomDisjointConcepts ( DLTree* C, DLTree* D ) : TDLAxiomNAry(C,D) {}
		/// c'tor: create an axiom for C1 = ... = Cn
	TDLAxiomDisjointConcepts ( const ExpressionArray& v ) : TDLAxiomNAry(v) {}
		/// d'tor
	virtual ~TDLAxiomDisjointConcepts ( void ) {}
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }
}; // TDLAxiomDisjointConcepts

//------------------------------------------------------------------
///	Role equivalence axiom
//------------------------------------------------------------------
class TDLAxiomEquivalentRoles: public TDLAxiomNAry
{
public:		// interface
		/// c'tor: create an axiom for C = D
	TDLAxiomEquivalentRoles ( DLTree* C, DLTree* D ) : TDLAxiomNAry(C,D) {}
		/// c'tor: create an axiom for C1 = ... = Cn
	TDLAxiomEquivalentRoles ( const ExpressionArray& v ) : TDLAxiomNAry(v) {}
		/// d'tor
	virtual ~TDLAxiomEquivalentRoles ( void ) {}
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }
}; // TDLAxiomEquivalentRoles

//------------------------------------------------------------------
///	Role disjointness axiom
//------------------------------------------------------------------
class TDLAxiomDisjointRoles: public TDLAxiomNAry
{
public:		// interface
		/// c'tor: create an axiom for C = D
	TDLAxiomDisjointRoles ( DLTree* C, DLTree* D ) : TDLAxiomNAry(C,D) {}
		/// c'tor: create an axiom for C1 = ... = Cn
	TDLAxiomDisjointRoles ( const ExpressionArray& v ) : TDLAxiomNAry(v) {}
		/// d'tor
	virtual ~TDLAxiomDisjointRoles ( void ) {}
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }
}; // TDLAxiomDisjointRoles

//------------------------------------------------------------------
///	Same individuals axiom
//------------------------------------------------------------------
class TDLAxiomSameIndividuals: public TDLAxiomNAry
{
public:		// interface
		/// c'tor: create an axiom for C = D
	TDLAxiomSameIndividuals ( DLTree* C, DLTree* D ) : TDLAxiomNAry(C,D) {}
		/// c'tor: create an axiom for C1 = ... = Cn
	TDLAxiomSameIndividuals ( const ExpressionArray& v ) : TDLAxiomNAry(v) {}
		/// d'tor
	virtual ~TDLAxiomSameIndividuals ( void ) {}
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }
}; // TDLAxiomSameIndividuals

//------------------------------------------------------------------
///	Different individuals axiom
//------------------------------------------------------------------
class TDLAxiomDifferentIndividuals: public TDLAxiomNAry
{
public:		// interface
		/// c'tor: create an axiom for C = D
	TDLAxiomDifferentIndividuals ( DLTree* C, DLTree* D ) : TDLAxiomNAry(C,D) {}
		/// c'tor: create an axiom for C1 = ... = Cn
	TDLAxiomDifferentIndividuals ( const ExpressionArray& v ) : TDLAxiomNAry(v) {}
		/// d'tor
	virtual ~TDLAxiomDifferentIndividuals ( void ) {}
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }
}; // TDLAxiomDifferentIndividuals

//------------------------------------------------------------------
///	Fairness constraint axiom
//------------------------------------------------------------------
class TDLAxiomFairnessConstraint: public TDLAxiomNAry
{
public:		// interface
		/// c'tor: create an axiom for C1 = ... = Cn
	TDLAxiomFairnessConstraint ( const ExpressionArray& v ) : TDLAxiomNAry(v) {}
		/// d'tor
	virtual ~TDLAxiomFairnessConstraint ( void ) {}
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }
}; // TDLAxiomFairnessConstraint

//------------------------------------------------------------------
//	Role-related axiom
//------------------------------------------------------------------

//------------------------------------------------------------------
///	General axiom that contains a single role
//------------------------------------------------------------------
class TDLAxiomSingleRole: public TDLAxiom
{
protected:	// members
	DLTree* Role;

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomSingleRole ( DLTree* role )
		: TDLAxiom()
		, Role(role)
		{}
		/// d'tor
	virtual ~TDLAxiomSingleRole ( void ) { deleteTree(Role); }

		/// access to role
	const DLTree* getRole ( void ) const { return Role; }
}; // TDLAxiomSingleRole

//------------------------------------------------------------------
///	Role subsumption axiom
//------------------------------------------------------------------
class TDLAxiomRoleSubsumption: public TDLAxiomSingleRole
{
protected:	// members
	DLTree* SubRole;

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRoleSubsumption ( DLTree* subRole, DLTree* supRole )
		: TDLAxiomSingleRole(supRole)
		, SubRole(subRole)
		{}
		/// d'tor
	virtual ~TDLAxiomRoleSubsumption ( void ) { deleteTree(SubRole); }
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }

		/// access to role
	const DLTree* getSubRole ( void ) const { return SubRole; }
}; // TDLAxiomRoleSubsumption

//------------------------------------------------------------------
///	Role domain axiom
//------------------------------------------------------------------
class TDLAxiomRoleDomain: public TDLAxiomSingleRole
{
protected:	// members
	DLTree* Domain;

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRoleDomain ( DLTree* role, DLTree* domain )
		: TDLAxiomSingleRole(role)
		, Domain(domain)
		{}
		/// d'tor; nothing to do as Domain is consumed
	virtual ~TDLAxiomRoleDomain ( void ) { deleteTree(Domain); }
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }

		/// access
	const DLTree* getDomain ( void ) const { return Domain; }
}; // TDLAxiomRoleDomain

//------------------------------------------------------------------
///	Role range axiom
//------------------------------------------------------------------
class TDLAxiomRoleRange: public TDLAxiomSingleRole
{
protected:	// members
	DLTree* Range;

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRoleRange ( DLTree* role, DLTree* range )
		: TDLAxiomSingleRole(role)
		, Range(range)
		{}
		/// d'tor; nothing to do as Domain is consumed
	virtual ~TDLAxiomRoleRange ( void ) { deleteTree(Range); }
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }

		/// access
	const DLTree* getRange ( void ) const { return Range; }
}; // TDLAxiomRoleRange

//------------------------------------------------------------------
///	Role transitivity axiom
//------------------------------------------------------------------
class TDLAxiomRoleTransitive: public TDLAxiomSingleRole
{
public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRoleTransitive ( DLTree* role )
		: TDLAxiomSingleRole(role)
		{}
		/// d'tor;
	virtual ~TDLAxiomRoleTransitive ( void ) {}
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }
}; // TDLAxiomRoleTransitive

//------------------------------------------------------------------
///	Role reflexivity axiom
//------------------------------------------------------------------
class TDLAxiomRoleReflexive: public TDLAxiomSingleRole
{
public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRoleReflexive ( DLTree* role )
		: TDLAxiomSingleRole(role)
		{}
		/// d'tor;
	virtual ~TDLAxiomRoleReflexive ( void ) {}
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }
}; // TDLAxiomRoleReflexive

//------------------------------------------------------------------
///	Role irreflexivity axiom
//------------------------------------------------------------------
class TDLAxiomRoleIrreflexive: public TDLAxiomSingleRole
{
public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRoleIrreflexive ( DLTree* role )
		: TDLAxiomSingleRole(role)
		{}
		/// d'tor;
	virtual ~TDLAxiomRoleIrreflexive ( void ) {}
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }
}; // TDLAxiomRoleIrreflexive

//------------------------------------------------------------------
///	Role symmetry axiom
//------------------------------------------------------------------
class TDLAxiomRoleSymmetric: public TDLAxiomSingleRole
{
public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRoleSymmetric ( DLTree* role )
		: TDLAxiomSingleRole(role)
		{}
		/// d'tor;
	virtual ~TDLAxiomRoleSymmetric ( void ) {}
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }
}; // TDLAxiomRoleSymmetric

//------------------------------------------------------------------
///	Role anti-symmetry axiom
//------------------------------------------------------------------
class TDLAxiomRoleAntiSymmetric: public TDLAxiomSingleRole
{
public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRoleAntiSymmetric ( DLTree* role )
		: TDLAxiomSingleRole(role)
		{}
		/// d'tor;
	virtual ~TDLAxiomRoleAntiSymmetric ( void ) {}
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }
}; // TDLAxiomRoleAntiSymmetric

//------------------------------------------------------------------
///	Role functionality axiom
//------------------------------------------------------------------
class TDLAxiomRoleFunctional: public TDLAxiomSingleRole
{
public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRoleFunctional ( DLTree* role )
		: TDLAxiomSingleRole(role)
		{}
		/// d'tor;
	virtual ~TDLAxiomRoleFunctional ( void ) {}
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }
}; // TDLAxiomRoleFunctional


//------------------------------------------------------------------
//	Concept/individual axioms
//------------------------------------------------------------------

//------------------------------------------------------------------
///	Concept inclusion axiom
//------------------------------------------------------------------
class TDLAxiomConceptInclusion: public TDLAxiom
{
protected:	// members
	DLTree* Sub;
	DLTree* Sup;

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomConceptInclusion ( DLTree* sub, DLTree* sup ) : TDLAxiom(), Sub(sub), Sup(sup) {}
		/// d'tor
	virtual ~TDLAxiomConceptInclusion ( void ) { deleteTree(Sub); deleteTree(Sup); }
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }

		/// access
	const DLTree* getSubC ( void ) const { return Sub; }
		/// access
	const DLTree* getSupC ( void ) const { return Sup; }
}; // TDLAxiomConceptInclusion

//------------------------------------------------------------------
///	general individual-based axiom
//------------------------------------------------------------------
class TDLAxiomIndividual: public TDLAxiom
{
protected:	// members
	DLTree* I;

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomIndividual ( DLTree* i ) : TDLAxiom(), I(i) {}
		/// d'tor
	virtual ~TDLAxiomIndividual ( void ) { deleteTree(I); }

		/// access
	const DLTree* getIndividual ( void ) const { return I; }
}; // TDLAxiomIndividual

//------------------------------------------------------------------
///	Instance axiom
//------------------------------------------------------------------
class TDLAxiomInstanceOf: public TDLAxiomIndividual
{
protected:	// members
	DLTree* C;

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomInstanceOf ( DLTree* i, DLTree* c ) : TDLAxiomIndividual(i), C(c) {}
		/// d'tor
	virtual ~TDLAxiomInstanceOf ( void ) { deleteTree(C); }
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }

		/// access
	const DLTree* getC ( void ) const { return C; }
}; // TDLAxiomInstanceOf

//------------------------------------------------------------------
///	Related To axiom
//------------------------------------------------------------------
class TDLAxiomRelatedTo: public TDLAxiomIndividual
{
protected:	// members
	DLTree* R;
	DLTree* J;

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRelatedTo ( DLTree* i, DLTree* r, DLTree* j ) : TDLAxiomIndividual(i), R(r), J(j) {}
		/// d'tor
	virtual ~TDLAxiomRelatedTo ( void ) { deleteTree(R); deleteTree(J); }
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }

		/// access
	const DLTree* getRelation ( void ) const { return R; }
		/// access
	const DLTree* getRelatedIndividual ( void ) const { return J; }
}; // TDLAxiomRelatedTo

//------------------------------------------------------------------
///	Related To Not axiom
//------------------------------------------------------------------
class TDLAxiomRelatedToNot: public TDLAxiomIndividual
{
protected:	// members
	DLTree* R;
	DLTree* J;

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRelatedToNot ( DLTree* i, DLTree* r, DLTree* j ) : TDLAxiomIndividual(i), R(r), J(j) {}
		/// d'tor
	virtual ~TDLAxiomRelatedToNot ( void ) { deleteTree(R); deleteTree(J); }
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }

		/// access
	const DLTree* getRelation ( void ) const { return R; }
		/// access
	const DLTree* getRelatedIndividual ( void ) const { return J; }
}; // TDLAxiomRelatedToNot

//------------------------------------------------------------------
///	Value Of axiom
//------------------------------------------------------------------
class TDLAxiomValueOf: public TDLAxiomIndividual
{
protected:	// members
	DLTree* A;
	DLTree* V;

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomValueOf ( DLTree* i, DLTree* a, DLTree* v ) : TDLAxiomIndividual(i), A(a), V(v) {}
		/// d'tor
	virtual ~TDLAxiomValueOf ( void ) { deleteTree(A); deleteTree(V); }
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }

		/// access to role
	const DLTree* getAttribute ( void ) const { return A; }
		/// access to value
	const DLTree* getValue ( void ) const { return V; }
}; // TDLAxiomValueOf

//------------------------------------------------------------------
///	Related To Not axiom
//------------------------------------------------------------------
class TDLAxiomValueOfNot: public TDLAxiomIndividual
{
protected:	// members
	DLTree* A;
	DLTree* V;

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomValueOfNot ( DLTree* i, DLTree* a, DLTree* v ) : TDLAxiomIndividual(i), A(a), V(v) {}
		/// d'tor
	virtual ~TDLAxiomValueOfNot ( void ) { deleteTree(A); deleteTree(V); }
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }

		/// access to role
	const DLTree* getAttribute ( void ) const { return A; }
		/// access to value
	const DLTree* getValue ( void ) const { return V; }
}; // TDLAxiomValueOfNot


#endif
