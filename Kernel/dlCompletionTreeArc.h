/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2008 by Dmitry Tsarkov

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

#ifndef _DLCOMPLETIONTREEARC_H
#define _DLCOMPLETIONTREEARC_H

#include <iostream>

#include "globaldef.h"
#include "DeletelessAllocator.h"
#include "DepSet.h"
#include "tRole.h"
#include "tRestorer.h"

//#include "SmallObj.h"

class DlCompletionTree;

class DlCompletionTreeArc//: public Loki::SmallObject<>
{
friend class DlCompletionGraph;
public:		// external type definitions
		/// type for the edges allocator
	typedef DeletelessAllocator<DlCompletionTreeArc> EdgeAllocator;

protected:	// members
		/// pointer to "to" node
	DlCompletionTree* Node;
		/// role, labelling given arc
	const TRole* Role;
		/// dep-set of the arc
	DepSet depSet;
		/// pointer to reverse arc
	DlCompletionTreeArc* Reverse;

private:	// no copy
		/// init an arc with R as a label and NODE on given LEVEL; use it inside MAKEARCS only
	void init ( const TRole* role, const DepSet& dep, DlCompletionTree* node )
	{
		Role = role;
		depSet = dep;
		Node = node;
		Reverse = NULL;
	}
		/// no copy c'tor
	DlCompletionTreeArc ( const DlCompletionTreeArc& v );
		/// no assignment
	DlCompletionTreeArc& operator = ( const DlCompletionTreeArc& v );

protected:	// classes
		/// class for restoring edge
	class TCTEdgeRestorer: public TRestorer
	{
	protected:
		DlCompletionTreeArc* p;
		const TRole* r;
	public:
		TCTEdgeRestorer ( DlCompletionTreeArc* q ) : p(q), r(q->Role) {}
		virtual ~TCTEdgeRestorer ( void ) {}
		void restore ( void ) { p->Role = r; p->Reverse->Role = r->inverse(); }
	}; // TCTEdgeRestorer

		/// class for restoring dep-set
	class TCTEdgeDepRestorer: public TRestorer
	{
	protected:
		DlCompletionTreeArc* p;
		DepSet dep;
	public:
		TCTEdgeDepRestorer ( DlCompletionTreeArc* q ) : p(q), dep(q->getDep()) {}
		virtual ~TCTEdgeDepRestorer ( void ) {}
		void restore ( void ) { p->depSet = dep; }
	}; // TCTEdgeDepRestorer

protected:	// methods

		/// set given arc as a reverse of current
	void setReverse ( DlCompletionTreeArc* v )
	{
		Reverse = v;
		v->Reverse = this;
	}

public:		// interface
		/// empty c'tor
	DlCompletionTreeArc ( void ) {}
		/// d'tor
	~DlCompletionTreeArc ( void ) {}

		/// get label of the edge
	const TRole* getRole ( void ) const { return Role; }
		/// get dep-set of the edge
	const DepSet& getDep ( void ) const { return depSet; }

		/// get (RW) access to the end of arc
	DlCompletionTree* getArcEnd ( void ) const { return Node; }
		/// get access to reverse arc
	DlCompletionTreeArc* getReverse ( void ) const { return Reverse; }

		/// check if arc is labelled by a super-role of PROLE
	bool isNeighbour ( const TRole* pRole ) const
	{
		return !isIBlocked() && ( *pRole >= *getRole() );
	}
		/// same as above; fills DEP with current DEPSET if so
	bool isNeighbour ( const TRole* pRole, DepSet& dep ) const
	{
		if ( isNeighbour(pRole) )
		{
			dep = depSet;
			return true;
		}
		return false;
	}

		/// is arc merged to another
	bool isIBlocked ( void ) const { return (Role == NULL); }
		/// check whether the edge is reflexive
	bool isReflexiveEdge ( void ) const { return getArcEnd() == getReverse()->getArcEnd(); }

	//----------------------------------------------
	// saving/restoring
	//----------------------------------------------

		/// save and invalidate arc (together with reverse arc)
	TRestorer* save ( void )
	{
		if ( Role == NULL )	// don't invalidate edge twice
			return NULL;

		TRestorer* ret = new TCTEdgeRestorer(this);
		Role = NULL;
		Reverse->Role = NULL;
		return ret;
	}

		/// add dep-set to an edge; return restorer
	TRestorer* addDep ( const DepSet& dep )
	{
		if ( dep.empty() )
			return NULL;
		TRestorer* ret = new TCTEdgeDepRestorer(this);
		depSet.add(dep);
		return ret;
	}

	// output

		/// print current arc
	void Print ( std::ostream& o ) const
		{ o << "<" << ( isIBlocked() ? "-" : Role->getName() ) << depSet << ">"; }
}; // DlCompletionTreeArc

#endif // _DLCOMPLETIONTREEARC_H
