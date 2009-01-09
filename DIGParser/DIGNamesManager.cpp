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

#include "DIGNamesManager.h"

/// DIG 1.x implementation
NamesManager_DIG_1x :: NamesManager_DIG_1x ( void )
	: DIGNamesManagerInterface ()
{
	// correct version info
	Version = 1;

	// general commands
	addName ( digGetIdentifier, "getIdentifier" );
	addName ( digNewKB, "newKB" );
	addName ( digReleaseKB, "releaseKB" );
	addName ( digClearKB, "clearKB" );
	addName ( digTells, "tells" );
	addName ( digAsks, "asks" );

	// concept definition commands
	addName ( digTop, "top" );
	addName ( digBottom, "bottom" );

	addName ( digCAtom, "catom" );
	addName ( digRAtom, "ratom" );
	addName ( digFeature, "feature" );
	addName ( digAttribute, "attribute" );
	addName ( digIndividual, "individual" );

	addName ( digNot, "not" );
	addName ( digAnd, "and" );
	addName ( digOr, "or" );
	addName ( digSome, "some" );
	addName ( digAll, "all" );
	addName ( digInverse, "inverse" );
	addName ( digAtLeast, "atleast" );
	addName ( digAtMost, "atmost" );
	addName ( digISet, "iset" );

	addName ( digIVal, "ival" );
	addName ( digSVal, "sval" );
	addName ( digDefined, "defined" );
	addName ( digChain, "chain" );

	addName ( digIntMin, "intmin" );
	addName ( digIntMax, "intmax" );
	addName ( digIntEquals, "intequals" );
	addName ( digIntRange, "intrange" );
	addName ( digStrMin, "stringmin" );
	addName ( digStrMax, "stringmax" );
	addName ( digStrEquals, "stringequals" );
	addName ( digStrRange, "stringrange" );

	// axioms
	addName ( digDefConcept, "defconcept" );
	addName ( digDefRole, "defrole" );
	addName ( digDefFeature, "deffeature" );
	addName ( digDefAttribute, "defattribute" );
	addName ( digDefIndividual, "defindividual" );

	addName ( digImpliesC, "impliesc" );
	addName ( digEqualC, "equalc" );

	addName ( digImpliesR, "impliesr" );
	addName ( digEqualR, "equalr" );
	addName ( digDomain, "domain" );
	addName ( digRange, "range" );
	addName ( digRangeInt, "rangeint" );
	addName ( digRangeStr, "rangestring" );
	addName ( digTransitive, "transitive" );
	addName ( digFunctional, "functional" );

	addName ( digInstanceOf, "instanceof" );
	addName ( digRelated, "related" );

	addName ( digValue, "value" );

	// ask members
	addName ( digAllConceptNames, "allConceptNames" );
	addName ( digAllRoleNames, "allRoleNames" );
	addName ( digAllIndividuals, "allIndividuals" );

	addName ( digSatisfiable, "satisfiable" );
	addName ( digSubsumes, "subsumes" );

	addName ( digCParents, "parents" );
	addName ( digCChildren, "children" );
	addName ( digCAncestors, "ancestors" );
	addName ( digCDescendants, "descendants" );
	addName ( digCEquivalents, "equivalents" );

	addName ( digRParents, "rparents" );
	addName ( digRChildren, "rchildren" );
	addName ( digRAncestors, "rancestors" );
	addName ( digRDescendants, "rdescendants" );
	addName ( digREquivalents, "rquivalents" );

	addName ( digInstances, "instances" );
	addName ( digTypes, "types" );
	addName ( digInstance, "instance" );
	addName ( digRoleFillers, "roleFillers" );
	addName ( digRelatedIndividuals, "relatedIndividuals" );
	addName ( digToldValue, "toldValues" );

	addName ( digAllAbsorbedPrimitiveConceptDefinitions, "allAbsorbedPrimitiveConceptDefinitions" );
	addName ( digUnabsorbableGCI, "unabsorbableGCI" );

	// fix DISJOINT issue
	addName ( digDisjoint, "disjoint" );
	Index[digDisjointAxiom] = Index[digDisjoint];
	Index[digDisjointQuery] = Index[digDisjoint];
}


