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

ConfSection* Configuration :: FindSection ( const std::string& pc )
{
	for ( ConfSectBase::iterator i = Base.begin (); i != Base.end (); ++i )
		if ( **i == pc )
			return *i;

	// can not find section
	return (ConfSection*) NULL;
}

ConfElem* ConfSection :: FindByName ( const std::string& name )
{
	for ( ConfBase::iterator i = Base.begin (); i != Base.end (); ++i )
		if ( (*i)->Name == name )
			return *i;

	// can not find element in section
	return (ConfElem*) NULL;
}

bool Configuration :: useSection ( const std::string& name )
{
	Current = FindSection ( name );
	return !Current;
}

// add section; set Current to new pointer. Ret 1 if couldn't.
bool Configuration :: createSection ( const std::string& name )
{
	// if section already exists -- nothing to do
	if ( !useSection ( name ) )
		return false;

	Current = new ConfSection ( name );
	if ( Current )
	{
		Base.push_back ( Current );
		isSave = false;
		return false;
	}
	else
		return true;
}

// add Field.value to current Section; sets Trying to new p.
bool Configuration :: setValue ( const std::string& Field, const std::string& Value )
{
	if ( !Current )
		return true;

	// check for existing field
	if ( (Trying = Current->FindByName(Field)) )
	{
		Trying->Value = Value;
		isSave = false;
		return false;
	}
	else
	{
		Current -> addEntry ( Field, Value );
		isSave = false;
		return !(Trying = Current->FindByName(Field));
	}
}

// check if Field exists if Current is set;
bool Configuration :: checkValue ( const std::string& Field )
{
	if ( !Current )
		return true;

	if ( (Trying = Current->FindByName(Field)) == NULL )
		return true;

	return false;
}

// check if Section:Field exists;
bool Configuration :: checkValue ( const std::string& Section, const std::string& Field )
{
	if ( useSection (Section) )
		return true;

	if ( (Trying = Current->FindByName(Field)) == NULL )
		return true;

	return false;
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
	register char* p = Line;

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

int Configuration :: Load ( const char* Filename )
{
	std::ifstream in ( Filename );
	char *pName, *pValue;

	isLoad = false;
	if ( !in ) return 1;

	loadString (in);
	while ( !in.eof () )
	{
		if ( isSection () )
			loadSection ();
		else
			return 2;

		do
		{
			loadString (in);
			if ( in.eof () )
				break;

			if ( isSection () )
				break;

			if ( SplitLine ( pName, pValue ) )
				return 3;

			if ( setValue ( pName, pValue ) )
				return 4;
		} while ( !in.eof () );
	}

	isLoad = isSave = true;
	fileName = Filename;
	return 0;
}

bool Configuration :: loadSection ( void )
{
	Line [strlen(Line)-1] = (char) 0;	// kill ']' of section
	#ifdef USE_DEBUG
		cerr << "\nfound section \'" << Line+1 << "\'";
	#endif
	return createSection ( Line+1 );	// skip '['
}

// Save part
int ConfElem :: Save ( std::ostream& o ) const
{
	o << ' ' << Name.c_str () << " = " << Value.c_str () << std::endl;
	return o.bad ();
}

int ConfSection :: Save ( std::ostream& o ) const
{
	o << "[" << Name.c_str () << "]\n";

	for ( ConfBase::const_iterator i = Base.begin (); i != Base.end (); ++i )
		(*i)->Save (o);

	o << std::endl;
	return o.bad ();
}

int Configuration :: Save ( const char* Filename )
{
	std::ofstream o ( Filename );
	if ( o.bad () )
		return 1;

	int res = 0;
	for ( ConfSectBase::iterator i = Base.begin (); !res && i != Base.end (); ++i )
		res = (*i)->Save (o);

	res |= o.bad ();

	if ( !res )
		isLoad = isSave = true;
	else
		isSave = false;

	return res;
}
