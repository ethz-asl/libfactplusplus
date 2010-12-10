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
TAxiomSet :: split ( const TAxiom* p )
{
	AxiomCollection Splitted;
	if ( !p->split(Splitted) )	// nothing to split
		return false;

	AxiomCollection::iterator q = Splitted.begin(), q_end = Splitted.end();
	bool cont = true;
	for ( ; q != q_end; ++q )
		if ( !needed(*q) )
		{	// there is already such an axiom in process; delete it
			cont = false;
			break;
		}
	// do the actual insertion if necessary
	for ( q = Splitted.begin(); q != q_end; ++q )
		if ( cont )
			insertGCI(*q);
		else
			delete *q;

	return cont;
}

unsigned int TAxiomSet :: absorb ( void )
{
	// absorbed- and unabsorbable GCIs
	AxiomCollection Absorbed, GCIs;

	// we will change Accum (via split rule), so indexing and compare with size
	for ( unsigned int i = 0; i < Accum.size(); ++i )
	{
#	ifdef RKG_DEBUG_ABSORPTION
		std::cout << "\nProcessing (" << i << "):";
#	endif
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

#ifdef RKG_DEBUG_ABSORPTION
	std::cout << "\nAbsorption done with " << Accum.size() << " GCIs left\n";
#endif
	PrintStatistics();
	return size();
}

bool TAxiomSet :: absorbGCI ( const TAxiom* p )
{
	Stat::SAbsAction();

	for ( AbsActVector::iterator f = ActionVector.begin(), f_end = ActionVector.end(); f != f_end; ++f )
		if ( (this->*(*f))(p) )
			return true;

#ifdef RKG_DEBUG_ABSORPTION
	std::cout << " keep as GCI";
#endif

	return false;
}

bool TAxiomSet :: initAbsorptionFlags ( const std::string& flags )
{
	ActionVector.clear();
	for ( std::string::const_iterator p = flags.begin(), p_end = flags.end(); p != p_end; ++p )
		switch ( *p )
		{
		case 'B': ActionVector.push_back(&TAxiomSet::absorbIntoBottom); break;
		case 'T': ActionVector.push_back(&TAxiomSet::absorbIntoTop); break;
		case 'E': ActionVector.push_back(&TAxiomSet::simplifyCN); break;
		case 'C': ActionVector.push_back(&TAxiomSet::absorbIntoConcept); break;
		case 'F': ActionVector.push_back(&TAxiomSet::simplifyForall); break;
		case 'R': ActionVector.push_back(&TAxiomSet::absorbIntoDomain); break;
		case 'S': ActionVector.push_back(&TAxiomSet::split); break;
		default: return true;
		}

	if ( LLM.isWritable(llAlways) )
		LL << "Init absorption order as " << flags.c_str() << "\n";

	return false;
}

void TAxiomSet :: PrintStatistics ( void ) const
{
	if ( Stat::SAbsAction::objects_created == 0 || !LLM.isWritable(llAlways) )
		return;

	LL << "\nAbsorption dealt with "
	   << Stat::SAbsInput::objects_created << " input axioms\nThere were made "
	   << Stat::SAbsAction::objects_created << " absorption actions, of which:";
	if ( Stat::SAbsRepCN::objects_created )
		LL << "\n\t" << Stat::SAbsRepCN::objects_created << " concept name replacements";
	if ( Stat::SAbsRepForall::objects_created )
		LL << "\n\t" << Stat::SAbsRepForall::objects_created << " universals replacements";
	if ( Stat::SAbsSplit::objects_created )
		LL << "\n\t" << Stat::SAbsSplit::objects_created << " conjunction splits";
	if ( Stat::SAbsBApply::objects_created )
		LL << "\n\t" << Stat::SAbsBApply::objects_created << " BOTTOM absorptions";
	if ( Stat::SAbsTApply::objects_created )
		LL << "\n\t" << Stat::SAbsTApply::objects_created << " TOP absorptions";
	if ( Stat::SAbsCApply::objects_created )
		LL << "\n\t" << Stat::SAbsCApply::objects_created << " concept absorption with "
		   << Stat::SAbsCAttempt::objects_created << " possibilities";
	if ( Stat::SAbsRApply::objects_created )
		LL << "\n\t" << Stat::SAbsRApply::objects_created << " role domain absorption with "
		   << Stat::SAbsRAttempt::objects_created << " possibilities";
	if ( !Accum.empty() )
		LL << "\nThere are " << Accum.size() << " GCIs left";
}
