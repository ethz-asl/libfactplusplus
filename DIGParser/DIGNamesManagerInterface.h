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

#ifndef _DIGNAMESMANAGERINTERFACE_H
#define _DIGNAMESMANAGERINTERFACE_H

#include <string>
#include <map>
#include <vector>

#include "fpp_assert.h"

/// constants for all names appearing in DIG interface
enum DIGTag
{
	dig_First = 0,		// global stopper
	dig_Error = dig_First,

	// general commands
	dig_General_Begin,	// general group stopper
	digGetIdentifier = dig_General_Begin,
	digNewKB,
	digReleaseKB,
	digClearKB,
	digTells,
	digAsks,
	dig_General_End,		// general group stopper

	// concept definition commands
	dig_CName_Begin,	// concept group stopper
	digTop = dig_CName_Begin,
	digBottom,

	digCAtom,
	digRAtom,
	digFeature,
	digAttribute,
	digIndividual,

	digNot,
	digAnd,
	digOr,
	digSome,
	digAll,
	digInverse,
	digAtLeast,
	digAtMost,
	digISet,

	digIVal,
	digSVal,
	digDefined,
	digChain,

	digIntMin,
	digIntMax,
	digIntEquals,
	digIntRange,
	digStrMin,
	digStrMax,
	digStrEquals,
	digStrRange,
	dig_CName_End,		// concept group stopper

	// axioms
	dig_Axioms_Begin,	// axiom group stopper
	digDefConcept = dig_Axioms_Begin,
	digDefRole,
	digDefFeature,
	digDefAttribute,
	digDefIndividual,

	digDisjointAxiom,	// warning: 'disjoint' axiom
	digImpliesC,
	digEqualC,

	digImpliesR,
	digEqualR,
	digDomain,
	digRange,
	digRangeInt,
	digRangeStr,
	digTransitive,
	digFunctional,

	digInstanceOf,
	digRelated,

	digValue,
	dig_Axioms_End,		// axiom group stopper

	// ask members
	dig_Ask_Begin,		// ask group stopper
	digAllConceptNames = dig_Ask_Begin,
	digAllRoleNames,
	digAllIndividuals,

	digSatisfiable,
	digSubsumes,
	digDisjointQuery,	// warning: 'disjoint' query

	digCParents,
	digCChildren,
	digCAncestors,
	digCDescendants,
	digCEquivalents,

	digRParents,
	digRChildren,
	digRAncestors,
	digRDescendants,
	digREquivalents,

	digInstances,
	digTypes,
	digInstance,
	digRoleFillers,
	digRelatedIndividuals,
	digToldValue,

	// extra (user-requested) DIG features
	digAllAbsorbedPrimitiveConceptDefinitions,
	digUnabsorbableGCI,

	dig_Ask_End,		// ask group stopper

	// other (auxiliary) members
	digDisjoint,		// transforms to digDisjointAxiom/digDisjointQuery
	dig_Last			// last possible value
}; // DIGNames

/// Map interface to DIG command parser
class DIGNamesManagerInterface
{
protected:	// types
	typedef std::map<const std::string, DIGTag> tTagMap;

protected:	// members
		/// DIG interface version (main number)
	unsigned int Version;
		/// map from DIG command name to DIG tag
	tTagMap Base;
		/// index for fast access to tag's names
	std::string Index[dig_Last-dig_First+1];

protected:	// methods
		/// add pair "name:tag" to Base; set up index as well
	void addName ( DIGTag tag, const std::string& name )
	{
		fpp_assert ( tag > dig_First && tag < dig_Last );	// range control
		fpp_assert ( Index[tag] == "" );	// each tag should be activated only once
		Base[name] = tag;
		Index[tag] = name;
	}

public:	// interface
		/// default c'tor
	DIGNamesManagerInterface ( void )
		: Version (0)
	{}
		/// virtual d'tor
	virtual ~DIGNamesManagerInterface ( void ) {}

		/// version of DIG interface implemented
	unsigned int DIGVersion ( void ) const { return Version; }

		/// get DIGTag tag according to given string
	DIGTag getTag ( const std::string& name ) const
	{
		tTagMap::const_iterator p = Base.find(name);
		if ( p == Base.end() )
			return dig_Error;
		return p->second;
	}
		/// get DIG command name according to given tag
	const std::string& getName ( DIGTag tag ) const
	{
		fpp_assert ( tag > dig_First && tag < dig_Last );	// range control
		return Index[tag];
	}
}; // DIGNamesManagerInterface

#endif
