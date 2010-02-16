/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2010 by Dmitry Tsarkov

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

#include <set>
#include <fstream>
#include <algorithm>

#include "tRole.h"
#include "taxVertex.h"

void TRole :: fillsComposition ( roleSet& Composition, const DLTree* tree ) const
{
	if ( tree->Element() == RCOMPOSITION )
	{
		fillsComposition ( Composition, tree->Left() );
		fillsComposition ( Composition, tree->Right() );
	}
	else
		Composition.push_back(resolveRole(tree));
}

/// copy role information (like transitivity, functionality, R&D etc) to synonym
void TRole :: addFeaturesToSynonym ( void )
{
	if ( !isSynonym() )
		return;

	TRole* syn = resolveSynonym(this);

	// don't copy parents: they are already copied during ToldSubsumers processing

	// copy functionality
	if ( isFunctional() && !syn->isFunctional() )
		syn->setFunctional();

	// copy transitivity
	if ( isTransitive() )
		syn->setBothTransitive ();

	// copy reflexivity
	if ( isReflexive() )
		syn->setBothReflexive ();

	// copy data type
	if ( isDataRole() )
		syn->setDataRole();

	// copy R&D
	if ( pDomain != NULL )
		syn->setDomain (clone(pDomain));

	// copy disjoint
	if ( isDisjoint() )
		syn->Disjoint.insert ( Disjoint.begin(), Disjoint.end() );

	// copy subCompositions
	syn->subCompositions.insert ( syn->subCompositions.end(),
								  subCompositions.begin(),
								  subCompositions.end() );

	// syn should be the only parent for synonym
	toldSubsumers.clear();
	addParent(syn);
}

// compare 2 TRoles wrt order of synonyms
class TRoleCompare
{
public:
	bool operator() ( TRole* p, TRole* q ) const
	{
		int n = p->getId(), m = q->getId();
		if ( n > 0 && m < 0 )
			return true;
		if ( n < 0 && m > 0 )
			return false;
		return abs(n) < abs(m);
	}
}; // TRoleCompare

TRole* TRole :: eliminateToldCycles ( SetOfRoles& RInProcess, roleSet& ToldSynonyms )
{
	// skip synonyms
	if ( isSynonym() )
		return NULL;

	// if we found a cycle...
	if ( RInProcess.find(this) != RInProcess.end() )
	{
		ToldSynonyms.push_back(this);
		return this;
	}

	TRole* ret = NULL;

	// start processing role
	RInProcess.insert(this);

	// ensure that parents does not contain synonyms
	removeSynonymsFromParents();

	// not involved in cycle -- check all told subsumers
	for ( ClassifiableEntry::const_iterator r = told_begin(); r != told_end(); ++r )
		// if cycle was detected
		if ( (ret = static_cast<TRole*>(*r)->eliminateToldCycles ( RInProcess, ToldSynonyms )) != NULL )
		{
			if ( ret == this )
			{
				std::sort ( ToldSynonyms.begin(), ToldSynonyms.end(), TRoleCompare() );
				// now first element is representative; save it as RET
				ret = *ToldSynonyms.begin();

				// make all others synonyms of RET
				for ( std::vector<TRole*>::iterator
						p = ToldSynonyms.begin()+1, p_end = ToldSynonyms.end(); p < p_end; ++p )
				{
					(*p)->setSynonym(ret);
					ret->addParents ( (*p)->told_begin(), (*p)->told_end() );
				}

				ToldSynonyms.clear();
				// restart search for the representative
				RInProcess.erase(this);
				return ret->eliminateToldCycles ( RInProcess, ToldSynonyms );
			}
			else	// some role inside a cycle: save it and return
			{
				ToldSynonyms.push_back(this);
				break;
			}
		}

	// finish processing role
	RInProcess.erase(this);
	return ret;
}

void TRole :: Print ( std::ostream& o ) const
{
	o << "Role \"" << getName() << "\"(" << getId() << ")";

//FIXME!! while it's not necessary
//		o << " [" << r.nUsageFreq << "]";

	// transitivity
	if ( isTransitive() )
		o << "T";

	// reflexivity
	if ( isReflexive() )
		o << "R";

	// functionality
	if ( isTopFunc() )
		o << "t";
	if ( isFunctional() )
		o << "F";

	// data role
	if ( isDataRole() )
		o << "D";

	if ( isSynonym() )
	{
		o << " = \"" << getSynonym()->getName() << "\"\n";
		return;
	}

	if ( !toldSubsumers.empty() )
	{
		ClassifiableEntry::linkSet::const_iterator q = toldSubsumers.begin();

		o << " parents={\"" << (*q)->getName();

		for ( ++q; q != toldSubsumers.end(); ++q )
			o << "\", \"" << (*q)->getName();

		o << "\"}";
	}

	if ( !Disjoint.empty() )
	{
		SetOfRoles::const_iterator p = Disjoint.begin(), p_end = Disjoint.end();

		o << " disjoint with {\"" << (*p)->getName();

		for ( ++p; p != p_end; ++p )
			o << "\", \"" << (*p)->getName();

		o << "\"}";
	}

	// range/domain
	if ( getTDomain() != NULL )
		o << " Domain=(" << getBPDomain() << ")=" << getTDomain();
	if ( getTRange() != NULL )
		o << " Range=(" << getBPRange() << ")=" << getTRange();

	o << "\nAutomaton:";
	A.Print(o);
	o << "\n";
}

// actor to fill vector by traversing taxonomy in a proper direction
class AddRoleActor
{
protected:
	TRole::roleSet& rset;
public:
	AddRoleActor ( TRole::roleSet& v ) : rset(v) {}
	~AddRoleActor ( void ) {}
	bool apply ( const TaxonomyVertex& v )
	{
		rset.push_back(const_cast<TRole*>(static_cast<const TRole*>(v.getPrimer())));
		return true;
	}
}; // AddRoleActor

/// init ancestors and descendants using Taxonomy
void TRole :: initADbyTaxonomy ( unsigned int nRoles )
{
	fpp_assert ( isClassified() );	// safety check
	fpp_assert ( Ancestor.empty() && Descendant.empty() );

	// Note that Top/Bottom are not connected to taxonomy yet.

	// fills ancestors by the taxonomy
	AddRoleActor anc(Ancestor);
	getTaxVertex()->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/false,
									 /*upDirection=*/true>(anc);
	// fills descendants by the taxonomy
	AddRoleActor desc(Descendant);
	getTaxVertex()->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/false,
									 /*upDirection=*/false>(desc);

	// determine Simple attribute
	initSimple();
	// resize maps for fast access
	DJRoles.resize(nRoles);
	AncMap.resize(nRoles);
	// init map for fast Anc/Desc access
	addAncestorsToBitMap(AncMap);
}

void TRole :: postProcess ( void )
{
	// set Topmost-Functional field
	initTopFunc();
	// init DJ roles map
	if ( isDisjoint() )
		initDJMap();
}

// simplicity checking (necessary for A+ rule)
// valid only after initAncDesc call
void TRole :: initSimple ( void )
{
	fpp_assert ( !isSynonym() );

	setSimple(false);

	// try to ensure that role is non-simple
	if ( isTransitive() || !subCompositions.empty() )
		return;

	for ( iterator p = begin_desc(); p != end_desc(); ++p )
		if ( (*p)->isTransitive() || !(*p)->subCompositions.empty() )
			return;

	// role is simple -- set it
	setSimple(true);
}

/// check if the role is topmost-functional (internal-use only)
// not very efficient, but good enough
bool TRole :: isRealTopFunc ( void ) const
{
	if ( !isFunctional() )	// all REAL top-funcs have their self-ref in TopFunc already
		return false;
	// if any of the parent is self-proclaimed top-func -- this one is not top-func
	for ( iterator p = begin_anc(); p != end_anc(); ++p )
		if ( (*p)->isTopFunc() )
			return false;

	// functional role with no functional parents is top-most functional
	return true;
}

/// set up TopFunc member (internal-use only)
// not very efficient, but good enough
void TRole :: initTopFunc ( void )
{
	if ( isRealTopFunc() )	// TF already set up -- nothing to do
		return;

	if ( isTopFunc() )		// sefl-proclaimed TF but not real -- need to be updated
		TopFunc.clear();

	// register all real TFs
	for ( iterator p = begin_anc(); p != end_anc(); ++p )
		if ( (*p)->isRealTopFunc() )
			TopFunc.push_back(*p);
}

// disjoint-related implementation

/// check (and correct) case whether R != S for R [= S
void TRole :: checkHierarchicalDisjoint ( TRole* R )
{
	// if element is disjoint with itself -- the role is empty
	if ( Disjoint.count(R) )
	{
		setDomain ( new DLTree(BOTTOM) );
		Disjoint.clear();
		return;
	}

	// check whether a sub-role is disjoint with the given one
	for ( iterator p = R->begin_desc(), p_end = R->end_desc(); p != p_end; ++p )
		if ( Disjoint.count(*p) )
		{
			(*p)->setDomain ( new DLTree(BOTTOM) );
			Disjoint.erase(*p);
			(*p)->Disjoint.clear();
		}
}

/// init map of all disjoint roles (N is a size of a bitmap)
void TRole :: initDJMap ( void )
{
	// role R is disjoint with every role S' [= S such that R != S
	for ( SetOfRoles::iterator q = Disjoint.begin(), q_end = Disjoint.end(); q != q_end; ++q )
		DJRoles[(*q)->getIndex()] = true;
}

// automaton-related implementation

void
TRole :: preprocessComposition ( roleSet& RS ) throw(EFPPCycleInRIA)
{
	bool same = false;
	unsigned int last = RS.size()-1;
	unsigned int i = 0;	// current element of the composition

	for ( roleSet::iterator p = RS.begin(), p_end = RS.end(); p < p_end; ++p, ++i )
	{
		TRole* R = resolveSynonym(*p);

		if ( R->isTop() )	// universal role in composition
			throw EFaCTPlusPlus("Universal role can not be used in role composition chain");
		if ( R->isBottom() )	// empty role in composition -- nothing to do
		{
			RS.clear();
			return;
		}
		if ( R == this )	// found R in composition
		{
			if ( i != 0 && i != last )		// R in the middle of the composition
				throw EFPPCycleInRIA(getName());
			if ( same )	// second one
			{
				if ( last == 1 )	// transitivity
				{
					RS.clear();
					setTransitive();
					return;
				}
				else					// wrong (undecidable) axiom
					throw EFPPCycleInRIA(getName());
			}
			else		// first one
				same = true;
		}

		*p = R;	// replace possible synonyms
	}
}

/// complete role automaton
void TRole :: completeAutomaton ( SetOfRoles& RInProcess )
{
	if ( A.isComplete() )
		return;

	// if we found a cycle...
	if ( RInProcess.find(this) != RInProcess.end() )
		throw EFPPCycleInRIA(getName());

	// start processing role
	RInProcess.insert(this);

	// make sure that all sub-roles already have completed automata
	for ( iterator p = begin_desc(); p != end_desc(); ++p )
		(*p)->completeAutomaton(RInProcess);

	// check for the transitivity
	if ( isTransitive() )
		A.addTransitionSafe ( 1, new RATransition(0) );

	// add automata for complex role inclusions
	for ( std::vector<roleSet>::iterator q = subCompositions.begin(); q != subCompositions.end(); ++q )
		addSubCompositionAutomaton ( *q, RInProcess );

	// complete automaton
	A.complete();

	for ( ClassifiableEntry::iterator p = told_begin(); p < told_end(); ++p )
		static_cast<TRole*>(resolveSynonym(*p))->addSubRoleAutomaton(this);

	// finish processing role
	RInProcess.erase(this);
}
