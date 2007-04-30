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

#ifndef _DIGINTERFACE_H
#define _DIGINTERFACE_H

#include <string>

#include "xercesc/parsers/SAXParser.hpp"

#include "DIGParserHandlers.h"

using namespace xercesc;

class DIGInterface
{
protected:	// members
	/// local parser declaration
	SAXParser* xmlParser;
	/// local parser handlers
	DIGParseHandlers digHandler;

protected:	// methods
	/** init DIG interface. @return true if error occures */
	bool initXMLinterface ( void );
	/** init SAX parser. @return true if error occures */
	bool initDIGparser ( void );

		/// aux method -- parse the whose query at once, transform XML exception to DIG one
	void timedParse ( const char* inpit );

public:
	DIGInterface ( void );
	virtual ~DIGInterface ( void );

	void processQuery ( const char* inp, std::string& res );
}; // DIGInterface

#endif
