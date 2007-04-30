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

#ifndef _PROGRESSINDICATORINTERFACE_H
#define _PROGRESSINDICATORINTERFACE_H

class ProgressIndicatorInterface
{
protected:
	unsigned long uLimit;
	unsigned long uCurrent;

	virtual void expose ( void ) = 0;
	bool checkMax ( void );

public:
	ProgressIndicatorInterface ( void ) : uLimit (0), uCurrent (0) {}
	ProgressIndicatorInterface ( unsigned long limit ) : uCurrent (0)
		{ setLimit (limit); }
	virtual ~ProgressIndicatorInterface ( void ) {}

	void setLimit ( unsigned long limit );
	void setIndicator ( unsigned long value );
	void incIndicator ( unsigned long delta = 1 );
	void reset ( void ) { setIndicator (0); }
}; // ProgressIndicatorInterface

inline void ProgressIndicatorInterface::setLimit ( unsigned long limit )
{
	uLimit = limit;

	if ( checkMax () )
		expose ();
}

inline bool ProgressIndicatorInterface::checkMax ( void )
{
	if ( uCurrent > uLimit )
	{
		uCurrent = uLimit;
		return true;
	}
	else
		return false;
}

inline void ProgressIndicatorInterface::setIndicator ( unsigned long value )
{
	if ( uCurrent != value )
	{
		uCurrent = value;
		checkMax ();
		expose ();
	}
}

inline void ProgressIndicatorInterface::incIndicator ( unsigned long delta )
{
	if ( delta != 0 )
	{
		uCurrent += delta;
		checkMax ();
		expose ();
	}
}

#endif
