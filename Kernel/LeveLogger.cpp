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

#include <fstream>

#include "LeveLogger.h"
#include "configure.h"

bool LeveLogger :: initLogger ( unsigned int l, const char* filename )
{
	// init file
	LL.open(filename);

	if ( LL.bad() )
		return true;

	// init level
	allowedLevel = l;
	LL << "Init allowedLevel = " << allowedLevel << "\n";

	return false;
}

bool LeveLogger :: initLogger ( Configuration& Config )
{
	// try to load LeveLogger's config section
	if ( Config.useSection ( "LeveLogger" ) )
		return true;

	unsigned int l;

	// try to load allowance level
	if ( Config.checkValue ( "allowedLevel" ) )
		l = 0;
	else
		l = Config.getLong ();

	// try to load input value
	if ( Config.checkValue ( "file" ) )
		return true;

	return initLogger ( l, Config.getString() );
}

// the only element of LL
LeveLogger LLM;
std::ofstream LL;
