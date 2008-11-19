/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2004 by Dmitry Tsarkov

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

#ifndef __IFOPTIONS_H
#define __IFOPTIONS_H

/********************************************************\
|* Interface for the options management for the FaCT++  *|
\********************************************************/

#include <cassert>
#include <string>
#include <map>
#include <iostream>

class Configuration;

/// class for working with general options with boolean, integer or text values
class ifOption
{
public:		// type interface
		/// type of an option
	enum ioType { iotBool, iotInt, iotText };

private:	// preventing copying (unimplemented)
		/// no empty c'tor
	ifOption ( void );
		/// no copy c'tor
	ifOption ( const ifOption& o );
		/// no assignment
	ifOption& operator = ( const ifOption& o );

protected:	// members
		/// option name
	std::string optionName;
		/// informal descriprion
	std::string optionDescription;
		// type of value: bool, int or text
	ioType type;
		/// boolean value [relevant iff (type == iotBool)]
	bool bValue;
		/// integer value [relevant iff (type == iotInt)]
	int iValue;
		/// textual value [relevant iff (type == iotText)]
	std::string tValue;
		// default value (name of type)
	std::string defaultValue;

public:		// interface
		/// c'tor (init all values including proper ?Value)
	ifOption ( const std::string& name, const std::string& desc, ioType t, const std::string& defVal );
		/// empty d'tor
	~ifOption (void ) {}

	// write methods

		/// set boolean value; @return false in case of error
	bool setValue ( bool b ) { bValue = b; return (type != iotBool); }
		/// set integer value; @return false in case of error
	bool setValue ( int i ) { iValue = i; return (type != iotInt); }
		/// set string value; @return false in case of error
	bool setValue ( const std::string& t ) { tValue = t; return (type != iotText); }
		/// set textualy given value of current type; @return false in case of error
	bool setAValue ( const std::string& s );

	// access methods
		/// get value of a Boolean option
	bool getBool ( void ) const { assert ( type == iotBool ); return bValue; }
		/// get value of an integer option
	int getInt ( void ) const { assert ( type == iotInt ); return iValue; }
		/// get value of a string option
	const std::string& getText ( void ) const { assert ( type == iotText ); return tValue; }

		/// output in the form of config file
	void printConfString ( std::ostream& o ) const;
}; // ifOption

// implementation of class ifOption

inline ifOption :: ifOption ( const std::string& name, const std::string& desc, ioType t, const std::string& defVal )
	: optionName (name)
	, optionDescription (desc)
	, type (t)
	, defaultValue (defVal)
{
	setAValue (defVal);
}

/// set of options with access by name
class ifOptionSet
{
protected:	// internal type definitions
		/// base internal type
	typedef std::map<std::string,ifOption*> OptionSet;

protected:	// members
		/// set of all avaliable (given) options
	OptionSet Base;

protected:	// methods
		/// get access option structure by name; @return NULL if no such option was registered
	const ifOption* locateOption ( const std::string& name ) const;

public:		// interface
		/// empty c'tor
	ifOptionSet ( void ) {}
		/// d'tor (delete all registered options)
	~ifOptionSet ( void )
	{
		for ( OptionSet::iterator p = Base.begin(); p != Base.end(); ++p )
			delete p->second;
	}

		/// register an option with given name, description, type and default. @return true iff such option exists
	bool RegisterOption (
		const std::string& name,
		const std::string& desc,
		ifOption::ioType t,
		const std::string& defVal )
	{
		if ( locateOption (name) != NULL )
			return true;
		Base[name] = new ifOption ( name, desc, t, defVal );
		return false;
	}
		/// init all registered option using given section of given configuration
	bool initByConfigure ( Configuration& conf, const std::string& Section );

	// read access

		/// get Boolean value of given option
	bool getBool ( const std::string& optionName ) const
	{
		const ifOption* p = locateOption ( optionName );
		assert ( p != NULL );
		return p->getBool ();
	}
		/// get integral value of given option
	int getInt ( const std::string& optionName ) const
	{
		const ifOption* p = locateOption ( optionName );
		assert ( p != NULL );
		return p->getInt ();
	}
		/// get string value of given option
	const std::string& getText ( const std::string& optionName ) const
	{
		const ifOption* p = locateOption ( optionName );
		assert ( p != NULL );
		return p->getText ();
	}

		/// output option set in the form of config file
	void printConfig ( std::ostream& o ) const;
}; // ifOptionSet

#endif
