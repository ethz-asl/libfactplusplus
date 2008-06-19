/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2005-2008 by Dmitry Tsarkov

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

#ifndef _FLAGS_H
#define _FLAGS_H

/// class for carrying different flags; real users shall inherit from it
class Flags
{
private:
		/// variable to keep all the flags
	unsigned int flags;

protected:	// methods for flags maintainance
		/// get given flag value
	bool getFlag ( unsigned int mask ) const { return flags & mask; }
		/// set given flag to 1
	void setFlag ( unsigned int mask ) { flags |= mask; }
		/// set given flag to 0
	void clearFlag ( unsigned int mask ) { flags &= ~mask; }
		/// set given flag to given value
	void setFlag ( unsigned int mask, bool Set ) { Set ? setFlag(mask) : clearFlag(mask); }

public:		// interface
		/// empty c'tor
	Flags ( void ) : flags(0) {}
		/// init flags with given set of flags
	explicit Flags ( unsigned int init ) : flags(init) {}
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
