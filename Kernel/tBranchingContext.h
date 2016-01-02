/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2015 by Dmitry Tsarkov

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

#include "ConceptWithDep.h"

class DlCompletionTree;

	/// class for saving branching context of a Reasoner
class BranchingContext
{
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
	BranchingContext ( void )
		: curNode{nullptr}
		, curConcept{bpINVALID}
		, pUsedIndex{0}
		, nUsedIndex{0}
		, SGsize{0}
		, branchDep{}
		{}
		/// no copy c'tor
	BranchingContext ( const BranchingContext& ) = delete;
		/// no assignment
	BranchingContext& operator = ( const BranchingContext& ) = delete;
		/// empty d'tor
	virtual ~BranchingContext ( void ) {}

		/// init indices (if necessary)
	virtual void init ( void ) {}
		/// give the next branching alternative
	virtual void nextOption ( void ) {}
}; // BranchingContext

		/// branching context for the OR operations
class BCOr: public BranchingContext
{
public:		// types
		/// short OR indexes
	typedef std::vector<BipolarPointer> OrIndex;
		/// short OR index iterator
	typedef OrIndex::const_iterator or_iterator;

public:		// members
		/// relevant disjuncts (ready to add)
	OrIndex applicableOrEntries;
		/// current branching index
	size_t branchIndex;

public:		// interface
		/// empty c'tor
	BCOr ( void ) : BranchingContext{}, branchIndex{0} {}
		/// empty d'tor
	virtual ~BCOr ( void ) {}
		/// init branch index
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
	BCChoose ( void ) : BranchingContext{} {}
		/// empty d'tor
	virtual ~BCChoose ( void ) {}
}; // BCChoose

	/// branching context for the NN-rule
class BCNN: public BranchingContext
{
public:		// members
		/// the value of M used in the NN rule
	unsigned int value;

public:		// interface
		/// empty c'tor
	BCNN ( void ) : BranchingContext{}, value{0} {}
		/// empty d'tor
	virtual ~BCNN ( void ) {}
		/// init value
	virtual void init ( void ) { value = 1; }
		/// give the next branching alternative
	virtual void nextOption ( void ) { ++value; }

	// access to the fields

		/// check if the NN has no option to process
	bool noMoreNNOptions ( unsigned int n ) const { return value > n; }
}; // BCNN

	/// branching context for the LE operations
template<class T>
class BCLE: public BranchingContext
{
public:		// types
		/// Cardinality Restriction index type
		// TODO: make it 8-bit or so
	typedef unsigned short int CRIndex;
		/// vector of edges
	typedef std::vector<T*> EdgeVector;

public:		// members
		/// vector of edges to be merged
	EdgeVector ItemsToMerge;
		/// index of a edge into which the merge is performing
	CRIndex toIndex;
		/// index of a merge candidate
	CRIndex fromIndex;

public:		// interface
		/// empty c'tor
	BCLE ( void ) : BranchingContext{}, toIndex{0}, fromIndex{0} {}
		/// empty d'tor
	virtual ~BCLE ( void ) {}
		/// init indices
	virtual void init ( void )
	{
		toIndex = 0;
		fromIndex = 0;
	}
		/// correct fromIndex after changing
	void resetMCI ( void ) { fromIndex = (CRIndex)ItemsToMerge.size()-1; }
		/// give the next branching alternative
	virtual void nextOption ( void )
	{
		--fromIndex;	// get new merge candidate
		if ( fromIndex == toIndex )	// nothing more can be mergeable to BI node
		{
			++toIndex;	// change the candidate to merge to
			resetMCI();
		}
	}

	// access to the fields

		/// get FROM pointer to merge
	T* getFrom ( void ) const { return ItemsToMerge[fromIndex]; }
		/// get FROM pointer to merge
	T* getTo ( void ) const { return ItemsToMerge[toIndex]; }
		/// check if the LE has no option to process
	bool noMoreLEOptions ( void ) const { return fromIndex <= toIndex; }
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
