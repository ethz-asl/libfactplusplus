/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2012 by Dmitry Tsarkov

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
#include <sstream>

#include "procTimer.h"
#include "parser.h"
#include "configure.h"
#include "logging.h"

#include "Kernel.h"
#include "cpm.h"
#include "tOntologyPrinterLISP.h"	// AD prints

TsProcTimer totalTimer, wTimer;
Configuration Config;
ReasoningKernel Kernel;

inline void Usage ( void )
{
	std::cerr << "\nUsage:\tFaCT++ <Conf file>  or\n\tFaCT++ -get-default-options\n\n";
	exit (1);
}

inline void error ( const char* mes )
{
	std::cerr << mes << "\n";
	exit(2);
}

inline void OutTime ( std::ostream& o )
{
	o << "Working time = " << totalTimer  << " seconds\n";
}

//----------------------------------------------------------------------------------
// atomic decomposition support
//----------------------------------------------------------------------------------

// print axioms of an atom
static void
printAtomAxioms ( const TOntologyAtom::AxiomSet& Axioms )
{
	static TLISPOntologyPrinter LP(LL);
	// do cycle via set to keep the order
	typedef std::set<TDLAxiom*> AxSet;
	const AxSet M ( Axioms.begin(), Axioms.end() );
	for ( AxSet::const_iterator p = M.begin(); p != M.end(); ++p )
		(*p)->accept(LP);
}

/// print dependencies of an atom
static void
printAtomDeps ( const TOntologyAtom::AtomSet& Dep )
{
	if ( unlikely(Dep.empty()) )
		LL << "Ground";
	else
		LL << "Depends on:";
	for ( TOntologyAtom::AtomSet::const_iterator q = Dep.begin(), q_end = Dep.end(); q != q_end; ++q )
		LL << " " << (*q)->getId();
	LL << "\n";
}

/// print the atom with an index INDEX of the AD
static void
printADAtom ( unsigned int index )
{
	const TOntologyAtom::AxiomSet& Axioms = Kernel.getAtomAxioms(index);
	const TOntologyAtom::AtomSet& Dep = Kernel.getAtomDependents(index);
	LL << "Atom " << index << " (size " << Axioms.size() << ", module size " << Kernel.getAtomModule(index).size() << "):\n";
	printAtomAxioms(Axioms);
	printAtomDeps(Dep);
}

/// @return all the axioms in the AD
static size_t
sizeAD ( unsigned int n )
{
	size_t ret = 0;
	for ( unsigned int i = 0; i < n; ++i )
		ret += Kernel.getAtomAxioms(i).size();
	return ret;
}

//----------------------------------------------------------------------------------
// SAT/SUB queries
//----------------------------------------------------------------------------------

std::string Query[2];

/// fill query target names by configure
void fillSatSubQuery ( void )
{
	// founds a target for checking
	if ( !Config.checkValue ( "Query", "Target" ) )
		Query[0] = Config.getString();

	if ( !Config.checkValue ( "Query", "Target2" ) )
		Query[1] = Config.getString();
}

ReasoningKernel::TConceptExpr*
getNextName ( TsScanner& sc, TExpressionManager* pEM )
{
	for (;;)
	{
		if ( sc.GetLex() == LEXEOF )
			return NULL;
		LispToken t = sc.getNameKeyword();

		if ( t != ID )
			return t == L_TOP ? pEM->Top() : pEM->Bottom();
		try
		{
			return pEM->Concept(sc.GetName());
		}
		catch ( const EFPPCantRegName& )
		{
			try
			{
				return pEM->OneOf(pEM->Individual(sc.GetName()));
			}
			catch ( const EFPPCantRegName& )
			{
				std::cout << "Query name " << sc.GetName() << " is undefined in TBox\n";
			}
		}
	}
}

const char* getConceptName ( ReasoningKernel::TConceptExpr* C )
{
	if ( dynamic_cast<const TDLConceptTop*>(C) != NULL )
		return "*TOP*";
	if ( dynamic_cast<const TDLConceptBottom*>(C) != NULL )
		return "*BOTTOM*";
	if ( dynamic_cast<const TDLConceptName*>(C) != NULL )
		return dynamic_cast<const TDLConceptName*>(C)->getName();
	return "concept expression";
}

/// try to do a reasoning; react if exception was thrown
#define TryReasoning(action)			\
	do {								\
		try { action; }					\
		catch ( const EFPPInconsistentKB& ) {}	\
		catch ( const EFPPCantRegName& crn )	\
		{ std::cout << "Query name " << crn.getName()		\
			<< " is undefined in TBox\n"; }					\
		catch ( const EFPPNonSimpleRole& nsr )	\
		{ std::cerr << "WARNING: KB is incorrect: " 		\
			<< nsr.what() << ". Query is NOT processed\n";	\
		  exit(0); }					\
		catch ( const EFPPCycleInRIA& cir )	\
		{ std::cerr << "WARNING: KB is incorrect: " 		\
			<< cir.what() << ". Query is NOT processed\n";	\
		  exit(0); }					\
	} while (0)

void testSat ( const std::string& names, ReasoningKernel& Kernel )
{
	std::stringstream s(names);
	TsScanner sc(&s);
	ReasoningKernel::TConceptExpr* sat;

	while ( (sat = getNextName(sc,Kernel.getExpressionManager())) != NULL )
	{
		bool result = false;
		if ( dynamic_cast<const TDLConceptTop*>(sat) != NULL )
			result = Kernel.isKBConsistent();
		else
			TryReasoning ( result = Kernel.isSatisfiable(sat) );

		std::cout << "The '" << getConceptName(sat) << "' concept is ";
		if ( !result )
			std::cout << "un";
		std::cout << "satisfiable w.r.t. TBox\n";
	}
}

void testSub ( const std::string& names1, const std::string& names2, ReasoningKernel& Kernel )
{
	std::stringstream s1(names1), s2(names2);
	TsScanner sc1(&s1), sc2(&s2);
	ReasoningKernel::TConceptExpr *sub, *sup;
	TExpressionManager* pEM = Kernel.getExpressionManager();

	while ( (sub = getNextName(sc1,pEM)) != NULL )
	{
		sc2.ReSet();
		while ( (sup = getNextName(sc2,pEM)) != NULL )
		{
			bool result = false;
			TryReasoning ( result = Kernel.isSubsumedBy ( sub, sup ) );

			std::cout << "The '" << getConceptName(sub) << " [= " << getConceptName(sup) << "' subsumption does";
			if ( !result )
				std::cout << " NOT";
			std::cout << " holds w.r.t. TBox\n";
		}
	}
}

//**********************  Main function  ************************************
int main ( int argc, char *argv[] )
{
	try{

	totalTimer. Start ();

	Kernel.setTopBottomRoleNames ( "*UROLE*", "*EROLE*", "*UDROLE*", "*EDROLE*" );
	Kernel.setUseUndefinedNames(false);

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

	// init timeout option
	unsigned long testTimeout = Kernel.getOptions()->getInt("testTimeout");
	Kernel.setOperationTimeout(testTimeout);
	if ( LLM.isWritable(llAlways) )
		LL << "Init testTimeout = " << testTimeout << "\n";

	// Load the ontology
	DLLispParser TBoxParser ( &iTBox, &Kernel );
	Kernel.setVerboseOutput(true);
	TProgressMonitor* pMon = new ConsoleProgressMonitor;
	Kernel.setProgressMonitor(pMon);

	// parsing input TBox
	std::cerr << "Loading KB...";
	wTimer.Start ();
	TBoxParser.Parse ();
	wTimer.Stop ();
	std::cerr << " done in " << wTimer << " seconds";

	Out << "loading time " << wTimer << " seconds\n";

	if ( Kernel.getOptions()->getBool("checkAD") )	// check atomic decomposition and exit
	{
		// do the atomic decomposition
		TsProcTimer timer;
		timer.Start();
//		AD->setProgressIndicator(new CPPI());
		unsigned int nAtoms = Kernel.getAtomicDecompositionSize(M_BOT);
		timer.Stop();
		if ( LLM.isWritable(llAlways) )
		{
			LL << "Atomic structure built in " << timer << " seconds\n";
			size_t sz = sizeAD(nAtoms);
			LL << "Atomic structure (" << sz << " axioms in " << nAtoms << " atoms; " << Kernel.getOntology().size()-sz << " tautologies):\n";
			for ( unsigned int i = 0; i < nAtoms; ++i )
				printADAtom(i);
		}
		return 0;
	}

	TsProcTimer pt;
	pt.Start();

	// parsing query targets
	fillSatSubQuery();

	TryReasoning(Kernel.preprocessKB());

	// do preprocessing
	if ( !Kernel.isKBConsistent() )
		std::cerr << "WARNING: KB is inconsistent. Query is NOT processed\n";
	else	// perform reasoning
	{
		if ( Query[0].empty() )
		{
			if ( Query[1].empty() )
				TryReasoning(Kernel.realiseKB());
			else
				error ( "Query: Incorrect options" );
		}
		else
		{
			if ( Query[1].empty() )		// sat
				testSat ( Query[0], Kernel );
			else
				testSub ( Query[0], Query[1], Kernel );
		}
	}

	pt.Stop();

	// save final TBox
	Kernel.writeReasoningResult ( Out, pt );

	// finish
	totalTimer.Stop ();
	OutTime (std::cout);
	OutTime (Out);
	delete pMon;

	}
	catch ( const EFaCTPlusPlus& e )
	{
		std::cerr << "\n" << e.what() << "\n";
		exit(1);
	}
	return 0;
}
