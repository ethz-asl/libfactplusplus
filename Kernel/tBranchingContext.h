/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2011 by Dmitry Tsarkov

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

#ifndef TBRANCHINGCONTEXT_H
#define TBRANCHINGCONTEXT_H

#include "dlCompletionTree.h"

	/// class for saving branching context of a Reasoner
class BranchingContext
{
private:	// prevent copy
		/// no copy c'tor
	BranchingContext ( const BranchingContext& s );
		/// no assignment
	BranchingContext& operator = ( const BranchingContext& s );

public:		// members
		/// currently processed node
	DlCompletionTree* curNode;
		/// currently processed concept
	ConceptWDep curConcept;
		/// positions of the Used members
	size_t pUsedIndex, nUsedIndex;
		/// size of a session GCIs vector
	size_t SGsize;
		/// dependences for branching clashes
	DepSet branchDep;

public:		// interface
		/// empty c'tor
	BranchingContext ( void ) : curNode (NULL), curConcept (bpINVALID) {}
		/// empty d'tor
	virtual ~BranchingContext ( void ) {}

		/// init indeces (if necessary)
	virtual void init ( void ) {}
		/// give the next branching alternative
	virtual void nextOption ( void ) {}
}; // BranchingContext

		/// branching context for the OR operations
class BCOr: public BranchingContext
{
public:		// types
		/// short OR indexes
	typedef std::vector<ConceptWDep> OrIndex;
		/// short OR index iterator
	typedef OrIndex::const_iterator or_iterator;

public:		// members
		/// current branching index; used in several branching rules
	unsigned int branchIndex;
		/// useful disjuncts (ready to add) in case of OR
	OrIndex applicableOrEntries;

public:		// interface
		/// empty c'tor
	BCOr ( void ) : BranchingContext() {}
		/// empty d'tor
	virtual ~BCOr ( void ) {}
		/// init tag and indeces
	virtual void init ( void ) { branchIndex = 0; }
		/// give the next branching alternative
	virtual void nextOption ( void ) { ++branchIndex; }

	// access to the fields

		/// check if the current processing OR entry is the last one
	bool isLastOrEntry ( void ) const { return applicableOrEntries.size() == branchIndex+1; }
		/// 1st element of OrIndex
	or_iterator orBeg ( void ) const { return applicableOrEntries.begin(); }
		/// current element of OrIndex
	or_iterator orCur ( void ) const { return orBeg() + branchIndex; }
}; // BCOr

	/// branching context for the Choose-rule
class BCChoose: public BranchingContext
{
public:		// interface
		/// empty c'tor
	BCChoose ( void ) : BranchingContext() {}
		/// empty d'tor
	virtual ~BCChoose ( void ) {}
}; // BCChoose

	/// branching context for the NN-rule
class BCNN: public BranchingContext
{
public:		// members
		/// current branching index; used in several branching rules
	unsigned int branchIndex;
		/// index of a merge-candidate (in LE concept)
	unsigned int mergeCandIndex;

public:		// interface
		/// empty c'tor
	BCNN ( void ) : BranchingContext() {}
		/// empty d'tor
	virtual ~BCNN ( void ) {}
		/// init tag and indeces
	virtual void init ( void ) { branchIndex = 1; }
		/// give the next branching alternative
	virtual void nextOption ( void ) { ++branchIndex; }

	// access to the fields

		/// check if the NN has no option to process
	bool noMoreNNOptions ( unsigned int n ) const { return branchIndex > n; }
}; // BCNN

	/// branching context for the LE operations
class BCLE: public BranchingContext
{
public:		// types
		/// vector of edges
	typedef std::vector<DlCompletionTreeArc*> EdgeVector;

public:		// members
		/// current branching index; used in several branching rules
	unsigned int branchIndex;
		/// index of a merge-candidate (in LE concept)
	unsigned int mergeCandIndex;
		/// vector of edges to be merged
	EdgeVector EdgesToMerge;

public:		// interface
		/// empty c'tor
	BCLE ( void ) : BranchingContext() {}
		/// empty d'tor
	virtual ~BCLE ( void ) {}
		/// init tag and indeces
	virtual void init ( void )
	{
		branchIndex = 0;
		mergeCandIndex = 0;
	}
		/// correct mergeCandIndex after changing
	void resetMCI ( void ) { mergeCandIndex = EdgesToMerge.size()-1; }
		/// give the next branching alternative
	virtual void nextOption ( void )
	{
		--mergeCandIndex;	// get new merge candidate
		if ( mergeCandIndex == branchIndex )	// nothing more can be mergeable to BI node
		{
			++branchIndex;	// change the candidate to merge to
			resetMCI();
		}
	}

	// access to the fields

		/// get FROM pointer to merge
	DlCompletionTreeArc* getFrom ( void ) const { return EdgesToMerge[mergeCandIndex]; }
		/// get FROM pointer to merge
	DlCompletionTreeArc* getTo ( void ) const { return EdgesToMerge[branchIndex]; }
		/// check if the LE has no option to process
	bool noMoreLEOptions ( void ) const { return mergeCandIndex <= branchIndex; }
}; // BCLE

	/// branching context for the barrier
class BCBarrier: public BranchingContext
{
public:		// interface
		/// empty c'tor
	BCBarrier ( void ) : BranchingContext() {}
		/// empty d'tor
	virtual ~BCBarrier ( void ) {}
}; // BCBarrier

#endif
