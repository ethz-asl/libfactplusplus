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

#include "RoleMaster.h"

// inverse the role composition
DLTree* inverseComposition ( const DLTree* tree )
{
	if ( tree->Element() == RCOMPOSITION )
		return new DLTree ( TLexeme(RCOMPOSITION),
							inverseComposition(tree->Right()),
							inverseComposition(tree->Left()) );
	else
		return new DLTree ( TLexeme ( RNAME, new TTreeNamedEntry(resolveRole(tree)->inverse()) ) );
}

void RoleMaster :: addRoleParent ( const DLTree* tree, TRole* parent )
{
	if ( !tree )	// nothing to do
		return;
	if ( tree->Element() == RCOMPOSITION )
	{
		parent->addComposition(tree);
		DLTree* inv = inverseComposition(tree);
		parent->inverse()->addComposition(inv);
		deleteTree(inv);
	}
	else if ( tree->Element() == PROJINTO )
	{
		// here -R->C became -PARENT->
		// encode this as PROJFROM(R-,PROJINTO(PARENT-,C)),
		// added to the range of R
		TRole* R = resolveRole(tree->Left());
		// can't do anything ATM for the data roles
		if ( R->isDataRole() )
			throw EFaCTPlusPlus("Projection into not implemented for the data role");
		DLTree* C = clone(tree->Right());
		DLTree* InvP = new DLTree ( TLexeme ( RNAME, new TTreeNamedEntry(parent->inverse()) ) );
		DLTree* InvR = new DLTree ( TLexeme ( RNAME, new TTreeNamedEntry(R->inverse()) ) );
		R->setRange ( new DLTree ( PROJFROM, InvR, new DLTree ( PROJINTO, InvP, C ) ) );
	}
	else if ( tree->Element() == PROJFROM )
	{
		// here C-R-> became -PARENT->
		// encode this as PROJFROM(R,PROJINTO(PARENT,C)),
		// added to the domain of R
		TRole* R = resolveRole(tree->Left());
		DLTree* C = clone(tree->Right());
		DLTree* P = new DLTree ( TLexeme ( RNAME, new TTreeNamedEntry(parent) ) );
		R->setDomain ( new DLTree ( PROJFROM, clone(tree->Left()), new DLTree ( PROJINTO, P, C ) ) );
	}
	else
		addRoleParent ( resolveRole(tree), parent );
}

void RoleMaster :: initAncDesc ( void )
{
	iterator p, p_end = end();

	// stage 0.1: eliminate told cycles
	for ( p = begin(); p != p_end; ++p )
		(*p)->eliminateToldCycles();	// not VERY efficient: quadratic vs (possible) linear

	// setting up all synonyms
	for ( p = begin(); p != p_end; ++p )
		if ( (*p)->isSynonym() )
		{
			(*p)->canonicaliseSynonym();
			(*p)->addFeaturesToSynonym();
		}

	// change all parents that are synonyms to their primers
	for ( p = begin(); p != p_end; ++p )
		if ( !(*p)->isSynonym() )
			(*p)->removeSynonymsFromParents();

	// preprocess all role compositions
	for ( p = begin(); p < p_end; ++p )
		if ( !(*p)->isSynonym() )
			(*p)->preprocessAllCompositions();

	// stage 2: perform classification
	pTax->setCompletelyDefined(true);

	for ( p = begin(); p != p_end; ++p )
		if ( !(*p)->isClassified() )
			pTax->classifyEntry(*p);

	// stage 3: fills ancestor/descendants using taxonomy
	for ( p = begin(); p != p_end; ++p )
		if ( !(*p)->isSynonym() )
			(*p)->initADbyTaxonomy(Roles.size());

	// complete role automaton's info
	for ( p = begin(); p != p_end; ++p )
		if ( !(*p)->isSynonym() )
			(*p)->completeAutomaton();

	// prepare taxonomy to the real usage
	pTax->finalise();

	// stage 3.5: apply Disjoint axioms to roles; check and correct disjoints in hierarchy
	if ( !DJRolesA.empty() )
	{
		for ( iterator q = DJRolesA.begin(), q_end = DJRolesA.end(), r = DJRolesB.begin();
			  q != q_end; ++q, ++r )
		{
			TRole* R = resolveSynonym(*q);
			TRole* S = resolveSynonym(*r);
			R->addDisjointRole(S);
			S->addDisjointRole(R);
			R->inverse()->addDisjointRole(S->inverse());
			S->inverse()->addDisjointRole(R->inverse());
		}

		for ( p = begin(); p != p_end; ++p )
			if ( !(*p)->isSynonym() && (*p)->isDisjoint() )
				(*p)->checkHierarchicalDisjoint();
	}

	// stage 4: init other fields for the roles. The whole hierarchy is known here
	for ( p = begin(); p != p_end; ++p )
		if ( !(*p)->isSynonym() )
			(*p)->postProcess();

	// the last stage: check whether all roles are consistent
	for ( p = begin(); p != p_end; ++p )
		if ( !(*p)->isSynonym() )
			(*p)->consistent();
}
