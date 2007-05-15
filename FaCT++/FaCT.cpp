/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2007 by Dmitry Tsarkov

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

#include <iostream>
#include <iomanip>
#include <fstream>

#include "procTimer.h"
#include "parser.h"
#include "comerror.h"
#include "configure.h"
#include "logging.h"

#include "Kernel.h"

TsProcTimer totalTimer, wTimer;
Configuration Config;

inline void Usage ( void )
{
	std::cerr << "\nUsage:\tFaCT++ <Conf file>  or\n\tFaCT++ -get-default-options\n\n";
	exit (1);
}

inline void OutTime ( std::ostream& o )
{
	o << "Working time = " << totalTimer  << " seconds\n";
}

DLTree* Query[2] = { NULL, NULL };

/// fill query target names by configure
void fillSatSubQuery ( ReasoningKernel& Kernel )
{
	std::string tName[2];

	// founds a target for checking
	if ( Config.checkValue ( "Query", "Target" ) )
		tName[0] = "";
	else
		tName[0] = Config.getString();

	if ( Config.checkValue ( "Query", "Target2" ) )
		tName[1] = "";
	else
		tName[1] = Config.getString();

	// setup names
	for ( register int i = 1; i >= 0; --i )
		if ( tName[i] != "" )
		{
			if ( tName[i] == "*TOP*" )
				Query[i] = new DLTree (TOP);
			else if ( tName[i] == "*BOTTOM*" )
				Query[i] = new DLTree (BOTTOM);
			else
			{
				try { Query[i] = Kernel.ensureConceptName(tName[i].c_str()); }
				catch ( CantRegName ex ) { error ( "Query: queried name is undefined" ); }
			}
		}

	if ( Query[1] != NULL && Query[0] == NULL )
		error ( "Query: Incorrect options" );
}

//**********************  Main function  ************************************
int main ( int argc, char *argv[] )
{
	totalTimer. Start ();

	// define [more-or-less] global Kernel
	ReasoningKernel Kernel;

	// parse options
	if ( argc > 3 || argc < 2 )
		Usage ();

	// test if we asked for default options
	if ( !strcmp ( argv[1], "-get-default-options" ) )
	{	// print Kernel's option set
		Kernel.getOptions()->printConfig ( std::cout );
		exit (0);
	}

	// loading config file...
	if ( Config. Load ( argv [1] ) )
		error ( "Cannot load Config file" );
	else
	{
		argv++;
		argc--;
	}

	// fills option values by Config
	if ( Kernel.getOptions()->initByConfigure ( Config, "Tuning" ) )
		error ( "Cannot fill options value by config file" );

	// getting TBox file name
	const char* tBoxName = NULL;
	if ( Config. checkValue ( "Query", "TBox" ) )
		error ( "Config: no TBox file defined" );
	else
		tBoxName = Config. getString ();

	// Open input file for TBox and so on...
	std::ifstream iTBox ( tBoxName );

	if ( iTBox.fail () )
		error ( "Cannot open input TBox file" );

	// output file...
	std::ofstream Out ( argc == 3 ? argv [2] : "dl.res" );

	if ( Out.fail () )
		error ( "Cannot open output file" );

#ifdef _USE_LOGGING
	// initialize LeveLogger
	if ( LLM.initLogger(Config) )
		error ( "LeveLogger: couldn't open logging file" );
#endif

	// Create a TBox...
	Kernel.newKB ();
	DLLispParser TBoxParser ( &iTBox, &Kernel );
	Kernel.useVerboseOutput();

	// parsing input TBox
	std::cerr << "Loading KB...";
	wTimer.Start ();
	TBoxParser.Parse ();
	wTimer.Stop ();
	std::cerr << " done";

	Out << "loading time " << wTimer << " seconds\n";

	TsProcTimer pt;
	pt.Start();

	// do preprocessing
	if ( !Kernel.isKBConsistent() )
		std::cerr << "WARNING: KB is inconsistent. Query is NOT processed\n";
	else	// perform reasoning
	{
		// parsing query targets
		fillSatSubQuery ( Kernel );
		bool result = false;

		if ( Query[0] == NULL )
			Kernel.realiseKB();
		else if ( Query[1] == NULL )
		{
			Kernel.isSatisfiable ( Query[0], result );

			std::cout << "The '" << Query[0] << "' concept is ";
			if ( !result )
				std::cout << "un";
			std::cout << "satisfiable w.r.t. TBox\n";
		}
		else
		{
			Kernel.isSubsumes ( Query[1], Query[0], result );

			std::cout << "The '" << Query[0] << " [= " << Query[1] << "' subsumption does";
			if ( !result )
				std::cout << " NOT";
			std::cout << " holds w.r.t. TBox\n";
		}
	}

	pt.Stop();

	// save final TBox
	Kernel.writeReasoningResult ( Out, pt );

	// finish
	totalTimer.Stop ();
	OutTime (std::cout);
	OutTime (Out);

	return 0;
}
