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

#include "modelCacheIan.h"
#include "logging.h"

void modelCacheIan :: processConcept ( const DLVertex& cur, BipolarPointer bp )
{
		switch ( cur.Type() )
		{
		case dtTop:		// safety checks
		case dtDataType:	// data entries can not be cached
		case dtDataValue:
		case dtDataExpr:
			assert (0);
			break;

		case dtNConcept:	// add concepts to Concepts
		case dtPConcept:
		case dtNSingleton:
		case dtPSingleton:
			if ( isNegative (bp) )
				negConcepts.insert (inverse(bp));
			else
				posConcepts.insert (bp);

			break;

		case dtIrr:		// for \neg \ER.Self: add R to AR-set
			// FIXME!! we special-case it because the non-simple roles in Irr
			// still doesn't catched during preprocessing
			if ( isPositive(bp) )
				forallRoles.insert(cur.getRole());
//			else
//				addExistsRole(cur.getRole());
			break;

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

/// process CT edges in given interval; return true iff no edges with deps were found
void modelCacheIan :: processEdgeInterval ( e_iterator start, e_iterator end )
{
	for ( e_iterator q = start; q != end; ++q )
		if ( !(*q)->isIBlocked() )
		{
			addExistsRole ( (*q)->getRole() );
			updateDet((*q)->getDep());
		}
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

	// check for the clash
	if ( ( isPositive (Singleton) && set_contains ( negConcepts, Singleton ) ) ||
		 ( isNegative (Singleton) && set_contains ( posConcepts, inverse(Singleton) ) ) )
		return correctInvalid(Deterministic);
	else
		return csValid;
}

modelCacheState modelCacheIan :: isMergableIan ( const modelCacheIan* q ) const
{
	if ( sets_intersect ( posConcepts, q->negConcepts ) || sets_intersect ( q->posConcepts, negConcepts ) )
		return correctInvalid ( Deterministic && q->Deterministic );
	else if ( sets_intersect ( existsRoles, q->forallRoles ) ||
			  sets_intersect ( q->existsRoles, forallRoles ) ||
			  sets_intersect ( funcRoles, q->funcRoles ) )
		return csFailed;
	else	// could be merged
		return csValid;
}

modelCacheState modelCacheIan :: merge ( const modelCacheInterface* p )
{
	assert ( p != NULL );

	switch ( p->getCacheType() )
	{
	case mctConst:		// adds TOP/BOTTOM
		curState = mergeStatus ( getState(), p->getState() );
		break;
	case mctSingleton:	// adds Singleton
	{
		BipolarPointer Singleton = static_cast<const modelCacheSingleton*>(p)->getValue();
		modelCacheState newState = csValid;

		// check for the clash
		if ( isNegative(Singleton) )
		{
			if ( set_contains ( posConcepts, inverse(Singleton) ) )
				newState = csInvalid;
			else
				negConcepts.insert(inverse(Singleton));
		}
		else if ( isPositive(Singleton) )
		{
			if ( set_contains ( negConcepts, Singleton ) )
				newState = csInvalid;
			else
				posConcepts.insert(Singleton);
		}
		else	// wrong cache
			newState = csFailed;

		curState = mergeStatus ( getState(), newState );
		break;
	}
	case mctIan:
	{
		const modelCacheIan* q = static_cast<const modelCacheIan*>(p);
		// setup curState
		curState = isMergableIan(q);

		// merge all sets:
		posConcepts.insert ( q->posConcepts.begin (), q->posConcepts.end () );
		negConcepts.insert ( q->negConcepts.begin (), q->negConcepts.end () );
		existsRoles.insert ( q->existsRoles.begin (), q->existsRoles.end () );
		forallRoles.insert ( q->forallRoles.begin (), q->forallRoles.end () );
		funcRoles.insert ( q->funcRoles.begin (), q->funcRoles.end () );

		// change flags
		Deterministic &= q->Deterministic;
		break;
	}
	default:
		assert (0);
	}

	updateNominalStatus(p);
	return getState();
}

// logging
void modelCacheIan :: logCacheEntry ( unsigned int level ) const
{
	CHECK_LL_RETURN(level);
	LL << "\nIan cache: det(" << Deterministic << "), posConcepts = ";
	logCacheSet ( posConcepts );
	LL << ", negConcepts = ";
	logCacheSet ( negConcepts );
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

