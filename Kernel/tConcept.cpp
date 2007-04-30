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

#include <set>

#include "tConcept.h"
#include "dlCompletionTree.h"
#include "tRelated.h"

void TConcept :: clear ( void )
{
	// TNamedEntry clean
	setId(0);
	// ClassifiableEntry clean
	taxVertex = NULL;
	toldSubsumers.clear();
	setCompletelyDefined(false);
	pSynonym = NULL;
	// TConcept clean
	removeDescription();
	setPrimitive();
	pName = pBody = bpINVALID;
}

// can't be inlined because RELATED implementation should be known
void TConcept :: updateToldFromRelated ( void )
{
	updateTold ( IndexFrom.begin(), IndexFrom.end(), /*from=*/true );
	updateTold ( IndexTo.begin(), IndexTo.end(), /*from=*/false );
}

void TConcept :: addDesc ( DLTree* Desc )
{
	assert (!isNonPrimitive());	// safety check

	// FIXME!! check about reverse order
	Description = createSNFAnd ( Desc, Description );
}

/// calculate value of classification TAG based on told subsumers. WARNING: no TS cycles allowed
CTTag TConcept :: determineClassTag ( void )
{
	// for synonyms -- set tag as a primer's one
	if ( isSynonym() )
		return resolveSynonym()->getClassTag();

	// check if it is non-primitive
	if ( isNonPrimitive() )
		return cttNonPrimitive;

	const ClassifiableEntry::linkSet& v = getTold();

	// no told subsumers
	if ( v.size() == 0 )
		return cttOrphan;

	// now need to check all the told subsumers
	bool hasLCD = false;
	bool hasOther = false;
	bool hasNP = false;

	for ( ClassifiableEntry::linkSet::const_iterator p = v.begin(); p != v.end(); ++p )
		switch ( static_cast<TConcept*>(*p)->getClassTag() )
		{
		case cttTrueCompletelyDefined:
			break;
		case cttOrphan:
		case cttLikeCompletelyDefined:
			hasLCD = true;
			break;
		case cttRegular:
			hasOther = true;
			break;
		case cttHasNonPrimitiveTS:
		case cttNonPrimitive:
			hasNP = true;
			break;
		default:
			assert(0);
		}

	// there are non-primitive TS
	if ( hasNP )
		return cttHasNonPrimitiveTS;

	// has something different from CD-like ones (and not CD)
	if ( hasOther || !isCompletelyDefined() )
		return cttRegular;

	// no more 'other' concepts here, and the CD-like structure
	if ( hasLCD )
		return cttLikeCompletelyDefined;

	// individual related to something
	if ( isSingleton() && isRelated() )
		return cttRegular;
	else
		return cttTrueCompletelyDefined;
}

DLTree*
TConcept :: replaceWithConst ( DLTree* t ) const
{
	if ( t == NULL )
		return NULL;

	DLTree* ret;

	switch ( t->Element().getToken() )
	{
	case NAME:	// if ID contains synonym of P
	{
		const ClassifiableEntry* name = static_cast<const ClassifiableEntry*>(t->Element().getName());
		if ( name == this || name->getSynonym() == this )
		{
			delete t;	// replace it for TOP
			return new DLTree(TOP);
		}
		else
			return t;
	}
	case AND:	// a [= (and a b) -> a [= b
		ret = createSNFAnd ( replaceWithConst(t->Left()), replaceWithConst(t->Right()) );
		delete t;	// delete just entry, not the whole tree
		return ret;

	case NOT:	// a [= (not a) -> a [= BOTTOM; a [= (a or b) -> a [= TOP
		switch ( t->Left()->Element().getToken() )
		{
		case AND:
		case NAME:
			ret = createSNFNot(replaceWithConst(t->Left()));
			delete t;	// delete just NOT as we re-use LEFT
			return ret;
		default:
			return t;
		}

	default:
		return t;
	}
}

/// init told subsumers of the concept by given DESCription; @return TRUE iff concept is CD
bool TConcept :: initToldSubsumers ( const DLTree* desc )
{
	// no description => nothing to do (and yes, it is told)
	if ( desc == NULL )
		return true;

	switch ( desc->Element().getToken() )
	{
	case TOP:	// the 1st node
		return true;
	case NAME:	// it is a concept ID
		return addToldSubsumer(static_cast<TConcept*>(desc->Element().getName()));
	case AND:	// add TS from BOTH parts of AND
		return initToldSubsumers(desc->Left()) & initToldSubsumers(desc->Right());
	case NOT:	// Domains from \ER.C and (>= n R.C) are told concepts
	{
		const TLexeme& cur = desc->Left()->Element();

		if ( cur.getToken() == FORALL || cur.getToken() == LE )
			SearchTSbyRoleAndSupers(resolveRole(desc->Left()->Left()));

		return false;
	}
	default:	// not told one
		return false;
	}
}

void TConcept :: SearchTSbyRole ( const TRole* R )
{
	assert ( R != NULL );
	const DLTree* Domain = R->getTDomain();
	if ( Domain == NULL || isConst(Domain) )
		return;

	// searchable set for roles in process
	typedef std::set<const TRole*> SearchableSet;
	static SearchableSet sSet;

	// check for the loop
	if ( sSet.find(R) != sSet.end() )
		return;

	// add role in processing; usually it's the only role, so set hint as a begin()
	SearchableSet::iterator i = sSet.insert ( sSet.begin(), R );

	// init TS by the domain of role
	initToldSubsumers(Domain);	// don't bother about result

	// remove processed role from set
	sSet.erase(i);
}

void TConcept :: SearchTSbyRoleAndSupers ( const TRole* r )
{
	SearchTSbyRole(r);

	// do the same for all super-roles if necessary
	if ( !RKG_UPDATE_RND_FROM_SUPERROLES )
		for ( TRole::iterator q = r->begin_anc(); q != r->end_anc(); ++q )
			SearchTSbyRole(*q);
}

unsigned int TConcept :: calculateTSDepth ( void )
{
	if ( tsDepth > 0 )
		return tsDepth;

	unsigned int max = 0;
	ClassifiableEntry::linkSet& tolds ( getTold() );

	for ( ClassifiableEntry::linkSet::iterator p = tolds.begin(); p != tolds.end(); ++p )
	{
		unsigned int cur = static_cast<TConcept*>(*p)->calculateTSDepth();
		if ( max < cur )
			max = cur;
	}

	return (tsDepth = max+1);
}

DlCompletionTree* TConcept :: getCorrespondingNode ( DepSet& dep ) const
{
	return node->resolvePBlocker(dep);
}
