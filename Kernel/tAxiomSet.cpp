/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2010 by Dmitry Tsarkov

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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

bool
TAxiomSet :: split ( TAxiom* p )
{
	TAxiom* q = NULL;
	TAxiom* clone = new TAxiom(*p);
	bool untouched = true;

	while ( (q=clone->split()) != NULL )
	{
		untouched = false;
		if ( insertIfNew(q) )
		{	// there is already such an axiom in process; stop now
			delete q;
			delete clone;
			return false;
		}
	}

	if ( untouched || insertIfNew(clone) )
	{
		delete clone;
		return false;
	}
	return true;
}

unsigned int TAxiomSet :: absorb ( void )
{
	// absorbed- and unabsorbable GCIs
	AxiomCollection Absorbed, GCIs;

	if ( !useAbsorption )
		goto final;

	// we will change Accum (via split rule), so indexing and compare with size
	for ( unsigned int i = 0; i < Accum.size(); ++i )
	{
		TAxiom* ax = Accum[i];
		if ( absorbGCI(ax) )
			Absorbed.push_back(ax);
		else
			GCIs.push_back(ax);
	}

	// clear absorbed and remove them from Accum
	for ( AxiomCollection::iterator p = Absorbed.begin(), p_end = Absorbed.end(); p != p_end; ++p )
		delete *p;
	Accum.swap(GCIs);

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

	++nAbsorptionTried;
	for (;;)	// 1) -- beginning
	{
		++nAbsorptionIterations;

		// always check absorption into TOP first
		if ( absorbIntoTop(p) )
			break;

		// steps 2-3. Simplify and unfold
		if ( absorbSimplifyFirst )
			if ( simplify(p) )
				continue;

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
			if ( simplify(p) )
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
			break;

		return false;
	}

	// here axiom is absorbed
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

void TAxiomSet :: PrintStatistics ( void ) const
{
	if ( useAbsorption && nAbsorptionTried > 0 && LLM.isWritable(llAlways) )
		LL << "\nThere were " << nAbsorptionIterations
		   << " absorption attempts for " << nAbsorptionTried << " axioms."
		   << "\nThere were used " << nConceptAbsorbed << " concept absorption with "
		   << nConceptAbsorbAlternatives << " possibilities\nThere were used "
		   << nRoleDomainAbsorbed << " role domain absorption with "
		   << nRoleDomainAbsorbAlternatives << " possibilities";
}
