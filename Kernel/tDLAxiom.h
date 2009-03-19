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

#include "dlTBox.h"

/// base class for the DL axiom, which include T-, A- and RBox ones
class TDLAxiom
{
protected:	// members
		/// id of the axiom
	unsigned int id;

protected:	// methods
		/// get role by the DLTree; throw exception if unable
	static TRole* getRole ( const DLTree* r, const char* reason )
	{
		try { return resolveRole(r); }
		catch ( EFaCTPlusPlus e ) { throw EFaCTPlusPlus(reason); }
	}
		/// get an individual be the DLTree; throw exception if unable
	static TIndividual* getIndividual ( DLTree* I, const char* reason )
	{
		if ( I->Element().getToken() == INAME )
			return static_cast<TIndividual*>(I->Element().getName());
		else
			throw EFaCTPlusPlus(reason);
	}
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb ) = 0;

public:
		/// empty c'tor
	TDLAxiom ( void ) {}
		/// empty d'tor
	virtual ~TDLAxiom ( void ) {}

	// id management

		/// set the id
	void setId ( unsigned int Id ) { id = Id; }
		/// get the id
	unsigned int getId ( void ) const { return id; }

		/// load axiom into the KB taking into account its ID
	void load ( TBox& kb )
	{
		kb.setAxiomId(getId());
		loadInto(kb);
	}
}; // TDLAxiom

//------------------------------------------------------------------
//	n-ary axioms
//------------------------------------------------------------------

//------------------------------------------------------------------
///	general n-argument axiom
//------------------------------------------------------------------
class TDLAxiomNAry: public TDLAxiom
{
protected:	// types
		/// base type
	typedef std::vector<DLTree*> ExpressionArray;
		/// RW iterator over base type
	typedef ExpressionArray::iterator iterator;

protected:	// members
		/// set of equivalent concept descriptions
	ExpressionArray Base;

protected:	// methods
		/// clear the array if necessary
	void clear ( void )
	{
		for ( iterator p = Base.begin(), p_end = Base.end(); p < p_end; ++p )
			deleteTree(*p);
	}
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb ) = 0;

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
	virtual ~TDLAxiomNAry ( void ) {}
}; // TDLAxiomNAry


//------------------------------------------------------------------
///	Concept equivalence axiom
//------------------------------------------------------------------
class TDLAxiomEquivalentConcepts: public TDLAxiomNAry
{
protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb ) { kb.processEquivalent(Base); }

public:		// interface
		/// c'tor: create an axiom for C = D
	TDLAxiomEquivalentConcepts ( DLTree* C, DLTree* D ) : TDLAxiomNAry(C,D) {}
		/// c'tor: create an axiom for C1 = ... = Cn
	TDLAxiomEquivalentConcepts ( const ExpressionArray& v ) : TDLAxiomNAry(v) {}
		/// d'tor
	virtual ~TDLAxiomEquivalentConcepts ( void ) {}
}; // TDLAxiomEquivalentConcepts

//------------------------------------------------------------------
///	Concept disjointness axiom
//------------------------------------------------------------------
class TDLAxiomDisjointConcepts: public TDLAxiomNAry
{
protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb ) { kb.processDisjoint(Base); }

public:		// interface
		/// c'tor: create an axiom for C = D
	TDLAxiomDisjointConcepts ( DLTree* C, DLTree* D ) : TDLAxiomNAry(C,D) {}
		/// c'tor: create an axiom for C1 = ... = Cn
	TDLAxiomDisjointConcepts ( const ExpressionArray& v ) : TDLAxiomNAry(v) {}
		/// d'tor
	virtual ~TDLAxiomDisjointConcepts ( void ) {}
}; // TDLAxiomDisjointConcepts

//------------------------------------------------------------------
///	Role equivalence axiom
//------------------------------------------------------------------
class TDLAxiomEquivalentRoles: public TDLAxiomNAry
{
protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb ) { kb.processEquivalentR(Base); }

public:		// interface
		/// c'tor: create an axiom for C = D
	TDLAxiomEquivalentRoles ( DLTree* C, DLTree* D ) : TDLAxiomNAry(C,D) {}
		/// c'tor: create an axiom for C1 = ... = Cn
	TDLAxiomEquivalentRoles ( const ExpressionArray& v ) : TDLAxiomNAry(v) {}
		/// d'tor
	virtual ~TDLAxiomEquivalentRoles ( void ) { clear(); }
}; // TDLAxiomEquivalentRoles

//------------------------------------------------------------------
///	Role disjointness axiom
//------------------------------------------------------------------
class TDLAxiomDisjointRoles: public TDLAxiomNAry
{
protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb ) { kb.processDisjointR(Base); }

public:		// interface
		/// c'tor: create an axiom for C = D
	TDLAxiomDisjointRoles ( DLTree* C, DLTree* D ) : TDLAxiomNAry(C,D) {}
		/// c'tor: create an axiom for C1 = ... = Cn
	TDLAxiomDisjointRoles ( const ExpressionArray& v ) : TDLAxiomNAry(v) {}
		/// d'tor
	virtual ~TDLAxiomDisjointRoles ( void ) { clear(); }
}; // TDLAxiomDisjointRoles

//------------------------------------------------------------------
///	Same individuals axiom
//------------------------------------------------------------------
class TDLAxiomSameIndividuals: public TDLAxiomNAry
{
protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb ) { kb.processSame(Base); }

public:		// interface
		/// c'tor: create an axiom for C = D
	TDLAxiomSameIndividuals ( DLTree* C, DLTree* D ) : TDLAxiomNAry(C,D) {}
		/// c'tor: create an axiom for C1 = ... = Cn
	TDLAxiomSameIndividuals ( const ExpressionArray& v ) : TDLAxiomNAry(v) {}
		/// d'tor
	virtual ~TDLAxiomSameIndividuals ( void ) {}
}; // TDLAxiomSameIndividuals

//------------------------------------------------------------------
///	Different individuals axiom
//------------------------------------------------------------------
class TDLAxiomDifferentIndividuals: public TDLAxiomNAry
{
protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb ) { kb.processDifferent(Base); }

public:		// interface
		/// c'tor: create an axiom for C = D
	TDLAxiomDifferentIndividuals ( DLTree* C, DLTree* D ) : TDLAxiomNAry(C,D) {}
		/// c'tor: create an axiom for C1 = ... = Cn
	TDLAxiomDifferentIndividuals ( const ExpressionArray& v ) : TDLAxiomNAry(v) {}
		/// d'tor
	virtual ~TDLAxiomDifferentIndividuals ( void ) { clear(); }
}; // TDLAxiomDifferentIndividuals

//------------------------------------------------------------------
///	Fairness constraint axiom
//------------------------------------------------------------------
class TDLAxiomFairnessConstraint: public TDLAxiomNAry
{
protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb ) { kb.setFairnessConstraint(Base); }

public:		// interface
		/// c'tor: create an axiom for C1 = ... = Cn
	TDLAxiomFairnessConstraint ( const ExpressionArray& v ) : TDLAxiomNAry(v) {}
		/// d'tor
	virtual ~TDLAxiomFairnessConstraint ( void ) {}
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

protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb ) = 0;

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomSingleRole ( DLTree* role )
		: TDLAxiom()
		, Role(role)
		{}
		/// d'tor
	virtual ~TDLAxiomSingleRole ( void ) { deleteTree(Role); }
}; // TDLAxiomSingleRole

//------------------------------------------------------------------
///	Role subsumption axiom
//------------------------------------------------------------------
class TDLAxiomRoleSubsumption: public TDLAxiomSingleRole
{
protected:	// members
	DLTree* SubRole;

protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb )
		{ kb.getRM()->addRoleParent ( SubRole, getRole ( Role, "Role expression expected in Roles Subsumption axiom" ) ); }

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRoleSubsumption ( DLTree* subRole, DLTree* supRole )
		: TDLAxiomSingleRole(supRole)
		, SubRole(subRole)
		{}
		/// d'tor
	virtual ~TDLAxiomRoleSubsumption ( void ) { deleteTree(SubRole); }
}; // TDLAxiomRoleSubsumption

//------------------------------------------------------------------
///	Role domain axiom
//------------------------------------------------------------------
class TDLAxiomRoleDomain: public TDLAxiomSingleRole
{
protected:	// members
	DLTree* Domain;

protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb ATTR_UNUSED )
		{ getRole ( Role, "Role expression expected in Roles Domain axiom" )->setDomain(Domain); }

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRoleDomain ( DLTree* role, DLTree* domain )
		: TDLAxiomSingleRole(role)
		, Domain(domain)
		{}
		/// d'tor; nothing to do as Domain is consumed
	virtual ~TDLAxiomRoleDomain ( void ) {}
}; // TDLAxiomRoleDomain

//------------------------------------------------------------------
///	Role range axiom
//------------------------------------------------------------------
class TDLAxiomRoleRange: public TDLAxiomSingleRole
{
protected:	// members
	DLTree* Range;

protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb ATTR_UNUSED )
		{ getRole ( Role, "Role expression expected in Roles Range axiom" )->setRange(Range); }

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRoleRange ( DLTree* role, DLTree* range )
		: TDLAxiomSingleRole(role)
		, Range(range)
		{}
		/// d'tor; nothing to do as Domain is consumed
	virtual ~TDLAxiomRoleRange ( void ) {}
}; // TDLAxiomRoleRange

//------------------------------------------------------------------
///	Role transitivity axiom
//------------------------------------------------------------------
class TDLAxiomRoleTransitive: public TDLAxiomSingleRole
{
protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb ATTR_UNUSED )
	{
		if ( !isUniversalRole(Role) )
			getRole ( Role, "Role expression expected in Roles Transitivity axiom" )->setBothTransitive();
	}

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRoleTransitive ( DLTree* role )
		: TDLAxiomSingleRole(role)
		{}
		/// d'tor;
	virtual ~TDLAxiomRoleTransitive ( void ) {}
}; // TDLAxiomRoleTransitive

//------------------------------------------------------------------
///	Role reflexivity axiom
//------------------------------------------------------------------
class TDLAxiomRoleReflexive: public TDLAxiomSingleRole
{
protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb ATTR_UNUSED )
	{
		if ( !isUniversalRole(Role) )
			getRole ( Role, "Role expression expected in Roles Reflexivity axiom" )->setBothReflexive();
	}

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRoleReflexive ( DLTree* role )
		: TDLAxiomSingleRole(role)
		{}
		/// d'tor;
	virtual ~TDLAxiomRoleReflexive ( void ) {}
}; // TDLAxiomRoleReflexive

//------------------------------------------------------------------
///	Role irreflexivity axiom
//------------------------------------------------------------------
class TDLAxiomRoleIrreflexive: public TDLAxiomSingleRole
{
protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb ATTR_UNUSED )
	{
		if ( isUniversalRole(Role) )	// KB became inconsistent
			throw EFPPInconsistentKB();
		getRole ( Role, "Role expression expected in Roles Irreflexivity axiom" )->setDomain(new DLTree(NOT,new DLTree(REFLEXIVE,clone(Role))));
	}

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRoleIrreflexive ( DLTree* role )
		: TDLAxiomSingleRole(role)
		{}
		/// d'tor;
	virtual ~TDLAxiomRoleIrreflexive ( void ) {}
}; // TDLAxiomRoleIrreflexive

//------------------------------------------------------------------
///	Role symmetry axiom
//------------------------------------------------------------------
class TDLAxiomRoleSymmetric: public TDLAxiomSingleRole
{
protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb )
	{
		if ( !isUniversalRole(Role) )
		{
			TRole* invR = getRole ( Role, "Role expression expected in Roles Symmetry axiom" )->inverse();
			kb.getRM()->addRoleParent ( Role, invR );
		}
	}

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRoleSymmetric ( DLTree* role )
		: TDLAxiomSingleRole(role)
		{}
		/// d'tor;
	virtual ~TDLAxiomRoleSymmetric ( void ) {}
}; // TDLAxiomRoleSymmetric

//------------------------------------------------------------------
///	Role anti-symmetry axiom
//------------------------------------------------------------------
class TDLAxiomRoleAntiSymmetric: public TDLAxiomSingleRole
{
protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb )
	{
		if ( isUniversalRole(Role) )	// KB became inconsistent
			throw EFPPInconsistentKB();
		TRole* R = getRole ( Role, "Role expression expected in Roles AntiSymmetry axiom" );
		kb.getRM()->addDisjointRoles ( R, R->inverse() );
	}

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRoleAntiSymmetric ( DLTree* role )
		: TDLAxiomSingleRole(role)
		{}
		/// d'tor;
	virtual ~TDLAxiomRoleAntiSymmetric ( void ) {}
}; // TDLAxiomRoleAntiSymmetric

//------------------------------------------------------------------
///	Role functionality axiom
//------------------------------------------------------------------
class TDLAxiomRoleFunctional: public TDLAxiomSingleRole
{
protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb ATTR_UNUSED )
	{
		if ( isUniversalRole(Role) )	// KB became inconsistent
			throw EFPPInconsistentKB();
		getRole ( Role, "Role expression expected in Roles Functionality axiom" )->setFunctional();
	}

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRoleFunctional ( DLTree* role )
		: TDLAxiomSingleRole(role)
		{}
		/// d'tor;
	virtual ~TDLAxiomRoleFunctional ( void ) {}
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

protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb ) { kb.addSubsumeAxiom ( Sub, Sup ); }

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomConceptInclusion ( DLTree* sub, DLTree* sup ) : TDLAxiom(), Sub(sub), Sup(sup) {}
		/// d'tor
	virtual ~TDLAxiomConceptInclusion ( void ) {}
}; // TDLAxiomConceptInclusion

//------------------------------------------------------------------
///	general individual-based axiom
//------------------------------------------------------------------
class TDLAxiomIndividual: public TDLAxiom
{
protected:	// members
	DLTree* I;

protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb ) = 0;

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomIndividual ( DLTree* i ) : TDLAxiom(), I(i) {}
		/// d'tor
	virtual ~TDLAxiomIndividual ( void ) { deleteTree(I); }
}; // TDLAxiomIndividual

//------------------------------------------------------------------
///	Instance axiom
//------------------------------------------------------------------
class TDLAxiomInstanceOf: public TDLAxiomIndividual
{
protected:	// members
	DLTree* C;

protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb ) { kb.RegisterInstance ( getIndividual ( I, "Individual expected in Instance axiom" ), C ); }

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomInstanceOf ( DLTree* i, DLTree* c ) : TDLAxiomIndividual(i), C(c) {}
		/// d'tor
	virtual ~TDLAxiomInstanceOf ( void ) {}
}; // TDLAxiomInstanceOf

//------------------------------------------------------------------
///	Related To axiom
//------------------------------------------------------------------
class TDLAxiomRelatedTo: public TDLAxiomIndividual
{
protected:	// members
	DLTree* R;
	DLTree* J;

protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb )
	{
		if ( !isUniversalRole(R) )	// nothing to do for universal role
			kb.RegisterIndividualRelation (
				getIndividual ( I, "Individual expected in Related To axiom" ),
				getRole ( R, "Role expression expected in Related To axiom" ),
				getIndividual ( J, "Individual expected in Related To axiom" ) );
	}

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRelatedTo ( DLTree* i, DLTree* r, DLTree* j ) : TDLAxiomIndividual(i), R(r), J(j) {}
		/// d'tor
	virtual ~TDLAxiomRelatedTo ( void ) { deleteTree(R); deleteTree(J); }
}; // TDLAxiomRelatedTo

//------------------------------------------------------------------
///	Related To Not axiom
//------------------------------------------------------------------
class TDLAxiomRelatedToNot: public TDLAxiomIndividual
{
protected:	// members
	DLTree* R;
	DLTree* J;

protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb )
	{
		if ( isUniversalRole(R) )	// inconsistent ontology
			throw EFPPInconsistentKB();
		// make sure everything is consistent
		getIndividual ( J, "Individual expected in Related To Not axiom" );
		// make an axiom i:AR.\neg{j}
		kb.RegisterInstance (
				getIndividual ( I, "Individual expected in Related To Not axiom" ),
				new DLTree(FORALL,R,new DLTree(NOT,J)) );
	}

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomRelatedToNot ( DLTree* i, DLTree* r, DLTree* j ) : TDLAxiomIndividual(i), R(r), J(j) {}
		/// d'tor
	virtual ~TDLAxiomRelatedToNot ( void ) {}
}; // TDLAxiomRelatedToNot

//------------------------------------------------------------------
///	Value Of axiom
//------------------------------------------------------------------
class TDLAxiomValueOf: public TDLAxiomIndividual
{
protected:	// members
	DLTree* A;
	DLTree* V;

protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb )
	{
		if ( isUniversalRole(A) )	// data role can't be universal
			throw EFPPInconsistentKB();
		kb.RegisterInstance (
				getIndividual ( I, "Individual expected in Value Of axiom" ),
				new DLTree(EXISTS,A,V) );
	}

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomValueOf ( DLTree* i, DLTree* a, DLTree* v ) : TDLAxiomIndividual(i), A(a), V(v) {}
		/// d'tor
	virtual ~TDLAxiomValueOf ( void ) {}
}; // TDLAxiomValueOf

//------------------------------------------------------------------
///	Related To Not axiom
//------------------------------------------------------------------
class TDLAxiomValueOfNot: public TDLAxiomIndividual
{
protected:	// members
	DLTree* A;
	DLTree* V;

protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb )
	{
		if ( isUniversalRole(A) )	// data role can't be universal
			throw EFPPInconsistentKB();
		// make an axiom i:AR.\neg{j}
		kb.RegisterInstance (
				getIndividual ( I, "Individual expected in Related To Not axiom" ),
				new DLTree(FORALL,A,new DLTree(NOT,V)) );
	}

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomValueOfNot ( DLTree* i, DLTree* a, DLTree* v ) : TDLAxiomIndividual(i), A(a), V(v) {}
		/// d'tor
	virtual ~TDLAxiomValueOfNot ( void ) {}
}; // TDLAxiomValueOfNot


#endif
