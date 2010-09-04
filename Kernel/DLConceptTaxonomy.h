/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2010 by Dmitry Tsarkov

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

/// Taxonomy of named DL concepts (and mapped individuals)
class DLConceptTaxonomy: public Taxonomy
{
protected:	// members
		/// host tBox
	TBox& tBox;
		/// common descendants of all parents of currently classified concept
	TaxonomyLink Common;

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

		/// mark as COMMON all vertexes that are sub-concepts of every parent of CURRENT
	bool propagateUp ( void );
		/// clear all COMMON infornation
	void clearCommon ( void );
		/// check if concept is unsat; add it as a synonym of BOTTOM if necessary
	bool isUnsatisfiable ( void );

	//-----------------------------------------------------------------
	//--	Tunable methods (depending on taxonomy type)
	//-----------------------------------------------------------------

		/// check if told subsumer P have to be classified during current session
	virtual bool needToldClassification ( ClassifiableEntry* p ) const
	{
		if ( useCompletelyDefined && !static_cast<TConcept*>(p)->isPrimitive() )
			return false;
		return true;
	}
		/// prepare told subsumers for given entry if necessary
//	virtual void buildToldSubsumers ( ClassifiableEntry* p ATTR_UNUSED ) {}
		/// check if no classification needed (synonym, orphan, unsatisfiable)
	virtual bool immediatelyClassified ( void );

		/// setup TD phase (ie, identify/set parent candidates)
	virtual void setupTopDown ( void );
		/// check if it is possible to skip TD phase
	virtual bool needTopDown ( void ) const;
		/// explicitely run TD phase
	virtual void runTopDown ( void ) { searchBaader ( /*upDirection=*/false, getTop() ); }
		/// setup BU phase (ie, identify/set children candidates)
	virtual void setupBottomUp ( void ) {}
		/// check if it is possible to skip BU phase
	virtual bool needBottomUp ( void ) const;
		/// explicitely run BU phase
	virtual void runBottomUp ( void )
	{
		if ( propagateUp() )	// Common is set up here
			goto finish;
		if ( !willInsertIntoTaxonomy )
		{	// after classification -- bottom set up already
			searchBaader ( /*upDirection=*/true, getBottom() );
			goto finish;
		}

		// during classification -- have to find leaf nodes
		for ( TaxonomyLink::iterator p = Common.begin(), p_end = Common.end(); p < p_end; ++p )
			if ( (*p)->noNeighbours(/*upDirection=*/false) )
				searchBaader ( /*upDirection=*/true, *p );

	finish:
		clearCommon();
	}

		/// actions that to be done BEFORE entry will be classified
	virtual void preClassificationActions ( ClassifiableEntry* cur ATTR_UNUSED )
	{
		++nConcepts;
		if ( pTaxProgress != NULL )
			pTaxProgress->nextClass();
	}

		/// check if it is necessary to log taxonomy action
	virtual bool needLogging ( void ) const { return true; }

public:		// interface
		/// the only c'tor
	DLConceptTaxonomy ( const TConcept* pTop, const TConcept* pBottom, TBox& kb,
						const TKBFlags& GCIs )
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
		, flagNeedBottomUp(GCIs.isGCI() || (GCIs.isReflexive() && GCIs.isRnD()))
	{
	}
		/// d'tor
	~DLConceptTaxonomy ( void ) {}

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

	// for unsatisfiable concepts:
	if ( willInsertIntoTaxonomy )
	{	// add to BOTTOM
		getBottom()->addSynonym(p);
		delete Current;
		Current = NULL;
	}
	else
		const_cast<TConcept*>(p)->setTaxVertex (getBottom());

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

inline void DLConceptTaxonomy :: setupTopDown ( void )
{
	setToldSubsumers();
	if ( !needTopDown() )
	{
		++nCDEntries;
		setNonRedundantCandidates();
	}
}

inline bool DLConceptTaxonomy :: needTopDown ( void ) const
{
	return !(useCompletelyDefined && curEntry->isCompletelyDefined ());
}

inline bool DLConceptTaxonomy :: needBottomUp ( void ) const
{
	// we DON'T need bottom-up phase for primitive concepts during CD-like reasoning
	// if no GCIs are in the TBox (C [= T, T [= X or Y, X [= D, Y [= D)
	// or no reflexive roles w/RnD precent (Refl(R), Range(R)=D)
	return flagNeedBottomUp || !useCompletelyDefined || curConcept()->isNonPrimitive();
}

//-----------------------------------------------------------------------------
//--		implemenation of taxonomy-related parts of TBox
//-----------------------------------------------------------------------------

inline void
TBox :: initTaxonomy ( void )
{
	pTax = new DLConceptTaxonomy ( pTop, pBottom, *this, GCIs );
}

#endif // _DLCONCEPTTAXONOMY_H
