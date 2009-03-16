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

	// methods to redefine in the derived classes

		/// check the correctness of the axiom
	virtual bool correct ( void ) const = 0;
}; // TDLAxiom

//------------------------------------------------------------------
//	Concept inclusion axiom
//------------------------------------------------------------------

	/// axiom for Sub [= Sup
class TDLAxiomConceptInclusion: public TDLAxiom
{
protected:	// members
	DLTree* Sub;
	DLTree* Sup;

protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb ) { kb.addSubsumeAxiom ( clone(Sub), clone(Sup) ); }

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomConceptInclusion ( DLTree* sub, DLTree* sup ) : TDLAxiom(), Sub(sub), Sup(sup) {}
		/// d'tor
	virtual ~TDLAxiomConceptInclusion ( void ) { deleteTree(Sub); deleteTree(Sup); }

	// methods to redefine in the derived classes

		/// check the correctness of the axiom
	virtual bool correct ( void ) const { return true; }
}; // TDLAxiomConceptInclusion

//------------------------------------------------------------------
//	Concept equivalence axiom
//------------------------------------------------------------------
class TDLAxiomConceptEquivalence: public TDLAxiom
{
protected:	// types
		/// base type
	typedef TBox::ConceptSet ConceptSet;
		/// RW iterator over base type
	typedef ConceptSet::iterator iterator;
		/// RO iterator over base type
	typedef ConceptSet::const_iterator const_iterator;

protected:	// members
		/// set of equivalent concept descriptions
	ConceptSet Base;

protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb ) { kb.processEquivalent(Base); }

public:		// interface
		/// c'tor: create an axiom for C = D
	TDLAxiomConceptEquivalence ( DLTree* C, DLTree* D )
		: TDLAxiom()
	{
		Base.push_back(C);
		Base.push_back(D);
	}
		/// c'tor: create an axiom for C1 = ... = Cn
	TDLAxiomConceptEquivalence ( const ConceptSet& v )
		: TDLAxiom()
	{
		for ( const_iterator p = v.begin(), p_end = v.end(); p < p_end; ++p )
			Base.push_back(clone(*p));
	}
		/// d'tor
	virtual ~TDLAxiomConceptEquivalence ( void ) {}

	// methods to redefine in the derived classes

		/// check the correctness of the axiom
	virtual bool correct ( void ) const { return true; }
}; // TDLAxiomConceptEquivalence

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
		/// ger role by the DLTree; throw exception if unable
	TRole* getRole ( const DLTree* r, const char* reason ) const
	{
		try { return resolveRole(r); }
		catch ( EFaCTPlusPlus e ) { throw EFaCTPlusPlus(reason); }
	}


public:		// interface
		/// c'tor: create an axiom
	TDLAxiomSingleRole ( DLTree* role )
		: TDLAxiom()
		, Role(role)
		{}
		/// d'tor
	virtual ~TDLAxiomSingleRole ( void ) { deleteTree(Role); }

	// methods to redefine in the derived classes

		/// check the correctness of the axiom
	virtual bool correct ( void ) const
	{
		try { resolveRole(Role); return true; }
		catch (...) { return false; }
	}
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

	// methods to redefine in the derived classes

		/// check the correctness of the axiom
	virtual bool correct ( void ) const
	{	// FIXME!! add check for the role/composition here later on
		return TDLAxiomSingleRole::correct();
	}
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


#endif
