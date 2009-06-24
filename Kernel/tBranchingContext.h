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

#ifndef TBRANCHINGCONTEXT_H
#define TBRANCHINGCONTEXT_H

#include "dlCompletionTree.h"

/// reason of the branching
enum BranchingTag
{
	btOr,		// branching because of disjunction
	btLE,		// brunching because of <=-rule application
	btChoose,	// branching because of choose-rule
	btNN,		// brunching because of NN-rule
	btBarrier,	// not a real branching, just to remember a state
};

/// class for saving branching context of a Reasoner
class BranchingContext
{
private:	// prevent copy
		/// no copy c'tor
	BranchingContext ( const BranchingContext& s );
		/// no assignment
	BranchingContext& operator = ( const BranchingContext& s );

public:		// types
		/// short OR indexes
	typedef std::vector<BipolarPointer> OrIndex;
		/// short OR index iterator
	typedef OrIndex::const_iterator or_iterator;
		/// vector of edges
	typedef std::vector<DlCompletionTreeArc*> EdgeVector;

public:		// members
		/// currently processed node
	DlCompletionTree* curNode;
		/// currently processed concept
	ConceptWDep curConcept;
		/// positions of the Used members
	size_t pUsedIndex, nUsedIndex;
		/// current branching index; used in several branching rules
	unsigned int branchIndex;
		/// index of a merge-candidate (in LE concept)
	unsigned int mergeCandIndex;
		/// useful disjuncts (ready to add) in case of OR
	OrIndex applicableOrEntries;
		/// vector of edges to be merged
	EdgeVector EdgesToMerge;
		/// dependences for branching clashes
	DepSet branchDep;
		/// branching tag
	BranchingTag tag;

public:		// interface
		/// empty c'tor
	BranchingContext ( void ) : curNode (NULL), curConcept (bpINVALID) {}
		/// empty d'tor
	~BranchingContext ( void ) {}
		/// init tag and indeces
	void init ( BranchingTag btag )
	{
		tag = btag;
		branchIndex = btag == btNN ? 1 : 0;
		mergeCandIndex = 0;
	}
		/// correct mergeCandIndex after changing
	void resetMCI ( void ) { mergeCandIndex = EdgesToMerge.size()-1; }
		/// give the next branching alternative
	void nextOption ( void )
	{
		switch ( tag )
		{
		case btOr:	// OR branching
		case btNN:	// NN branching -- nothing to do (all will be done in tacticNN)
			++branchIndex;		// inc index in order to get next entry
			break;
		case btLE:	// LE branching
			--mergeCandIndex;	// get new merge candidate
			if ( mergeCandIndex == branchIndex )	// nothing more can be mergeable to BI node
			{
				++branchIndex;	// change the candidate to merge to
				resetMCI();
			}
			break;
		case btChoose:	// choose-rule branching
		case btBarrier:	// nothing to do
			break;
		default:	// safety
			fpp_unreachable();
		}
	}

	// access to the fields

		/// get FROM pointer to merge
	DlCompletionTreeArc* getFrom ( void ) const { return EdgesToMerge[mergeCandIndex]; }
		/// get FROM pointer to merge
	DlCompletionTreeArc* getTo ( void ) const { return EdgesToMerge[branchIndex]; }
		/// check if the current processing OR entry is the last one
	bool isLastOrEntry ( void ) const { return applicableOrEntries.size() == branchIndex+1; }
		/// check if the LE has no option to process
	bool noMoreLEOptions ( void ) const { return mergeCandIndex <= branchIndex; }
		/// check if the NN has no option to process
	bool noMoreNNOptions ( unsigned int n ) const { return branchIndex > n; }
		/// 1st element of OrIndex
	or_iterator orBeg ( void ) const { return applicableOrEntries.begin(); }
		/// current element of OrIndex
	or_iterator orCur ( void ) const { return orBeg() + branchIndex; }
}; // BranchingContext

#endif
