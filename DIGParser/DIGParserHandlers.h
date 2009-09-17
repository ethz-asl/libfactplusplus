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

#ifndef __DIGPARSERHANDLERS_H
#define __DIGPARSERHANDLERS_H

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/sax/HandlerBase.hpp>
#include <ostream>
#include <sstream>
#include <stack>

#include "commonXml.h"
#include "KernelFactory.h"
#include "Kernel.h"
#include "DIGNamesManager.h"

XERCES_CPP_NAMESPACE_USE

XERCES_CPP_NAMESPACE_BEGIN
class AttributeList;
XERCES_CPP_NAMESPACE_END

/// type of DIG query
enum DIGQueryType { dqtGetId, dqtTells, dqtAsks };

/// class containing information about exceptions in DIG interface
class DIGParserException
{
protected:	// members
	unsigned int errorNumber;
	std::string reasonAttr;
	std::string reasonText;

public:		// interface
		/// create c'tor
	DIGParserException ( unsigned int n, const char* attr, const std::string& text )
		: errorNumber(n)
		, reasonAttr(attr)
		, reasonText(text)
		{}
		/// create c'tor
	DIGParserException ( unsigned int n, const char* attr,
			const std::string& text1, const std::string& text2 )
		: errorNumber(n)
		, reasonAttr(attr)
		, reasonText(text1)
	{
		reasonText += text2;
	}
		/// create c'tor
	DIGParserException ( unsigned int n, const char* attr,
			const std::string& text1, const std::string& text2, const std::string& text3 )
		: errorNumber(n)
		, reasonAttr(attr)
		, reasonText(text1)
	{
		reasonText += text2;
		reasonText += text3;
	}
		/// empty d'tor
	~DIGParserException ( void ) {}

	// access methods

		/// access to the number of DIG error
	unsigned int number ( void ) const { return errorNumber; }
		/// access to the reason of exception
	const char* reason ( void ) const { return reasonAttr.c_str(); }
		/// detailed explanation of exception
	const char* note ( void ) const { return reasonText.c_str(); }
}; // DIGParserException

/// output error to the stream
void outError ( std::ostream& o, const DIGParserException& e );

class DIGParseHandlers : public HandlerBase
{
public:
	// -----------------------------------------------------------------------
	//  Constructors and Destructor
	// -----------------------------------------------------------------------
	DIGParseHandlers();
	~DIGParseHandlers();

	// -----------------------------------------------------------------------
	//  Handlers for the SAX DocumentHandler interface
	// -----------------------------------------------------------------------
	void startDocument();
	void endDocument();
	void resetDocument();

	void startElement(const XMLCh* const name, AttributeList& attributes);
	void processingInstruction(const XMLCh* const target, const XMLCh* const data);
	void endElement(const XMLCh* const name);

	void characters(const XMLCh* const chars, const unsigned int length);

	void ignorableWhitespace(const XMLCh* const chars ATTR_UNUSED, const unsigned int length ATTR_UNUSED) {}
	void setDocumentLocator(const Locator* const locator ATTR_UNUSED) {}

	// -----------------------------------------------------------------------
	//  Handlers for the SAX ErrorHandler interface
	// -----------------------------------------------------------------------
	void warning(const SAXParseException& exception);
	void error(const SAXParseException& exception);
	void fatalError(const SAXParseException& exception);

public:
	// -----------------------------------------------------------------------
	//  Getting methods
	// -----------------------------------------------------------------------
	void getResult ( std::string& res ) const { res = o->str(); }

protected:
	/// stream with results of query processing
	std::ostringstream* o;

	/// Type of an answer
	XMLEntry* pEnv;

	/// names manager
	NamesManager_DIG_1x NamesManager;

	/// factory of kernels
	KernelFactory kFactory;

	/// DL reasoner kernel
	ReasoningKernel* pKernel;

	/// internal stack for the parsing operations
	std::stack <DLTree*> workStack;

	/// local id name for current ask
	std::string curId;

		/// value of string/integer data
	std::string data;
		/// flag if it is necessary to save data. Setted up by {i,s}val
	bool useData;
		/// flag if current XML is TELL query
	bool inTell;
		/// flag if current XML is ASK query
	bool inAsk;
		/// flag if it was an error during the query processing
	bool wasError;

protected:	// methods
	// found starting element methods

	/// start of command (getId, tells, asks)
	void startCommand ( DIGTag tag, AttributeList& attributes );

	/// start of concept expression (id, and, or)
	void startConcept ( DIGTag tag, AttributeList& attributes );

	/// start of axiom (implies, defconcept)
	void startAxiom ( DIGTag tag, AttributeList& attributes );

	/// start of ask element (allNames, satisfy)
	void startAsk ( DIGTag tag, AttributeList& attributes );

	// found finishing element methods

	/// end of command (getId, tells, asks)
	void endCommand ( DIGTag tag );

	/// end of concept expression (id, and, or)
	void endConcept ( DIGTag tag );

	/// end of axiom (implies, defconcept)
	void endAxiom ( DIGTag tag );

	/// end of ask element (allNames, satisfy)
	void endAsk ( DIGTag tag );

	// auxiliary methods

		/// throw "Malformed XML" exception in for absent attribute
	void throwAttributeAbsence ( const char* attrName, DIGTag entry )
	{
		std::string acc("Attribute '");
		acc += attrName;
		acc += "' expected in '";
		throw DIGParserException ( 102, "Malformed Request", acc,
								   NamesManager.getName(entry), "' entry" );
	}
		/// throw "Malformed XML" exception in for absent argument
	void throwArgumentAbsence ( const char* argName, DIGTag entry )
	{
		std::string acc(argName);
		acc += " expected in '";
		throw DIGParserException ( 102, "Malformed Request", acc,
								   NamesManager.getName(entry), "' entry" );
	}
		/// throw "Malformed XML" exception in for absent arguments
	void throwArgumentAbsence ( const char* argName1, const char* argName2, DIGTag entry )
	{
		std::string acc(argName1);
		acc += " and ";
		acc += argName2;
		acc += " are expected in '";
		throw DIGParserException ( 102, "Malformed Request", acc,
								   NamesManager.getName(entry), "' entry" );
	}
		/// throw "corrupted stack" exception in for multi-argument operations processing
	void throwCorruptedStack ( DIGTag entry )
	{
		throw DIGParserException ( 900, "internal Reasoning Error",
			"Corrupted stack during '", NamesManager.getName(entry), "' processing" );
	}

		/// transform given name to tag
	DIGTag getTag ( const std::string& name )
	{
		DIGTag ret = NamesManager.getTag(name);
		if ( ret == digDisjoint )
		{
			if ( inTell )
				return digDisjointAxiom;
			if ( inAsk )
				return digDisjointQuery;
		}
		return ret;
	}

/// macro for the ensure* methods
#define CatchNameEx(action)				\
		do {							\
			try { x = action; }			\
			catch (EFPPCantRegName ex)	\
			{ throw DIGParserException ( 99, "Undefined name", ex.what() ); }	\
		} while(0)

		/// return concept by given name; throw exception if unable
	DLTree* tryConceptName ( const std::string& name )
	{
		DLTree* x = NULL;
		CatchNameEx(pKernel->ensureConceptName(name));
		return x;
	}
		/// return individual by given name; throw exception if unable
	DLTree* tryIndividualName ( const std::string& name )
	{
		DLTree* x = NULL;
		CatchNameEx(pKernel->ensureSingletonName(name));
		return x;
	}
		/// return role by given name; throw exception if unable
	DLTree* tryRoleName ( const std::string& name )
	{
		DLTree* x = NULL;
		CatchNameEx(pKernel->ensureObjectRoleName(name));
		return x;
	}
		/// return role by given name; throw exception if unable
	DLTree* tryDataRoleName ( const std::string& name )
	{
		DLTree* x = NULL;
		CatchNameEx(pKernel->ensureDataRoleName(name));
		try
		{
			pKernel->setFunctional(clone(x));	// in DIG 1.1 data roles are always functional
		}
		catch(...)
		{
			throw DIGParserException ( 99, "Internal Error",
				"Unable to set functional attribute to data role '", name, "'" );
		}
		return x;
	}
		/// return data value of a type TYPE by given name; throw exception if unable
	DLTree* tryDataValue ( const std::string& name, DLTree* type )
	{
		DLTree* x = NULL;
		CatchNameEx(pKernel->getDataTypeCenter().getDataValue(name,type));
		return x;
	}

#undef CatchNameEx

		/// output supported DIG fragment to local stream
	void outputSupportedLanguage ( void );

		/// classify current working KB
	void classifyCurrentKB ( void );

		/// output error to local stream
	void outError ( unsigned int Number, const char* Reason, const char* Note );
		/// output warning to local stream
	void outWarning ( unsigned int Number, const char* Reason, const char* Note );
}; // DIGParseHandlers

#endif
