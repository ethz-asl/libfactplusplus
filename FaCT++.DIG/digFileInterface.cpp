/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2004 by Dmitry Tsarkov

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

#include "digFileInterface.h"

#include <sstream>

// XML parsing part
#include "xercesc/util/XMLException.hpp"
#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/parsers/SAXParser.hpp"
#include "xercesc/framework/MemBufInputSource.hpp"

#include "strx.h"

using namespace xercesc;

void DIGFileInterface :: processFile ( const char* filename, std::string& res )
{
	//
	//  Get the starting time and kick off the parse of the indicated
	//  file. Catch any exceptions that might propogate out of it.
	//
	int errorCount = 0;
	try
	{
		try
		{
			xmlParser->parse(filename);
			errorCount = xmlParser->getErrorCount();
		}
		catch (const XMLException& e)
		{
			throw ( DIGParserException ( 102, "Malformed Request", StrX(e.getMessage()).localForm() ) );
		}
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
}
