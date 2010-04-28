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

#include "DIGParserHandlers.h"

#include <xercesc/sax/AttributeList.hpp>

#include "strx.h"
#include "globaldef.h"

using namespace xercesc;

// ---------------------------------------------------------------------------
// Implementation of a DIG parser in details: input part
// ---------------------------------------------------------------------------
/// start of command (getId, tells, asks)
void DIGParseHandlers :: startCommand ( DIGTag tag, AttributeList& attributes )
{
	fpp_assert ( tag >= dig_General_Begin && tag < dig_General_End );	// safety check

	// clear current KB
	if ( tag == digClearKB )
	{
		if ( pKernel->clearKB() )	// error
			throw DIGParserException ( 204, "KB release error", "Could not clear KB" );
#		ifdef RKG_PRINT_DIG_MESSAGES
		else
			cerr << "current KB is cleared\n";
#		endif
		return;
	}

	{
#	ifdef RKG_PRINT_DIG_MESSAGES
		cerr << "Processing command " << NamesManager.getName(tag).c_str();
#	endif
		// find parameters
		string KB ( "default" );	// default KB name

		const XMLCh* kbUri = attributes.getValue ( "uri" );
		if ( kbUri != NULL )
			KB = StrX(kbUri).localForm();

#	ifdef RKG_PRINT_DIG_MESSAGES
		if ( tag != digGetIdentifier && tag != digNewKB )
			cerr << " for KB \"" << KB.c_str() << "\"";
		cerr << "...\n";
#	endif

		// print standart XML header
		*o << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";

		// create proper answer
		if ( tag == digGetIdentifier )
			pEnv = new complexXMLEntry ( "identifier", *o );	// FIXME!! names
		else if ( tag == digAsks )
			pEnv = new complexXMLEntry ( "responses", *o );		// FIXME!! names
		else	// single response
			pEnv = new complexXMLEntry ( "response", *o );		// FIXME!! names

		// define xmlns
		*o << "\n  xmlns=\"http://dl.kr.org/dig/2003/02/lang\"";	// FIXME!! set ns via DIG version
		// define xmlns:xsi
		*o << "\n  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"";
		// define xsi:schemaLocation
		*o << "\n  xsi:schemaLocation=\"http://dl.kr.org/dig/2003/02/lang\n  http://dl-web.man.ac.uk/dig/2003/02/dig.xsd\"";

		if ( tag == digGetIdentifier )
		{
			// output reasoner info
			*o << "\n  name=\"FaCT++\" version=\"" << kFactory.getKernel()->getVersion() << "\"";
		}

		pEnv->closeStart();
		*o << "\n";

		// setup flags
		if ( tag == digTells )
			inTell = true;
		if ( tag == digAsks )
			inAsk = true;

		// other actions
		if ( tag == digNewKB )
		{
			// create a KB; name will be in KB
			if ( kFactory.newKB(KB) )	// error
				throw DIGParserException ( 201, "Cannot create new knoweledge base", "Such base aready exists" );
			else
			{	// return URI of a new KB
				simpleXMLEntry kb ( "kb", *o );
				*o << "\n  uri=\"" << KB.c_str() << "\"";
#			ifdef RKG_PRINT_DIG_MESSAGES
				cerr << "KB " << KB.c_str() << " created\n";
#			endif
			}
		}
		else if ( tag == digReleaseKB )
		{
			// release KB; URI is in KB;
			if ( kFactory.releaseKB(KB) )	// error
				throw DIGParserException ( 204, "KB release error", "No such KB" );
			else
			{
#			ifdef RKG_PRINT_DIG_MESSAGES
				cerr << "KB " << KB.c_str() << " released\n";
#			endif
				simpleXMLEntry ok ( "ok", *o );
			}
		}
		else if ( tag != digGetIdentifier )	// tells or asks
		{
			// switch to proper KB
			pKernel = kFactory.getKernel(KB);
			if ( pKernel == NULL )
				throw DIGParserException ( 203, "Unknown KB URI", "No such KB" );
			pEM = pKernel->getExpressionManager();
			if ( tag == digAsks )	// late classification: after all tells
				try
				{
					classifyCurrentKB();
				}
				catch ( EFPPInconsistentKB )
				{
					throw DIGParserException ( 900, "Incoherent KB",
						"KB with URI=\"", KB, "\" is incoherent" );
				}
				catch ( EFPPNonSimpleRole nsr )
				{
					throw DIGParserException ( 99, "Incorrect KB", nsr.what() );
				}
				catch ( EFPPCycleInRIA cir )
				{
					throw DIGParserException ( 99, "Incorrect KB", cir.what() );
				}
		}
		else // for Identifier
			outputSupportedLanguage();
	}
}

/// start of concept expression (id, and, or)
void DIGParseHandlers :: startConcept ( DIGTag tag, AttributeList& attributes )
{
	fpp_assert ( tag >= dig_CName_Begin && tag < dig_CName_End );	// safety check
	// set up name for the named stuff
	const XMLCh* parm_name = attributes.getValue ( "name" );
	const XMLCh* parm_num = attributes.getValue ( "num" );
	const XMLCh* parm_val = attributes.getValue ( "val" );
	const XMLCh* parm_min = attributes.getValue ( "min" );
	const XMLCh* parm_max = attributes.getValue ( "max" );

	int n = 0;
	string name, val, min, max;

	if ( parm_name != NULL )
		name = StrX (parm_name).localForm();
	if ( parm_num != NULL )
		n = atoi ( StrX (parm_num).localForm () );
	if ( parm_val != NULL )
		val = StrX (parm_val).localForm();
	if ( parm_min != NULL )
		min = StrX (parm_min).localForm();
	if ( parm_max != NULL )
		max = StrX (parm_max).localForm();

	switch (tag)
	{
	case digTop:
		workStack.push(pEM->Top());
		return;

	case digBottom:
		workStack.push(pEM->Bottom());
		return;

	case digCAtom:
		if ( parm_name == NULL )
			throwAttributeAbsence ( "name", tag );
		workStack.push(tryConceptName(name));
		return;

	case digRAtom:
		if ( parm_name == NULL )
			throwAttributeAbsence ( "name", tag );
		workStack.push(tryRoleName(name));
		return;

	case digFeature:
		if ( parm_name == NULL )
			throwAttributeAbsence ( "name", tag );
		try
		{
			TORoleExpr* role = tryRoleName(name);
			pKernel->setOFunctional(role);
			workStack.push(role);
		}
		catch(...)
		{
			wasError = true;
			string reason ( "Feature '" );
			reason += name;
			reason += "' undefined";
			outWarning ( 99, "Undefined name", reason.c_str() );
		}
		return;

	case digAttribute:
		if ( parm_name == NULL )
			throwAttributeAbsence ( "name", tag );
		workStack.push(tryDataRoleName(name));
		return;

	case digIndividual:
		if ( parm_name == NULL )
			throwAttributeAbsence ( "name", tag );
		workStack.push(tryIndividualName(name));
		return;

	// actions which does nothing
	case digNot:
	case digSome:
	case digAll:
	case digInverse:
		return;

	// number stuff
	case digAtMost:
	case digAtLeast:
		if ( parm_num == NULL )
			throwAttributeAbsence ( "num", tag );
		numStack.push(n);
		return;

	// n-argument staff
	case digAnd:
	case digOr:
	case digISet:
		workStack.push(NULL);	// put stopper into stack
		return;

	// value staff
	case digIVal:
	case digSVal:
		useData = true;	// save the following data value
		return;

	// general datatype
	case digDefined:
		return;

	// int datatype constructors
	case digIntMin:
	case digIntMax:
	case digIntEquals:
		if ( parm_val == NULL )
			throwAttributeAbsence ( "val", tag );
		workStack.push(tryIntDataValue(val));
		return;

	case digIntRange:
		if ( parm_min == NULL )
			throwAttributeAbsence ( "min", tag );
		if ( parm_max == NULL )
			throwAttributeAbsence ( "max", tag );
		workStack.push(tryIntDataValue(min));
		workStack.push(tryIntDataValue(max));	// max is on top!
		return;

	// string datatype constructors
	case digStrMin:
	case digStrMax:
	case digStrEquals:
		if ( parm_val == NULL )
			throwAttributeAbsence ( "val", tag );
		workStack.push(tryStrDataValue(val));
		return;

	case digStrRange:
		if ( parm_min == NULL )
			throwAttributeAbsence ( "min", tag );
		if ( parm_max == NULL )
			throwAttributeAbsence ( "max", tag );
		workStack.push(tryStrDataValue(min));
		workStack.push(tryStrDataValue(max));	// max is on top!
		return;

	// unsupported stuff
	case digChain:
		// throw (Unsupported)
		return;
	default:	// safety
		fpp_unreachable();
	}
}

/// start of axiom (implies, defconcept)
void DIGParseHandlers :: startAxiom ( DIGTag tag, AttributeList& attributes )
{
	fpp_assert ( tag >= dig_Axioms_Begin && tag < dig_Axioms_End );	// safety check
	// set up name for the named stuff
	const XMLCh* parm = attributes.getValue ( "name" );

	// check whether axioms are allowed
	if ( pKernel->isKBClassified() )
		throw DIGParserException ( 901, "Inclremental classification unsupported",
				"No tells are allowed (except clearKB) after any ASK query" );

	// macro for error printing
#ifdef RKG_PRINT_DIG_MESSAGES
#	define ERROR_IF(action)\
	try {action;} catch(...) { cerr << "ERROR encountered in " << NamesManager.getName(tag).c_str() << " command\n"; }
#else
#	define ERROR_IF(action)\
	try {action;} catch(...) {/*put error message here*/}
#endif

	switch (tag)
	{
	case digDefConcept:
		if ( parm == NULL )
			throwAttributeAbsence ( "name", tag );
		pKernel->declare(tryConceptName(StrX(parm).localForm()));
		return;

	case digDefRole:
		if ( parm == NULL )
			throwAttributeAbsence ( "name", tag );
		pKernel->declare(tryRoleName(StrX(parm).localForm()));
		return;

	case digDefFeature:
		if ( parm == NULL )
			throwAttributeAbsence ( "name", tag );
		pKernel->declare(tryRoleName(StrX(parm).localForm()));
		ERROR_IF ( pKernel->setTransitive(tryRoleName(StrX(parm).localForm())) )
		return;

	case digDefAttribute:
		if ( parm == NULL )
			throwAttributeAbsence ( "name", tag );
		pKernel->declare(tryDataRoleName(StrX(parm).localForm()));
		return;

	case digDefIndividual:
		if ( parm == NULL )
			throwAttributeAbsence ( "name", tag );
		pKernel->declare(tryIndividualName(StrX(parm).localForm()));
		return;

	case digDisjointAxiom:	// push begining of disjoint
		workStack.push(NULL);
		return;

	case digImpliesC:
	case digEqualC:
	case digImpliesR:
	case digEqualR:
	case digDomain:
	case digRange:
	case digRangeInt:
	case digRangeStr:
	case digTransitive:
	case digFunctional:
	case digInstanceOf:
	case digRelated:
	case digValue:
		return;	// nothing to do here. everything will be done by endElement

	default:
		fpp_unreachable();	// safety check
	}
#undef ERROR_IF
}

/// end of command (getId, tells, asks)
void DIGParseHandlers :: endCommand ( DIGTag tag )
{
	fpp_assert ( tag >= dig_General_Begin && tag < dig_General_End );	// safety check

	if ( tag == digTells )		// tells query -- return OK
		*o << "\n<ok/>";
}

/// end of concept expression (id, and, or)
void DIGParseHandlers :: endConcept ( DIGTag tag )
{
	fpp_assert ( tag >= dig_CName_Begin && tag < dig_CName_End );	// safety check

	switch (tag)
	{
	case digTop:		// nothing to do -- it's just name
	case digBottom:
	case digCAtom:
	case digRAtom:
	case digFeature:
	case digAttribute:
	case digIndividual:
		return;

	case digNot:	// top := NOT (top)
		if ( workStack.empty() )
			throwArgumentAbsence ( "concept", tag );
		workStack.top() = pEM->Not(dynamic_cast<TConceptExpr*>(workStack.top()));
		return;

	case digInverse:
		if ( workStack.empty() )
			throwArgumentAbsence ( "role", tag );
		workStack.top() = pEM->Inverse(dynamic_cast<TORoleExpr*>(workStack.top()));
		return;

	case digSome:
	case digAll:	// top := \? R.C [now top = ... R C]
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "concept", "role", tag );
		TConceptExpr* C = dynamic_cast<TConceptExpr*>(workStack.top());
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "concept", "role", tag );
		workStack.top() = tag == digSome
			? pEM->Exists ( dynamic_cast<TORoleExpr*>(workStack.top()), C )
			: pEM->Forall ( dynamic_cast<TORoleExpr*>(workStack.top()), C );
		return;
	}

	case digAtMost:
	case digAtLeast:	// top := \?e n R.C [now top = ... R C]
	{
		fpp_assert ( !numStack.empty() );
		unsigned int n = numStack.top();
		numStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "concept", "role", tag );
		TConceptExpr* C = dynamic_cast<TConceptExpr*>(workStack.top());
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "concept", "role", tag );
		TORoleExpr* R = dynamic_cast<TORoleExpr*>(workStack.top());
		if ( workStack.empty() )
			throwArgumentAbsence ( "number", tag );

		// create \?e n R.C
		workStack.top() = tag == digAtMost
			? pEM->MaxCardinality ( n, R, C )
			: pEM->MinCardinality ( n, R, C );
		return;
	}
	case digAnd:
	case digOr:
	case digISet:
		pEM->newArgList();

		if ( workStack.empty() )
			throwCorruptedStack(tag);

		// cycle until we find begin marker
		while ( workStack.top() != NULL  )
		{
			pEM->addArg(workStack.top());
			workStack.pop();

			if ( workStack.empty() )
				throwCorruptedStack(tag);
		}

		// replace marker
		workStack.top() = tag == digAnd ? pEM->And() : tag == digOr ? pEM->Or() : pEM->OneOf();
		return;

	// data staff
	case digIVal:
		workStack.push(tryIntDataValue(data));
		useData = false;	// stop saving data
		return;

	case digSVal:
		workStack.push(tryStrDataValue(data));
		useData = false;	// stop saving data
		return;

	// general data role: domain of an attribute
	case digDefined:
		if ( workStack.empty() )
			throwArgumentAbsence ( "data property", tag );
		workStack.top() = pEM->Exists ( dynamic_cast<TDRoleExpr*>(workStack.top()), pEM->DataTop() );
		return;

	// min/max data restrictions
	case digStrMin:
	case digStrMax:
	case digIntMin:	// >= n
	case digIntMax:	// <= n
	{
		// get property
		if ( workStack.empty() )
			throwArgumentAbsence ( "role", "data expression", tag );
		TDRoleExpr* A = dynamic_cast<TDRoleExpr*>(workStack.top());
		// get initial type
		TDataTypeExpr* type = ( tag == digIntMin || tag == digIntMax )
			? pEM->getIntDataType()
			: pEM->getStrDataType();
		// get the facet value
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "role", "data expression", tag );
		TDataValueExpr* value = dynamic_cast<TDataValueExpr*>(workStack.top());
		workStack.pop();
		// build facet
		TFacetExpr* facet = ( tag == digStrMin || tag == digIntMin )
			? pEM->FacetMinInclusive(value)
			: pEM->FacetMaxInclusive(value);
		// update type
		type = pEM->RestrictedType ( type, facet );
		// build a restriction
		workStack.push( pEM->Exists ( A, type ) );
		return;
	}

	// range data restrictions
	case digStrRange:
	case digIntRange:
	{
		// get property
		if ( workStack.empty() )
			throwArgumentAbsence ( "role", "two data expressions", tag );
		TDRoleExpr* A = dynamic_cast<TDRoleExpr*>(workStack.top());
		workStack.pop();
		// get initial type
		TDataTypeExpr* type = ( tag == digIntRange )
			? pEM->getIntDataType()
			: pEM->getStrDataType();

		// add max facet
		if ( workStack.empty() )
			throwArgumentAbsence ( "role", "two data expressions", tag );
		TFacetExpr* facet = pEM->FacetMaxInclusive(dynamic_cast<TDataValueExpr*>(workStack.top()));
		workStack.pop();
		type = pEM->RestrictedType ( type, facet );

		// add min facet
		if ( workStack.empty() )
			throwArgumentAbsence ( "role", "two data expressions", tag );
		facet = pEM->FacetMinInclusive(dynamic_cast<TDataValueExpr*>(workStack.top()));
		workStack.pop();
		type = pEM->RestrictedType ( type, facet );

		// build a restriction
		workStack.push( pEM->Exists ( A, type ) );
		return;
	}

	// =c data restrictions
	case digIntEquals:	// == n
	case digStrEquals:	// == s
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "role", "data expression", tag );
		TDRoleExpr* A = dynamic_cast<TDRoleExpr*>(workStack.top());
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "role", "data expression", tag );
		TDataValueExpr* value = dynamic_cast<TDataValueExpr*>(workStack.top());
		workStack.pop();
		workStack.push ( pEM->Value ( A, value ) );

		return;
	}

	// unsupported stuff
	case digChain:
		// throw (Unsupported)
		return;

	default:
		fpp_unreachable();		// safety check
	}
}

/// end of axiom (implies, defconcept)
void DIGParseHandlers :: endAxiom ( DIGTag tag )
{
	fpp_assert ( tag >= dig_Axioms_Begin && tag < dig_Axioms_End );	// safety check

	// macro for error printing
#ifdef RKG_PRINT_DIG_MESSAGES
#	define ERROR_IF(action)\
	try {action;} catch(...) { cerr << "ERROR encountered in " << NamesManager.getName(tag).c_str() << " command\n"; }
#else
#	define ERROR_IF(action)\
	try {action;} catch(...) {/*put error message here*/}
#endif

	switch (tag)
	{
	case digDefConcept:	// nothing to do -- all was done during startAxiom
	case digDefRole:
	case digDefFeature:
	case digDefAttribute:
	case digDefIndividual:
		return;

	case digImpliesC:
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "two concept expressions", tag );
		TConceptExpr* C2 = dynamic_cast<TConceptExpr*>(workStack.top());
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "two concept expressions", tag );
		TConceptExpr* C1 = dynamic_cast<TConceptExpr*>(workStack.top());
		workStack.pop();
		ERROR_IF ( pKernel->impliesConcepts ( C1, C2 ) );
		return;
	}
	case digEqualC:
		if ( workStack.empty() )
			throwArgumentAbsence ( "two concept expressions", tag );
		pEM->newArgList();
		pEM->addArg(workStack.top());
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "two concept expressions", tag );
		pEM->addArg(workStack.top());
		workStack.pop();
		ERROR_IF ( pKernel->equalConcepts() );
		return;
	case digDisjointAxiom:
		pEM->newArgList();

		if ( workStack.empty() )
			throwCorruptedStack(tag);

		// cycle until we find begin marker
		while ( workStack.top() != NULL  )
		{
			pEM->addArg(workStack.top());
			workStack.pop();

			if ( workStack.empty() )
				throwCorruptedStack(tag);
		}

		//FIXME!! try..catch block here
		ERROR_IF ( pKernel->disjointConcepts() );
		return;
	case digImpliesR:
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "two role expressions", tag );
		TExpr* R2 = workStack.top();
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "two role expressions", tag );
		TExpr* R1 = workStack.top();
		workStack.pop();
		ERROR_IF ( isDataRole(R1)
			? pKernel->impliesDRoles ( dynamic_cast<TDRoleExpr*>(R1), dynamic_cast<TDRoleExpr*>(R2) )
			: pKernel->impliesORoles ( dynamic_cast<TORoleExpr*>(R1), dynamic_cast<TORoleExpr*>(R2) ) );
		return;
	}
	case digEqualR:
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "two role expressions", tag );
		bool data = isDataRole(workStack.top());
		pEM->newArgList();
		pEM->addArg(workStack.top());
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "two role expressions", tag );
		pEM->addArg(workStack.top());
		workStack.pop();
		ERROR_IF ( data ? pKernel->equalDRoles() : pKernel->equalORoles() );
		return;
	}
	case digDomain:
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "role", "concept", tag );
		TConceptExpr* C = dynamic_cast<TConceptExpr*>(workStack.top());
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "role", "concept", tag );
		TExpr* R = workStack.top();
		workStack.pop();
		ERROR_IF ( isDataRole(R)
			? pKernel->setDDomain ( dynamic_cast<TDRoleExpr*>(R), C )
			: pKernel->setODomain ( dynamic_cast<TORoleExpr*>(R), C ) );
		return;
	}
	case digRange:
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "role", "concept", tag );
		TConceptExpr* C = dynamic_cast<TConceptExpr*>(workStack.top());
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "role", "concept", tag );
		TORoleExpr* R = dynamic_cast<TORoleExpr*>(workStack.top());
		workStack.pop();
		ERROR_IF ( pKernel->setORange ( R, C ) );
		return;
	}
	case digRangeInt:
	case digRangeStr:
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "data property", tag );
		TDRoleExpr* R = dynamic_cast<TDRoleExpr*>(workStack.top());
		workStack.pop();
		ERROR_IF ( pKernel->setDRange ( R, tag == digRangeInt ? pEM->getIntDataType() : pEM->getStrDataType() ) );
		return;
	}
	case digTransitive:
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "role", tag );
		TORoleExpr* R = dynamic_cast<TORoleExpr*>(workStack.top());
		workStack.pop();
		ERROR_IF ( pKernel->setTransitive(R) );
		return;
	}
	case digFunctional:
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "role", tag );
		TRoleExpr* R = dynamic_cast<TRoleExpr*>(workStack.top());
		workStack.pop();
		ERROR_IF ( isDataRole(R)
			? pKernel->setDFunctional(dynamic_cast<TDRoleExpr*>(R))
			: pKernel->setOFunctional(dynamic_cast<TORoleExpr*>(R)) );
		return;
	}
	case digInstanceOf:
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "individual", "concept", tag );
		TConceptExpr* C = dynamic_cast<TConceptExpr*>(workStack.top());
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "individual", "concept", tag );
		TIndividualExpr* I = dynamic_cast<TIndividualExpr*>(workStack.top());
		workStack.pop();
		ERROR_IF ( pKernel->instanceOf ( I, C ) );
		return;
	}
	case digRelated:
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "two individuals", "role", tag );
		TIndividualExpr* I2 = dynamic_cast<TIndividualExpr*>(workStack.top());
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "two individuals", "role", tag );
		TORoleExpr* R = dynamic_cast<TORoleExpr*>(workStack.top());
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "two individuals", "role", tag );
		TIndividualExpr* I1 = dynamic_cast<TIndividualExpr*>(workStack.top());
		workStack.pop();
		ERROR_IF ( pKernel->relatedTo ( I1, R, I2 ) );
		return;
	}
	case digValue:
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "individual, data property", "data value", tag );
		TDataValueExpr* V = dynamic_cast<TDataValueExpr*>(workStack.top());
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "individual, data property", "data value", tag );
		TDRoleExpr* A = dynamic_cast<TDRoleExpr*>(workStack.top());
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "individual, data property", "data value", tag );
		TIndividualExpr* I = dynamic_cast<TIndividualExpr*>(workStack.top());
		workStack.pop();
		ERROR_IF ( pKernel->valueOf ( I, A, V ) );
		return;
	}
	default:
		fpp_unreachable();	// safety check
	}
#undef ERROR_IF
}
