/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2012 by Dmitry Tsarkov

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

#ifndef ROLEMASTER_H
#define ROLEMASTER_H

#include "globaldef.h"
#include "tNameSet.h"
#include "eFPPCantRegName.h"
#include "tRole.h"
#include "Taxonomy.h"

class RoleMaster
{
public:		// types
		/// vector of roles
	typedef TRole::TRoleVec TRoleVec;
		/// RW access to roles
	typedef TRoleVec::iterator iterator;
		/// RO access to roles
	typedef TRoleVec::const_iterator const_iterator;

protected:	// members
		/// number of the last registered role
	int newRoleId;

		/// all registered roles
	TRoleVec Roles;
		/// internal empty role (bottom in the taxonomy)
	TRole emptyRole;
		/// internal universal role (top in the taxonomy)
	TRole universalRole;

		/// roles nameset
	TNameSet<TRole> roleNS;
		/// Taxonomy of roles
	Taxonomy* pTax;

		/// two halves of disjoint roles axioms
	TRoleVec DJRolesA, DJRolesB;

		/// flag whether to create data roles or not
	bool DataRoles;

		/// flag if it is possible to introduce new names
	bool useUndefinedNames;

private:	// methods
		/// no copy c'tor
	RoleMaster ( const RoleMaster& );
		/// no assignment
	RoleMaster& operator = ( const RoleMaster& );
		/// constant defining first user role in the RBox
	static unsigned int firstRoleIndex ( void ) { return 2; }

protected:	// methods
		/// register TRole and it's inverse in RoleBox
	void registerRole ( TRole* r )
	{
		fpp_assert ( r != NULL && r->Inverse == NULL );	// sanity check
		fpp_assert ( r->getId() == 0 );	// only call it for the new roles

		if ( DataRoles )
			r->setDataRole();

		Roles.push_back (r);
		r->setId (newRoleId);

		// create new role which would be inverse of R
		std::string iname ("-");
		iname += r->getName();
		TRole* ri = new TRole(iname);

		// set up inverse
		r->setInverse(ri);
		ri->setInverse(r);

		Roles.push_back (ri);
		ri->setId (-newRoleId);
		++newRoleId;
	}
		/// @return true if P is a role that is registered in the RM
	bool isRegisteredRole ( const TNamedEntry* p ) const
	{
		const TRole* R = reinterpret_cast<const TRole*>(p);
		if ( R == NULL )
			return false;
		unsigned int ind = R->getIndex();
		return ( ind >= firstRoleIndex() &&
				 ind < Roles.size() &&
				 Roles[ind] == p );
	}

		/// add parent for the input role; both roles are not synonyms
	void addRoleParentProper ( TRole* role, TRole* parent ) const;

		/// get number of roles
	size_t size ( void ) const { return Roles.size()/2-1; }

public:		// interface
		/// the only c'tor
	RoleMaster ( bool dataRoles, const std::string& TopRoleName, const std::string& BotRoleName )
		: newRoleId(1)
		, emptyRole(BotRoleName == "" ? "emptyRole" : BotRoleName)
		, universalRole(TopRoleName == "" ? "universalRole" : TopRoleName)
		, roleNS()
		, pTax(NULL)
		, DataRoles(dataRoles)
		, useUndefinedNames(true)
	{
		// no zero-named roles allowed
		Roles.push_back(NULL);
		Roles.push_back(NULL);
		// setup empty role
		emptyRole.setId(0);
		emptyRole.setInverse(&emptyRole);
		emptyRole.setDataRole(dataRoles);
		emptyRole.setBPDomain(bpBOTTOM);
		emptyRole.setBottom();
		// setup universal role
		universalRole.setId(0);
		universalRole.setInverse(&universalRole);
		universalRole.setDataRole(dataRoles);
		universalRole.setBPDomain(bpTOP);
		universalRole.setTop();
		// FIXME!! now it is not transitive => simple
		const_cast<RoleAutomaton&>(universalRole.getAutomaton()).setCompleted();
	}
		/// d'tor (delete taxonomy)
	~RoleMaster ( void ) { delete pTax; }

		/// create role entry with given name
	TNamedEntry* ensureRoleName ( const std::string& name ) throw(EFPPCantRegName)
	{
		// check for the Top/Bottom names
		if ( name == emptyRole.getName() )
			return &emptyRole;
		if ( name == universalRole.getName() )
			return &universalRole;

		// new name from NS
		TRole* p = roleNS.insert(name);
		// check what happens
		if ( p == NULL )			// role registration attempt failed
			throw EFPPCantRegName ( name, DataRoles ? "data role" : "role" );

		if ( isRegisteredRole(p) )	// registered role
			return p;
		if ( p->getId() != 0 ||		// not registered but has non-null ID
			 !useUndefinedNames )	// new names are disallowed
			throw EFPPCantRegName ( name, DataRoles ? "data role" : "role" );

		registerRole(p);
		return p;
	}

		/// add parent for the input role or role composition; delete ROLE afterwards
	void addRoleParent ( DLTree* role, TRole* parent ) const;
		/// add parent for the input role
	void addRoleParent ( TRole* role, TRole* parent ) const { addRoleParentProper ( resolveSynonym(role), resolveSynonym(parent) ); }
		/// add synonym to existing role
	void addRoleSynonym ( TRole* role, TRole* syn ) const
	{
		// no synonyms
//		role = resolveSynonym(role);
//		syn = resolveSynonym(syn);
		// FIXME!! 1st call can make one of them a synonym of a const
		addRoleParentProper ( resolveSynonym(role), resolveSynonym(syn) );
		addRoleParentProper ( resolveSynonym(syn), resolveSynonym(role) );
	}

		/// register a pair of disjoint roles
	void addDisjointRoles ( TRole* R, TRole* S )
	{
		// object- and data roles are always disjoint
		if ( R->isDataRole() != S->isDataRole() )
			return;
		DJRolesA.push_back(R);
		DJRolesB.push_back(S);
	}

		/// create taxonomy of roles (using the Parent data)
	void initAncDesc ( void );

		/// change the undefined names usage policy
	void setUndefinedNames ( bool val ) { useUndefinedNames = val; }

	// access to roles

		/// @return pointer to a TOP role
	TRole* getTopRole ( void ) { return &universalRole; }
		/// @return pointer to a BOTTOM role
	TRole* getBotRole ( void ) { return &emptyRole; }
		/// RW pointer to the first user-defined role
	iterator begin ( void ) { return Roles.begin()+firstRoleIndex(); }
		/// RW pointer after the last user-defined role
	iterator end ( void ) { return Roles.end(); }
		/// RO pointer to the first user-defined role
	const_iterator begin ( void ) const { return Roles.begin()+firstRoleIndex(); }
		/// RO pointer after the last user-defined role
	const_iterator end ( void ) const { return Roles.end(); }

		/// get access to the taxonomy
	Taxonomy* getTaxonomy ( void ) const { return pTax; }

		/// @return true iff there is a reflexive role
	bool hasReflexiveRoles ( void ) const;
		/// put all reflexive roles to a RR array
	void fillReflexiveRoles ( TRoleVec& RR ) const;

	// output interface

	void Print ( std::ostream& o, const char* type ) const
	{
		if ( size() == 0 )
			return;
		o << type << " Roles (" << size() << "):\n";
		for ( const_iterator p = begin(); p != end(); ++p )
			(*p)->Print(o);
	}


	// save/load interface; implementation is in SaveLoad.cpp

		/// save entry
	void Save ( std::ostream& o ) const;
		/// load entry
	void Load ( std::istream& i );
}; // RoleMaster

inline bool
RoleMaster :: hasReflexiveRoles ( void ) const
{
	for  ( const_iterator p = begin(), p_end = end(); p < p_end; ++p )
		if ( (*p)->isReflexive() )
			return true;

	return false;
}

inline void
RoleMaster :: fillReflexiveRoles ( TRoleVec& RR ) const
{
	RR.clear();
	for  ( const_iterator p = begin(), p_end = end(); p < p_end; ++p )
		if ( !(*p)->isSynonym() && (*p)->isReflexive() )
			RR.push_back(*p);
}

#endif
