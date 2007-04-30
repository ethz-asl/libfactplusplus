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

#include <string>

#include "procTimer.h"
#include "digFileInterface.h"

TsProcTimer totalTimer, wTimer;

inline void Usage ( void )
{
	std::cerr << "\nUsage: digFaCT++ <tells-file> [<tells-file>]* <ask-file> [<ask-file>]*\n";
	exit (1);
}

//**********************  Main function  ************************************
int main ( int argc, char *argv[] )
{
	totalTimer. Start ();

	// parse options
	if ( argc < 3 )
		Usage ();

	DIGFileInterface dig;

	// output data
	std::string res;

	// parsing i'th query
	for ( int i = 1; i < argc; ++i )
	{
		std::cerr << "Processing file " << argv[i] << "...\n";
		wTimer.Start ();
		dig.processFile ( argv[i], res );
		wTimer.Stop ();
		std::cerr << "Processing file " << argv[i] << " done in " << wTimer << " seconds\n";

		// output results
		std::cout << res.c_str();
		res.clear();
		wTimer.Reset();
	}

	// finish
	totalTimer.Stop ();
	std::cerr << "Working time = " << totalTimer  << " seconds\n";

	return 0;
}
