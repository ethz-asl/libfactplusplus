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
void TBox :: addSubsumeAxiom ( DLTree* left, DLTree* right )
{
	// for C [= C: nothing to do
	if ( equalTrees ( left,right ) )
	{
		deleteTree(left);
		deleteTree(right);
		return;
	}

	// try to apply C [= CN
	if ( isCN(right) )
		if ( applyAxiomCToCN ( left, right ) )
			return;

	// try to apply CN [= C
	if ( isCN(left) )
		if ( applyAxiomCNToC ( left, right ) )
			return;

	// check if an axiom looks like T [= \AR.C
	if ( axiomToRangeDomain ( left, right ) )
		;
	else // general axiom
		processGCI ( left, right );
}

/// tries to apply axiom D [= CN; @return true if applicable
bool
TBox :: applyAxiomCToCN ( DLTree* D, DLTree*& CN )
{
	TConcept* C = resolveSynonym(getCI(CN));
	assert ( C != NULL );

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
	// FIXME!! check for C->isPrimitive()
	else if ( equalTrees ( C->Description, D ) )
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
	assert ( C != NULL );
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

bool TBox :: axiomToRangeDomain ( DLTree* l, DLTree* r )
{
	if ( !useRangeDomain )
		return false;
	// applicability check for T [= A R.C
	if ( l->Element() == TOP && r->Element () == FORALL )
	{
		resolveRole(r->Left())->setRange(r->Right());
		// free unused memory
		delete l;
		r->SetRight(NULL);
		deleteTree(r);
		return true;
	}
	// applicability check for E R.T [= D
	if ( l->Element() == NOT && l->Left()->Element() == FORALL && l->Left()->Right()->Element() == BOTTOM )
	{
		resolveRole(l->Left()->Left())->setDomain(r);
		deleteTree(l);
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
DLTree* TBox :: processAnd ( const ConceptSet& v )
{
	DLTree* ret = new DLTree(TOP);

	for ( ConceptSet::const_iterator p = v.begin(), p_end = v.end(); p != p_end; ++p )
		ret = createSNFAnd ( ret, *p );

	return ret;
}

// return OR (concepts)
DLTree* TBox :: processOr ( const ConceptSet& v )
{
	DLTree* ret = new DLTree(BOTTOM);

	for ( ConceptSet::const_iterator p = v.begin(), p_end = v.end(); p != p_end; ++p )
		ret = createSNFOr ( ret, *p );

	return ret;
}

// return OR (nominals)
DLTree* TBox :: processOneOf ( const ConceptSet& v, bool data )
{
	DLTree* ret = new DLTree(BOTTOM);

	for ( ConceptSet::const_iterator p = v.begin(), p_end = v.end(); p != p_end; ++p )
		if ( (data && isDataValue(*p)) || (!data && isIndividual(*p)) )
			ret = createSNFOr ( ret, *p );
		else
		{
			deleteTree(ret);
			return NULL;	// sanity check; FIXME!! exception later on
		}

	return ret;
}

//-----------------------------------------------------------------------------
//--		N-ary role expressions
//-----------------------------------------------------------------------------

// return Composition (roles)
DLTree* TBox :: processRComposition ( const ConceptSet& v )
{
	if ( v.empty() )
		throw EFaCTPlusPlus("Empty role composition chain");

	ConceptSet::const_iterator p, p_end = v.end();

	// check that all id's are correct role names
	for ( p = v.begin(); p != p_end; ++p )
		if ( isUniversalRole(*p) )
			throw EFaCTPlusPlus("Universal role can not be used in role composition chain");

	p = v.begin();
	DLTree* ret = *p;

	while ( ++p < p_end )
		ret = new DLTree ( TLexeme(RCOMPOSITION), ret, *p );

	return ret;
}

//-----------------------------------------------------------------------------
//--		N-ary concept axioms
//-----------------------------------------------------------------------------

void TBox :: processDisjoint ( const ConceptSet& v )
{
	if ( v.empty() )
		return;

	ConceptSet prim, rest;

	for ( ConceptSet::const_iterator p = v.begin(), p_end = v.end(); p < p_end; ++p )
		if ( isName(*p) &&
			 static_cast<const TConcept*>((*p)->Element().getName())->isPrimitive() )
			prim.push_back(*p);
		else
			rest.push_back(*p);

	// both primitive concept and others are in DISJ statement
	if ( !prim.empty() && !rest.empty() )
	{
		DLTree* nrest = buildDisjAux ( rest.begin(), rest.end() );

		for ( ConceptSet::iterator q = prim.begin(), q_end = prim.end(); q < q_end; ++q )
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

void TBox :: processEquivalent ( const ConceptSet& v )
{
	ConceptSet::const_iterator p = v.begin(), p_end = v.end();

	// FIXME!! rewrite it with the only iterator
	for ( ConceptSet::const_iterator q = p+1; q < p_end; ++p, ++q )
		addEqualityAxiom ( *p, clone(*q) );

	deleteTree(*p);	// delete the last entry
}

//-----------------------------------------------------------------------------
//--		N-ary individual axioms
//-----------------------------------------------------------------------------

void TBox :: processDifferent ( const ConceptSet& v )
{
	SingletonVector acc;
	for ( ConceptSet::const_iterator q = v.begin (); q != v.end(); ++q )
		if ( isIndividual(*q) )	// only nominals in DIFFERENT command
			acc.push_back(static_cast<TIndividual*>((*q)->Element().getName()));
		else
			throw EFaCTPlusPlus("Only individuals allowed in processDifferent()");

	// register vector of disjoint nominals in proper place
	if ( acc.size() > 1 )
		Different.push_back(acc);
}

void TBox :: processSame ( const ConceptSet& v )
{
	ConceptSet::const_iterator p = v.begin(), p_end = v.end();

	if ( !isIndividual(*p) )	// only nominals in SAME command
		throw EFaCTPlusPlus("Only individuals allowed in processSame()");

	// FIXME!! rewrite it with the only iterator
	for ( ConceptSet::const_iterator q = p+1; q < p_end; ++p, ++q )
	{
		if ( !isIndividual(*q) )
			throw EFaCTPlusPlus("Only individuals allowed in processSame()");
		addEqualityAxiom ( *p, clone(*q) );
	}

	delete *p;	// delete the last entry
}

//-----------------------------------------------------------------------------
//--		N-ary role axioms
//-----------------------------------------------------------------------------

void TBox :: processDisjointR ( const ConceptSet& v )
{
	if ( v.empty() )
		throw EFaCTPlusPlus("Empty disjoint role axiom");

	ConceptSet::const_iterator p, q, p_end = v.end();

	// check that all id's are correct role names
	for ( p = v.begin(); p != p_end; ++p )
		if ( isUniversalRole(*p) )
			throw EFaCTPlusPlus("Universal role in the disjoint roles axiom");

	// make a disjoint roles
	for ( p = v.begin(); p != p_end; ++p )
	{
		TRole* r = resolveRole(*p);
		deleteTree(*p);

		// FIXME: this could be done more optimal...
		for ( q = p+1; q != p_end; ++q )
			RM.addDisjointRoles ( r, resolveRole(*q) );
	}
}

void TBox :: processEquivalentR ( const ConceptSet& v )
{
	if ( v.size () > 1 )
		for ( ConceptSet::const_iterator p = v.begin(), p_end = v.end()-1; p != p_end; ++p )
			RM.addRoleSynonym ( resolveRole(*p), resolveRole(*(p+1)) );
}


