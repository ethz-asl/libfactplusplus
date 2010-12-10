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
	typedef std::vector<const_iterator> WorkSet;

protected:	// members
		/// GCI is presented in the form (or Disjuncts);
	absorptionSet Disjuncts;

protected:	// methods
	// access to labels

		/// RW begin
	iterator begin ( void ) { return Disjuncts.begin(); }
		/// RW end
	iterator end ( void ) { return Disjuncts.end(); }
		/// RO begin
	const_iterator begin ( void ) const { return Disjuncts.begin(); }
		/// RO end
	const_iterator end ( void ) const { return Disjuncts.end(); }

		/// create a copy of a given GCI; ignore SKIP entry
	TAxiom* copy ( const_iterator skip ) const
	{
		TAxiom* ret = new TAxiom();
		for ( const_iterator i = begin(), i_end = end(); i != i_end; ++i )
			if ( i != skip )
				ret->Disjuncts.push_back(clone(*i));
		return ret;
	}

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
		/// check whether P is in the form (and C D)
	static bool isAnd ( const DLTree* p )
		{ return p->Element() == NOT && p->Left()->Element() == AND; }
		/// check whether P is in the form (all R C)
	static bool isForall ( const DLTree* p )
	{
		return p->Element() == NOT && p->Left()->Element() == FORALL
			   && (!isName(p->Left()->Right())
				 	 || !getConcept(p->Left()->Right())->isSystem());
	}

	// single disjunct's optimisations

		/// simplify (OR C ...) for a non-primitive C in a given position
	TAxiom* simplifyPosNP ( const_iterator pos ) const
	{
		Stat::SAbsSimplify();
		TAxiom* ret = copy(pos);
		ret->add(createSNFNot(clone(getConcept((*pos)->Left())->Description)));
#	ifdef RKG_DEBUG_ABSORPTION
		std::cout << " simplify CN expression for" << (*pos)->Left();
#	endif
		return ret;
	}
		/// simplify (OR ~C ...) for a non-primitive C in a given position
	TAxiom* simplifyNegNP ( const_iterator pos ) const
	{
		Stat::SAbsSimplify();
		TAxiom* ret = copy(pos);
		ret->add(clone(getConcept(*pos)->Description));
#	ifdef RKG_DEBUG_ABSORPTION
		std::cout << " simplify ~CN expression for" << *pos;
#	endif
		return ret;
	}
		/// simplify (OR (SOME R C) ...)) in a given position
	TAxiom* simplifyForall ( const_iterator pos, TBox& KB ) const;
		/// split (OR (AND...) ...) in a given position
	void split ( std::vector<TAxiom*>& acc, const_iterator pos, DLTree* pAnd ) const
	{
		if ( pAnd->Element().getToken() == AND )
		{	// split the AND
			split ( acc, pos, pAnd->Left() );
			split ( acc, pos, pAnd->Right() );
		}
		else
		{
			TAxiom* ret = copy(pos);
			ret->add(createSNFNot(clone(pAnd)));
			acc.push_back(ret);
		}
	}
		/// create a concept expression corresponding to a given GCI; ignore SKIP entry
	DLTree* createAnAxiom ( const_iterator skip ) const;

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
	void add ( DLTree* p );
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

		/// replace a defined concept with its description
	TAxiom* simplifyCN ( void ) const;
		/// replace a universal restriction with a fresh concept
	TAxiom* simplifyForall ( TBox& KB ) const;
		/// split an axiom; @return new axiom and/or NULL
	bool split ( std::vector<TAxiom*>& acc ) const
	{
		acc.clear();
		for ( const_iterator p = begin(), p_end = end(); p != p_end; ++p )
			if ( isAnd(*p) )
			{
				Stat::SAbsSplit();
#			ifdef RKG_DEBUG_ABSORPTION
				std::cout << " split AND expression" << (*p)->Left();
#			endif
				split ( acc, p, (*p)->Left() );
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
	bool absorbIntoBottom ( void ) const
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
	bool absorbIntoTop ( TBox& KB ) const;
		/// absorb into concept; @return true if absorption is performed
	bool absorbIntoConcept ( TBox& KB ) const;
		/// absorb into role domain; @return true if absorption is performed
	bool absorbIntoDomain ( void ) const;
		/// create a concept expression corresponding to a given GCI
	DLTree* createAnAxiom ( void ) const { return createAnAxiom(end());	}

#ifdef RKG_DEBUG_ABSORPTION
		/// dump GCI for debug purposes
	void dump ( std::ostream& o ) const;
#endif
}; // TAxiom;

#endif
