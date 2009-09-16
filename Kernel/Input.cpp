/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2009 by Dmitry Tsarkov

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

#include "dlTBox.h"

//-----------------------------------------------------------------------------
//--		Subsumption axioms and support
//-----------------------------------------------------------------------------

// return true if undefined concept found
void TBox :: addSubsumeAxiom ( DLTree* sub, DLTree* sup )
{
	// for C [= C: nothing to do
	if ( equalTrees ( sub, sup ) )
	{
		deleteTree(sub);
		deleteTree(sup);
		return;
	}

	// try to apply C [= CN
	if ( isCN(sup) )
		if ( applyAxiomCToCN ( sub, sup ) )
			return;

	// try to apply CN [= C
	if ( isCN(sub) )
		if ( applyAxiomCNToC ( sub, sup ) )
			return;

	// check if an axiom looks like T [= \AR.C
	if ( axiomToRangeDomain ( sub, sup ) )
		;
	else // general axiom
		processGCI ( sub, sup );
}

/// tries to apply axiom D [= CN; @return true if applicable
bool
TBox :: applyAxiomCToCN ( DLTree* D, DLTree*& CN )
{
	TConcept* C = resolveSynonym(getCI(CN));
	fpp_assert ( C != NULL );

	// check whether name is a synonym of a constant (mainly for owl:Thing)
	if ( C == pBottom )
	{
		deleteTree(CN);
		CN = new DLTree(BOTTOM);
		return false;
	}

	// D [= TOP: nothing to do
	if ( C == pTop )
		deleteTree(D);
	// check for D [= CN with CN [= D already defined
	// don't do this for D is a DN and C is an individual as cycle detection will do it better
	// FIXME!! check for C->isPrimitive()
	else if ( equalTrees ( C->Description, D ) && !( C->isSingleton() && isName(D) ) )
		deleteTree ( makeNonPrimitive(C,D) );
	else	// n/a
		return false;

	deleteTree(CN);
	return true;
}

/// tries to apply axiom CN [= D; @return true if applicable
bool
TBox :: applyAxiomCNToC ( DLTree*& CN, DLTree* D )
{
	TConcept* C = resolveSynonym(getCI(CN));
	fpp_assert ( C != NULL );
	// TOP [= D: n/a
	if ( C == pTop )
	{
		deleteTree(CN);
		CN = new DLTree(TOP);
		return false;
	}

	// BOTTOM [= D: nothing to do
	if ( C == pBottom )
		deleteTree(D);
	else if ( C->isPrimitive() )
		C->addDesc(D);
	else	// C is defined
		addSubsumeForDefined ( C, D );

	deleteTree(CN);
	return true;
}

/// add an axiom CN [= D for defined CN (CN=E already in base)
void
TBox :: addSubsumeForDefined ( TConcept* C, DLTree* D )
{
	// if D is a syntactic sub-class of E, then nothing to do
	if ( isSubTree ( D, C->Description ) )
	{
		deleteTree(D);
		return;
	}
	DLTree* oldDesc = clone(C->Description);
	// try to see whether C contains a reference to itself at the top level
	C->removeSelfFromDescription();
	if ( equalTrees ( oldDesc, C->Description ) )
	{
		processGCI ( oldDesc, D );
		return;
	}

	// note that we don't know exact semantics of C for now;
	// we need to split it's definition and work via GCIs
	C->setPrimitive();	// now we have C [= B
	C->addDesc(D);		// here C [= (B and D)
	// all we need is to add (old C's desc) [= C
	addSubsumeAxiom ( oldDesc, getTree(C) );
}

bool TBox :: axiomToRangeDomain ( DLTree* sub, DLTree* sup )
{
	if ( !useRangeDomain )
		return false;
	// applicability check for T [= A sup.C
	if ( sub->Element() == TOP && sup->Element () == FORALL )
	{
		resolveRole(sup->Left())->setRange(sup->Right());
		// free unused memory
		delete sub;
		sup->SetRight(NULL);
		deleteTree(sup);
		return true;
	}
	// applicability check for E sup.T [= D
	if ( sub->Element() == NOT && sub->Left()->Element() == FORALL && sub->Left()->Right()->Element() == BOTTOM )
	{
		resolveRole(sub->Left()->Left())->setDomain(sup);
		deleteTree(sub);
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
//--		Equality axioms and support
//-----------------------------------------------------------------------------

// return true if undefined concept found
void
TBox :: addEqualityAxiom ( DLTree* left, DLTree* right )
{
	// try to make a concept definition LEFT = RIGHT
	if ( addNonprimitiveDefinition ( left, right ) )
		return;

	// try to make a concept definition RIGHT = LEFT
	if ( addNonprimitiveDefinition ( right, left ) )
		return;

	if ( switchToNonprimitive ( left, right ) )
		return;

	if ( switchToNonprimitive ( right, left ) )
		return;

	/// here either C and D are complex expressions, or definition fails
	addSubsumeAxiom ( clone(left), clone(right) );
	addSubsumeAxiom ( right, left );
}

/// tries to add LEFT = RIGHT for the concept LEFT; @return true if OK
bool
TBox :: addNonprimitiveDefinition ( DLTree* left, DLTree* right )
{
	TConcept* C = resolveSynonym(getCI(left));

	// not a named concept
	if ( C == NULL || C == pTop || C == pBottom )
		return false;

	// check whether the case is C=D for a (concept-like) D
	TConcept* D = getCI(right);

	// nothing to do for the case C := D for named concepts C,D with D = C already
	if ( D && resolveSynonym(D) == C )
	{
		delete left;
		delete right;
		return true;
	}

	// can't have C=D where C is a nominal and D is a concept
	if ( C->isSingleton() && D != NULL && !D->isSingleton() )
		return false;

	// if axiom is in form C=... or C=D, D [= ...
	if ( D == NULL || C->Description == NULL || D->isPrimitive() )
	{	// try to define C
		if ( !initNonPrimitive ( C, right ) )
		{
			delete left;
			return true;
		}
	}

	// can't make definition
	return false;
}

/// tries to add LEFT = RIGHT for the concept LEFT [= X; @return true if OK
bool
TBox :: switchToNonprimitive ( DLTree* left, DLTree* right )
{
	TConcept* C = resolveSynonym(getCI(left));

	// not a named concept
	if ( C == NULL || C == pTop || C == pBottom )
		return false;

	// make sure that we avoid making an individual equals to smth-else
	TConcept* D = resolveSynonym(getCI(right));
	if ( C->isSingleton() && D && !D->isSingleton() )
		return false;

	// check whether we process C=D where C is defined as C[=E
	if ( alwaysPreferEquals && C->isPrimitive() )	// change C to C=... with additional GCI C[=x
	{
		delete left;
		addSubsumeForDefined ( C, makeNonPrimitive(C,right) );
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
//--		N-ary concept expressions
//-----------------------------------------------------------------------------

// return AND (concepts)
DLTree* TBox :: processAnd ( const ExpressionArray& v )
{
	DLTree* ret = new DLTree(TOP);

	for ( ea_iterator p = v.begin(), p_end = v.end(); p != p_end; ++p )
		ret = createSNFAnd ( ret, *p );

	return ret;
}

// return OR (concepts)
DLTree* TBox :: processOr ( const ExpressionArray& v )
{
	DLTree* ret = new DLTree(BOTTOM);

	for ( ea_iterator p = v.begin(), p_end = v.end(); p != p_end; ++p )
		ret = createSNFOr ( ret, *p );

	return ret;
}

// return OR (nominals)
DLTree* TBox :: processOneOf ( const ExpressionArray& v, bool data )
{
	DLTree* ret = new DLTree(BOTTOM);

	for ( ea_iterator p = v.begin(), p_end = v.end(); p != p_end; ++p )
		if ( (data && isDataValue(*p)) || (!data && (*p)->Element().getToken() == INAME) )
			ret = createSNFOr ( ret, *p );
		else
		{
			deleteTree(ret);
			if ( data )
				throw EFaCTPlusPlus("Only data values in DataOneOf()");
			else
				throw EFaCTPlusPlus("Only individuals in OneOf()");
		}

	return ret;
}

//-----------------------------------------------------------------------------
//--		N-ary role expressions
//-----------------------------------------------------------------------------

// return Composition (roles)
DLTree* TBox :: processRComposition ( const ExpressionArray& v )
{
	if ( v.empty() )
		throw EFaCTPlusPlus("Empty role composition chain");

	ea_iterator p = v.begin(), p_end = v.end();
	DLTree* ret = *p;

	while ( ++p < p_end )
		ret = new DLTree ( TLexeme(RCOMPOSITION), ret, *p );

	return ret;
}

//-----------------------------------------------------------------------------
//--		N-ary concept axioms
//-----------------------------------------------------------------------------

void TBox :: processDisjointC ( ea_iterator beg, ea_iterator end )
{
	ExpressionArray prim, rest;

	for ( ; beg < end; ++beg )
		if ( isName(*beg) &&
			 static_cast<const TConcept*>((*beg)->Element().getNE())->isPrimitive() )
			prim.push_back(clone(*beg));
		else
			rest.push_back(clone(*beg));

	// both primitive concept and others are in DISJ statement
	if ( !prim.empty() && !rest.empty() )
	{
		DLTree* nrest = buildDisjAux ( rest.begin(), rest.end() );

		for ( ea_iterator q = prim.begin(), q_end = prim.end(); q < q_end; ++q )
			addSubsumeAxiom ( clone(*q), clone(nrest) );

		deleteTree(nrest);
	}

	// no primitive concepts between DJ elements

	if ( !rest.empty() )
		processDisjoint ( rest.begin(), rest.end() );

	// all non-PC are done; prim is non-empty
	// FIXME!! do it in more optimal way later
	if ( !prim.empty() )
		processDisjoint ( prim.begin(), prim.end() );
}

void TBox :: processEquivalentC ( ea_iterator beg, ea_iterator end )
{
	// FIXME!! rewrite it with the only iterator
	for ( ea_iterator q = beg+1; q < end; ++beg, ++q )
		addEqualityAxiom ( clone(*beg), clone(*q) );
}

//-----------------------------------------------------------------------------
//--		N-ary individual axioms
//-----------------------------------------------------------------------------

void TBox :: processDifferent ( ea_iterator beg, ea_iterator end )
{
	SingletonVector acc;
	for ( ; beg < end; ++beg )
		if ( isIndividual(*beg) )	// only nominals in DIFFERENT command
			acc.push_back(static_cast<TIndividual*>((*beg)->Element().getNE()));
		else
			throw EFaCTPlusPlus("Only individuals allowed in processDifferent()");

	// register vector of disjoint nominals in proper place
	if ( acc.size() > 1 )
		Different.push_back(acc);
}

void TBox :: processSame ( ea_iterator beg, ea_iterator end )
{
	if ( beg == end )
		return;

	if ( !isIndividual(*beg) )	// only nominals in SAME command
		throw EFaCTPlusPlus("Only individuals allowed in processSame()");

	// FIXME!! rewrite it with the only iterator
	for ( ea_iterator q = beg+1; q < end; ++beg, ++q )
	{
		if ( !isIndividual(*q) )
			throw EFaCTPlusPlus("Only individuals allowed in processSame()");
		addEqualityAxiom ( clone(*beg), clone(*q) );
	}
}

//-----------------------------------------------------------------------------
//--		N-ary role axioms
//-----------------------------------------------------------------------------

void TBox :: processDisjointR ( ea_iterator beg, ea_iterator end )
{
	if ( beg == end )
		throw EFaCTPlusPlus("Empty disjoint role axiom");

	ea_iterator p, q;

	// check that all id's are correct role names
	for ( p = beg; p < end; ++p )
		if ( isUniversalRole(*p) )
			throw EFaCTPlusPlus("Universal role in the disjoint roles axiom");

	RoleMaster* RM = getRM(resolveRole(*beg));

	// make a disjoint roles
	for ( p = beg; p < end; ++p )
	{
		TRole* r = resolveRole(*p);

		// FIXME: this could be done more optimal...
		for ( q = p+1; q < end; ++q )
			RM->addDisjointRoles ( r, resolveRole(*q) );
	}
}

void TBox :: processEquivalentR ( ea_iterator beg, ea_iterator end )
{
	if ( beg != end )
	{
		RoleMaster& RM = resolveRole(*beg)->isDataRole() ? DRM : ORM;
		for ( ; beg != end-1; ++beg )
			RM.addRoleSynonym ( resolveRole(*beg), resolveRole(*(beg+1)) );
	}
}


