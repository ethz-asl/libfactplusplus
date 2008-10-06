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

#include "DIGParserHandlers.h"

#include <xercesc/sax/AttributeList.hpp>

#include "strx.h"
#include "globaldef.h"

using namespace xercesc;

// ---------------------------------------------------------------------------
// Implementation of a parser in details
// ---------------------------------------------------------------------------

inline void writeConcept ( ostream& o, const char* name )
{
	o << "<catom name=\"" << name << "\"/>";
}
inline void writeRole ( ostream& o, const char* name )
{
	o << "<ratom name=\"" << name << "\"/>";
}
inline void writeIndividual ( ostream& o, const char* name )
{
	o << "<individual name=\"" << name << "\"/>";
}

// Actor for Concept hierarchy
class ConceptActor
{
protected:
	std::ostream& o;
	closedXMLEntry* syn;
	closedXMLEntry* pEntry;

		/// process single entry in a vertex label
	bool tryEntry ( const ClassifiableEntry* p )
	{
		// check the applicability
		if ( p->isSystem() || static_cast<const TConcept*>(p)->isSingleton() )
			return false;

		// set the context
		if ( syn == NULL )
			syn = new closedXMLEntry ( "synonyms", o );

		// print the concept
		o << "\n  ";
		const std::string name(p->getName());

		if ( p->getId () >= 0 )
			writeConcept ( o, name.c_str() );
		else if ( name == std::string("TOP") )
			simpleXMLEntry top ( "top", o );
		else if ( name == std::string("BOTTOM") )
			simpleXMLEntry bottom ( "bottom", o );
		else	// error
			return false;

		return true;
	}

public:
	ConceptActor ( std::ostream& oo, const char* id )
		: o(oo)
		, syn(NULL)
		, pEntry ( new closedXMLEntry ( "conceptSet", oo, id ) )
		{}
	~ConceptActor ( void ) { delete pEntry; }

	bool apply ( const TaxonomyVertex& v )
	{
		tryEntry(v.getPrimer());

		for ( TaxonomyVertex::syn_iterator p = v.begin_syn(), p_end=v.end_syn(); p != p_end; ++p )
			tryEntry(*p);

		if ( syn )
		{
			delete syn;
			syn = NULL;
			return true;
		}
		else
			return false;
	}
}; // ConceptActor

// Actor for Individual "hierarchy"
class IndividualActor
{
protected:
	std::ostream& o;
	closedXMLEntry* pEntry;

		/// process single entry in a vertex label
	bool tryEntry ( const ClassifiableEntry* p )
	{
		// check the applicability
		if ( p->isSystem() || !static_cast<const TConcept*>(p)->isSingleton() )
			return false;

		// print the concept
		o << "\n  <individual name=\"" << p->getName() << "\"/>";
		return true;
	}

public:
	IndividualActor ( std::ostream& oo, const char* id )
		: o(oo)
		, pEntry ( new closedXMLEntry ( "individualSet", oo, id ) )
		{}
	~IndividualActor ( void ) { delete pEntry; }

	bool apply ( const TaxonomyVertex& v )
	{
		bool ret = tryEntry(v.getPrimer());

		for ( TaxonomyVertex::syn_iterator p = v.begin_syn(), p_end=v.end_syn(); p != p_end; ++p )
			ret |= tryEntry(*p);

		return ret;
	}
}; // IndividualActor

// Actor for Role hierarchy for DIG 1.1
class RoleActor
{
protected:
	std::ostream& o;
	closedXMLEntry* syn;
	closedXMLEntry* pEntry;

		/// process single entry in a vertex label
	bool tryEntry ( const ClassifiableEntry* p )
	{
		// check the applicability: system and inverse roles are useless
		if ( p->isSystem() || p->getId() <= 0 )
			return false;

		// set the context
		if ( syn == NULL )
			syn = new closedXMLEntry ( "synonyms", o );

		// print the role
		o << "\n  <ratom name=\"" << p->getName() << "\"/>";
		return true;
	}

public:
	RoleActor ( std::ostream& oo, const char* id )
		: o(oo)
		, syn(NULL)
		, pEntry ( new closedXMLEntry ( "roleSet", oo, id ) )
		{}
	~RoleActor ( void ) { delete pEntry; }

	bool apply ( const TaxonomyVertex& v )
	{
		tryEntry(v.getPrimer());

		for ( TaxonomyVertex::syn_iterator p = v.begin_syn(), p_end=v.end_syn(); p != p_end; ++p )
			tryEntry(*p);

		if ( syn )
		{
			delete syn;
			syn = NULL;
			return true;
		}
		else
			return false;
	}
}; // RoleActor

/// start of ask element (allNames, satisfy)
void DIGParseHandlers :: startAsk ( DIGTag tag, AttributeList& attributes )
{
	assert ( tag >= dig_Ask_Begin && tag < dig_Ask_End );	// safety check

	// set up id of the ask
	const XMLCh* parm = attributes.getValue ( "id" );

	if ( parm == NULL )
		throwAttributeAbsence ( "id", tag );

	// create current id string
	curId = " id=\"";
	curId += StrX (parm).localForm ();
	curId += "\"";
}


/// end of ask element (allNames, satisfy)
void DIGParseHandlers :: endAsk ( DIGTag tag )
{
	assert ( tag >= dig_Ask_Begin && tag < dig_Ask_End );	// safety check

#define ERROR_400														\
	do {																\
		string Reason ( "General 'ask' error\"" );						\
		Reason += curId;												\
		*Reason.rbegin() = '\0';										\
		outError ( 400, Reason.c_str(), "undefined names in query" );	\
		return;															\
	} while(0)

#define ERROR_401											\
	do {													\
		string Reason ( "Unsupported 'ask' command\"" );	\
		Reason += curId;									\
		*Reason.rbegin() = '\0';							\
		outError ( 401, Reason.c_str(), "" );				\
	} while(0)

	bool fail = false;

	switch (tag)
	{
	case digAllConceptNames:	// all
	{
		ConceptActor actor ( *o, curId.c_str() );
		if ( pKernel->getAllConcepts(actor) )
			ERROR_401;
		return;
	}
	case digAllRoleNames:
	{
		RoleActor actor ( *o, curId.c_str() );
		if ( pKernel->getAllRoles(actor) )
			ERROR_401;
		return;
	}
	case digAllIndividuals:
	{
		IndividualActor actor ( *o, curId.c_str() );
		if ( pKernel->getAllIndividuals(actor) )
			ERROR_401;
		return;
	}
	case digSatisfiable:	// sat
	case digSubsumes:
	case digInstance:		// the same as subsumes
	case digDisjointQuery:
	{
		bool ret = false;

		if ( workStack.empty() )
			throwArgumentAbsence ( "concept", tag );
		DLTree* q = workStack.top();
		workStack.pop();

		if ( wasError )
			fail = true;
		else if ( tag == digSatisfiable )
			fail = pKernel->isSatisfiable ( q, ret );
		else
		{
			if ( workStack.empty() )
				throwArgumentAbsence ( "concept", tag );
			DLTree* p = workStack.top();
			workStack.pop();

			if ( tag == digSubsumes )
				fail = pKernel->isSubsumedBy ( q, p, ret );
			else if ( tag == digInstance )
				fail = pKernel->isInstance ( p, q, ret );
			else	// ( name == "disjoint" )
				fail = pKernel->isDisjoint ( p, q, ret );

			deleteTree(p);
		}

		simpleXMLEntry ( fail?"error":(ret?"true":"false"), *o, curId.c_str() );

		deleteTree(q);
		return;
	}
	case digCParents:			// concept hierarchy
	case digCChildren:
	case digCAncestors:
	case digCDescendants:
	case digTypes:
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "concept or individual", tag );

		DLTree* p = workStack.top();
		workStack.pop();
		ConceptActor actor ( *o, curId.c_str() );

		// if were any errors during concept expression construction
		if ( wasError )
			fail = true;
		else if ( tag == digCParents )
			fail = pKernel->getParents ( p, actor );
		else if ( tag == digCChildren )
			fail = pKernel->getChildren ( p, actor );
		else if ( tag == digCAncestors )
			fail = pKernel->getAncestors ( p, actor );
		else if ( tag == digCDescendants )
			fail = pKernel->getDescendants ( p, actor );
		else if ( tag == digTypes )
			fail = pKernel->getDirectTypes ( p, actor );

		deleteTree(p);

		if ( fail )	// error
			ERROR_400;

		return;
	}
	case digEquivalents:
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "concept expression", tag );
		DLTree* p = workStack.top();
		workStack.pop();
		ConceptActor actor ( *o, curId.c_str() );

		fail = wasError || pKernel->getEquivalents ( p, actor );
		deleteTree(p);

		if ( fail )	// error
			ERROR_400;

		return;
	}
	case digRParents:			// role hierarchy
	case digRChildren:
	case digRAncestors:
	case digRDescendants:
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "role", tag );

		DLTree* p = workStack.top();
		workStack.pop();
		RoleActor actor ( *o, curId.c_str() );

		// if were any errors during concept expression construction
		if ( wasError )
			fail = true;
		else if ( tag == digRParents )
			fail = pKernel->getRParents ( p, actor );
		else if ( tag == digRChildren )
			fail = pKernel->getRChildren ( p, actor );
		else if ( tag == digRAncestors )
			fail = pKernel->getRAncestors ( p, actor );
		else if ( tag == digRDescendants )
			fail = pKernel->getRDescendants ( p, actor );

		deleteTree(p);

		if ( fail )	// error
			ERROR_400;

		return;
	}

	case digInstances:			// individual queries
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "concept expression", tag );
		DLTree* p = workStack.top();
		workStack.pop();
		IndividualActor actor ( *o, curId.c_str() );

		// to find instances just locate all descendants and remove non-nominals
		fail = wasError || pKernel->getInstances ( p, actor );
		deleteTree(p);

		if ( fail )
			ERROR_400;

		return;
	}
	case digRoleFillers:
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "role", tag );
		DLTree* R = workStack.top();
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "individual", tag );
		DLTree* I = workStack.top();
		workStack.pop();
		ReasoningKernel::IndividualSet Js;

		// if were any errors during concept expression construction
		if ( wasError )
			fail = true;
		else
			fail = pKernel->getRoleFillers ( I, R, Js );

		deleteTree(I);
		deleteTree(R);

		if ( fail )
			ERROR_400;

		// output individual set
		closedXMLEntry x ( "individualSet", *o, curId.c_str() );
		for ( ReasoningKernel::IndividualSet::const_iterator
			  p = Js.begin(), p_end = Js.end(); p < p_end; ++p )
			writeIndividual ( *o, (*p)->getName() );

		return;
	}
	case digRelatedIndividuals:
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "role", tag );
		DLTree* R = workStack.top();
		workStack.pop();
		ReasoningKernel::IndividualSet Is, Js;

		// if were any errors during concept expression construction
		if ( wasError )
			fail = true;
		else
			fail = pKernel->getRelatedIndividuals ( R, Is, Js );

		deleteTree(R);

		if ( fail )
			ERROR_400;

		// output individual set
		closedXMLEntry x ( "individualPairSet", *o, curId.c_str() );
		for ( ReasoningKernel::IndividualSet::const_iterator
			  p = Is.begin(), p_end = Is.end(), q = Js.begin(); p < p_end; ++p, ++q )
		{
			*o << "\n  <individualPair>";
			writeIndividual ( *o, (*p)->getName() );
			writeIndividual ( *o, (*q)->getName() );
			*o << "</individualPair>";
		}

		return;
	}
	case digToldValue:
		 ERROR_401;		// unsupported
		 return;

	case digAllAbsorbedPrimitiveConceptDefinitions:	// get all PConcept's definitions
	{
		closedXMLEntry x ( "absorbedPrimitiveConceptDefinitions", *o, curId.c_str() );
		pKernel->absorbedPrimitiveConceptDefinitions(*o);
		return;
	}
	case digUnabsorbableGCI:
	{
		closedXMLEntry x ( "unabsorbed", *o, curId.c_str() );
		pKernel->unabsorbed(*o);
		return;
	}

	default:
		assert (0);	// safety check
	}

	*o << "\n";
#undef ERROR_400
#undef ERROR_401
}

void DIGParseHandlers :: outputSupportedLanguage ( void )
{
	// write down supported syntax
	closedXMLEntry supp ( "supports", *o );

	// language part
	{
		*o << "\n";
		closedXMLEntry lang ( "language", *o );

		simpleXMLEntry ( "top", *o );
		simpleXMLEntry ( "bottom", *o );
		simpleXMLEntry ( "catom", *o );

		simpleXMLEntry ( "and", *o );
		simpleXMLEntry ( "or", *o );
		simpleXMLEntry ( "not", *o );

		simpleXMLEntry ( "some", *o );
		simpleXMLEntry ( "all", *o );
		simpleXMLEntry ( "atmost", *o );
		simpleXMLEntry ( "atleast", *o );
		simpleXMLEntry ( "iset", *o );

		simpleXMLEntry ( "defined", *o );
		simpleXMLEntry ( "stringmin", *o );
		simpleXMLEntry ( "stringmax", *o );
		simpleXMLEntry ( "stringequals", *o );
		simpleXMLEntry ( "stringrange", *o );
		simpleXMLEntry ( "intmin", *o );
		simpleXMLEntry ( "intmax", *o );
		simpleXMLEntry ( "intequals", *o );
		simpleXMLEntry ( "intrange", *o );

		simpleXMLEntry ( "ratom", *o );
		simpleXMLEntry ( "feature", *o );
		simpleXMLEntry ( "inverse", *o );
		simpleXMLEntry ( "attribute", *o );
//		simpleXMLEntry ( "chain", *o );

		simpleXMLEntry ( "individual", *o );
	}

	// tell part
	{
		*o << "\n";
		closedXMLEntry tell ( "tell", *o );

		simpleXMLEntry ( "defconcept", *o );
		simpleXMLEntry ( "defrole", *o );
		simpleXMLEntry ( "deffeature", *o );
		simpleXMLEntry ( "defattribute", *o );
		simpleXMLEntry ( "defindividual", *o );

		simpleXMLEntry ( "impliesc", *o );
		simpleXMLEntry ( "equalc", *o );
		simpleXMLEntry ( "disjoint", *o );

		simpleXMLEntry ( "impliesr", *o );
		simpleXMLEntry ( "equalr", *o );
		simpleXMLEntry ( "domain", *o );
		simpleXMLEntry ( "range", *o );
		simpleXMLEntry ( "rangeint", *o );
		simpleXMLEntry ( "rangestring", *o );
		simpleXMLEntry ( "transitive", *o );
		simpleXMLEntry ( "functional", *o );

		simpleXMLEntry ( "instanceof", *o );
		simpleXMLEntry ( "related", *o );
		simpleXMLEntry ( "value", *o );
	}

	// ask part
	{
		*o << "\n";
		closedXMLEntry ask ( "ask", *o );

		simpleXMLEntry ( "allConceptNames", *o );
		simpleXMLEntry ( "allRoleNames", *o );
		simpleXMLEntry ( "allIndividuals", *o );

		simpleXMLEntry ( "satisfiable", *o );
		simpleXMLEntry ( "subsumes", *o );
		simpleXMLEntry ( "disjoint", *o );

		simpleXMLEntry ( "parents", *o );
		simpleXMLEntry ( "children", *o );
		simpleXMLEntry ( "ancestors", *o );
		simpleXMLEntry ( "descendants", *o );
		simpleXMLEntry ( "equivalents", *o );

		simpleXMLEntry ( "rparents", *o );
		simpleXMLEntry ( "rchildren", *o );
		simpleXMLEntry ( "rancestors", *o );
		simpleXMLEntry ( "rdescendants", *o );

		simpleXMLEntry ( "instances", *o );
		simpleXMLEntry ( "types", *o );
		simpleXMLEntry ( "instance", *o );
		simpleXMLEntry ( "roleFillers", *o );
		simpleXMLEntry ( "relatedIndividuals", *o );
//		simpleXMLEntry ( "toldValues", *o );
	}
	*o << "\n";
}
