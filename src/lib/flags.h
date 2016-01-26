/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2005-2015 by Dmitry Tsarkov

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

#ifndef FLAGS_H
#define FLAGS_H

/// class for carrying different flags; real users shall inherit from it
class Flags
{
public:		// types
		/// type of base integer used for bit-set
	typedef uint32_t IntType;

private:	// members
		/// variable to keep all the flags
	IntType flags;

protected:	// methods for flags maintainance
		/// get given flag value
	bool getFlag ( IntType mask ) const { return flags & mask; }
		/// set given flag to 1
	void setFlag ( IntType mask ) { flags |= mask; }
		/// set given flag to 0
	void clearFlag ( IntType mask ) { flags &= ~mask; }
		/// set given flag to given value
	void setFlag ( IntType mask, bool Set ) { Set ? setFlag(mask) : clearFlag(mask); }

		/// get all the flags at once
	IntType getAllFlags ( void ) const { return flags; }
		/// set all flags to a given value; @return old value of the flags
	IntType setAllFlags ( IntType value ) { IntType old = flags; flags = value; return old; }

public:		// interface
		/// empty c'tor
	Flags ( void ) : flags(0) {}
		/// init flags with given set of flags
	explicit Flags ( IntType init ) : flags(init) {}
		/// copy c'tor
	Flags ( const Flags& f ) : flags(f.flags) {}
		/// assignment
	Flags& operator = ( const Flags& f ) { flags = f.flags; return *this; }
		/// empty d'tor
	virtual ~Flags ( void ) {}
}; // Flags

// use this macro to create a new flag
#define FPP_ADD_FLAG(Name,Mask)		\
	bool is##Name ( void ) const	\
		{ return getFlag(Mask); }	\
	void set##Name ( void )			\
		{ setFlag(Mask); }			\
	void clear##Name ( void ) 		\
		{ clearFlag(Mask); }		\
	void set##Name ( bool action )	\
		{ setFlag(Mask,action); }	\

#endif
