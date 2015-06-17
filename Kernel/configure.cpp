/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2015 by Dmitry Tsarkov

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

#include "configure.h"

#include <fstream>

#undef USE_DEBUG

Configuration :: ~Configuration ( void )
{
	for ( ConfSectBase::iterator i = Base.begin (); i != Base.end (); ++i )
		delete *i;
}

ConfSection :: ~ConfSection ( void )
{
	for ( ConfBase::iterator i = Base.begin (); i != Base.end (); ++i )
		delete *i;
}

ConfSection* Configuration :: FindSection ( const std::string& pc ) const
{
	for ( ConfSectBase::const_iterator i = Base.begin (); i != Base.end (); ++i )
		if ( **i == pc )
			return *i;

	// can not find section
	return (ConfSection*) NULL;
}

ConfElem* ConfSection :: FindByName ( const std::string& name ) const
{
	for ( ConfBase::const_iterator i = Base.begin (); i != Base.end (); ++i )
		if ( (*i)->Name == name )
			return *i;

	// can not find element in section
	return (ConfElem*) NULL;
}


// add section; set Section to new pointer. Ret 1 if couldn't.
void Configuration :: createSection ( const std::string& name )
{
	// if section already exists -- nothing to do
	if ( !useSection ( name ) )
		return;

	Section = new ConfSection ( name );
	Base.push_back(Section);
	isSaved = false;
}

// add Field.value to current Section; sets Element to new p.
bool Configuration :: setValue ( const std::string& Field, const std::string& Value )
{
	if ( !Section )
		return true;

	//changing things
	isSaved = false;

	// check for existing field
	if ( (Element = Section->FindByName(Field)) )
	{
		Element->Value = Value;
		return false;
	}
	else
	{
		Section -> addEntry ( Field, Value );
		return !(Element = Section->FindByName(Field));
	}
}

// check if Field exists if Section is set;
bool Configuration :: checkValue ( const std::string& Field )
{
	if ( !Section )
		return true;

	Element = Section->FindByName(Field);
	return Element == NULL;
}

// check if Section:Field exists;
bool Configuration :: checkValue ( const std::string& Sect, const std::string& Field )
{
	if ( useSection(Sect) )
		return true;

	Element = Section->FindByName(Field);
	return Element == NULL;
}

// Manipulation part
void ConfSection :: addEntry ( const std::string& name, const std::string& value )
{
	#ifdef USE_DEBUG
		cerr << "\nadd pair \'" << name.c_str() << "\',\'" << value.c_str() << "\' to section \'" << Name.c_str() << "\'";
	#endif
	Base.push_back ( new ConfElem ( name, value ) );
}

// Load part
int Configuration :: SplitLine ( char*& pName, char*& pValue )
{
	char* p = Line;

	while ( *p && isspace (*p) ) ++p;	// skip leading spaces
	pName = p;
	// skip the property name
	for ( ; *p && *p != '='; ++p ) (void)NULL;
	if (!*p) return 1;

	// we found '='
	pValue = p+1;	// the next char after '='
	// skip last spaces
	for ( *p=0, --p; p!=Line && isspace (*p); --p ) *p=0;
	if ( p == Line && isspace (*p) ) return 2;

	// here we have name
	for ( p=pValue; *p && isspace (*p); ++p ) (void)NULL; // skip leading spaces
	if (!*p) return 3;
	pValue = p;

	// skip last spaces
	for ( p=pValue+strlen(pValue)-1; isspace (*p) && p!=pValue; --p ) *p=0;
	if ( p == pValue && isspace (*p) ) return 4;

	#ifdef USE_DEBUG
		cerr << "\nfound string \'" << pName << "\'=\'" << pValue << "\'";
	#endif
	// all right!
	return 0;
}

void Configuration :: loadString ( std::istream& i )
{
	do
		i.getline ( Line, MaxConfLineLen );
	while ( i && isComment () );

	#ifdef USE_DEBUG
		cerr << "\nload string \'" << Line << "\'";
	#endif
}

bool Configuration :: isComment ( void ) const
{
	size_t n = strlen (Line);

	if ( n == 0 )
		return true;

	if ( Line [0] == ';' || Line [0] == '#' || ( Line [0] == '/' && Line [1] == '/' ) )
		return true;

	for ( size_t i = 0; i < n; i++ )
		if ( !isspace (Line [i]) )
			return false;

	return true;
}

bool Configuration :: Load ( const char* Filename )
{
	std::ifstream in ( Filename );
	char *pName, *pValue;

	isLoaded = false;
	if ( !in ) return true;

	loadString (in);
	while ( !in.eof () )
	{
		if ( isSection () )
			loadSection ();
		else
			return true;

		do
		{
			loadString (in);
			if ( in.eof () )
				break;

			if ( isSection () )
				break;

			if ( SplitLine ( pName, pValue ) )
				return true;

			if ( setValue ( pName, pValue ) )
				return true;
		} while ( !in.eof () );
	}

	isLoaded = isSaved = true;
	fileName = Filename;
	return false;
}

void Configuration :: loadSection ( void )
{
	Line [strlen(Line)-1] = (char) 0;	// kill ']' of section
	#ifdef USE_DEBUG
		cerr << "\nfound section \'" << Line+1 << "\'";
	#endif
	createSection ( Line+1 );	// skip '['
}

// Save part
void ConfElem :: Save ( std::ostream& o ) const
{
	o << ' ' << Name.c_str () << " = " << Value.c_str () << std::endl;
}

void ConfSection :: Save ( std::ostream& o ) const
{
	o << "[" << Name.c_str () << "]\n";

	for ( ConfBase::const_iterator i = Base.begin (); i != Base.end (); ++i )
		(*i)->Save (o);

	o << std::endl;
}

bool Configuration :: Save ( const char* Filename )
{
	std::ofstream o ( Filename );
	if ( o.bad () )
		return true;

	for ( ConfSectBase::iterator i = Base.begin (); i != Base.end (); ++i )
		(*i)->Save (o);

	isLoaded = isSaved = true;
	return false;
}
