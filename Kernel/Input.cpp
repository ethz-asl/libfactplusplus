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

#include "dlTBox.h"

//-----------------------------------------------------------------------------
//--		Subsumption axioms and support
//-----------------------------------------------------------------------------

// return true if undefined concept found
bool TBox :: addSubsumeAxiom ( DLTree* left, DLTree* right )
{
	// for C [= TOP: nothing to do
	if ( right->Element() == TOP )
	{
		deleteTree(left);
		delete right;
		return false;
	}

	// for BOTTOM [= C: nothing to do
	if ( left->Element() == BOTTOM )
	{
		deleteTree(right);
		delete left;
		return false;
	}

	// if axiom is A\in smth, add to a appr. concept
	if ( left->Element () == CNAME )
	{
		TConcept* p = getConcept (left);

		if ( p == NULL )	// no such concept
			return true;

		// resolve synonyms (if any)
		// FIXME!! this should be done earlier
		if ( p->isSynonym() )
		{
			p = p->resolveSynonym();

			// check whether name is a synonym of a constant (mainly for owl:Thing)
			if ( p == pBottom )
			{	// BOTTOM [= C: nothing to do
				deleteTree(right);
				delete left;
				return false;
			}

			if ( p == pTop )
			{	// TOP [= C; re-run subsumption axiom
				delete left;
				return addSubsumeAxiom ( new DLTree(TOP), right );
			}
		}

		if ( p->isPrimitive() )
		{
			p->addDesc ( right );
			delete left;
			return false;
		}
	}

	// check the case D [= CN, where CN is defined as D
	if ( right->Element() == CNAME )
	{
		TConcept* p = getConcept(right);

		if ( p == NULL )
			return true;

		// resolve synonyms (if any)
		if ( p->isSynonym() )
		{
			p = p->resolveSynonym();

			// check whether name is a synonym of a constant (mainly for owl:Thing)
			if ( p == pBottom )
			{	// C [= BOTTOM: re-run subsumption axiom
				delete right;
				return addSubsumeAxiom ( left, new DLTree(BOTTOM) );
			}

			if ( p == pTop )
			{	// C [= TOP: nothing to do
				deleteTree(left);
				delete right;
				return false;
			}
		}

		/// check for D [= CN with CN [= D (CN [= D) already defined
		if ( equalTrees ( p->Description, left ) )
		{
			deleteTree ( makeNonPrimitive(p,left) );
			delete right;
			return false;
		}
	}

	// check if an axiom looks like T [= \AR.C
	if ( axiomToRangeDomain ( left, right ) )
		return false;

	// else -- general axiom
	return processGCI ( left, right );
}

bool TBox :: addSubsumeAxiom ( TNamedEntry* C, DLTree* right )	// special form: CN [= D
{
	TConcept* p = getConcept ( C );

	if ( p == NULL )	// no such concept
		return true;

	if ( p->isPrimitive() )
	{
		p->addDesc ( right );
		return false;
	}
	else	// general axiom
		return addSubsumeAxiom ( new DLTree ( TLexeme (CNAME, C) ), right );
}

bool TBox :: axiomToRangeDomain ( DLTree* l, DLTree* r )
{
	// applicability check
	if ( useRangeDomain && l->Element() == TOP && r->Element () == FORALL )
	{
		TRole* Role = resolveRole(r->Left());
		assert ( Role != NULL );
		Role->setRange(r->Right());
		// free unused memory
		delete l;
		r->SetRight(NULL);
		delete r;
		return true;
	}
	else
		return false;
}

//-----------------------------------------------------------------------------
//--		Equality axioms and support
//-----------------------------------------------------------------------------

// return true if undefined concept found
bool TBox :: addEqualityAxiom ( DLTree* left, DLTree* right )
{
	assert ( left != NULL && right != NULL );

	// try to make a concept definition LEFT = RIGHT
	if ( addNonprimitiveDefinition ( left, right ) )
		return false;

	// try to make a concept definition LEFT = RIGHT
	if ( addNonprimitiveDefinition ( right, left ) )
		return false;

	if ( switchToNonprimitive ( left, right ) )
		return false;

	if ( switchToNonprimitive ( right, left ) )
		return false;

	/// here either C and D are complex expressions, or definition fails
	bool ret = addSubsumeAxiom ( clone(left), clone(right) );
	if ( !ret )
		ret = addSubsumeAxiom ( right, left );
	return ret;
}

/// tries to add LEFT = RIGHT for the concept LEFT; @return true if OK
bool
TBox :: addNonprimitiveDefinition ( DLTree* left, DLTree* right )
{
	TConcept* C = getConcept(left);

	// not a named concept
	if ( C == NULL || C == pTop || C == pBottom )
		return false;

	// check whether the case is C=D for a (concept-like) D
	TConcept* D = getConcept(right);

	// nothing to do for the case C := D for named concepts C,D with D = C already
	if ( D && D->isSynonym() && D->resolveSynonym() == C )
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
	TConcept* C = getConcept(left);

	// not a named concept
	if ( C == NULL || C == pTop || C == pBottom )
		return false;

	// check whether we process C=D where C is defined as C[=E
	if ( alwaysPreferEquals && C->isPrimitive() )	// change C to C=... with additional GCI C[=x
		return !processGCI ( left, makeNonPrimitive(C,right) );

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
DLTree* TBox :: processOneOf ( const ConceptSet& v )
{
	DLTree* ret = new DLTree(BOTTOM);

	for ( ConceptSet::const_iterator p = v.begin(), p_end = v.end(); p != p_end; ++p )
		if ( ensureNominal(*p) )
		{
			deleteTree(ret);
			return NULL;	// sanity check; FIXME!! exception later on
		}
		else
			ret = createSNFOr ( ret, *p );

	return ret;
}

//-----------------------------------------------------------------------------
//--		N-ary role expressions
//-----------------------------------------------------------------------------

// return Composition (roles)
DLTree* TBox :: processRComposition ( const ConceptSet& v )
{
	if ( v.empty() )
		return NULL;

	ConceptSet::const_iterator p, p_end = v.end();

	// check that all id's are correct role names
	for ( p = v.begin(); p != p_end; ++p )
		if ( isUniversalRole(*p) || resolveRole(*p) == NULL )
			return NULL;

	p = v.begin();
	DLTree* ret = new DLTree ( TLexeme(RNAME,resolveRole(*p)) );

	while ( ++p < p_end )
		ret = new DLTree ( TLexeme(RCOMPOSITION), ret, new DLTree ( TLexeme(RNAME,resolveRole(*p)) ) );

	return ret;
}

//-----------------------------------------------------------------------------
//--		N-ary concept axioms
//-----------------------------------------------------------------------------

bool TBox :: processDisjoint ( const ConceptSet& v )
{
	if ( v.empty() )
		return true;

	ConceptSet prim, rest;

	for ( ConceptSet::const_iterator p = v.begin(), p_end = v.end(); p < p_end; ++p )
		if ( isCN(*p) &&
			 static_cast<const TConcept*>((*p)->Element().getName())->isPrimitive() )
			prim.push_back(*p);
		else
			rest.push_back(*p);

	// both primitive concept and others are in DISJ statement
	if ( !prim.empty() && !rest.empty() )
	{
		DLTree* nrest = buildDisjAux ( rest.begin(), rest.end() );

		for ( ConceptSet::iterator q = prim.begin(), q_end = prim.end(); q < q_end; ++q )
			if ( addSubsumeAxiom ( clone(*q), clone(nrest) ) )
				return true;

		deleteTree(nrest);
	}

	// no primitive concepts between DJ elements
	if ( prim.empty() )
		return processDisjoint ( rest.begin(), rest.end() );

	// all non-PC are done; prim is non-empty
	// FIXME!! do it in more optimal way later
	return processDisjoint ( prim.begin(), prim.end() );
}

bool TBox :: processEquivalent ( const ConceptSet& v )
{
	ConceptSet::const_iterator p = v.begin(), p_end = v.end();

	if ( v.size() < 2 )	// nothing to do
	{
		for ( ; p < p_end; ++p )
			delete *p;

		return false;
	}

	// FIXME!! rewrite it with the only iterator
	for ( ConceptSet::const_iterator q = p+1; q != v.end(); ++p, ++q )
		if ( addEqualityAxiom ( *p, clone(*q) ) )
			return true;

	delete *p;	// delete the last entry
	return false;
}

//-----------------------------------------------------------------------------
//--		N-ary individual axioms
//-----------------------------------------------------------------------------

bool TBox :: processDifferent ( const ConceptSet& v )
{
	SingletonVector acc;
	for ( ConceptSet::const_iterator q = v.begin (); q != v.end(); ++q )
		if ( ensureNominal(*q) )	// only nominals in DIFFERENT command
			return true;
		else
			acc.push_back(static_cast<TConcept*>((*q)->Element().getName()));

	// register vector of disjoint nominals in proper place
	if ( acc.size() < 2 )
		return false;

	Different.push_back(acc);
	return false;
}

bool TBox :: processSame ( const ConceptSet& v )
{
	ConceptSet::const_iterator p = v.begin(), p_end = v.end();

	if ( v.size() < 2 )	// nothing to do
	{
		for ( ; p < p_end; ++p )
			delete *p;

		return false;
	}

	if ( ensureNominal(*p) )	// only nominals in SAME command
		return true;

	// FIXME!! rewrite it with the only iterator
	for ( ConceptSet::const_iterator q = p+1; q < p_end; ++p, ++q )
		if ( ensureNominal(*q) )	// only nominals in SAME command
			return true;
		else	// take an existing p and a copy of q
			if ( addEqualityAxiom ( *p, clone(*q) ) )
				return true;

	delete *p;	// delete the last entry
	return false;
}

//-----------------------------------------------------------------------------
//--		N-ary role axioms
//-----------------------------------------------------------------------------

bool TBox :: processDisjointR ( const ConceptSet& v )
{
	if ( v.empty() )
		return true;

	ConceptSet::const_iterator p, q, p_end = v.end();

	// check that all id's are correct role names
	for ( p = v.begin(); p != p_end; ++p )
		if ( isUniversalRole(*p) || resolveRole(*p) == NULL )
			return true;

	// make a disjoint roles
	for ( p = v.begin(); p != p_end; ++p )
	{
		TRole* r = resolveRole(*p);
		deleteTree(*p);

		// FIXME: this could be done more optimal...
		for ( q = p+1; q != p_end; ++q )
			RM.addDisjointRoles ( r, resolveRole(*q) );
	}

	// all OK
	return false;
}

bool TBox :: processEquivalentR ( const ConceptSet& v )
{
	if ( v.size () < 2 )
		return false;

	bool ret = false;

	for ( ConceptSet::const_iterator p = v.begin(), p_end = v.end()-1; !ret && p != p_end; ++p )
		ret |= RM.addRoleSynonym ( resolveRole(*p), resolveRole(*(p+1)) );

	return ret;
}


