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
	DLTree* x;

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
		x = tryRoleName(name);
		try
		{
			pKernel->setOFunctional(x);
		}
		catch(...)
		{
			wasError = true;
			string reason ( "Feature '" );
			reason += name;
			reason += "' undefined";
			outWarning ( 99, "Undefined name", reason.c_str() );
		}
		workStack.push(x);
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
		workStack.push ( new DLTree ( TLexeme (NUM,n) ) );
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
		x = pKernel->getDataTypeCenter().getNumberType();
		if ( parm_val == NULL )
			throwAttributeAbsence ( "val", tag );
		workStack.push(tryDataValue(val,x));
		return;

	case digIntRange:
		x = pKernel->getDataTypeCenter().getNumberType();
		if ( parm_min == NULL )
			throwAttributeAbsence ( "min", tag );
		if ( parm_max == NULL )
			throwAttributeAbsence ( "max", tag );
		workStack.push(tryDataValue(min,x));
		workStack.push(tryDataValue(max,x));	// max is on top!
		return;

	// string datatype constructors
	case digStrMin:
	case digStrMax:
	case digStrEquals:
		x = pKernel->getDataTypeCenter().getStringType();
		if ( parm_val == NULL )
			throwAttributeAbsence ( "val", tag );
		workStack.push(tryDataValue(val,x));
		return;

	case digStrRange:
		x = pKernel->getDataTypeCenter().getStringType();
		if ( parm_min == NULL )
			throwAttributeAbsence ( "min", tag );
		if ( parm_max == NULL )
			throwAttributeAbsence ( "max", tag );
		workStack.push(tryDataValue(min,x));
		workStack.push(tryDataValue(max,x));	// max is on top!
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
	DLTree* x;

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
		pKernel->declare(x = tryRoleName(StrX(parm).localForm()));
		ERROR_IF ( pKernel->setTransitive(x) )
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
		workStack.push ( new DLTree (DISJOINT) );
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
		workStack.top() = pEM->Not(workStack.top());
		return;

	case digInverse:
		if ( workStack.empty() )
			throwArgumentAbsence ( "role", tag );
		workStack.top() = pEM->Inverse(workStack.top());
		return;

	case digSome:
	case digAll:	// top := \? R.C [now top = ... R C]
	{
		Token op = (tag == digSome) ? EXISTS : FORALL;
		if ( workStack.empty() )
			throwArgumentAbsence ( "concept", "role", tag );
		DLTree* C = workStack.top();	// C
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "concept", "role", tag );
		workStack.top() = pEM->ComplexExpression ( op, 0, workStack.top(), C );
		return;
	}

	case digAtMost:
	case digAtLeast:	// top := \?e n R.C [now top = ... R C]
	{
		Token op = (tag == digAtMost) ? LE : GE;

		if ( workStack.empty() )
			throwArgumentAbsence ( "concept", "role", tag );
		DLTree* C = workStack.top();	// C
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "concept", "role", tag );
		DLTree* R = workStack.top();	// R
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "number", tag );
		DLTree* pN = workStack.top();	// n
		workStack.pop();
		fpp_assert ( pN->Element() == NUM );
		unsigned int n = pN->Element().getData();
		deleteTree(pN);

		// create \?e n R.C
		workStack.push ( pEM->ComplexExpression ( op, n, R, C ) );
		return;
	}
	case digAnd:
	case digOr:
	case digISet:

		pEM->openArgList();

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
		workStack.push(tryDataValue(data,pKernel->getDataTypeCenter().getNumberType()));
		useData = false;	// stop saving data
		return;

	case digSVal:
		workStack.push(tryDataValue(data,pKernel->getDataTypeCenter().getStringType()));
		useData = false;	// stop saving data
		return;

	// general data role: domain of an attribute
	case digDefined:
		if ( workStack.empty() )
			throwArgumentAbsence ( "data property", tag );
		workStack.top() = pEM->Exists ( workStack.top(), pEM->Top() );
		return;

	// min/max data restrictions
	case digStrMin:
	case digStrMax:
	case digIntMin:	// >= n
	case digIntMax:	// <= n
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "role", "data expression", tag );
		DLTree* R = workStack.top();
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "role", "data expression", tag );
		DLTree* N = workStack.top();
		// FIXME: throw exception
		fpp_assert ( N->Element().getToken() == DATAEXPR );
		workStack.pop();
		DLTree* expr = pKernel->getDataTypeCenter().getDataExpr(N);

		if ( tag==digIntMin||tag==digStrMin )
			expr = pKernel->getDataTypeCenter().applyFacet ( expr,
				pKernel->getDataTypeCenter().getMinInclusiveFacet(N) );
		else
			expr = pKernel->getDataTypeCenter().applyFacet ( expr,
				pKernel->getDataTypeCenter().getMaxInclusiveFacet(N) );

		workStack.push( pEM->Exists ( R, expr ) );
		return;
	}

	// range data restrictions
	case digStrRange:
	case digIntRange:
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "role", "two data expressions", tag );
		DLTree* R = workStack.top();
		workStack.pop();

		if ( workStack.empty() )
			throwArgumentAbsence ( "role", "two data expressions", tag );
		DLTree* max = workStack.top();
		workStack.pop();
		DLTree* expr = pKernel->getDataTypeCenter().getDataExpr(max);
		expr = pKernel->getDataTypeCenter().applyFacet ( expr,
			pKernel->getDataTypeCenter().getMaxInclusiveFacet(max) );

		if ( workStack.empty() )
			throwArgumentAbsence ( "role", "two data expressions", tag );
		DLTree* min = workStack.top();
		workStack.pop();
		expr = pKernel->getDataTypeCenter().applyFacet ( expr,
			pKernel->getDataTypeCenter().getMinInclusiveFacet(min) );

		workStack.push( pEM->Exists ( R, expr ) );
		return;
	}

	// =c data restrictions
	case digIntEquals:	// == n
	case digStrEquals:	// == s
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "role", "data expression", tag );
		DLTree* A = workStack.top();
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "role", "data expression", tag );
		DLTree* ps = workStack.top();
		// FIXME: throw exception
		fpp_assert ( ps->Element().getToken() == DATAEXPR );
		workStack.pop();
		workStack.push ( pEM->Exists ( A, ps ) );

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
		DLTree* C2 = workStack.top();
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "two concept expressions", tag );
		DLTree* C1 = workStack.top();
		workStack.pop();
		ERROR_IF ( pKernel->impliesConcepts ( C1, C2 ) );
		return;
	}
	case digEqualC:
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "two concept expressions", tag );
		DLTree* C2 = workStack.top();
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "two concept expressions", tag );
		DLTree* C1 = workStack.top();
		workStack.pop();
		ERROR_IF ( pKernel->equalConcepts ( C1, C2 ) );
		return;
	}
	case digDisjointAxiom:
	{
		pKernel->openArgList();

		if ( workStack.empty() )
			throwCorruptedStack(tag);

		// init concept list
		DLTree* cur = workStack.top();
		DLTree* stop = new DLTree(DISJOINT);

		while ( !equalTrees(cur,stop) )
		{
			pKernel->addArg(cur);

			if ( workStack.empty() )
				throwCorruptedStack(tag);

			workStack.pop();
			cur = workStack.top();
		}

		// here cur = DISJOINT
		deleteTree(cur);
		deleteTree(stop);
		workStack.pop();

		//FIXME!! try..catch block here
		ERROR_IF ( pKernel->disjointConcepts() );
		return;
	}
	case digImpliesR:
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "two role expressions", tag );
		DLTree* R2 = workStack.top();
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "two role expressions", tag );
		DLTree* R1 = workStack.top();
		workStack.pop();
		ERROR_IF ( isDataRole(R1) ? pKernel->impliesDRoles ( R1, R2 ) : pKernel->impliesORoles ( R1, R2 ) );
		return;
	}
	case digEqualR:
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "two role expressions", tag );
		bool data = isDataRole(workStack.top());
		pEM->openArgList();
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
		DLTree* C = workStack.top();
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "role", "concept", tag );
		DLTree* R = workStack.top();
		workStack.pop();
		ERROR_IF ( isDataRole(R) ? pKernel->setDDomain ( R, C ) : pKernel->setODomain ( R, C ) );
		return;
	}
	case digRange:
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "role", "concept", tag );
		DLTree* C = workStack.top();
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "role", "concept", tag );
		DLTree* R = workStack.top();
		workStack.pop();
		ERROR_IF ( isDataRole(R) ? pKernel->setDRange ( R, C ) : pKernel->setORange ( R, C ) );
		return;
	}
	case digRangeInt:
	case digRangeStr:
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "data property", tag );
		DLTree* R = workStack.top();
		workStack.pop();
		ERROR_IF ( pKernel->setDRange ( R, tag == digRangeInt ?
											pKernel->getDataTypeCenter().getNumberType() :
											pKernel->getDataTypeCenter().getStringType() ) );
		return;
	}
	case digTransitive:
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "role", tag );
		DLTree* R = workStack.top();
		workStack.pop();
		ERROR_IF ( pKernel->setTransitive(R) );
		return;
	}
	case digFunctional:
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "role", tag );
		DLTree* R = workStack.top();
		workStack.pop();
		ERROR_IF ( isDataRole(R) ? pKernel->setDFunctional(R) : pKernel->setOFunctional(R) );
		return;
	}
	case digInstanceOf:
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "individual", "concept", tag );
		DLTree* C = workStack.top();
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "individual", "concept", tag );
		DLTree* I = workStack.top();
		workStack.pop();
		ERROR_IF ( pKernel->instanceOf ( I, C ) );
		return;
	}
	case digRelated:
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "two individuals", "role", tag );
		DLTree* I2 = workStack.top();
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "two individuals", "role", tag );
		DLTree* R = workStack.top();
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "two individuals", "role", tag );
		DLTree* I1 = workStack.top();
		workStack.pop();
		ERROR_IF ( pKernel->relatedTo ( I1, R, I2 ) );
		return;
	}
	case digValue:
	{
		if ( workStack.empty() )
			throwArgumentAbsence ( "individual, data property", "data value", tag );
		DLTree* V = workStack.top();
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "individual, data property", "data value", tag );
		DLTree* A = workStack.top();
		workStack.pop();
		if ( workStack.empty() )
			throwArgumentAbsence ( "individual, data property", "data value", tag );
		DLTree* I = workStack.top();
		workStack.pop();
		ERROR_IF ( pKernel->valueOf ( I, A, V ) );
		return;
	}
	default:
		fpp_unreachable();	// safety check
	}
#undef ERROR_IF
}
