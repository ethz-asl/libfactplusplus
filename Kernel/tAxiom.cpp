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

bool TAxiom :: simplify ( void )
{
	for ( unsigned int i = 0; i < Disjuncts.size(); ++i )
	{
		DLTree* p = Disjuncts[i];

		if ( isPosNP(p) )
			simplifyPosNP(i);
		else if ( isNegNP(p) )
			simplifyNegNP(i);
		else if ( isOr(p) )
			simplifyOr(i);
		else
			continue;

		// something was simplified
		return true;
	}

	return false;
}

DLTree*
TAxiom :: createAnAxiom ( const_iterator replaced ) const
{
	fpp_assert ( !Disjuncts.empty() );	// could not create an axiom from empty absorption set

	// create new OR vertex for the axiom:
	DLTree* Or = createTop();
	for ( const_iterator p = begin(), p_end = end(); p != p_end; ++p )
		if ( p != replaced )
			Or = createSNFAnd ( clone(*p), Or );

	fpp_assert ( isSNF(Or) );	// safety check for G

	return createSNFNot(Or);
}

#ifdef RKG_DEBUG_ABSORPTION
void TAxiom :: dump ( std::ostream& o ) const
{
	o << "(neg-and";
	for ( const_iterator p = begin(), p_end = end(); p != p_end; ++p )
		o << *p;
	o << ")" << std::endl;
}
#endif

bool
TAxiom :: absorbIntoTop ( TBox& KB )
{
	TConcept* C = NULL;

	// check whether the axiom is Top [= C
	for ( iterator p = begin(); p != end(); ++p )
		if ( (*p)->Element().getToken() == TOP )	// TOP here is fine
			continue;
		else if ( (*p)->Element() == NOT && isName((*p)->Left()) )	// C found
		{
			if ( C != NULL )	// more than one concept
				return false;
			C = getConcept((*p)->Left());
			if ( C->isSingleton() )	// doesn't work with nominals
				return false;
		}
		else
			return false;

	if ( C == NULL )
		return false;

	// make an absorption
	DLTree* desc = KB.makeNonPrimitive ( C, createTop() );

#ifdef RKG_DEBUG_ABSORPTION
	std::cout << " T-Absorb GCI to axiom";
	if ( desc )
		std::cout << "s *TOP* [=" << desc << " and";
	std::cout << " " << C->getName() << " = *TOP*: ";
	dump(std::cout);
#endif
	if ( desc )
		KB.addSubsumeAxiom ( createTop(), desc );

	return true;
}

bool
TAxiom :: absorbIntoConcept ( TBox& KB )
{
	WorkSet Cons;

	// finds all primitive concept names
	for ( iterator p = begin(); p != end(); ++p )
		if ( isName(*p) &&		// FIXME!! review this during implementation of Nominal Absorption
			 getConcept(*p)->isPrimitive() )
		{
			Stat::SAbsCAttempt();
			Cons.push_back(p);
		}

	// if no concept names -- return;
	if ( Cons.empty() )
		return false;

	Stat::SAbsCApply();
	// FIXME!! as for now: just take the 1st concept name
	iterator bestConcept = Cons[0];

	// locate concept
	TConcept* Concept = getConcept(*bestConcept);

#ifdef RKG_DEBUG_ABSORPTION
	std::cout << " C-Absorb GCI to concept " << Concept->getName() << ": ";
	dump(std::cout);
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

bool
TAxiom :: absorbIntoDomain ( void )
{
	std::vector<iterator> Cons;
	iterator bestSome = end();

	// find all forall concepts
	for ( iterator p = begin(); p != end(); ++p )
		if ( (*p)->Element() == NOT &&
			 ( (*p)->Left()->Element() == FORALL	// \neg ER.C
			   || (*p)->Left()->Element() == LE ))	// \neg >= n R.C
		{
			Stat::SAbsRAttempt();
			Cons.push_back(p);
			// check for the direct domain case
			if ( (*p)->Left()->Right()->Element() == BOTTOM )
			{	// found proper absorption candidate
				bestSome = p;
				break;
			}
		}

	// if there are no EXISTS concepts -- return;
	if ( Cons.empty() )
		return false;

	Stat::SAbsRApply();
	TRole* Role;

	if ( bestSome != end() )
		Role = resolveRole ( (*bestSome)->Left()->Left() );
	else
		// FIXME!! as for now: just take the 1st concept name
		Role = resolveRole ( (*Cons[0])->Left()->Left() );

#ifdef RKG_DEBUG_ABSORPTION
	std::cout << " R-Absorb GCI to the domain of role " << Role->getName() << ": ";
	dump(std::cout);
#endif

	// here bestSome is either actual domain, or END(); both cases are fine
	Role->setDomain(createAnAxiom(bestSome));

	return true;
}
