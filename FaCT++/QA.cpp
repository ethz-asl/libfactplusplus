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

#include "Kernel.h"

#include "QR.h"
extern TExpressionManager* pEM;
extern VariableFactory VarFact;
#include "lubm2.h"
#include "NCIT_Queries.h"
#include "BSPO_Queries.h"
extern QRQuery* createQuery(void);
extern void doQuery(QRQuery * query, ReasoningKernel* kernel);

//----------------------------------------------------------------------------------
// SAT/SUB queries
//----------------------------------------------------------------------------------

/// try to do a reasoning; react if exception was thrown
#define TryReasoning(action)			\
	do {								\
		try { action; }					\
		catch ( const EFPPInconsistentKB& ) {} 	\
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

//**********************  Main function  ************************************
void
doQueryAnswering ( ReasoningKernel& Kernel )
{
	TryReasoning(Kernel.realiseKB());
	// perform query answering
	pEM = Kernel.getExpressionManager();

//	for ( int i = 1; i < 11; i++ )
//	if ( i != 6 && i != 7 )
	for ( int i = 0; i < 2; i++ )
	{
		QRQuery* query =
//			buildLUBM2Query(i,&VarFact, pEM);
//			buildNCITQuery(i,&VarFact, pEM);
			buildBSPOQuery(i,&VarFact, pEM);
		doQuery(query, &Kernel);
		delete query;
	}
}
