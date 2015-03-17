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

		/// init indices (if necessary)
	virtual void init ( void ) {}
		/// give the next branching alternative
	virtual void nextOption ( void ) {}
}; // BranchingContext

		/// branching context for the OR operations
class BCOr: public BranchingContext
{
public:		// types
		/// single OR argument
	class OrArg
	{
	public:	// struct for now
			/// argument itself
		BipolarPointer C;
			/// argument negation
		BipolarPointer NotC;
			/// dep-set representing clash reason
		DepSet clashReason;
			/// true iff this one is currently chosen
		bool chosen;
			/// true iff currently available
		bool free;
			/// true iff was chosen before and ruled out now
		bool tried;
	public:		// interface
			/// empty c'tor
		OrArg ( void ) : chosen(false), free(true), tried(false) {}
			/// empty d'tor
		~OrArg ( void ) {}

			/// mark argument as tried
		void setTried ( const DepSet& ds )
		{
			fpp_assert(!tried);
			chosen = false;
			free = false;
			tried = true;
			clashReason = ds;
			std::cerr << "Not free option " << C << "\n";
		}

			/// init free argument
		void initFree ( BipolarPointer c )
		{
			C = c;
			NotC = inverse(c);
			clashReason = DepSet();
			chosen = false;
			free = true;
			tried = false;
		}
			/// init argument with clash
		void initClash ( BipolarPointer c, const DepSet& ds )
		{
			initFree(c);
			setTried(ds);
		}
	}; // OrArg
public:		// types
		/// short OR indexes
	typedef std::vector<OrArg> OrIndex;
		/// short OR index iterator
	typedef OrIndex::iterator or_iterator;
		/// short OR index const iterator
	typedef OrIndex::const_iterator or_const_iterator;

private:	// members
		/// relevant disjuncts (ready to add)
	OrIndex applicableOrEntries;
		/// level (global place in the stack)
	unsigned int level;
		/// current branching index
	int branchIndex;
		/// number of available options
	unsigned int freeChoices;

public:		// interface
		/// empty c'tor
	BCOr ( void ) : BranchingContext(), level(0), branchIndex(0), freeChoices(0) {}
		/// empty d'tor
	virtual ~BCOr ( void ) {}
		/// init branch index
	virtual void init ( void )
	{
		applicableOrEntries.clear();
		level = 0;
		branchIndex = 0;
		freeChoices = 0;
	}
		/// give the next branching alternative
	virtual void nextOption ( void ) { ++branchIndex; }

		// init the options
	void setOrIndex ( OrIndex& index )
	{
		applicableOrEntries.swap(index);
		freeChoices = 0;
		for ( int i = 0; i < applicableOrEntries.size(); i++ )
			if ( applicableOrEntries[i].free )
				++freeChoices;
	}

	bool noMoreOptions ( void ) const { return freeChoices == 0; }
	void gatherClashSet ( void )
	{
		for ( int i = 0; i < applicableOrEntries.size(); i++ )
			branchDep.add(applicableOrEntries[i].clashReason);
	}
	BipolarPointer chooseFreeOption ( void )
	{
		// try to return chosen one
		or_iterator p, p_beg = applicableOrEntries.begin(), p_end = applicableOrEntries.end();
		for ( p = p_beg; p != p_end; ++p )
			if ( p->chosen )
				return p->C;
		for ( p = p_beg; p != p_end; ++p )
			if ( p->free )
			{
				p->free = false;
				p->chosen = true;
				return p->C;
			}
		return bpINVALID;
	}
	void failCurOption ( const DepSet& dep, unsigned int curLevel )
	{
		fpp_assert ( curLevel == level );
		for ( or_iterator p = applicableOrEntries.begin(), p_end = applicableOrEntries.end(); p != p_end; ++p )
			if ( p->chosen )
			{
				--freeChoices;
				p->chosen = false;
				p->tried = true;
				p->clashReason = dep;
				p->clashReason.restrict(curLevel);
				std::cerr << "BC-" << level << ", add ";
				p->clashReason.Print(std::cerr);
				std::cerr << " (from ";
				dep.Print(std::cerr);
				std::cerr << ") for alternative " << p->C << "\n";
				return;
			}
	}
	bool clearDep ( unsigned int curLevel )
	{
		bool changeSelected = false;
//		std::cerr << "Clear " << curLevel << " for BC-" << level << "\n";
		for ( or_iterator p = applicableOrEntries.begin(), p_end = applicableOrEntries.end(); p != p_end; ++p )
			if ( p->clashReason.contains(curLevel) )
			{
				std::cerr << "BC-" << level << ", clear ";
				p->clashReason.Print(std::cerr);
				std::cerr << " for alternative " << p->C << "\n";
				p->clashReason.clear();
				if ( p->tried )
				{
					p->free = true;
					p->tried = false;
					++freeChoices;
				}
				else if ( p->chosen )
				{
					std::cerr << "BC-" << level << ", clear selection";
					std::cerr << " for alternative " << p->C << "\n";
					changeSelected = true;
					p->free = true;
					p->chosen = false;
				}
			}
		return changeSelected;
	}
	void setChosenDep ( const DepSet& ds )
	{
		for ( or_iterator p = applicableOrEntries.begin(), p_end = applicableOrEntries.end(); p != p_end; ++p )
			if ( p->chosen )
			{
				std::cerr << "BC-" << level << ", selection dependent on ";
				ds.Print(std::cerr);
				std::cerr << " for alternative " << p->C << "\n";
				p->clashReason = ds;
			}
	}
	// access to the fields

		/// check if the current processing OR entry is the last one
	bool isLastOrEntry ( void ) const { return applicableOrEntries.size() == branchIndex+1; }
		/// 1st element of OrIndex
	or_const_iterator orBeg ( void ) const { return applicableOrEntries.begin(); }
		/// current element of OrIndex
	or_const_iterator orCur ( void ) const { return orBeg() + branchIndex; }
		/// last (completely or used) element of OrIndex
	or_const_iterator orEnd ( void ) const { return RKG_USE_DYNAMIC_BACKJUMPING ? applicableOrEntries.end() : orCur(); }

	void setLevel ( unsigned int l ) { level = l; }
	unsigned int getLevel ( void ) const { return level; }
	void print ( std::ostream& o ) const
	{
		o << "BC-" << level << " (" << freeChoices << "/" << applicableOrEntries.size() << "): [";
		for ( or_const_iterator p = orBeg(), p_end = orEnd(); p != p_end; ++p )
		{
			if ( p->free )
				o << ' ';
			if ( p->tried )
				o << 'x';
			if ( p->chosen )
				o << '*';
//			o << "arg " << p->C << ", fct=" << p->free << p->chosen << p->tried << ", clash set ";
			o << p->C;
			if ( p->tried )
			{
				if ( p->clashReason.empty() )
					o << "{}";
				else
					p->clashReason.Print(o);
			}
			else if ( p->chosen )
				p->clashReason.Print(o);

			o << " ";
		}
		o << "]\n";
	}
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
		/// the value of M used in the NN rule
	unsigned int value;

public:		// interface
		/// empty c'tor
	BCNN ( void ) : BranchingContext() {}
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
	BCLE ( void ) : BranchingContext(), toIndex(0), fromIndex(0) {}
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
