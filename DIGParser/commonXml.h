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

#ifndef __COMMONXML_H
#define __COMMONXML_H

#include <string>
#include <iostream>

using namespace std;

/// interface for printing any XML entry
class XMLEntry
{
protected:	// members
	/// output stream
	ostream& o;

public:		// interface
	/// standart c'tor -- necessary to use closeStart()
	XMLEntry ( const char* _name, ostream& oo ) : o(oo) { o << "<" << _name; }
	/// extended c'tor -- prints feature and closes start of entry
	XMLEntry ( const char* _name, ostream& oo, const char* feature ) : o(oo)
	{
		o << "<" << _name << " " << feature;
		closeStart();
	}
	/// virtual d'tor (for compatibility); closes given XML entry
	virtual ~XMLEntry ( void ) {}

	/// close the beginning of entry
	virtual void closeStart ( void ) {}
}; // XMLEntry

/// class for printing simple (<ok/>) XML entries
class simpleXMLEntry : public XMLEntry
{
public:	// interface
	/// standart c'tor
	simpleXMLEntry ( const char* _name, ostream& oo ) : XMLEntry (_name,oo) {}
	/// extended c'tor
	simpleXMLEntry ( const char* _name, ostream& oo, const char* feature ) : XMLEntry (_name,oo,feature) {}
	/// virtual d'tor; closes given XML entry -- print closing bracket
	virtual ~simpleXMLEntry ( void ) { o << "/>"; }

	/// close the beginning of entry -- do nothing
	virtual void closeStart ( void ) {}
}; // simpleXMLEntry

/// class for printing complex (<aaa>...</aaa>) XML entries
class complexXMLEntry : public XMLEntry
{
protected:	// members
	/// save name of the entry
	std::string name;

public:
	/// standart c'tor
	complexXMLEntry ( const char* _name, ostream& oo ) : XMLEntry (_name,oo), name (_name) {}
	/// extended c'tor
	complexXMLEntry ( const char* _name, ostream& oo, const char* feature ) : XMLEntry (_name,oo,feature), name (_name) {}
	/// virtual d'tor; closes given XML entry
	virtual ~complexXMLEntry ( void ) { o << "\n</" << name.c_str() << ">"; }

	/// close the beginning of entry -- print closing bracket
	virtual void closeStart ( void ) { o << ">"; }
}; // complexXMLEntry

/// class for printing complex (<aaa>...</aaa>) XML entries with only closed headers
class closedXMLEntry : public complexXMLEntry
{
public:
	/// standart c'tor
	closedXMLEntry ( const char* _name, ostream& oo ) : complexXMLEntry (_name,oo) { closeStart(); }
	/// extended c'tor
	closedXMLEntry ( const char* _name, ostream& oo, const char* feature ) : complexXMLEntry (_name,oo,feature) { closeStart(); }
	/// virtual d'tor; closes given XML entry
	virtual ~closedXMLEntry ( void ) {}
}; // closedXMLEntry

#endif
