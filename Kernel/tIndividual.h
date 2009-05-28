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

#ifndef _TINDIVIDUAL_H
#define _TINDIVIDUAL_H

#include "tConcept.h"

class DlCompletionTree;
class TRelated;

class TIndividual;

/// class to map roles to set of individuals
class TRelatedMap
{
public:		// interface types
		/// vector of individuals
	typedef std::vector<const TIndividual*> CIVec;

protected:	// types
		/// base class
	typedef std::map<const TRole*,CIVec> BaseType;

protected:	// members
		/// base that contains all info
	BaseType Base;

public:		// interface
		/// empty c'tor
	TRelatedMap ( void ) {}
		/// empty d'tor
	~TRelatedMap ( void ) {}

	// access

		/// check whether role is in map
	bool hasRole ( const TRole* R ) const { return Base.find(R) != Base.end(); }
		/// get array related to role
	CIVec getRelated ( const TRole* R ) const
	{
		fpp_assert ( hasRole(R) );
		return Base.find(R)->second;
	}
		/// add related wrt role
	void setRelated ( const TRole* R, const CIVec& v )
	{
		fpp_assert ( !hasRole(R) );
		Base[R] = v;
	}
}; // TRelatedMap

/// class to represent individuals
class TIndividual: public TConcept
{
public:		// types
		/// pointers to RELATED constructors
	typedef std::vector<TRelated*> RelatedSet;
		/// pointers to concept expressions
	typedef std::vector<const DLTree*> ConceptSearchSet;

public:		// members
		/// pointer to nominal node (works for singletons only)
	DlCompletionTree* node;

		/// index for axioms <this,C>:R
	RelatedSet RelatedIndex;
		/// index for the roles used in <this,C>:R
	std::vector<bool> RelatedRoleMap;
		/// map for the related individuals: Map[R]={i:R(this,i)}
	TRelatedMap* pRelatedMap;

	// precompletion support

		/// vector that contains LINKS to the concept expressions that are labels of an individual
	ConceptSearchSet CSSet;
		/// new concept expression to be added to the label
	DLTree* PCConcept;

private:	// no copy
		/// no copy c'tor
	TIndividual ( const TIndividual& );
		/// no asssignment
	TIndividual& operator = ( const TIndividual& );

public:		// interface
		/// the only c'tor
	explicit TIndividual ( const std::string& name )
		: TConcept(name)
		, node(NULL)
		, pRelatedMap(NULL)
		, PCConcept(NULL)
		{}
		/// empty d'tor
	virtual ~TIndividual ( void ) { delete pRelatedMap; delete PCConcept; }

		/// check whether a concept is indeed a singleton
	virtual bool isSingleton ( void ) const { return true; }
		/// init told subsumers of the individual by it's description
	virtual void initToldSubsumers ( void )
	{
		toldSubsumers.clear();
		clearHasSP();
		if ( isRelated() )	// check if domain and range of RELATED axioms affects TS
			updateToldFromRelated();
		// normalise description if the only parent is TOP
		if ( isPrimitive() && Description && Description->Element() == TOP )
			removeDescription();

		// not a completely defined if there are extra rules or related individuals
		bool CD = !hasExtraRules() && isPrimitive() && !isRelated();
		if ( Description != NULL || hasToldSubsumers() )
			CD &= TConcept::initToldSubsumers(Description);
		setCompletelyDefined(CD);
	}

	// related things

		/// update told subsumers from the RELATED axioms in a given range
	template<class Iterator>
	void updateTold ( Iterator begin, Iterator end )
	{
		for ( Iterator p = begin; p < end; ++p )
			SearchTSbyRoleAndSupers((*p)->getRole());
	}
		/// update told subsumers from all relevant RELATED axioms
	void updateToldFromRelated ( void );
		/// check if individual connected to something with RELATED statement
	bool isRelated ( void ) const { return !RelatedIndex.empty(); }
		/// set individual related
	void addRelated ( TRelated* p ) { RelatedIndex.push_back(p); }
		/// add all the related elements from the given P
	void addRelated ( TIndividual* p )
		{ RelatedIndex.insert ( RelatedIndex.end(), p->RelatedIndex.begin(), p->RelatedIndex.end() ); }

	// related map access

		/// get RM
	const TRelatedMap* getRelatedMap ( void ) const { return pRelatedMap; }
		/// set RM
	void setRelatedMap ( TRelatedMap* map ) { pRelatedMap = map; }
		/// clear RM
	void clearRelatedMap ( void ) { delete pRelatedMap; pRelatedMap = NULL; }

	// precompletion interface

		/// check whether EXPR already exists in the precompletion set
	bool containsPCExpr ( const DLTree* expr ) const
	{
		if ( expr == NULL )	// special case it
			return true;
		for ( ConceptSearchSet::const_iterator p = CSSet.begin(), p_end = CSSet.end(); p < p_end; ++p )
			if ( equalTrees ( *p, expr ) )
				return true;

		return false;
	}
		/// unconditionally adds EXPR to the precompletion information
	void addPCExprAlways ( const DLTree* expr )
	{
		CSSet.push_back(expr);
		PCConcept = createSNFAnd ( PCConcept, clone(expr) );
	}
		/// add EXPR to th ePC information if it is a new expression; @return true if was added
	bool addPCExpr ( const DLTree* expr )
	{
		if ( containsPCExpr(expr) )
			return false;
		addPCExprAlways(expr);
		return true;
	}
		/// update individual's description from precompletion information
	void usePCInfo ( void )
	{
		delete Description;
		Description = PCConcept;
		PCConcept = NULL;

		// we change description of a concept, so we need to rebuild the TS info
		// note that precompletion succeed; so there is no need to take into account
		// RELATED information
		TConcept::initToldSubsumers();
	}
		/// remove all precompletion-related information
	void clearPCInfo ( void ) { delete PCConcept; PCConcept = NULL; CSSet.clear(); }

	// save/load interface; implementation is in SaveLoad.cpp

		/// save entry
	virtual void Save ( std::ostream& o ) const;
		/// load entry
	virtual void Load ( std::istream& i );
}; // TIndividual

#endif

