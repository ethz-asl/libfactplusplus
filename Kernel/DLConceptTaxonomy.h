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

#ifndef DLCONCEPTTAXONOMY_H
#define DLCONCEPTTAXONOMY_H

#include "Taxonomy.h"
#include "dlTBox.h"
#include "tProgressMonitor.h"
#include "tSplitVars.h"

/// Taxonomy of named DL concepts (and mapped individuals)
class DLConceptTaxonomy: public Taxonomy
{
protected:	// types
		/// all the derived subsumers of a class (came from the model)
	class DerivedSubsumers: public KnownSubsumers
	{
	protected:	// typedefs
			/// set of the subsumers
		typedef Taxonomy::SubsumerSet SubsumerSet;
			/// SS RW iterator
		typedef SubsumerSet::iterator ss_iterator;

	protected:	// members
			/// set of sure- and possible subsumers
		SubsumerSet Sure, Possible;

	public:		// interface
			/// c'tor: copy given sets
		DerivedSubsumers ( const SubsumerSet& sure, const SubsumerSet& possible )
			: KnownSubsumers()
			, Sure(sure)
			, Possible(possible)
			{}
			/// empty d'tor
		virtual ~DerivedSubsumers ( void ) {}

		// iterators

			/// begin of the Sure subsumers interval
		virtual ss_iterator s_begin ( void ) { return Sure.begin(); }
			/// end of the Sure subsumers interval
		virtual ss_iterator s_end ( void ) { return Sure.end(); }
			/// begin of the Possible subsumers interval
		virtual ss_iterator p_begin ( void ) { return Possible.begin(); }
			/// end of the Possible subsumers interval
		virtual ss_iterator p_end ( void ) { return Possible.end(); }
	}; // DerivedSubsumers

protected:	// members
		/// host tBox
	TBox& tBox;
		/// common descendants of all parents of currently classified concept
	TaxonomyLink Common;
		/// number of processed common parents
	unsigned int nCommon;

	// statistic counters
	unsigned long nConcepts;
	unsigned long nTries;
	unsigned long nPositives;
	unsigned long nNegatives;
	unsigned long nSearchCalls;
	unsigned long nSubCalls;
	unsigned long nNonTrivialSubCalls;

		/// number of positive cached subsumptions
	unsigned long nCachedPositive;
		/// number of negative cached subsumptions
	unsigned long nCachedNegative;
		/// number of non-subsumptions detected by a sorted reasoning
	unsigned long nSortedNegative;

		/// indicator of taxonomy creation progress
	TProgressMonitor* pTaxProgress;

	// flags

		/// flag to use Bottom-Up search
	bool flagNeedBottomUp;
		/// flag shows that subsumption check could be simplified
	bool inSplitCheck;

private:	// no copy
		/// no copy c'tor
	DLConceptTaxonomy ( const DLConceptTaxonomy& );
		/// no assignment
	DLConceptTaxonomy& operator = ( const DLConceptTaxonomy& );

protected:	// methods
	//-----------------------------------------------------------------
	//--	General support for DL concept classification
	//-----------------------------------------------------------------

		/// get access to curEntry as a TConcept
	const TConcept* curConcept ( void ) const { return static_cast<const TConcept*>(curEntry); }
		/// tests subsumption (via tBox) and gather statistics.  Use cache and other optimisations.
	bool testSub ( const TConcept* p, const TConcept* q );
		/// test subsumption via TBox explicitely
	bool testSubTBox ( const TConcept* p, const TConcept* q )
	{
		bool res = tBox.isSubHolds ( p, q );

		// update statistic
		++nTries;

		if ( res )
			++nPositives;
		else
			++nNegatives;

		return res;
	}

	// interface from BAADER paper

		/// SEARCH procedure from Baader et al paper
	void searchBaader ( bool upDirection, TaxonomyVertex* cur );
		/// ENHANCED_SUBS procedure from Baader et al paper
	bool enhancedSubs1 ( bool upDirection, TaxonomyVertex* cur );
		/// short-cuf from ENHANCED_SUBS
	bool enhancedSubs2 ( bool upDirection, TaxonomyVertex* cur )
	{
		// if bottom-up search and CUR is not a successor of checking entity -- return false
		if ( unlikely(upDirection && !cur->isCommon()) )
			return false;
		// for top-down search it's enough to look at defined concepts and non-det ones
		if ( likely(!inSplitCheck && !upDirection) && !possibleSub(cur) )
			return false;
		return enhancedSubs1 ( upDirection, cur );
	}
		// wrapper for the ENHANCED_SUBS
	inline bool enhancedSubs ( bool upDirection, TaxonomyVertex* cur )
	{
		++nSubCalls;

		if ( cur->isValued(valueLabel) )
			return cur->getValue();
		else
			return cur->setValued ( enhancedSubs2 ( upDirection, cur ), valueLabel );
	}
		/// explicetely test appropriate subsumption relation
	bool testSubsumption ( bool upDirection, TaxonomyVertex* cur );
		/// test whether a node could be a super-node of CUR
	bool possibleSub ( TaxonomyVertex* v ) const
	{
		const TConcept* C = static_cast<const TConcept*>(v->getPrimer());
		// non-prim concepts are candidates
		if ( !C->isPrimitive() )
			return true;
		// all others should be in the possible sups list
		return ksStack.top()->isPossibleSub(C);
	}

		/// propagate common value from NODE to all its descendants; save visited nodes
	void propagateOneCommon ( TaxonomyVertex* node );
		/// mark as COMMON all vertexes that are sub-concepts of every parent of CURRENT
	bool propagateUp ( void );
		/// clear all COMMON infornation
	void clearCommon ( void );
		/// check if concept is unsat; add it as a synonym of BOTTOM if necessary
	bool isUnsatisfiable ( void );

	//-----------------------------------------------------------------
	//--	Tunable methods (depending on taxonomy type)
	//-----------------------------------------------------------------

		/// prepare told subsumers for given entry if necessary
	virtual KnownSubsumers* buildKnownSubsumers ( ClassifiableEntry* p );
		/// check if no classification needed (synonym, orphan, unsatisfiable)
	virtual bool immediatelyClassified ( void );
		/// check if no BU classification is required as C=TOP
	bool isEqualToTop ( void );

		/// check if it is possible to skip TD phase
	virtual bool needTopDown ( void ) const
		{ return !(useCompletelyDefined && curEntry->isCompletelyDefined ()); }
		/// explicitely run TD phase
	virtual void runTopDown ( void ) { searchBaader ( /*upDirection=*/false, getTopVertex() ); }
		/// check if it is possible to skip BU phase
	virtual bool needBottomUp ( void ) const
	{
		// we DON'T need bottom-up phase for primitive concepts during CD-like reasoning
		// if no GCIs are in the TBox (C [= T, T [= X or Y, X [= D, Y [= D) or (T [= {o})
		// or no reflexive roles w/RnD precent (Refl(R), Range(R)=D)
		return flagNeedBottomUp || !useCompletelyDefined || curConcept()->isNonPrimitive();
	}
		/// explicitely run BU phase
	virtual void runBottomUp ( void )
	{
		if ( propagateUp() )	// Common is set up here
			return;
		if ( isEqualToTop() )	// nothing to do
			return;
		if ( !willInsertIntoTaxonomy )	// after classification -- bottom set up already
			searchBaader ( /*upDirection=*/true, getBottomVertex() );
		else	// during classification -- have to find leaf nodes
			for ( TaxonomyLink::iterator p = Common.begin(), p_end = Common.end(); p < p_end; ++p )
				if ( (*p)->noNeighbours(/*upDirection=*/false) )
					searchBaader ( /*upDirection=*/true, *p );
	}

		/// actions that to be done BEFORE entry will be classified
	virtual void preClassificationActions ( void )
	{
		++nConcepts;
		if ( pTaxProgress != NULL )
			pTaxProgress->nextClass();
	}
		/// @return true iff curEntry is classified as a synonym
	virtual bool classifySynonym ( void );

		/// merge vars came from a given SPLIT together
	void mergeSplitVars ( TSplitVar* split );
		/// merge a single vertex V to a node represented by Current
	void mergeVertex ( TaxonomyVertex* v, const std::set<TaxonomyVertex*>& excludes )
	{
		Current->mergeIndepNode ( v, excludes, curEntry );
		removeNode(v);
	}
		/// after merging, check whether there are extra neighbours that should be taken into account
	void checkExtraParents ( void );
		/// check if it is necessary to log taxonomy action
	virtual bool needLogging ( void ) const { return true; }

public:		// interface
		/// the only c'tor
	DLConceptTaxonomy ( const TConcept* pTop, const TConcept* pBottom, TBox& kb )
		: Taxonomy ( pTop, pBottom )
		, tBox(kb)
		, nConcepts (0), nTries (0), nPositives (0), nNegatives (0)
		, nSearchCalls(0)
		, nSubCalls(0)
		, nNonTrivialSubCalls(0)
		, nCachedPositive(0)
		, nCachedNegative(0)
		, nSortedNegative(0)
		, pTaxProgress (NULL)
		, inSplitCheck(false)
	{
	}
		/// d'tor
	virtual ~DLConceptTaxonomy ( void ) {}

		/// process all splits
	void processSplits ( void )
	{
		for ( TSplitVars::iterator p = tBox.Splits->begin(), p_end = tBox.Splits->end(); p != p_end; ++p )
			mergeSplitVars(*p);
	}
		/// set bottom-up flag
	void setBottomUp ( const TKBFlags& GCIs ) { flagNeedBottomUp = (GCIs.isGCI() || (GCIs.isReflexive() && GCIs.isRnD())); }
		/// set progress indicator
	void setProgressIndicator ( TProgressMonitor* pMon ) { pTaxProgress = pMon; }
		/// output taxonomy to a stream
	virtual void print ( std::ostream& o ) const;
}; // DLConceptTaxonomy

//
// DLConceptTaxonomy implementation
//

inline bool DLConceptTaxonomy :: isUnsatisfiable ( void )
{
	const TConcept* p = curConcept();

	if ( tBox.isSatisfiable(p) )
		return false;

	insertCurrent(getBottomVertex());
	return true;
}

inline bool DLConceptTaxonomy :: immediatelyClassified ( void )
{
	if ( classifySynonym() )
		return true;

	if ( curConcept()->getClassTag() == cttTrueCompletelyDefined )
		return false;	// true CD concepts can not be unsat

	// after SAT testing plan would be implemented
	tBox.initCache(const_cast<TConcept*>(curConcept()));

	return isUnsatisfiable();
}

//-----------------------------------------------------------------------------
//--		implemenation of taxonomy-related parts of TBox
//-----------------------------------------------------------------------------

inline void
TBox :: classifyEntry ( TConcept* entry )
{
	if ( unlikely(isBlockedInd(entry)) )
		classifyEntry(getBlockingInd(entry));	// make sure that the possible synonym is already classified
	if ( !entry->isClassified() )
		pTax->classifyEntry(entry);
}

#endif // DLCONCEPTTAXONOMY_H
