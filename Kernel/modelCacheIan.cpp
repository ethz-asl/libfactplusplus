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
			if ( isNegative (bp) )
				(det ? negDConcepts : negNConcepts).insert(inverse(bp));
			else
				(det ? posDConcepts : posNConcepts).insert(bp);

			break;

		case dtIrr:		// for \neg \ER.Self: add R to AR-set
		case dtForall:	// add xR.C roles to *Roles.
			if ( isPositive (bp) )	// WARNING!!! it's possible to add also every sub-role, but it is extra...
			{
				if ( cur.getRole()->isSimple() )
					forallRoles.insert(cur.getRole());
				else
					processAutomaton(cur);
			}
//			else
//				addExistsRole(cur.getRole());

			break;

		case dtLE:	// for <= n R: add R to forallRoles
			if ( isPositive(bp) )
				forallRoles.insert(cur.getRole());
			else	// for >= n R: add R to existRoles
				existsRoles.insert(cur.getRole());
		default:	// all other -- nothing to do
			break;
		}
}

void
modelCacheIan :: processAutomaton ( const DLVertex& cur )
{
	const RoleAutomaton& A = cur.getRole()->getAutomaton();
	RAState state = cur.getState();

	// for every transition starting from a given state,
	// add the role that is accepted by a transition
	for ( RoleAutomaton::const_trans_iterator
		  i = A.begin(state), i_end = A.end(state); i < i_end; ++i )
		for ( RATransition::const_iterator r = (*i)->begin(), r_end = (*i)->end(); r < r_end; ++r )
			forallRoles.insert(*r);
}

/// adds role (and all its super-roles) to exists- and funcRoles
void modelCacheIan :: addExistsRole ( const TRole* R )
{
	existsRoles.insert(R);
	if ( R->isTopFunc() )	// all other top-funcs would be added later on
		funcRoles.insert(R);

	for ( TRole::iterator r = R->begin_anc(); r != R->end_anc(); ++r )
	{
		existsRoles.insert(*r);
		if ( (*r)->isTopFunc() )
			funcRoles.insert(*r);
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
		return isMergableSingleton ( static_cast<const modelCacheSingleton*>(p) );
	case mctIan:
		return isMergableIan ( static_cast<const modelCacheIan*>(p) );
	default:			// something unexpected
		return csUnknown;
	}
}


template <class T>
bool set_contains ( const std::set<T>& s, T value )
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

modelCacheState modelCacheIan :: isMergableSingleton ( const modelCacheSingleton* p ) const
{
	BipolarPointer Singleton = p->getValue();
	fpp_assert ( isValid(Singleton) );

	// check for the clash
	if ( isPositive (Singleton) )
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
		Singleton = inverse(Singleton);
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
		fpp_assert ( isValid(Singleton) );
		modelCacheState newState = csValid;

		// check for the clash
		if ( isNegative(Singleton) )
		{
			if ( set_contains ( posDConcepts, inverse(Singleton) ) )
				newState = csInvalid;
			else if ( set_contains ( posNConcepts, inverse(Singleton) ) )
				newState = csFailed;
			else
				negDConcepts.insert(inverse(Singleton));
		}
		else
		{
			if ( set_contains ( negDConcepts, Singleton ) )
				newState = csInvalid;
			else if ( set_contains ( negNConcepts, Singleton ) )
				newState = csFailed;
			else
				posDConcepts.insert(Singleton);
		}

		curState = mergeStatus ( getState(), newState );
		break;
	}
	case mctIan:
	{
		const modelCacheIan* q = static_cast<const modelCacheIan*>(p);
		// setup curState
		curState = isMergableIan(q);

		// merge all sets:
		posDConcepts.insert ( q->posDConcepts.begin(), q->posDConcepts.end() );
		posNConcepts.insert ( q->posNConcepts.begin(), q->posNConcepts.end() );
		negDConcepts.insert ( q->negDConcepts.begin(), q->negDConcepts.end() );
		negNConcepts.insert ( q->negNConcepts.begin(), q->negNConcepts.end() );
#	ifdef RKG_USE_SIMPLE_RULES
		extraDConcepts.insert ( q->extraDConcepts.begin(), q->extraDConcepts.end() );
		extraNConcepts.insert ( q->extraNConcepts.begin(), q->extraNConcepts.end() );
#	endif
		existsRoles.insert ( q->existsRoles.begin (), q->existsRoles.end () );
		forallRoles.insert ( q->forallRoles.begin (), q->forallRoles.end () );
		funcRoles.insert ( q->funcRoles.begin (), q->funcRoles.end () );
		break;
	}
	default:
		fpp_unreachable();
	}

	updateNominalStatus(p);
	return getState();
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

void modelCacheIan :: logCacheSet ( const conceptSet& s ) const
{
	LL << "{";
	if ( !s.empty() )
	{
		conceptSet::const_iterator p = s.begin ();
		LL << *p;
		for ( ++p; p != s.end(); ++p )
			LL << ',' << *p;
	}
	LL << "}";
}

void modelCacheIan :: logCacheSet ( const roleSet& s ) const
{
	LL << "{";
	if ( !s.empty() )
	{
		roleSet::const_iterator p = s.begin ();
		LL << '"' << (*p)->getName() << '"';
		for ( ++p; p != s.end(); ++p )
			LL << ',' << '"' << (*p)->getName() << '"';
	}
	LL << "}";
}

