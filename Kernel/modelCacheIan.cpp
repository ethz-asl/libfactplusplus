/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2015 by Dmitry Tsarkov

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

#include "modelCacheIan.h"

/// clear the cache
void
modelCacheIan :: clear ( void )
{
	posDConcepts.clear();
	posNConcepts.clear();
	negDConcepts.clear();
	negNConcepts.clear();
#ifdef RKG_USE_SIMPLE_RULES
	extraDConcepts.clear();
	extraNConcepts.clear();
#endif
	existsRoles.clear();
	forallRoles.clear();
	funcRoles.clear();
	curState = csValid;
}

void modelCacheIan :: processConcept ( const DLVertex& cur, bool pos, bool det )
{
		switch ( cur.Type() )
		{
		case dtTop:			// sanity checks
		case dtDataType:	// data entries can not be cached
		case dtDataValue:
		case dtDataExpr:
			fpp_unreachable();
			break;

		case dtNConcept:	// add concepts to Concepts
		case dtPConcept:
		case dtNSingleton:
		case dtPSingleton:
			(det ? getDConcepts(pos) : getNConcepts(pos)).insert(static_cast<const ClassifiableEntry*>(cur.getConcept())->index());
			break;

		case dtIrr:		// for \neg \ER.Self: add R to AR-set
		case dtForall:	// add AR.C roles to forallRoles
		case dtLE:		// for <= n R: add R to forallRoles
			if ( unlikely ( cur.getRole()->isTop() ) )	// force clash to every other edge
				(pos ? forallRoles : existsRoles).completeSet();
			else if ( pos )	// no need to deal with existentials here: they would be created through edges
			{
				if ( cur.getRole()->isSimple() )
					forallRoles.insert(cur.getRole()->index());
				else
					processAutomaton(cur);
			}
			break;

		default:	// all other -- nothing to do
			break;
		}
}

void
modelCacheIan :: processAutomaton ( const DLVertex& cur )
{
	const RAStateTransitions& RST = cur.getRole()->getAutomaton()[cur.getState()];

	// for every transition starting from a given state,
	// add the role that is accepted by a transition
	for ( RAStateTransitions::const_iterator i = RST.begin(), i_end = RST.end(); i < i_end; ++i )
		for ( RATransition::const_iterator r = (*i)->begin(), r_end = (*i)->end(); r < r_end; ++r )
			forallRoles.insert((*r)->index());
}

modelCacheState modelCacheIan :: canMerge ( const modelCacheInterface* p ) const
{
	if ( hasNominalClash(p) )	// fail to merge due to nominal precense
		return csFailed;

	// check if something goes wrong
	if ( p->getState () != csValid || getState () != csValid )
		return mergeStatus ( p->getState (), getState () );

	// here both models are valid;

	switch ( p->getCacheType() )
	{
	case mctConst:		// check for TOP (as the model is valid)
		return csValid;
	case mctSingleton:	// check for the Singleton
	{
		BipolarPointer Singleton = static_cast<const modelCacheSingleton*>(p)->getValue();
		return isMergableSingleton ( getValue(Singleton), isPositive(Singleton) );
	}
	case mctIan:
		return isMergableIan ( static_cast<const modelCacheIan*>(p) );
	default:			// something unexpected
		return csUnknown;
	}
}

modelCacheState modelCacheIan :: isMergableSingleton ( unsigned int Singleton, bool pos ) const
{
	fpp_assert ( Singleton != 0 );

	// deterministic clash
	if ( getDConcepts(!pos).contains(Singleton) )
		return csInvalid;
	// non-det clash
	else if ( getNConcepts(!pos).contains(Singleton) )
		return csFailed;

	return csValid;
}

modelCacheState modelCacheIan :: isMergableIan ( const modelCacheIan* q ) const
{
	if ( posDConcepts.intersects(q->negDConcepts)
		 || q->posDConcepts.intersects(negDConcepts)
#	ifdef RKG_USE_SIMPLE_RULES
		 || getExtra(/*det=*/true).intersects(q->getExtra(/*det=*/true))
#	endif
		)
		return csInvalid;
	else if (  posDConcepts.intersects(q->negNConcepts)
			|| posNConcepts.intersects(q->negDConcepts)
			|| posNConcepts.intersects(q->negNConcepts)
  			|| q->posDConcepts.intersects(negNConcepts)
  			|| q->posNConcepts.intersects(negDConcepts)
  			|| q->posNConcepts.intersects(negNConcepts)
#		ifdef RKG_USE_SIMPLE_RULES
			|| getExtra(/*det=*/true).intersects(q->getExtra(/*det=*/false))
			|| getExtra(/*det=*/false).intersects(q->getExtra(/*det=*/true))
			|| getExtra(/*det=*/false).intersects(q->getExtra(/*det=*/false))
#		endif
			|| existsRoles.intersects(q->forallRoles)
			|| q->existsRoles.intersects(forallRoles)
			|| funcRoles.intersects(q->funcRoles) )
		return csFailed;
	else	// could be merged
		return csValid;
}

modelCacheState modelCacheIan :: merge ( const modelCacheInterface* p )
{
	fpp_assert ( p != nullptr );

	// check for nominal clash
	if ( hasNominalClash(p) )
	{
		curState = csFailed;
		return getState();
	}

	switch ( p->getCacheType() )
	{
	case mctConst:		// adds TOP/BOTTOM
		curState = mergeStatus ( getState(), p->getState() );
		break;
	case mctSingleton:	// adds Singleton
	{
		BipolarPointer Singleton = static_cast<const modelCacheSingleton*>(p)->getValue();
		mergeSingleton ( getValue(Singleton), isPositive(Singleton) );
		break;
	}
	case mctIan:
		mergeIan(static_cast<const modelCacheIan*>(p));
		break;
	default:
		fpp_unreachable();
	}

	updateNominalStatus(p);
	return getState();
}

/// actual merge with a singleton cache
void
modelCacheIan :: mergeSingleton ( unsigned int Singleton, bool pos )
{
	modelCacheState newState = isMergableSingleton ( Singleton, pos );

	if ( newState != csValid )	// some clash occured: adjust state
		curState = mergeStatus ( getState(), newState );
	else	// add singleton; no need to change state here
		getDConcepts(pos).insert(Singleton);
}

/// actual merge with an Ian's cache
void
modelCacheIan :: mergeIan ( const modelCacheIan* p )
{
	// setup curState
	curState = isMergableIan(p);

	// merge all sets:
	posDConcepts |= p->posDConcepts;
	posNConcepts |= p->posNConcepts;
	negDConcepts |= p->negDConcepts;
	negNConcepts |= p->negNConcepts;
#ifdef RKG_USE_SIMPLE_RULES
	extraDConcepts |= p->extraDConcepts;
	extraNConcepts |= p->extraNConcepts;
#endif
	existsRoles |= p->existsRoles;
	forallRoles |= p->forallRoles;
	funcRoles |= p->funcRoles;
}

// logging
#ifdef _USE_LOGGING
void modelCacheIan :: logCacheEntry ( unsigned int level ) const
{
	CHECK_LL_RETURN(level);
	LL << "\nIan cache: posDConcepts = ";
	posDConcepts.print(LL);
	LL << ", posNConcepts = ";
	posNConcepts.print(LL);
	LL << ", negDConcepts = ";
	negDConcepts.print(LL);
	LL << ", negNConcepts = ";
	negNConcepts.print(LL);
#ifdef RKG_USE_SIMPLE_RULES
	LL << ", extraDConcepts = ";
	extraDConcepts.print(LL);
	LL << ", extraNConcepts = ";
	extraNConcepts.print(LL);
#endif
	LL << ", existsRoles = ";
	existsRoles.print(LL);
	LL << ", forallRoles = ";
	forallRoles.print(LL);
	LL << ", funcRoles = ";
	funcRoles.print(LL);
}
#endif
