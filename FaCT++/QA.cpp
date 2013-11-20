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
#include "ConjunctiveQuerySet.h"

extern TExpressionManager* pEM;
extern VariableFactory VarFact;
#include "lubm2.h"
#include "NCIT_Queries.h"
#include "BSPO_Queries.h"
extern void runQueries ( CQSet& queries, ReasoningKernel* kernel );

//----------------------------------------------------------------------------------
// SAT/SUB queries
//----------------------------------------------------------------------------------

void
doQueryAnswering ( ReasoningKernel& Kernel )
{
	// perform query answering
	CQSet* queries =
			new BSPOQuery ( Kernel.getExpressionManager(), &VarFact );

	if ( queries->isArtificialABox() )
		Kernel.classifyKB();
	else
		Kernel.realiseKB();

	runQueries ( *queries, &Kernel );
	delete queries;
}
