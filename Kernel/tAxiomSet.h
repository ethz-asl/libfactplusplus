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

#ifndef TAXIOMSET_H
#define TAXIOMSET_H

#include <iostream>

#include "tAxiom.h"

class TBox;

namespace Stat
{
	class SAbsInput: public counter<SAbsInput> {};
	class SAbsAction: public counter<SAbsAction> {};
}

/// set of GCIs, absorbable and not
class TAxiomSet
{
protected:	// internal types
		/// set of GCIs
	typedef std::vector<TAxiom*> AxiomCollection;

protected:	// members
		/// host TBox that holds all concepts/etc
	TBox& Host;
		/// set of axioms that accumilates incoming (and newly created) axioms; Tg
	AxiomCollection Accum;

	// flags that manages absorption

		/// flag for using absorption
	bool useAbsorption;
		/// flag for simplifying absorption set before every check
	bool absorbSimplifyFirst;
		/// flag for using role absorption
	bool useRoleAbsorption;
		/// flag for choosing if C will be at the beginning of absorption process
	bool begC;
		/// flag for choosing if R will be at the beginning of absorption process
	bool begR;
		/// flag for choosing if C will be absorbed late
	bool lateC;
		/// flag for choosing if R will be absorbed late
	bool lateR;

protected:	// methods

		/// add already built GCI p
	void insertGCI ( TAxiom* p )
	{
#	ifdef RKG_DEBUG_ABSORPTION
		std::cout << "\n new axiom (" << Accum.size() << "):";
		p->dump(std::cout);
#	endif
		Accum.push_back(p);
	}
		/// @return true iff axiom Q is new in the set
	bool needed ( TAxiom* q )
	{
		for ( AxiomCollection::const_iterator p = Accum.begin(), p_end = Accum.end(); p != p_end; ++p )
			if ( *q == **p )
			{
#			ifdef RKG_DEBUG_ABSORPTION
				std::cout << " same as (" << p-Accum.begin() << ")";
#			endif
				return false;
			}
		return true;
	}
		/// insert GCI if new; @return true iff already exists
	bool insertIfNew ( TAxiom* q )
	{
		if ( needed(q) )
		{
			insertGCI(q);
			return false;
		}
		return true;
	}
		/// absorb single GCI wrt absorption flags
	bool absorbGCI ( TAxiom* p );
		/// split given axiom
	bool split ( TAxiom* p );
		/// simplify given axiom.
	bool simplify ( TAxiom* p )
	{
		TAxiom* q = p->simplify();
		if ( q == NULL )
			return false;
		if ( insertIfNew(q) )
		{
			delete q;
			return false;
		}
		return true;
	}

		/// absorb single axiom AX into TOP; @return true if succeed
	bool absorbIntoTop ( TAxiom* ax ) { return ax->absorbIntoTop(Host); }
		/// absorb single axiom AX into concept; @return true if succeed
	bool absorbIntoConcept ( TAxiom* ax ) { return ax->absorbIntoConcept(Host); }
		/// absorb single axiom AX into role domain; @return true if succeed
	bool absorbIntoDomain ( TAxiom* ax ) { return useRoleAbsorption && ax->absorbIntoDomain(); }

public:		// interface
		/// c'tor
	TAxiomSet ( TBox& host )
		: Host(host)
		{}
		/// d'tor
	~TAxiomSet ( void );

		/// init all absorption-related flags using given set of option
	bool initAbsorptionFlags ( const std::string& flags );
		/// check if absorption flags are set correctly wrt RangeAndDomain flag
	bool isAbsorptionFlagsCorrect ( bool useRnD ) const;
		/// add axiom for the GCI C [= D
	void addAxiom ( DLTree* C, DLTree* D )
	{
		Stat::SAbsInput();
		TAxiom* p = new TAxiom();
		p->add(C);
		p->add(createSNFNot(D));
		insertGCI(p);
	}

		/// absorb set of axioms; @return size of not absorbed set
	unsigned int absorb ( void );
		/// get number of (not absorbed) GCIs
	unsigned int size ( void ) const { return Accum.size(); }
		/// @return true if non-concept aborption were executed
	bool wasRoleAbsorptionApplied ( void ) const { return Stat::SAbsRApply::objects_created > 0; }
		/// get GCI of all non-absorbed axioms
	DLTree* getGCI ( void ) const
	{
		DLTree* ret = createTop();
		for ( AxiomCollection::const_iterator p = Accum.begin(), p_end = Accum.end(); p != p_end; ++p )
			ret = createSNFAnd ( ret, (*p)->createAnAxiom() );

		return ret;
	}

		/// print (not absorbed) GCIs
	void Print ( std::ostream& o ) const;
		/// print absorption statistics
	void PrintStatistics ( void ) const;
}; // TAxiomSet

#endif
