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

#include "modelCacheIan.h"
#include "logging.h"

void modelCacheIan :: processConcept ( const DLVertex& cur, BipolarPointer bp, bool det )
{
		switch ( cur.Type() )
		{
		case dtTop:		// safety checks
		case dtDataType:	// data entries can not be cached
		case dtDataValue:
		case dtDataExpr:
			fpp_unreachable();
			break;

		case dtNConcept:	// add concepts to Concepts
		case dtPConcept:
		case dtNSingleton:
		case dtPSingleton:
			( isNegative(bp)
				? (det ? negDConcepts : negNConcepts)
				: (det ? posDConcepts : posNConcepts) )
			.insert(static_cast<const ClassifiableEntry*>(cur.getConcept())->index());

			break;

		case dtIrr:		// for \neg \ER.Self: add R to AR-set
		case dtForall:	// add AR.C roles to forallRoles
		case dtLE:		// for <= n R: add R to forallRoles
			if ( isPositive (bp) )	// no need to deal with existantionals here: they would be created through edges
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

/// adds role (and all its super-roles) to exists- and funcRoles
void modelCacheIan :: addExistsRole ( const TRole* R )
{
	existsRoles.insert(R->index());
	if ( R->isTopFunc() )	// all other top-funcs would be added later on
		funcRoles.insert(R->index());

	for ( TRole::const_iterator r = R->begin_anc(), r_end = R->end_anc(); r != r_end; ++r )
	{
		existsRoles.insert((*r)->index());
		if ( (*r)->isTopFunc() )
			funcRoles.insert((*r)->index());
	}
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


template <class T>
inline bool
set_contains ( const std::set<T>& s, T value )
{
	return s.find(value) != s.end();
}

/// checks if 2 sets are intersects.
template <class T>
inline bool
sets_intersect ( const std::set<T>& s1, const std::set<T>& s2 )
{
	if ( s1.empty() || s2.empty() )
		return false;

	typename std::set<T>::const_iterator
		p1 = s1.begin(), p1_end = s1.end(),
		p2 = s2.begin(), p2_end = s2.end();
	while ( p1 != p1_end && p2 != p2_end )
	{
		if ( *p1 == *p2 )
			return true;
		if ( *p1 < *p2 )
			++p1;
		else
			++p2;
	}

	return false;
}

modelCacheState modelCacheIan :: isMergableSingleton ( unsigned int Singleton, bool pos ) const
{
	// check for the clash
	if ( pos )
	{
		// deterministic clash
		if ( set_contains ( negDConcepts, Singleton ) )
			return csInvalid;
		// non-det clash
		else if ( set_contains ( negNConcepts, Singleton ) )
			return csFailed;
	}
	else
	{
		// deterministic clash
		if ( set_contains ( posDConcepts, Singleton ) )
			return csInvalid;
		// non-det clash
		else if ( set_contains ( posNConcepts, Singleton ) )
			return csFailed;
	}

	return csValid;
}

modelCacheState modelCacheIan :: isMergableIan ( const modelCacheIan* q ) const
{
	if ( sets_intersect ( posDConcepts, q->negDConcepts )
		 || sets_intersect ( q->posDConcepts, negDConcepts )
#	ifdef RKG_USE_SIMPLE_RULES
		 || sets_intersect ( extraDConcepts, q->extraDConcepts )
#	endif
		)
		return csInvalid;
	else if (  sets_intersect ( posDConcepts, q->negNConcepts )
			|| sets_intersect ( posNConcepts, q->negDConcepts )
			|| sets_intersect ( posNConcepts, q->negNConcepts )
  			|| sets_intersect ( q->posDConcepts, negNConcepts )
  			|| sets_intersect ( q->posNConcepts, negDConcepts )
  			|| sets_intersect ( q->posNConcepts, negNConcepts )
#		ifdef RKG_USE_SIMPLE_RULES
			|| sets_intersect ( extraDConcepts, q->extraNConcepts )
			|| sets_intersect ( extraNConcepts, q->extraDConcepts )
			|| sets_intersect ( extraNConcepts, q->extraNConcepts )
#		endif
			|| sets_intersect ( existsRoles, q->forallRoles )
			|| sets_intersect ( q->existsRoles, forallRoles )
			|| sets_intersect ( funcRoles, q->funcRoles ) )
		return csFailed;
	else	// could be merged
		return csValid;
}

modelCacheState modelCacheIan :: merge ( const modelCacheInterface* p )
{
	fpp_assert ( p != NULL );

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
	fpp_assert ( Singleton != 0 );
	modelCacheState newState = csValid;

	// check for the clash
	if ( pos )
	{
		if ( set_contains ( negDConcepts, Singleton ) )
			newState = csInvalid;
		else if ( set_contains ( negNConcepts, Singleton ) )
			newState = csFailed;
		else
			posDConcepts.insert(Singleton);
	}
	else
	{
		if ( set_contains ( posDConcepts, Singleton ) )
			newState = csInvalid;
		else if ( set_contains ( posNConcepts, Singleton ) )
			newState = csFailed;
		else
			negDConcepts.insert(Singleton);
	}

	curState = mergeStatus ( getState(), newState );
}

/// actual merge with an Ian's cache
void
modelCacheIan :: mergeIan ( const modelCacheIan* p )
{
	// setup curState
	curState = isMergableIan(p);

	// merge all sets:
	posDConcepts.insert ( p->posDConcepts.begin(), p->posDConcepts.end() );
	posNConcepts.insert ( p->posNConcepts.begin(), p->posNConcepts.end() );
	negDConcepts.insert ( p->negDConcepts.begin(), p->negDConcepts.end() );
	negNConcepts.insert ( p->negNConcepts.begin(), p->negNConcepts.end() );
#ifdef RKG_USE_SIMPLE_RULES
	extraDConcepts.insert ( p->extraDConcepts.begin(), p->extraDConcepts.end() );
	extraNConcepts.insert ( p->extraNConcepts.begin(), p->extraNConcepts.end() );
#endif
	existsRoles.insert ( p->existsRoles.begin (), p->existsRoles.end () );
	forallRoles.insert ( p->forallRoles.begin (), p->forallRoles.end () );
	funcRoles.insert ( p->funcRoles.begin (), p->funcRoles.end () );
}

// logging
void modelCacheIan :: logCacheEntry ( unsigned int level ) const
{
	CHECK_LL_RETURN(level);
	LL << "\nIan cache: posDConcepts = ";
	logCacheSet ( posDConcepts );
	LL << ", posNConcepts = ";
	logCacheSet ( posNConcepts );
	LL << ", negDConcepts = ";
	logCacheSet ( negDConcepts );
	LL << ", negNConcepts = ";
	logCacheSet ( negNConcepts );
#ifdef RKG_USE_SIMPLE_RULES
	LL << ", extraDConcepts = ";
	logCacheSet ( extraDConcepts );
	LL << ", extraNConcepts = ";
	logCacheSet ( extraNConcepts );
#endif
	LL << ", existsRoles = ";
	logCacheSet ( existsRoles );
	LL << ", forallRoles = ";
	logCacheSet ( forallRoles );
	LL << ", funcRoles = ";
	logCacheSet ( funcRoles );
}

void modelCacheIan :: logCacheSet ( const IndexSet& s ) const
{
	LL << "{";
	if ( !s.empty() )
	{
		IndexSet::const_iterator p = s.begin ();
		LL << *p;
		for ( ++p; p != s.end(); ++p )
			LL << ',' << *p;
	}
	LL << "}";
}
