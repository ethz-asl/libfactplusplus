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

#ifndef _CPPI_H
#define _CPPI_H

/*
 * Console percent progress indicator
 */

#include <iostream>
#include <iomanip>

#include "ProgressIndicatorInterface.h"

class CPPI: public ProgressIndicatorInterface
{
protected:
	unsigned int oldPercent, curPercent;

protected:
	void initExpose ( void )
	{
		std::cerr << "   0%";
	}
	void expose ( void )
	{
		curPercent = (unsigned int)(((float)uCurrent/uLimit)*100);

		if ( curPercent != oldPercent )
		{
			std::cerr << "\b\b\b\b\b" << std::setw(4) << curPercent << '%';
			oldPercent = curPercent;
		}
	}
public:
	CPPI ( void ): ProgressIndicatorInterface (), oldPercent (0), curPercent (0)
		{ initExpose (); }
	CPPI ( unsigned long limit ): ProgressIndicatorInterface (), oldPercent (0), curPercent (0)
	{
		initExpose ();
		setLimit (limit);
	}
	~CPPI ( void ) {}
}; // CPPI

#endif
