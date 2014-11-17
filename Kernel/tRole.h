/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2014 by Dmitry Tsarkov

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

#ifndef TROLE_H
#define TROLE_H

#include <set>
#include <vector>

#include "globaldef.h"	// sorted reasoning
#include "BiPointer.h"
#include "dltree.h"
#include "taxNamEntry.h"
#include "tLabeller.h"
#include "RAutomaton.h"
#include "eFPPNonSimpleRole.h"
#include "eFPPCycleInRIA.h"

#ifdef RKG_USE_SORTED_REASONING
#	include "mergableLabel.h"
#endif

class TBox;
class RoleMaster;
class Taxonomy;
class SaveLoadManager;

/// Define class with all information about DL role
class TRole: public ClassifiableEntry
{
	friend class RoleMaster;

private:	// no copy
		/// no empty c'tor
	TRole ();
		/// no copy c'tor
	TRole ( const TRole& );
		/// no assignment
	TRole& operator = ( const TRole& );

protected:	// types
		/// Class for values that can change wrt ontology
	class TKnownValue
	{
	protected:	// members
			/// flag value
		bool value;
			/// whether flag set or not
		bool known;

	public:		// interface
			/// init c'tor
		TKnownValue ( bool val = false ) : value(val), known(false) {}
			/// empty d'tor
		~TKnownValue ( void ) {}

			/// @return true iff the value is known to be set
		bool isKnown ( void ) const { return known; }
			/// @return the value
		bool getValue ( void ) const { return value; }
			/// set the value; it is now known
		void setValue ( bool val ) { value = val; known = true; }
	}; // TKnownValue

public:		// types
		/// vector of roles
	typedef std::vector<TRole*> TRoleVec;
		/// RO iterator over role vector
	typedef TRoleVec::const_iterator const_iterator;
		/// set of roles
	typedef std::set<TRole*> TRoleSet;
		/// bitmap for roles
	typedef std::vector<bool> TRoleBitMap;

protected:	// members
		/// role that are inverse of given one
	TRole* Inverse;

		/// Domain of role as a concept description; default NULL
	DLTree* pDomain;
		/// Domain of role as a concept description; default NULL
	DLTree* pSpecialDomain;
		/// Domain of role as a pointer to DAG entry
	BipolarPointer bpDomain;
		/// domain in the form AR.Range for the complex roles
	BipolarPointer bpSpecialDomain;

		/// pointer to role's functional definition DAG entry (or just TOP)
	BipolarPointer Functional;

		/// is role relevant to current query
	TLabeller::LabelType rel;

#ifdef RKG_USE_SORTED_REASONING
		/// label of a domain (inverse role is used for a range label)
	mergableLabel domLabel;
#endif

	// for later filling
	// FIXME!! const was removed for Relevance setting
	TRoleVec Ancestor, Descendant;
		/// set of the most functional super-roles
	TRoleVec TopFunc;
		/// set of the roles that are disjoint with a given one
	TRoleSet Disjoint;
		/// all compositions in the form R1*R2*\ldots*Rn [= R
	std::vector<TRoleVec> subCompositions;

		/// bit-vector of all parents
	TRoleBitMap AncMap;
		/// bit-vector of all roles disjoint with current
	TRoleBitMap DJRoles;

		/// automaton for role
	RoleAutomaton A;

		/// value for functionality
	TKnownValue Functionality;
		/// value for symmetry
	TKnownValue Symmetry;
		/// value for asymmetricity
	TKnownValue Asymmetry;
		/// value for transitivity
	TKnownValue Transitivity;
		/// value for reflexivity
	TKnownValue Reflexivity;
		/// value for reflexivity
	TKnownValue Irreflexivity;
		/// flag to show that this role needs special R&D processing
	bool SpecialDomain;

protected:	// methods
	// support for Anc/Desc filling and such

		/// check if the role is REALLY topmost-functional (internal-use only)
	bool isRealTopFunc ( void ) const;
		/// set up TopFunc member properly (internal-use only)
	void initTopFunc ( void );
		/// init map of all disjoint roles
	void initDJMap ( void );

		/// eliminate told role cycle, carrying aux arrays of processed roles and synonyms
	TRole* eliminateToldCycles ( TRoleSet& RInProcess, TRoleVec& ToldSynonyms );

	// support for automaton construction

		/// complete role automaton; keep track of processed roles in RINPROCESS
	void completeAutomaton ( TRoleSet& RInProcess );
		/// replace RoR [= R with Trans(R), replace synonyms in RS
	void preprocessComposition ( TRoleVec& RS );
		/// add transition to automaton with the role
	void addTrivialTransition ( const TRole* r )
		{ A.addTransitionSafe ( A.initial(), new RATransition ( A.final(), r ) ); }
		/// add automaton of a sub-role to a given one
	void addSubRoleAutomaton ( const TRole* R )
	{
		if ( this != R )	// non-trivial addition
			A.addRA(R->getAutomaton());
	}
		/// get an automaton by a (possibly synonymical) role
	const RoleAutomaton& completeAutomatonByRole ( TRole* R, TRoleSet& RInProcess ) const
	{
		fpp_assert ( !R->isSynonym() );	// no synonyms here
		fpp_assert ( R != this );		// no case ...*S*... [= S
		R->completeAutomaton(RInProcess);
		return R->getAutomaton();
	}
		/// add automaton for a role composition; simplify composition
	void addSubCompositionAutomaton ( TRoleVec& RS, TRoleSet& RInProcess );

		/// check (and correct) case whether R != S for R [= S
	void checkHierarchicalDisjoint ( TRole* R );
		/// check whether there is a sub-property with special domain; propagate this
	void checkSpecialDomain ( void )
	{
		if ( hasSpecialDomain() )
			return;
		for ( const_iterator p = begin_desc(), p_end = end_desc(); p != p_end; ++p )
			if ( (*p)->hasSpecialDomain() )
			{
				SpecialDomain = true;
				return;
			}
	}

public:		// interface
		/// the only c'tor
	TRole ( const std::string& name );
		/// d'tor
	virtual ~TRole ( void );

		/// get (unsigned) unique index of the role
	unsigned int getIndex ( void ) const
	{
		int i = 2*getId();
		return i > 0
			? (unsigned int)i
			: (unsigned int)(1-i);
	}

	// synonym operations

		/// copy role information (like transitivity, functionality, R&D etc) to synonym
	void addFeaturesToSynonym ( void );

	// inverse of the role

		/// get inverse of given role (non-const version)
	TRole* inverse ( void ) { fpp_assert (Inverse != NULL); return resolveSynonym(Inverse); }
		/// get inverse of given role (const version)
	const TRole* inverse ( void ) const { fpp_assert (Inverse != NULL); return resolveSynonym(Inverse); }
		/// get real inverse of a role (RO)
	const TRole* realInverse ( void ) const { fpp_assert (Inverse != NULL); return Inverse; }
		/// set inverse to given role
	void setInverse ( TRole* p ) { fpp_assert (Inverse == NULL); Inverse = p; }

	// different flags

		/// distinguish data- and non-data role
	FPP_ADD_FLAG(DataRole,0x10);

	// simple

		// @return true iff the role is simple
	bool isSimple ( void ) const { return A.isSimple(); }

	// functionality

		/// test if role is functional (ie, have some functional ancestors)
	bool isFunctional ( void ) const { return Functionality.getValue(); }
		/// check whether the functionality of a role is known
	bool isFunctionalityKnown ( void ) const { return Functionality.isKnown(); }
		/// check if the role is topmost-functional (ie, has no functional ancestors).
	bool isTopFunc ( void ) const
	{	// check for emptyness is here due to case where a role is determined to be a functional
		return !TopFunc.empty() && *TopFunc.begin() == this;
	}
		/// set role functionality value
	void setFunctional ( bool value ) { Functionality.setValue(value); }
		/// mark role (topmost) functional
	void setFunctional ( void )
	{
		if ( TopFunc.empty() )
			TopFunc.push_back(this);
		setFunctional(true);
	}
		/// set functional attribute to given value (functional DAG vertex)
	void setFunctional ( BipolarPointer fNode ) { Functional = fNode; }
		/// get the Functional DAG vertex
	BipolarPointer getFunctional ( void ) const { return Functional; }

	// transitivity

		/// check whether the role is transitive
	bool isTransitive ( void ) const { return Transitivity.getValue(); }
		/// check whether the transitivity of a role is known
	bool isTransitivityKnown ( void ) const { return Transitivity.isKnown(); }
		/// set the transitivity of both role and it's inverse
	void setTransitive ( bool value = true ) { Transitivity.setValue(value); inverse()->Transitivity.setValue(value); }

	// symmetry

		/// check whether the role is symmetric
	bool isSymmetric ( void ) const { return Symmetry.getValue(); }
		/// check whether the symmetry of a role is known
	bool isSymmetryKnown ( void ) const { return Symmetry.isKnown(); }
		/// set the symmetry of both role and it's inverse
	void setSymmetric ( bool value = true ) { Symmetry.setValue(value); inverse()->Symmetry.setValue(value); }

	// asymmetry

		/// check whether the role is asymmetric
	bool isAsymmetric ( void ) const { return Asymmetry.getValue(); }
		/// check whether the asymmetry of a role is known
	bool isAsymmetryKnown ( void ) const { return Asymmetry.isKnown(); }
		/// set the asymmetry of both role and it's inverse
	void setAsymmetric ( bool value = true ) { Asymmetry.setValue(value); inverse()->Asymmetry.setValue(value); }

	// reflexivity

		/// check whether the role is reflexive
	bool isReflexive ( void ) const { return Reflexivity.getValue(); }
		/// check whether the reflexivity of a role is known
	bool isReflexivityKnown ( void ) const { return Reflexivity.isKnown(); }
		/// set the reflexivity of both role and it's inverse
	void setReflexive ( bool value = true ) { Reflexivity.setValue(value); inverse()->Reflexivity.setValue(value); }

	// irreflexivity

		/// check whether the role is irreflexive
	bool isIrreflexive ( void ) const { return Irreflexivity.getValue(); }
		/// check whether the irreflexivity of a role is known
	bool isIrreflexivityKnown ( void ) const { return Irreflexivity.isKnown(); }
		/// set the irreflexivity of both role and it's inverse
	void setIrreflexive ( bool value = true ) { Irreflexivity.setValue(value); inverse()->Irreflexivity.setValue(value); }

	// relevance

		/// is given role relevant to given Labeller's state
	bool isRelevant ( const TLabeller& lab ) const { return lab.isLabelled(rel); }
		/// make given role relevant to given Labeller's state
	void setRelevant ( const TLabeller& lab ) { lab.set(rel); }

#ifdef RKG_USE_SORTED_REASONING
	// Sorted reasoning interface

		/// get label of the role's domain
	mergableLabel& getDomainLabel ( void ) { return domLabel; }
		/// get label of the role's range
	mergableLabel& getRangeLabel ( void ) { return inverse()->getDomainLabel(); }
		/// merge label of given role and all its super-roles
	void mergeSupersDomain ( void );
#endif

	// domain and range

		/// add p to domain of the role
	void setDomain ( DLTree* p )
	{
		if ( equalTrees ( pDomain, p ) )	// not just a CName
			deleteTree(p);	// usual case when you have a name for inverse role
		else if ( isFunctionalExpr ( p, this ) )
		{
			setFunctional();
			deleteTree(p);	// functional restriction in the role domain means the role is functional
		}
		else
			pDomain = createSNFReducedAnd ( pDomain, p );
	}
		/// add p to range of the role
	void setRange ( DLTree* p ) { inverse()->setDomain(p); }

		/// get domain-as-a-tree of the role
	DLTree* getTDomain ( void ) const { return pDomain; }
		/// get range-as-a-tree of the role
	DLTree* getTRange ( void ) const { return inverse()->pDomain; }

		/// get special-domain-as-a-tree
	DLTree* getTSpecialDomain ( void ) { return pSpecialDomain; }

#ifdef RKG_UPDATE_RND_FROM_SUPERROLES
		/// merge to Domain all domains from super-roles
	void collectDomainFromSupers ( void )
	{
		for ( iterator p = begin_anc(); p != end_anc(); ++p )
			setDomain ( clone((*p)->getTDomain()) );
	}
#endif

		/// set domain-as-a-bipointer to a role
	void setBPDomain ( BipolarPointer p ) { bpDomain = p; }
		/// get domain-as-a-bipointer of the role
	BipolarPointer getBPDomain ( void ) const { return bpDomain; }
		/// get range-as-a-bipointer of the role
	BipolarPointer getBPRange ( void ) const { return inverse()->bpDomain; }
		/// @return true iff role has a special domain
	bool hasSpecialDomain ( void ) const { return SpecialDomain; }
		/// init special domain; call this only after *ALL* the domains are known
	void initSpecialDomain ( void )
	{
		if ( !hasSpecialDomain() || getTRange() == NULL )
			pSpecialDomain = createTop();
		else
			pSpecialDomain = createSNFForall ( createRole(this), clone(getTRange()) );
	}
		/// set the special domain value
	void setSpecialDomain ( BipolarPointer bp ) { bpSpecialDomain = bp; }
		/// get special domain value
	BipolarPointer getSpecialDomain ( void ) const { return bpSpecialDomain; }
		/// get special range value
	BipolarPointer getSpecialRange ( void ) const { return inverse()->bpSpecialDomain; }

	// disjoint roles

		/// set R and THIS as a disjoint; use it after Anc/Desc are determined
	void addDisjointRole ( TRole* R )
	{
		Disjoint.insert(R);
		for ( const_iterator p = R->begin_desc(), p_end = R->end_desc(); p != p_end; ++p )
		{
			Disjoint.insert(*p);
			(*p)->Disjoint.insert(this);
		}
	}
		/// check (and correct) case whether R != S for R [= S
	void checkHierarchicalDisjoint ( void )
	{
		checkHierarchicalDisjoint(this);
		if ( isReflexive() )	// for reflexive roles check for R^- is necessary
			checkHierarchicalDisjoint(inverse());
	}
		/// check whether a role is disjoint with anything
	bool isDisjoint ( void ) const { return !Disjoint.empty(); }
		/// check whether a role is disjoint with R
	bool isDisjoint ( const TRole* r ) const { return DJRoles[r->getIndex()]; }

	// role relations checking

		/// two roles are the same iff thy are synonyms of the same role
	bool operator == ( const TRole& r ) const { return this == &r; }
		/// check if role is a strict sub-role of R
	bool operator < ( const TRole& r ) const { return (isDataRole() == r.isDataRole()) && AncMap[r.getIndex()]; }
		/// check if role is a non-strict sub-role of R
	bool operator <= ( const TRole& r ) const { return (*this == r) || (*this < r); }
		/// check if role is a strict super-role of R
	bool operator > ( const TRole& r ) const { return r < *this; }
		/// check if role is a non-strict super-role of R
	bool operator >= ( const TRole& r ) const { return (*this == r) || (*this > r); }

	// iterators

		/// get access to all super-roles via iterator
	const_iterator begin_anc ( void ) const { return Ancestor.begin(); }
	const_iterator end_anc ( void ) const { return Ancestor.end(); }
		/// get access to all sub-roles via iterator
	const_iterator begin_desc ( void ) const { return Descendant.begin(); }
	const_iterator end_desc ( void ) const { return Descendant.end(); }
		/// get access to the func super-roles w/o func parents via iterator
	const_iterator begin_topfunc ( void ) const { return TopFunc.begin(); }
	const_iterator end_topfunc ( void ) const { return TopFunc.end(); }

		/// fills BITMAP with the role's ancestors
	void addAncestorsToBitMap ( TRoleBitMap& bitmap ) const
	{
		fpp_assert ( !bitmap.empty() );	// use only after the size is known
		for ( const_iterator p = begin_anc(), p_end = end_anc(); p != p_end; ++p )
			bitmap[(*p)->getIndex()] = true;
	}

	// automaton construction

		/// add composition to a role
	void addComposition ( const DLTree* tree )
	{
		TRoleVec RS;
		fillsComposition ( RS, tree );
		subCompositions.push_back(RS);
	}
		/// get access to a RA for the role
	const RoleAutomaton& getAutomaton ( void ) const { return A; }

	// completing internal constructions

		/// eliminate told role cycle
	TRole* eliminateToldCycles ( void )
	{
		TRoleSet RInProcess;
		TRoleVec ToldSynonyms;
		return eliminateToldCycles ( RInProcess, ToldSynonyms );
	}
		/// init ancestors and descendants using Taxonomy
	void initADbyTaxonomy ( Taxonomy* pTax, unsigned int ADMapSize );
		/// init other fields that requires Anc/Desc for all roles
	void postProcess ( void );
		/// fills role composition by given TREE
	void fillsComposition ( TRoleVec& Composition, const DLTree* tree ) const;
		/// complete role automaton
	void completeAutomaton ( unsigned int nRoles )
	{
		TRoleSet RInProcess;
		completeAutomaton(RInProcess);
		A.setup ( nRoles, isDataRole() );
	}
		/// check whether role description is consistent
	void consistent ( void ) const
	{
		if ( isSimple() )		// all simple roles are consistent
			return;
		if ( isFunctional() )	// non-simple role can not be functional
			throw EFPPNonSimpleRole(getName());
		if ( isDataRole() )		// data role can not be non-simple
			throw EFPPNonSimpleRole(getName());
		if ( isDisjoint() )		// non-simple role can not be disjoint with any role
			throw EFPPNonSimpleRole(getName());
	}

	// output

		/// print role to given stream
	void Print ( std::ostream& o ) const;

	// save/load interface; implementation is in SaveLoad.cpp

		/// save entry
	virtual void Save ( SaveLoadManager& m ) const;
		/// load entry
	virtual void Load ( SaveLoadManager& m );
}; // TRole

/// @return R or -R for T in the form (inv ... (inv R)...)
inline
TRole*
resolveRoleHelper ( const DLTree* t )
{
	if ( t == NULL )			// empty tree -- error
		throw EFaCTPlusPlus("Role expression expected");
	switch ( t->Element().getToken() )
	{
	case RNAME:	// role name
	case DNAME:	// data role name
		return static_cast<TRole*>(t->Element().getNE());
	case INV:	// inversion
		return resolveRoleHelper(t->Left())->inverse();
	default:	// error
		throw EFaCTPlusPlus("Invalid role expression");
	}
}

/// @return R or -R for T in the form (inv ... (inv R)...); remove synonyms
inline TRole* resolveRole ( const DLTree* t ) { return resolveSynonym(resolveRoleHelper(t)); }

//--------------------------------------------------
//	TRole implementation
//--------------------------------------------------
inline TRole :: TRole ( const std::string& name )
	: ClassifiableEntry(name)
	, Inverse(NULL)
	, pDomain(NULL)
	, pSpecialDomain(NULL)
	, bpDomain(bpINVALID)
	, bpSpecialDomain(bpINVALID)
	, Functional(bpINVALID)
	, rel(0)
	, SpecialDomain(false)
{
	setCompletelyDefined (true);	// role hierarchy is completely defined by it's parents
	addTrivialTransition (this);
}

inline TRole :: ~TRole ( void )
{
	deleteTree(pDomain);
	deleteTree(pSpecialDomain);
	if ( Inverse != NULL && Inverse != this )
	{
		Inverse->Inverse = NULL;
		delete Inverse;
	}
}

//--------------------------------------------------
//	RAStateTransitions implementation depending on TRole
//--------------------------------------------------

/// check whether one of the transitions accept R
inline bool
RAStateTransitions :: recognise ( const TRole* R ) const { return R != NULL && R->isDataRole() == DataRole && ApplicableRoles.in(R->getIndex()); }

#endif
