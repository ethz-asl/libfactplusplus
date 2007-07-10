/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2007 by Dmitry Tsarkov

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef _TCONCEPT_H
#define _TCONCEPT_H

#include "taxNamEntry.h"
#include "tNameSet.h"
#include "tLabeller.h"
#include "dltree.h"
#include "dlVertex.h"
#include "LogicFeature.h"
#include "DepSet.h"

class TBox;
class TRelated;

/// type of concept wrt classifiability
enum CTTag
{
	/// not specified
	cttUnspecified,
	/// concept with all parents -- TCD
	cttTrueCompletelyDefined,
	/// concept w/o any told subsumers
	cttOrphan,
	/// concept with all parents -- LCD, TCD or Orptans
	cttLikeCompletelyDefined,
	/// concept with non-primitive TS
	cttHasNonPrimitiveTS,
	/// any other primitive concept
	cttRegular,
	/// any non-primitive concept (except synonyms)
	cttNonPrimitive,
};

inline char getCTTagName ( CTTag tag )
{
	switch(tag)
	{
	case cttUnspecified:			return 'u';
	case cttTrueCompletelyDefined:	return 'T';
	case cttOrphan:					return 'O';
	case cttLikeCompletelyDefined:	return 'L';
	case cttHasNonPrimitiveTS:		return 'N';
	case cttRegular:				return 'r';
	case cttNonPrimitive:			return 'n';
	default:						return char();
	}
}

class DlCompletionTree;

/// class for representing concept-like entries
class TConcept: public ClassifiableEntry
{
private:	// members
		/// label to use in relevant-only checks
	TLabeller::LabType rel;

public:		// types
		/// pointers to RELATED constructors (individuals only)
	typedef std::vector<TRelated*> RelatedIndex;

public:		// members
		/// description of a concept
	DLTree* Description;
		/// classification type of concept: completely defined (true- or like-), no TS, other
	CTTag classTag;
		/// depth of the concept wrt told subsumers
	unsigned int tsDepth;

		/// pointer to the entry in DAG with concept name
	BipolarPointer pName;
		/// pointer to the entry in DAG with concept definition
	BipolarPointer pBody;

		/// pointer to nominal node (works for singletons only)
	DlCompletionTree* node;

		/// features for C
	LogicFeatures posFeatures;
		/// features for ~C
	LogicFeatures negFeatures;

		/// index for axioms <this,C>:R
	RelatedIndex IndexFrom;
		/// index for axioms <C,this>:R
	RelatedIndex IndexTo;

protected:	// methods
	// classification TAGs manipulation

		/// calculate value of classification TAG based on told subsumers. WARNING: no TS cycles allowed
	CTTag determineClassTag ( void );

		/// replace every entry of THIS in the P with true/false; return updated tree
	DLTree* replaceWithConst ( DLTree* p ) const;

public:		// methods
		/// the only c'tor
	explicit TConcept ( const std::string& name )
		: ClassifiableEntry (name)
		, rel(0)
		, Description(NULL)
		, classTag(cttUnspecified)
		, tsDepth(0)
		, pName (bpINVALID)
		, pBody (bpINVALID)
		, node(NULL)
	{
		setSatisfiable(true);
		setPrimitive();
		// not a told subsumer by default; this would be change during TS construction.
		setNaTS(true);
	}
		/// d'tor
	virtual ~TConcept ( void ) { deleteTree(Description); }

		/// clear all info of the concept. Use it in removeConcept()
	void clear ( void );

		/// returns associated synonym or concept itself if none found (non-const version)
	TConcept* resolveSynonym ( void ) { return isSynonym() ? (TConcept*)getSynonym(): this; }
		/// returns associated synonym or concept itself if none found (const version)
	const TConcept* resolveSynonym ( void ) const { return isSynonym() ? (const TConcept*)getSynonym(): this; }

	// related things

		/// update told subsumers from the RELATED axioms in a given range
	template<class Iterator> void updateTold ( Iterator begin, Iterator end, bool from )
	{
		for ( Iterator p = begin; p < end; ++p )
			SearchTSbyRoleAndSupers((*p)->getRole(from));
	}
		/// update told subsumers from all relevant RELATED axioms
	void updateToldFromRelated ( void );
		/// check if individual connected to something with RELATED statement
	bool isRelated ( void ) const { return !IndexFrom.empty() || !IndexTo.empty(); }
		/// set individual related
	void addRelated ( bool first, TRelated* p )
	{
		if ( first )
			IndexFrom.push_back(p);
		else
			IndexTo.push_back(p);
	}

	// classification TAGs manipulation

		/// get value of tag as it is
	CTTag getClassTag ( void ) const { return classTag; }
		/// get value of a tag; determine it if unset
	CTTag getClassTag ( void )
	{
		if ( classTag == cttUnspecified )
			classTag = determineClassTag();
		return classTag;
	}

	// description manipulation

		/// add concept expression to concept description
	void addDesc ( DLTree* Desc );
		/// remove concept from its own definition (like in case C [= (or C ...)
	void removeSelfFromDescription ( void ) { Description = replaceWithConst(Description); }
		/// remove concept description (to save space)
	void removeDescription ( void )
	{	// save Synonym value
		deleteTree(Description);
		Description = NULL;
	}
		/// check whether it is possible to init this as a non-primitive concept with DESC
	bool canMakeNonPrim ( DLTree* desc )
	{
		if ( Description == NULL )
			return true;
		if ( isNonPrimitive() && equalTrees(Description,desc) )
			return true;
		return false;
	}
		/// switch primitive concept to non-primitive with new definition; @return old definition
	DLTree* makeNonPrimitive ( DLTree* desc )
	{
		DLTree* ret = Description;
		Description = desc;
		setPrimitive(false);
		return ret;
	}

	// told subsumers interface

		/// adds concept as a told subsumer of current one; @return value for CDC analisys
	bool addToldSubsumer ( TConcept* p )
	{
		if ( p->isSynonym() )
			p = resolveSynonym();

		if ( p != this )
		{
			includeParent (p);
			p->setNaTS(false);	// p is a told subsumer (for current concept)
		}

		// if non-primitive concept was found in a description, it's not CD
		return p->isPrimitive();
	}
		/// init told subsumers of the concept by given DESCription; @return TRUE iff concept is CD
	bool initToldSubsumers ( const DLTree* desc );
		/// find told subsumers by given role's domain
	void SearchTSbyRole ( const TRole* R );
		/// find told subsumers by given role and its supers domains
	void SearchTSbyRoleAndSupers ( const TRole* R );
		/// init told subsumers of the concept by it's description
	void initToldSubsumers ( TConcept* top )
	{
		getTold().clear();
		if ( isRelated() )	// check if domain and range of RELATED axioms affects TS
			updateToldFromRelated();
		// normalise description if the only parent is TOP
		if ( isPrimitive() && Description && Description->Element() == TOP )
			removeDescription();

		if ( Description == NULL && getTold().empty() )
		{
			addParent(top);
			if ( !isRelated() )	// related individuals are NOT completely defined
				setCompletelyDefined(true);
		}
		else	// init (additional) told subsumers from definition
			setCompletelyDefined ( initToldSubsumers(Description) && isPrimitive() );
	}
		/// calculate depth wrt told subsumers; return the depth
	unsigned int calculateTSDepth ( void );

	// used in the start procedure of SAT/SUBSUME tests
	BipolarPointer resolveId ( void ) const;	// returns either pName or pBody

		/// register a Satisfiable flag
	FPP_ADD_FLAG(Satisfiable,0x10);
		/// register a Singleton flag
	FPP_ADD_FLAG(Singleton,0x20);
		/// register a Primitive flag
	FPP_ADD_FLAG(Primitive,0x40);

	// concept non-primitivity methods

		/// check if concept is non-primitive concept
	bool isNonPrimitive ( void ) const { return !isPrimitive(); }

	// node corresponding to nominal

		/// get node in CGraph corresponding to given nominal; set DEP according to merge path
	DlCompletionTree* getCorrespondingNode ( DepSet& dep ) const;

	// relevance part

		/// is given concept relevant to given Labeller's state
	bool isRelevant ( const TLabeller& lab ) const { return lab.isLabelled(rel); }
		/// make given concept relevant to given Labeller's state
	void setRelevant ( const TLabeller& lab ) { lab.set(rel); }
}; // TConcept

/// Class for comparison of TConcepts wrt told subsumer depth
class TSDepthCompare
{
public:
	bool operator() ( const TConcept* p, const TConcept* q ) const
		{ return p->tsDepth < q->tsDepth; }
}; // TSDepthCompare

//----------------------------------------------------------------------------
//-- 		implementation
//----------------------------------------------------------------------------

inline BipolarPointer TConcept :: resolveId ( void ) const
{
	if ( getId() == -1 )	// it is special concept: Top or Bottom
		return pBody;

	if ( isSynonym() )	// resolve synonyms
		return resolveSynonym()->resolveId();

	return pName;	// return concept's name
}

#endif
