/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2006 by Dmitry Tsarkov

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

#include "DIGinterface.h"

#include <sstream>

// XML parsing part
#include "xercesc/util/XMLException.hpp"
#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/parsers/SAXParser.hpp"
#include "xercesc/framework/MemBufInputSource.hpp"

#include "strx.h"
#include "DIGParserHandlers.h"

// Local define for saving all XML strings
//#define __TO_SAVE_ALL_XML

using namespace std;
using namespace xercesc;

const char* const xmlBufId = "FaCT++";

#ifdef __TO_SAVE_ALL_XML
#	include <fstream>
	static ofstream outXMLstream ("DIG.trace");
#endif

DIGInterface :: DIGInterface ( void ) : xmlParser (NULL), digHandler()
{
	if ( initXMLinterface () )
		return;
	if ( initDIGparser () )
		return;
#ifdef __TO_SAVE_ALL_XML
	if ( !outXMLstream.good() )
		cerr << "Could not open trace file\n";
#endif
}

DIGInterface :: ~DIGInterface ( void )
{
	delete xmlParser;	// do this before XML termination

	// And call the termination method
	XMLPlatformUtils::Terminate();
}

bool DIGInterface :: initXMLinterface ( void )
{
	// Initialize the XML4C2 system
	try
	{
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException& toCatch)
	{
		cerr << "Error during initialization! Message:\n"
			 << StrX(toCatch.getMessage()) << endl;
		return true;	// error
	}

	// initialisation OK
	return false;
}

bool DIGInterface :: initDIGparser ( void )
{
	//
	//  Create a SAX parser object. Then set some options
	//
	xmlParser = new SAXParser;
	//parser->setValidationScheme(valScheme);
	//parser->setDoNamespaces(doNamespaces);
	//parser->setDoSchema(doSchema);
	//parser->setValidationSchemaFullChecking(schemaFullChecking);

	//
	//  Create our SAX handler object and install it on the parser, as the
	//  document and error handlers.
	//
	xmlParser->setDocumentHandler(&digHandler);
	xmlParser->setErrorHandler(&digHandler);

	return false;
}

void DIGInterface :: timedParse ( const char* input )
{
	try
	{
		MemBufInputSource buffer ( (const XMLByte*)input, strlen(input), xmlBufId, false );
		const unsigned long startMillis = XMLPlatformUtils::getCurrentMillis();
		xmlParser->parse(buffer);
		const unsigned long duration = XMLPlatformUtils::getCurrentMillis() - startMillis;
#	ifdef RKG_PRINT_DIG_MESSAGES
		cerr << "Time of operation: " << duration << " milliseconds\n" << endl;
#	endif
	}
	catch (const XMLException& e)
	{
		throw ( DIGParserException ( 102, "Malformed Request", StrX(e.getMessage()).localForm() ) );
	}
}

void DIGInterface :: processQuery ( const char* inp, std::string& res )
{
#ifdef __TO_SAVE_ALL_XML
	outXMLstream << "\nIn:  " << inp << endl;
#endif

	//
	//  Get the starting time and kick off the parse of the indicated
	//  file. Catch any exceptions that might propogate out of it.
	//
	try
	{
		timedParse(inp);
	}
	catch (const DIGParserException& e)
	{
		// stream for exceptional situations parser could not handle by itself
		std::ostringstream s;

		// print standart XML header
		s << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
		// create response
		complexXMLEntry* resp = new complexXMLEntry ( "responses", s );

		// define xmlns
		s << "\n  xmlns=\"http://dl.kr.org/dig/2003/02/lang\"";
		// define xmlns:xsi
		s << "\n  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"";
		// define xsi:schemaLocation
		s << "\n  xsi:schemaLocation=\"http://dl.kr.org/dig/2003/02/lang\n  http://dl-web.man.ac.uk/dig/2003/02/dig.xsd\"";

		resp->closeStart();

		// output error
		outError ( s, e );
		// close response
		delete resp;
		res = s.str ();
		return;
	}

	// everything OK -- create responce
	digHandler.getResult ( res );

#ifdef __TO_SAVE_ALL_XML
	outXMLstream << "\nOut: " << res.c_str() << endl;
#endif
}
