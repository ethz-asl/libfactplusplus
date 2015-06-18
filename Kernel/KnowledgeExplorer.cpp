/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2011-2015 by Dmitry Tsarkov

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

#include "KnowledgeExplorer.h"
#include "dlCompletionTree.h"
#include "dlTBox.h"

		/// init c'tor
KnowledgeExplorer :: KnowledgeExplorer ( const TBox* box, TExpressionManager* pEM )
	: D2I ( box->getDag(), pEM )
{
	// init all concepts
	for ( TBox::c_const_iterator c = box->c_begin(), c_end = box->c_end(); c != c_end; ++c )
		addE ( Cs, *c );
	// init all individuals
	for ( TBox::i_const_iterator i = box->i_begin(), i_end = box->i_end(); i != i_end; ++i )
		addE ( Is, *i );
	RoleMaster::const_iterator r, r_end;
	// init all object roles
	for ( r = box->getORM()->begin(), r_end = box->getORM()->end(); r != r_end; ++r )
	{
		TRole* R = *r;
		addE ( ORs, R );
		for ( TRole::const_iterator p = R->begin_anc(), p_end = R->end_anc(); p != p_end; ++p )
			ORs.add ( R, *p );
	}
	// init all data roles
	for ( r = box->getDRM()->begin(), r_end = box->getDRM()->end(); r != r_end; ++r )
	{
		TRole* R = *r;
		addE ( DRs, R );
		for ( TRole::const_iterator p = R->begin_anc(), p_end = R->end_anc(); p != p_end; ++p )
			DRs.add ( R, *p );
	}
}

/// add concept-like entity E (possibly with synonyms) to CONCEPTS
void
KnowledgeExplorer :: addC ( const TDLExpression* e )
{
	// check named concepts
	const TDLConceptName* C = dynamic_cast<const TDLConceptName*>(e);
	if ( C != nullptr )
	{
		for ( EE2Map<TDLConceptName>::iterator p = Cs.begin(C), p_end = Cs.end(C); p != p_end; ++p )
			if ( unlikely(*p == nullptr) )
				std::cerr << "Null found while processing class " << C->getName() << "\n";
			else
				Concepts.push_back(*p);
		return;
	}
	// check named individuals
	const TDLIndividualName* I = dynamic_cast<const TDLIndividualName*>(e);
	if ( I != nullptr )
	{
		for ( EE2Map<TDLIndividualName>::iterator p = Is.begin(I), p_end = Is.end(I); p != p_end; ++p )
			if ( unlikely(*p == nullptr) )
				std::cerr << "Null found while processing individual " << I->getName() << "\n";
			else
				Concepts.push_back(*p);
		return;
	}
	Concepts.push_back(e);
}

// couldn't get it as a method, so
#define addAll(S) do { Roles.insert(S.begin(),S.end()); } while(0)

const KnowledgeExplorer::TCGRoleSet&
KnowledgeExplorer :: getDataRoles ( const TCGNode* node, bool onlyDet )
{
	Roles.clear();
	for ( DlCompletionTree::const_edge_iterator p = node->begin(), p_end = node->end(); p != p_end; ++p )
		if ( likely(!(*p)->isIBlocked()) && (*p)->getArcEnd()->isDataNode() && (!onlyDet || (*p)->getDep().empty()) )
			addAll(DRs.get((*p)->getRole()->getEntity()));
	return Roles;
}
/// build the set of object neighbours of a NODE; incoming edges are counted iff NEEDINCOMING is true
const KnowledgeExplorer::TCGRoleSet&
KnowledgeExplorer :: getObjectRoles ( const TCGNode* node, bool onlyDet, bool needIncoming )
{
	Roles.clear();
	for ( DlCompletionTree::const_edge_iterator p = node->begin(), p_end = node->end(); p != p_end; ++p )
		if ( likely(!(*p)->isIBlocked()) && !(*p)->getArcEnd()->isDataNode() && (!onlyDet || (*p)->getDep().empty()) && (needIncoming || (*p)->isSuccEdge() ) )
			addAll(ORs.get((*p)->getRole()->getEntity()));
	return Roles;
}
/// build the set of neighbours of a NODE via role ROLE; put the resulting list into RESULT
const KnowledgeExplorer::TCGNodeVec&
KnowledgeExplorer :: getNeighbours ( const TCGNode* node, const TRole* R )
{
	Nodes.clear();
	for ( DlCompletionTree::const_edge_iterator p = node->begin(), p_end = node->end(); p != p_end; ++p )
		if ( likely(!(*p)->isIBlocked()) && (*p)->isNeighbour(R) )
			Nodes.push_back((*p)->getArcEnd());
	return Nodes;
}
/// put into RESULT all the data expressions from the NODE label
const KnowledgeExplorer::TCGItemVec&
KnowledgeExplorer :: getLabel ( const TCGNode* node, bool onlyDet )
{
	// prepare D2I translator
	D2I.ensureDagSize();
	DlCompletionTree::const_label_iterator p, p_end;
	bool data = node->isDataNode();
	Concepts.clear();
	for ( p = node->beginl_sc(), p_end = node->endl_sc(); p != p_end; ++p )
		if ( !onlyDet || p->getDep().empty() )
			addC(D2I.getExpr(p->bp(),data));
	for ( p = node->beginl_cc(), p_end = node->endl_cc(); p != p_end; ++p )
		if ( !onlyDet || p->getDep().empty() )
			addC(D2I.getExpr(p->bp(),data));
	return Concepts;
}

/// @return blocker of a blocked node NODE or NULL if node is not blocked
const KnowledgeExplorer::TCGNode*
KnowledgeExplorer :: getBlocker ( const TCGNode* node ) const { return node->getBlocker(); }
