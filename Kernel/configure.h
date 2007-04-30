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

#ifndef __CONFIGUR_HPP
#define __CONFIGUR_HPP

const unsigned MaxConfLineLen = 1024;

#include <string.h>
#include <stdlib.h>
#include <iostream>

#include <string>
#include <vector>

// configure element
class ConfElem
{
public:
	const std::string Name;
	std::string Value;

	ConfElem () {}
	ConfElem ( const std::string& n, const std::string& v ) : Name (n), Value (v) {}

	int Save ( std::ostream& o ) const;

	// interface
	long GetLong ( void ) const { return atol ( Value.c_str() ); }
	double GetDouble ( void ) const { return atof ( Value.c_str() ); }
	const char* GetString ( void ) const { return Value.c_str(); }
}; // ConfElem

// class for storing configure
class ConfSection
{
protected:
	const std::string Name;

	typedef std::vector <ConfElem*> ConfBase;
	ConfBase Base;

public:
	ConfSection ( const std::string& pc ) : Name ( pc ) {}
	~ConfSection ( void );

	int operator == ( const std::string& pc ) const
		{ return ( Name == pc ); }
	int operator != ( const std::string& pc ) const
		{ return ( Name != pc ); }

	void addEntry ( const std::string& Name, const std::string& Value );

	// find element; return NULL if not found
	ConfElem* FindByName ( const std::string& name );

	int Save ( std::ostream& o ) const;
}; // ConfSection

// class for reading general configuration file
class Configuration
{
protected:	// parsing part
	std::string fileName;	// fileName
	char Line [MaxConfLineLen+1];	// \0
	bool isLoad, isSave;	// flags

	// parser methods
	void loadString ( std::istream& );
	bool isComment ( void ) const;
	int isSection ( void ) const
		{ return ( Line [0] == '[' && Line [strlen(Line)-1] == ']' ); }
	bool loadSection ( void );
	// splits line 'name=value' on 2 ASCIIZ parts
	int SplitLine ( char*& pName, char*& pValue );

protected:	// logic part
	typedef std::vector <ConfSection*> ConfSectBase;
	ConfSectBase Base;
	ConfSection* Current;
	ConfElem* Trying;

	// navigation methods
	ConfSection* FindSection ( const std::string& pc );

public:
	Configuration ( void ) :
		isLoad (false), isSave (false), Current (NULL), Trying (NULL)  {}
	~Configuration ( void );

	// load config from file; -1 if any error
	int Load ( const char* Filename );
	// save config to file; -1 if any error
	int Save ( const char* Filename );
	// save config to the loaden' file; -1 if any error
	int Save ( void ) { return isLoad ? Save(fileName.c_str()) : -1; }

	// status methods
	bool Loaded ( void ) const { return isLoad; }
	bool Saved  ( void ) const { return isSave; }
	// add section; set Current to new pointer. Ret 1 if couldn't.
	bool createSection ( const std::string& name );
	// check if Sect.Field exists;
	bool checkValue ( const std::string& Section, const std::string& Field );
	// check if Field exists if Current is set;
	bool checkValue ( const std::string& Field );
	// add Field.value to current Section; sets Trying to new p.
	bool setValue ( const std::string& Field, const std::string& Value );
	// get Sect.Field value; ret. empty string if error
//	string getValue ( const char* Section, const char* Field ) const;
	// get Field value if Current is set; empty string if error
//	string getValue ( const char* Field ) const;

	// get checked value
	std::string getValue ( void ) const
		{ return Trying -> Value; }
	long getLong ( void ) const
		{ return Trying -> GetLong (); }
	double getDouble ( void ) const
		{ return Trying -> GetDouble (); }
	const char* getString ( void ) const
		{ return Trying -> GetString (); }
	// set Sect as a default
	bool useSection ( const std::string& Section );
}; // Configuration

#endif
