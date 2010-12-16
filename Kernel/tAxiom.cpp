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

#include "tAxiom.h"
#include "tRole.h"
#include "dlTBox.h"

/// add DLTree to an axiom
void
TAxiom :: add ( DLTree* p )
{
	if ( InAx::isBot(p) )	// BOT or X == X
		return;	// nothing to do
	// flatten the disjunctions on the fly
	if ( InAx::isOr(p) )
	{
		add(p->Left());
		add(p->Right());
		delete p;	// delete just AND entry, not the trees
		return;
	}
	for ( const_iterator i = begin(), i_end = end(); i != i_end; ++i )
		if ( equalTrees(p,*i) )
		{
			deleteTree(p);
			return;
		}
	Disjuncts.push_back(p);
}

TAxiom*
TAxiom :: simplifyCN ( void ) const
{
	for ( const_iterator i = begin(), i_end = end(); i != i_end; ++i )
	{
		const DLTree* p = *i;

		if ( InAx::isPosNP(p) )
			return simplifyPosNP(p);
		else if ( InAx::isNegNP(p) )
			return simplifyNegNP(p);
	}

	return NULL;
}

TAxiom*
TAxiom :: simplifyForall ( TBox& KB ) const
{
	for ( const_iterator i = begin(), i_end = end(); i != i_end; ++i )
		if ( InAx::isAbsForall(*i) )
			return simplifyForall ( *i, KB );

	return NULL;
}

TAxiom*
TAxiom :: simplifyForall ( const DLTree* rep, TBox& KB ) const
{
	Stat::SAbsRepForall();
	DLTree* pAll = rep->Left();	// (all R ~C)
#ifdef RKG_DEBUG_ABSORPTION
	std::cout << " simplify ALL expression" << pAll;
#endif
	TAxiom* ret = copy(rep);
	ret->add(KB.getTree(KB.replaceForall(clone(pAll))));
	return ret;
}

DLTree*
TAxiom :: createAnAxiom ( const DLTree* skip ) const
{
	// create new OR vertex for the axiom:
	DLTree* Or = createTop();
	for ( const_iterator p = begin(), p_end = end(); p != p_end; ++p )
		if ( *p != skip )
			Or = createSNFAnd ( clone(*p), Or );

	return createSNFNot(Or);
}

#ifdef RKG_DEBUG_ABSORPTION
void TAxiom :: dump ( std::ostream& o ) const
{
	o << " (neg-and";
	for ( const_iterator p = begin(), p_end = end(); p != p_end; ++p )
		o << *p;
	o << ")";
}
#endif

/// absorb into BOTTOM; @return true if absorption is performed
bool
TAxiom :: absorbIntoBottom ( void ) const
{
	absorptionSet Pos, Neg;
	for ( const_iterator p = begin(), p_end = end(); p != p_end; ++p )
		switch ( (*p)->Element().getToken() )
		{
		case BOTTOM:	// axiom in the form T [= T or ...; nothing to do
			Stat::SAbsBApply();
#		ifdef RKG_DEBUG_ABSORPTION
			std::cout << " Absorb into BOTTOM";
#		endif
			return true;
		case TOP:	// skip it here
			break;
		case NOT:	// something negated: put it into NEG
			Neg.push_back((*p)->Left());
			break;
		default:	// something positive: save in POS
			Pos.push_back(*p);
			break;
		}

	// now check whether there is a concept in both POS and NEG
	for ( const_iterator q = Neg.begin(), q_end = Neg.end(); q != q_end; ++q )
		for ( const_iterator s = Pos.begin(), s_end = Pos.end(); s != s_end; ++s )
			if ( equalTrees ( *q, *s ) )
			{
				Stat::SAbsBApply();
#			ifdef RKG_DEBUG_ABSORPTION
				std::cout << " Absorb into BOTTOM due to (not" << *q << ") and" << *s;
#			endif
				return true;
			}
	return false;
}

bool
TAxiom :: absorbIntoTop ( TBox& KB ) const
{
	TConcept* C = NULL;

	// check whether the axiom is Top [= C
	for ( const_iterator p = begin(), p_end = end(); p != p_end; ++p )
		if ( InAx::isBot(*p) )	// BOTTOMS are fine
			continue;
		else if ( InAx::isPosCN(*p) )	// CN found
		{
			if ( C != NULL )	// more than one concept
				return false;
			C = InAx::getConcept((*p)->Left());
			if ( C->isSingleton() )	// doesn't work with nominals
				return false;
		}
		else
			return false;

	if ( C == NULL )
		return false;

	// make an absorption
	Stat::SAbsTApply();
	DLTree* desc = KB.makeNonPrimitive ( C, createTop() );

#ifdef RKG_DEBUG_ABSORPTION
	std::cout << " T-Absorb GCI to axiom";
	if ( desc )
		std::cout << "s *TOP* [=" << desc << " and";
	std::cout << " " << C->getName() << " = *TOP*";
#endif
	if ( desc )
		KB.addSubsumeAxiom ( createTop(), desc );

	return true;
}

bool
TAxiom :: absorbIntoConcept ( TBox& KB ) const
{
	WorkSet Cons;
	DLTree* bestConcept = NULL;

	// finds all primitive concept names
	for ( const_iterator p = begin(), p_end = end(); p != p_end; ++p )
		if ( InAx::isNegPC(*p) )	// FIXME!! review this during implementation of Nominal Absorption
		{
			Stat::SAbsCAttempt();
			Cons.push_back(*p);
			if ( InAx::getConcept(*p)->isSystem() )
				bestConcept = *p;
		}

	// if no concept names -- return;
	if ( Cons.empty() )
		return false;

	Stat::SAbsCApply();
	// FIXME!! as for now: just take the 1st concept name
	if ( bestConcept == NULL )
		bestConcept = Cons[0];

	// normal concept absorption
	TConcept* Concept = InAx::getConcept(bestConcept);

#ifdef RKG_DEBUG_ABSORPTION
	std::cout << " C-Absorb GCI to concept " << Concept->getName();
	if ( Cons.size() > 1 )
	{
		std::cout << " (other options are";
		for ( WorkSet::iterator q = Cons.begin(), q_end = Cons.end(); q != q_end; ++q )
			if ( *q != bestConcept )
				std::cout << " " << InAx::getConcept(*q)->getName();
		std::cout << ")";
	}
#endif

	// adds a new definition
	Concept->addDesc(createAnAxiom(bestConcept));
	Concept->removeSelfFromDescription();
	// in case T [= (A or \neg B) and (B and \neg A) there appears a cycle A [= B [= A
	// so remove potential cycle
	// FIXME!! just because TConcept can't get rid of cycle by itself
	KB.clearRelevanceInfo();
	KB.checkToldCycle(Concept);
	KB.clearRelevanceInfo();

	return true;
}

/// absorb into negation of a concept; @return true if absorption is performed
bool
TAxiom :: absorbIntoNegConcept ( TBox& KB ) const
{
	WorkSet Cons;
	TConcept* Concept;
	const DLTree* bestConcept = NULL;

	// finds all primitive negated concept names without description
	for ( const_iterator p = begin(), p_end = end(); p != p_end; ++p )
		if ( (*p)->Element().getToken() == NOT && isName((*p)->Left())
			 && (Concept = InAx::getConcept((*p)->Left()))->isPrimitive()
			 && !Concept->isSingleton() && Concept->Description == NULL )
		{
			Stat::SAbsNAttempt();
			Cons.push_back(*p);
		}

	// if no concept names -- return;
	if ( Cons.empty() )
		return false;

	Stat::SAbsNApply();
	// FIXME!! as for now: just take the 1st concept name
	if ( bestConcept == NULL )
		bestConcept = Cons[0];

	// normal concept absorption
	Concept = InAx::getConcept(bestConcept->Left());

#ifdef RKG_DEBUG_ABSORPTION
	std::cout << " N-Absorb GCI to concept " << Concept->getName();
	if ( Cons.size() > 1 )
	{
		std::cout << " (other options are";
		for ( WorkSet::iterator q = Cons.begin(), q_end = Cons.end(); q != q_end; ++q )
			if ( *q != bestConcept )
				std::cout << " " << InAx::getConcept((*q)->Left())->getName();
		std::cout << ")";
	}
#endif

	// replace ~C [= D with C=~notC, notC [= D:
	// make notC [= D
	TConcept* nC = KB.getAuxConcept(createAnAxiom(bestConcept));
	// define C = ~notC; C had an empty desc, so it's safe not to delete it
	KB.makeNonPrimitive ( Concept, createSNFNot(KB.getTree(nC)) );
	return true;
}

bool
TAxiom :: absorbIntoDomain ( void ) const
{
	WorkSet Cons;
	const DLTree* bestSome = NULL;

	// find all forall concepts
	for ( const_iterator p = begin(), p_end = end(); p != p_end; ++p )
		if ( (*p)->Element() == NOT &&
			 ( (*p)->Left()->Element() == FORALL	// \neg ER.C
			   || (*p)->Left()->Element() == LE ))	// \neg >= n R.C
		{
			Stat::SAbsRAttempt();
			Cons.push_back(*p);
			// check for the direct domain case
			if ( (*p)->Left()->Right()->Element() == BOTTOM )
			{	// found proper absorption candidate
				bestSome = *p;
				break;
			}
		}

	// if there are no EXISTS concepts -- return;
	if ( Cons.empty() )
		return false;

	Stat::SAbsRApply();
	TRole* Role;

	if ( bestSome != NULL )
		Role = resolveRole(bestSome->Left()->Left());
	else
		// FIXME!! as for now: just take the 1st role name
		Role = resolveRole(Cons[0]->Left()->Left());

#ifdef RKG_DEBUG_ABSORPTION
	std::cout << " R-Absorb GCI to the domain of role " << Role->getName();
	if ( Cons.size() > 1 )
	{
		std::cout << " (other options are";
		for ( WorkSet::iterator q = Cons.begin(), q_end = Cons.end(); q != q_end; ++q )
			if ( *q != bestSome )
				std::cout << " " << resolveRole((*q)->Left()->Left())->getName();
		std::cout << ")";
	}
#endif

	// here bestSome is either actual domain, or END(); both cases are fine
	Role->setDomain(createAnAxiom(bestSome));

	return true;
}
