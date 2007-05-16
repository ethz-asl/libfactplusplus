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

#include "tAxiomSet.h"

#include "logging.h"
#include "dlTBox.h"

/// d'tor
TAxiomSet :: ~TAxiomSet ( void )
{
	for ( AxiomCollection::iterator p = Accum.begin(), p_end = Accum.end(); p < p_end; ++p )
		delete *p;
}

unsigned int TAxiomSet :: absorb ( void )
{
	// GCIs to process
	AxiomCollection Process;

	if ( !useAbsorption || earlyAbsorption )
		goto final;

	do
	{
		absoprtionApplied = false;
		Process.swap(Accum);
		Accum.clear();

		for ( AxiomCollection::iterator p = Process.begin(), p_end = Process.end(); p != p_end; ++p )
			if ( !absorbGCI(*p) )
				insertGCI(*p);
	} while ( absoprtionApplied == true );

final:
	PrintStatistics();
	return size();
}

bool TAxiomSet :: absorbGCI ( TAxiom* p )
{
#ifdef RKG_DEBUG_ABSORPTION
	std::cout << "\nBegin absorption of ";
	p->dump(std::cout);
#endif

	for (;;)	// 1) -- beginning
	{
		// steps 2-3. Simplify and unfold
		if ( absorbSimplifyFirst )
			p->simplify();

		// R-or-C part a): necessary
		if ( begC )	// C is first
			if ( absorbIntoConcept(p) )	// (C)
				break;

		if ( begR )	// R is first
			if ( absorbIntoDomain(p) )	// (R)
				break;

		// R-or-C part b): optional
		if ( begR && !lateC )	// C is second
			if ( absorbIntoConcept(p) )	// (C)
				break;

		if ( begC && !lateR )	// R is second
			if ( absorbIntoDomain(p) )	// (R)
				break;

		// steps 2-3. Simplify and unfold
		if ( !absorbSimplifyFirst )
			if ( p->simplify() )
				continue;

		// R-or-C part c): late
		if ( lateC )	// C is late
			if ( absorbIntoConcept(p) )	// (C)
				break;

		if ( lateR )	// R is late
			if ( absorbIntoDomain(p) )	// (R)
				break;

		// step 5: recursive step -- split OR verteces
		if ( split(p) )
			continue;

		return false;
	}

	// here axiom is absorbed
	delete p;
	return true;
}

bool TAxiomSet :: initAbsorptionFlags ( const std::string& flags )
{
	if ( flags.size() != 5 )
		return true;

	// init general (concept) absorption
	switch ( flags[0] )
	{
	case 'n':	useAbsorption = false; break;
	case 'c':	useAbsorption = true; break;
	default:	return true;
	}

	// init role absorption flags
	if ( flags[1] == 'e' )
		useRoleAbsorption = true;
	else
		useRoleAbsorption = false;

	// init simplification order
	absorbSimplifyFirst = ( flags[2] == 'S' );

	// setup the other absorption steps' order
	begC = false;
	begR = false;
	lateC = false;
	lateR = false;

	// define the late- flags
	if ( flags[4] == 'C' )
	{
		lateC = true;
		begR = true;
	}
	else if ( flags[4] == 'R' )
	{
		lateR = true;
		begC = true;
	}
	// here 'S' is the latest option, so the 1st one defines the rest
	else if ( flags[2] == 'C' )
		begC = true;
	else if ( flags[2] == 'R' )
		begR = true;
	else	//error found
		return true;

	if ( LLM.isWritable(llAlways) )
		LL << "Init absorption order as " << flags.c_str() << "\n";

	return false;
}

/// check if absorption flags are set correctly
bool TAxiomSet :: isAbsorptionFlagsCorrect ( bool useRnD ) const
{
	// no absorption at all -- OK
	if ( !useAbsorption )
		return true;

	// no R&D absorption -- ensure no role absorption
	if ( !useRnD )
		return !useRoleAbsorption;

	// exactly one from beg* should be set
	if ( ( begC && begR ) || ( !begC && !begR ) )
		return false;

	// at most one from late* should be set
	if ( lateC && lateR )
		return false;

	// Neither C nor R can be beg- and late- at the same time
	if ( (begC && lateC) || (begR && lateR) )
		return false;

	return true;
}
/*
void TAxiomSet :: Print ( std::ostream& o ) const
{
	o << "Axioms (" << size() << "): \n";
	for ( AxiomCollection::const_iterator
		  p = Accum.begin(), p_end = Accum.end(); p != p_end; ++p )
		p->Print(o);
}
*/
void TAxiomSet :: PrintStatistics ( void ) const
{
	if ( LLM.isWritable(llAlways) )
		LL << "\nThere were used " << nConceptAbsorbed << " concept absorption with "
		   << nConceptAbsorbAlternatives << " possibilities\nThere were used "
		   << nRoleDomainAbsorbed << " role domain absorption with "
		   << nRoleDomainAbsorbAlternatives << " possibilities";
}
