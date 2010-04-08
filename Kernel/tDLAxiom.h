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
class TDLAxiomEquivalentORoles;
class TDLAxiomEquivalentDRoles;
class TDLAxiomDisjointORoles;
class TDLAxiomDisjointDRoles;
class TDLAxiomSameIndividuals;
class TDLAxiomDifferentIndividuals;
class TDLAxiomFairnessConstraint;

class TDLAxiomRoleInverse;
class TDLAxiomORoleSubsumption;
class TDLAxiomDRoleSubsumption;
class TDLAxiomORoleDomain;
class TDLAxiomDRoleDomain;
class TDLAxiomORoleRange;
class TDLAxiomDRoleRange;
class TDLAxiomRoleTransitive;
class TDLAxiomRoleReflexive;
class TDLAxiomRoleIrreflexive;
class TDLAxiomRoleSymmetric;
class TDLAxiomRoleAntiSymmetric;
class TDLAxiomORoleFunctional;
class TDLAxiomDRoleFunctional;
class TDLAxiomRoleInverseFunctional;

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
	virtual void visit ( TDLAxiomEquivalentORoles& axiom ) = 0;
	virtual void visit ( TDLAxiomEquivalentDRoles& axiom ) = 0;
	virtual void visit ( TDLAxiomDisjointORoles& axiom ) = 0;
	virtual void visit ( TDLAxiomDisjointDRoles& axiom ) = 0;
	virtual void visit ( TDLAxiomSameIndividuals& axiom ) = 0;
	virtual void visit ( TDLAxiomDifferentIndividuals& axiom ) = 0;
	virtual void visit ( TDLAxiomFairnessConstraint& axiom ) = 0;

	virtual void visit ( TDLAxiomRoleInverse& axiom ) = 0;
	virtual void visit ( TDLAxiomORoleSubsumption& axiom ) = 0;
	virtual void visit ( TDLAxiomDRoleSubsumption& axiom ) = 0;
	virtual void visit ( TDLAxiomORoleDomain& axiom ) = 0;
	virtual void visit ( TDLAxiomDRoleDomain& axiom ) = 0;
	virtual void visit ( TDLAxiomORoleRange& axiom ) = 0;
	virtual void visit ( TDLAxiomDRoleRange& axiom ) = 0;
	virtual void visit ( TDLAxiomRoleTransitive& axiom ) = 0;
	virtual void visit ( TDLAxiomRoleReflexive& axiom ) = 0;
	virtual void visit ( TDLAxiomRoleIrreflexive& axiom ) = 0;
	virtual void visit ( TDLAxiomRoleSymmetric& axiom ) = 0;
	virtual void visit ( TDLAxiomRoleAntiSymmetric& axiom ) = 0;
	virtual void visit ( TDLAxiomORoleFunctional& axiom ) = 0;
	virtual void visit ( TDLAxiomDRoleFunctional& axiom ) = 0;
	virtual void visit ( TDLAxiomRoleInverseFunctional& axiom ) = 0;

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
///	Object Role equivalence axiom
//------------------------------------------------------------------
class TDLAxiomEquivalentORoles: public TDLAxiomNAry
{
public:		// interface
		/// c'tor: create an axiom for C = D
	TDLAxiomEquivalentORoles ( DLTree* C, DLTree* D ) : TDLAxiomNAry(C,D) {}
		/// c'tor: create an axiom for C1 = ... = Cn
	TDLAxiomEquivalentORoles ( const ExpressionArray& v ) : TDLAxiomNAry(v) {}
		/// d'tor
	virtual ~TDLAxiomEquivalentORoles ( void ) {}
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }
}; // TDLAxiomEquivalentORoles

//------------------------------------------------------------------
///	Data Role equivalence axiom
//------------------------------------------------------------------
class TDLAxiomEquivalentDRoles: public TDLAxiomNAry
{
public:		// interface
		/// c'tor: create an axiom for C = D
	TDLAxiomEquivalentDRoles ( DLTree* C, DLTree* D ) : TDLAxiomNAry(C,D) {}
		/// c'tor: create an axiom for C1 = ... = Cn
	TDLAxiomEquivalentDRoles ( const ExpressionArray& v ) : TDLAxiomNAry(v) {}
		/// d'tor
	virtual ~TDLAxiomEquivalentDRoles ( void ) {}
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }
}; // TDLAxiomEquivalentDRoles

//------------------------------------------------------------------
///	Object Role disjointness axiom
//------------------------------------------------------------------
class TDLAxiomDisjointORoles: public TDLAxiomNAry
{
public:		// interface
		/// c'tor: create an axiom for C = D
	TDLAxiomDisjointORoles ( DLTree* C, DLTree* D ) : TDLAxiomNAry(C,D) {}
		/// c'tor: create an axiom for C1 = ... = Cn
	TDLAxiomDisjointORoles ( const ExpressionArray& v ) : TDLAxiomNAry(v) {}
		/// d'tor
	virtual ~TDLAxiomDisjointORoles ( void ) {}
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }
}; // TDLAxiomDisjointORoles

//------------------------------------------------------------------
///	Object Role disjointness axiom
//------------------------------------------------------------------
class TDLAxiomDisjointDRoles: public TDLAxiomNAry
{
public:		// interface
		/// c'tor: create an axiom for C = D
	TDLAxiomDisjointDRoles ( DLTree* C, DLTree* D ) : TDLAxiomNAry(C,D) {}
		/// c'tor: create an axiom for C1 = ... = Cn
	TDLAxiomDisjointDRoles ( const ExpressionArray& v ) : TDLAxiomNAry(v) {}
		/// d'tor
	virtual ~TDLAxiomDisjointDRoles ( void ) {}
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }
}; // TDLAxiomDisjointDRoles

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
///	General axiom that contains a single object role
//------------------------------------------------------------------
class TDLAxiomSingleORole: public TDLAxiom
{
protected:	// members
	DLTree* Role;

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomSingleORole ( DLTree* role )
		: TDLAxiom()
		, Role(role)
		{}
		/// d'tor
	virtual ~TDLAxiomSingleORole ( void ) { deleteTree(Role); }

		/// access to role
	const DLTree* getRole ( void ) const { return Role; }
}; // TDLAxiomSingleORole

//------------------------------------------------------------------
///	General axiom that contains a single data role
//------------------------------------------------------------------
class TDLAxiomSingleDRole: public TDLAxiom
{
protected:	// members
	DLTree* Role;

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomSingleDRole ( DLTree* role )
		: TDLAxiom()
		, Role(role)
		{}
		/// d'tor
	virtual ~TDLAxiomSingleDRole ( void ) { deleteTree(Role); }

		/// access to role
	const DLTree* getRole ( void ) const { return Role; }
}; // TDLAxiomSingleDRole

//------------------------------------------------------------------
///	Role inverse axiom
//------------------------------------------------------------------
class TDLAxiomRoleInverse: public TDLAxiomSingleORole
{
protected:	// members
	DLTree* InvRole;

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRoleInverse ( DLTree* dirRole, DLTree* invRole )
		: TDLAxiomSingleORole(dirRole)
		, InvRole(invRole)
		{}
		/// d'tor
	virtual ~TDLAxiomRoleInverse ( void ) { deleteTree(InvRole); }
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }

		/// access to role
	const DLTree* getInvRole ( void ) const { return InvRole; }
}; // TDLAxiomRoleInverse

//------------------------------------------------------------------
///	Object Role subsumption axiom
//------------------------------------------------------------------
class TDLAxiomORoleSubsumption: public TDLAxiomSingleORole
{
protected:	// members
	DLTree* SubRole;

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomORoleSubsumption ( DLTree* subRole, DLTree* supRole )
		: TDLAxiomSingleORole(supRole)
		, SubRole(subRole)
		{}
		/// d'tor
	virtual ~TDLAxiomORoleSubsumption ( void ) { deleteTree(SubRole); }
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }

		/// access to role
	const DLTree* getSubRole ( void ) const { return SubRole; }
}; // TDLAxiomORoleSubsumption

//------------------------------------------------------------------
///	Data Role subsumption axiom
//------------------------------------------------------------------
class TDLAxiomDRoleSubsumption: public TDLAxiomSingleDRole
{
protected:	// members
	DLTree* SubRole;

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomDRoleSubsumption ( DLTree* subRole, DLTree* supRole )
		: TDLAxiomSingleDRole(supRole)
		, SubRole(subRole)
		{}
		/// d'tor
	virtual ~TDLAxiomDRoleSubsumption ( void ) { deleteTree(SubRole); }
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }

		/// access to role
	const DLTree* getSubRole ( void ) const { return SubRole; }
}; // TDLAxiomDRoleSubsumption

//------------------------------------------------------------------
///	Object Role domain axiom
//------------------------------------------------------------------
class TDLAxiomORoleDomain: public TDLAxiomSingleORole
{
protected:	// members
	DLTree* Domain;

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomORoleDomain ( DLTree* role, DLTree* domain )
		: TDLAxiomSingleORole(role)
		, Domain(domain)
		{}
		/// d'tor; nothing to do as Domain is consumed
	virtual ~TDLAxiomORoleDomain ( void ) { deleteTree(Domain); }
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }

		/// access
	const DLTree* getDomain ( void ) const { return Domain; }
}; // TDLAxiomORoleDomain

//------------------------------------------------------------------
///	Data Role domain axiom
//------------------------------------------------------------------
class TDLAxiomDRoleDomain: public TDLAxiomSingleDRole
{
protected:	// members
	DLTree* Domain;

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomDRoleDomain ( DLTree* role, DLTree* domain )
		: TDLAxiomSingleDRole(role)
		, Domain(domain)
		{}
		/// d'tor; nothing to do as Domain is consumed
	virtual ~TDLAxiomDRoleDomain ( void ) { deleteTree(Domain); }
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }

		/// access
	const DLTree* getDomain ( void ) const { return Domain; }
}; // TDLAxiomDRoleDomain

//------------------------------------------------------------------
///	Object Role range axiom
//------------------------------------------------------------------
class TDLAxiomORoleRange: public TDLAxiomSingleORole
{
protected:	// members
	DLTree* Range;

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomORoleRange ( DLTree* role, DLTree* range )
		: TDLAxiomSingleORole(role)
		, Range(range)
		{}
		/// d'tor; nothing to do as Domain is consumed
	virtual ~TDLAxiomORoleRange ( void ) { deleteTree(Range); }
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }

		/// access
	const DLTree* getRange ( void ) const { return Range; }
}; // TDLAxiomORoleRange

//------------------------------------------------------------------
///	Data Role range axiom
//------------------------------------------------------------------
class TDLAxiomDRoleRange: public TDLAxiomSingleDRole
{
protected:	// members
	DLTree* Range;

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomDRoleRange ( DLTree* role, DLTree* range )
		: TDLAxiomSingleDRole(role)
		, Range(range)
		{}
		/// d'tor; nothing to do as Domain is consumed
	virtual ~TDLAxiomDRoleRange ( void ) { deleteTree(Range); }
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }

		/// access
	const DLTree* getRange ( void ) const { return Range; }
}; // TDLAxiomDRoleRange

//------------------------------------------------------------------
///	Role transitivity axiom
//------------------------------------------------------------------
class TDLAxiomRoleTransitive: public TDLAxiomSingleORole
{
public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRoleTransitive ( DLTree* role )
		: TDLAxiomSingleORole(role)
		{}
		/// d'tor;
	virtual ~TDLAxiomRoleTransitive ( void ) {}
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }
}; // TDLAxiomRoleTransitive

//------------------------------------------------------------------
///	Role reflexivity axiom
//------------------------------------------------------------------
class TDLAxiomRoleReflexive: public TDLAxiomSingleORole
{
public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRoleReflexive ( DLTree* role )
		: TDLAxiomSingleORole(role)
		{}
		/// d'tor;
	virtual ~TDLAxiomRoleReflexive ( void ) {}
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }
}; // TDLAxiomRoleReflexive

//------------------------------------------------------------------
///	Role irreflexivity axiom
//------------------------------------------------------------------
class TDLAxiomRoleIrreflexive: public TDLAxiomSingleORole
{
public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRoleIrreflexive ( DLTree* role )
		: TDLAxiomSingleORole(role)
		{}
		/// d'tor;
	virtual ~TDLAxiomRoleIrreflexive ( void ) {}
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }
}; // TDLAxiomRoleIrreflexive

//------------------------------------------------------------------
///	Role symmetry axiom
//------------------------------------------------------------------
class TDLAxiomRoleSymmetric: public TDLAxiomSingleORole
{
public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRoleSymmetric ( DLTree* role )
		: TDLAxiomSingleORole(role)
		{}
		/// d'tor;
	virtual ~TDLAxiomRoleSymmetric ( void ) {}
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }
}; // TDLAxiomRoleSymmetric

//------------------------------------------------------------------
///	Role anti-symmetry axiom
//------------------------------------------------------------------
class TDLAxiomRoleAntiSymmetric: public TDLAxiomSingleORole
{
public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRoleAntiSymmetric ( DLTree* role )
		: TDLAxiomSingleORole(role)
		{}
		/// d'tor;
	virtual ~TDLAxiomRoleAntiSymmetric ( void ) {}
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }
}; // TDLAxiomRoleAntiSymmetric

//------------------------------------------------------------------
///	Object Role functionality axiom
//------------------------------------------------------------------
class TDLAxiomORoleFunctional: public TDLAxiomSingleORole
{
public:		// interface
		/// c'tor: create an axiom
	TDLAxiomORoleFunctional ( DLTree* role )
		: TDLAxiomSingleORole(role)
		{}
		/// d'tor;
	virtual ~TDLAxiomORoleFunctional ( void ) {}
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }
}; // TDLAxiomORoleFunctional

//------------------------------------------------------------------
///	Data Role functionality axiom
//------------------------------------------------------------------
class TDLAxiomDRoleFunctional: public TDLAxiomSingleDRole
{
public:		// interface
		/// c'tor: create an axiom
	TDLAxiomDRoleFunctional ( DLTree* role )
		: TDLAxiomSingleDRole(role)
		{}
		/// d'tor;
	virtual ~TDLAxiomDRoleFunctional ( void ) {}
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }
}; // TDLAxiomDRoleFunctional

//------------------------------------------------------------------
///	Role inverse functionality axiom
//------------------------------------------------------------------
class TDLAxiomRoleInverseFunctional: public TDLAxiomSingleORole
{
public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRoleInverseFunctional ( DLTree* role )
		: TDLAxiomSingleORole(role)
		{}
		/// d'tor;
	virtual ~TDLAxiomRoleInverseFunctional ( void ) {}
		/// accept method for the visitor pattern
	void accept ( DLAxiomVisitor& visitor ) { visitor.visit(*this); }
}; // TDLAxiomRoleInverseFunctional


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
