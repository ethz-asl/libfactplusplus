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

#ifndef TAXIOM_H
#define TAXIOM_H

#include <vector>

#include "globaldef.h"
#include "dltree.h"
#include "tConcept.h"
#include "counter.h"

// uncomment this to have absorption debug messages
//#define RKG_DEBUG_ABSORPTION

class TBox;

namespace Stat
{
class SAbsSimplify: public counter<SAbsSimplify> {};
class SAbsFlatten: public counter<SAbsFlatten> {};
class SAbsSplit: public counter<SAbsSplit> {};
class SAbsBApply: public counter<SAbsBApply> {};
class SAbsTApply: public counter<SAbsTApply> {};
class SAbsCApply: public counter<SAbsCApply> {};
class SAbsCAttempt: public counter<SAbsCAttempt> {};
class SAbsRApply: public counter<SAbsRApply> {};
class SAbsRAttempt: public counter<SAbsRAttempt> {};
}

class TAxiom
{
private:	// no assignment
	TAxiom& operator = ( const TAxiom& ax );

protected:	// types
		/// type for axiom's representation, suitable for absorption
	typedef std::vector<DLTree*> absorptionSet;
		/// RW iterator for the elements of GCI
	typedef absorptionSet::iterator iterator;
		/// RO iterator for the elements of GCI
	typedef absorptionSet::const_iterator const_iterator;
		/// set of iterators to work with
	typedef std::vector<iterator> WorkSet;

protected:	// members
		/// GCI is presented in the form (or Disjuncts);
	absorptionSet Disjuncts;

protected:	// methods
		/// update internal indeces for an expression P in a given POSition
	void gather ( const DLTree* p ATTR_UNUSED, unsigned int pos ATTR_UNUSED ) {}
		/// replace element in POS with P
	void replace ( DLTree* p, unsigned int pos )
	{
		deleteTree(Disjuncts[pos]);
		gather ( p, pos );
		// don't create duplicated operands
		for ( unsigned int i = 0; i < Disjuncts.size(); ++i )
			if ( i != pos && equalTrees ( Disjuncts[i], p ) )
			{
				deleteTree(p);
				Disjuncts[pos] = createTop();
				return;
			}
		Disjuncts[pos] = p;
	}

	// access to labels

		/// RW begin
	iterator begin ( void ) { return Disjuncts.begin(); }
		/// RW end
	iterator end ( void ) { return Disjuncts.end(); }
		/// RO begin
	const_iterator begin ( void ) const { return Disjuncts.begin(); }
		/// RO end
	const_iterator end ( void ) const { return Disjuncts.end(); }

	// recognisable patterns in disjuncts

		/// build an RW concept from a given [C|I]NAME-rooted DLTree
	static TConcept* getConcept ( DLTree* p )
		{ return static_cast<TConcept*>(p->Element().getNE()); }
		/// build an RO concept from a given [C|I]NAME-rooted DLTree
	static const TConcept* getConcept ( const DLTree* p )
		{ return static_cast<const TConcept*>(p->Element().getNE()); }

		/// check whether P is in the form C for non-primitive C
	static bool isPosNP ( const DLTree* p )
	{
		return p->Element() == NOT && isName(p->Left()) &&
			   !getConcept(p->Left())->isPrimitive();
	}
		/// check whether P is in the form ~C for non-primitive C
	static bool isNegNP ( const DLTree* p )
	{
		return isName(p) && !getConcept(p)->isPrimitive();
	}
		/// check whether P is in the form (or C D)
	static bool isOr ( const DLTree* p ) { return p->Element() == AND; }
		/// check whether P is in the form (and C D)
	static bool isAnd ( const DLTree* p )
		{ return p->Element() == NOT && p->Left()->Element() == AND; }

	// single disjunct's optimisations

		/// simplify (OR C ...) for a non-primitive C in a given position
	TAxiom* simplifyPosNP ( unsigned int pos )
	{
		Stat::SAbsSimplify();
		TAxiom* ret = new TAxiom(*this);
		ret->replace ( createSNFNot(clone(getConcept(Disjuncts[pos]->Left())->Description)), pos );
#	ifdef RKG_DEBUG_ABSORPTION
		std::cout << " simplify CN at position " << pos;
#	endif
		return ret;
	}
		/// simplify (OR ~C ...) for a non-primitive C in a given position
	TAxiom* simplifyNegNP ( unsigned int pos )
	{
		Stat::SAbsSimplify();
		TAxiom* ret = new TAxiom(*this);
		ret->replace ( clone(getConcept(Disjuncts[pos])->Description), pos );
#	ifdef RKG_DEBUG_ABSORPTION
		std::cout << " simplify ~CN at position " << pos;
#	endif
		return ret;
	}
		/// simplify (OR (OR ...)) in a given position
	TAxiom* simplifyOr ( unsigned int pos )
	{
		Stat::SAbsFlatten();
		TAxiom* ret = new TAxiom(*this);
		DLTree* pAnd = Disjuncts[pos];
		ret->add(clone(pAnd->Right()));
		ret->replace ( clone(pAnd->Left()), pos );
#	ifdef RKG_DEBUG_ABSORPTION
		std::cout << " simplify OR at position " << pos;
#	endif
		return ret;
	}
		/// split (OR (AND...) ...) in a given position
	void split ( std::vector<TAxiom*>& acc, unsigned int pos, DLTree* pAnd ) const
	{
		if ( pAnd->Element().getToken() == AND )
		{	// split the AND
			split ( acc, pos, pAnd->Left() );
			split ( acc, pos, pAnd->Right() );
		}
		else
		{
			TAxiom* ret = new TAxiom(*this);
			ret->replace ( createSNFNot(clone(pAnd)), pos );
			acc.push_back(ret);
		}
	}

public:		// interface
		/// create an empty GCI
	TAxiom ( void ) {}
		/// create a copy of a given GCI
	TAxiom ( const TAxiom& ax )
	{
		for ( const_iterator i = ax.begin(), i_end = ax.end(); i != i_end; ++i )
			Disjuncts.push_back(clone(*i));
	}
		/// d'tor: delete elements if AX is not in use
	~TAxiom ( void )
	{
		for ( iterator i = begin(), i_end = end(); i != i_end; ++i )
			deleteTree(*i);
	}

		/// add DLTree to an axiom
	void add ( DLTree* p )
	{
		for ( iterator i = begin(), i_end = end(); i != i_end; ++i )
			if ( equalTrees(p,*i) )
			{
				deleteTree(p);
				return;
			}
		gather ( p, Disjuncts.size() );
		Disjuncts.push_back(p);
	}
		/// check whether 2 axioms are the same
	bool operator == ( const TAxiom& ax ) const
	{
#	ifdef RKG_DEBUG_ABSORPTION
//		std::cout << "\n  comparing "; dump(std::cout);
//		std::cout << "  with      "; ax.dump(std::cout);
#	endif
		if ( Disjuncts.size() != ax.Disjuncts.size() )
		{
#		ifdef RKG_DEBUG_ABSORPTION
//			std::cout << "  different size";
#		endif
			return false;
		}
		const_iterator p = begin(), q = ax.begin(), p_end = end();
		for ( ; p != p_end; ++p, ++q )
			if ( !equalTrees(*p,*q) )
			{
#			ifdef RKG_DEBUG_ABSORPTION
//				std::cout << "  different tree:" << *p << " vs" << *q;
#			endif
				return false;
			}
#	ifdef RKG_DEBUG_ABSORPTION
//		std::cout << "  equal!";
#	endif
		return true;
	}

		/// simplify an axiom
	TAxiom* simplify ( void );
		/// split an axiom; @return new axiom and/or NULL
	bool split ( std::vector<TAxiom*>& acc ) const
	{
		acc.clear();
		for ( const_iterator p_beg = begin(), p_end = end(), p = p_beg; p != p_end; ++p )
			if ( isAnd(*p) )
			{
				Stat::SAbsSplit();
				unsigned int pos = p-p_beg;
#			ifdef RKG_DEBUG_ABSORPTION
				std::cout << " split AND at position " << pos;
#			endif
				split ( acc, pos, (*p)->Left() );
				// no need to split more than once:
				// every extra splits would be together with unsplitted parts
				// like: (A or B) and (C or D) would be transform into
				// A and (C or D), B and (C or D), (A or B) and C, (A or B) and D
				// so just return here
				return true;
			}

		return false;
	}
		/// absorb into BOTTOM; @return true if absorption is performed
	bool absorbIntoBottom ( void )
	{
		for ( const_iterator p = begin(), p_end = end(); p != p_end; ++p )
			if ( (*p)->Element().getToken() == BOTTOM )
			{	// axiom in the form T [= T or ...; nothing to do
				Stat::SAbsBApply();
#			ifdef RKG_DEBUG_ABSORPTION
				std::cout << " Absorb into BOTTOM";
#			endif
				return true;
			}

		return false;
	}
		/// absorb into TOP; @return true if absorption is performed
	bool absorbIntoTop ( TBox& KB );
		/// absorb into concept; @return true if absorption is performed
	bool absorbIntoConcept ( TBox& KB );
		/// absorb into role domain; @return true if absorption is performed
	bool absorbIntoDomain ( void );
		/// create a concept expression corresponding to a given GCI; ignore REPLACED entry
	DLTree* createAnAxiom ( const_iterator replaced ) const;
		/// create a concept expression corresponding to a given GCI
	DLTree* createAnAxiom ( void ) const { return createAnAxiom(end());	}

#ifdef RKG_DEBUG_ABSORPTION
		/// dump GCI for debug purposes
	void dump ( std::ostream& o ) const;
#endif
}; // TAxiom;

#endif
