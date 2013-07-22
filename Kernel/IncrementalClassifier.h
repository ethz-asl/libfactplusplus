/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2013 by Dmitry Tsarkov

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

#ifndef INCREMENTALCLASSIFIER_H
#define INCREMENTALCLASSIFIER_H

#include "TaxonomyCreator.h"
#include "dlTBox.h"
#include "tProgressMonitor.h"
#include "tSplitVars.h"
#include "Kernel.h"

/// Taxonomy of named DL concepts (and mapped individuals)
class IncrementalClassifier: public TaxonomyCreator
{
protected:	// types
	typedef std::vector<TaxonomyVertex*> TaxVertexVec;

protected:	// members
		/// delegate reasoner
	ReasoningKernel* reasoner;
		/// session signature
	const TSignature* sig;
		/// re-classified taxonomy node
	TaxonomyVertex* curNode;
		/// tested concept
	const TDLConceptName* Current;
		/// set of possible parents
	std::set<TaxonomyVertex*> candidates;
		/// whether look into it
	bool useCandidates;

	// statistic counters
	unsigned long nConcepts;
	unsigned long nTries;
	unsigned long nPositives;
	unsigned long nNegatives;
	unsigned long nSearchCalls;
	unsigned long nSubCalls;
	unsigned long nNonTrivialSubCalls;

		/// number of non-subsumptions because of module reasons
	unsigned long nModuleNegative;

		/// indicator of taxonomy creation progress
	TProgressMonitor* pTaxProgress;

private:	// no copy
		/// no copy c'tor
	IncrementalClassifier ( const IncrementalClassifier& );
		/// no assignment
	IncrementalClassifier& operator = ( const IncrementalClassifier& );

protected:	// methods

		/// helper
	static const TDLConceptName* getCName ( const ClassifiableEntry* entry )
		{ return dynamic_cast<const TDLConceptName*>(entry->getEntity()); }
	//-----------------------------------------------------------------
	//--	General support for DL concept classification
	//-----------------------------------------------------------------

		/// test whether Current [= q
	bool testSub ( const TConcept* q );
		/// test subsumption via TBox explicitely
	bool testSubTBox ( const TConcept* q )
	{
		bool res = reasoner->isSubsumedBy ( Current, getCName(q) );

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
	void searchBaader ( TaxonomyVertex* cur );
		/// ENHANCED_SUBS procedure from Baader et al paper
	bool enhancedSubs1 ( TaxonomyVertex* cur );
		/// short-cut from ENHANCED_SUBS
	bool enhancedSubs2 ( TaxonomyVertex* cur )
	{
//		std::cout << "checking subs: " << cur->getPrimer()->getName() << std::endl;
		if ( useCandidates && candidates.find(cur) != candidates.end() )
			return false;
		return enhancedSubs1(cur);
	}
		// wrapper for the ENHANCED_SUBS
	inline bool enhancedSubs ( TaxonomyVertex* cur )
	{
		++nSubCalls;

		if ( isValued(cur) )
			return getValue(cur);
		else
			return setValue ( cur, enhancedSubs2(cur) );
	}
		/// explicitly test appropriate subsumption relation
	bool testSubsumption ( TaxonomyVertex* cur ) { return testSub ( static_cast<const TConcept*>(cur->getPrimer()) ); }
		/// @return true if non-subsumption is due to ENTITY is not in the \bot-module
	bool isNotInModule ( const TNamedEntity* entity ) const { return entity && sig && !sig->contains(entity); }
		/// fill candidates
	void fillCandidates ( TaxonomyVertex* cur );
		/// check if concept is unsat; add it as a synonym of BOTTOM if necessary
	bool isUnsatisfiable ( void );

	//-----------------------------------------------------------------
	//--	Tunable methods (depending on taxonomy type)
	//-----------------------------------------------------------------

		/// check if no BU classification is required as C=TOP
	bool isEqualToTop ( void );

		/// check if it is possible to skip TD phase
	virtual bool needTopDown ( void ) const
		{ return !(useCompletelyDefined && curEntry->isCompletelyDefined ()); }
		/// explicitly run TD phase
	virtual void runTopDown ( void ) { searchBaader(pTax->getTopVertex()); }
		/// check if it is possible to skip BU phase
	virtual bool needBottomUp ( void ) const
	{
		return false;
	}

		/// actions that to be done BEFORE entry will be classified
	virtual void preClassificationActions ( void )
	{
		++nConcepts;
		if ( pTaxProgress != NULL )
			pTaxProgress->nextClass();
	}

		/// check if it is necessary to log taxonomy action
	virtual bool needLogging ( void ) const { return true; }

public:		// interface
		/// the only c'tor
	IncrementalClassifier ( Taxonomy* tax )
		: TaxonomyCreator(tax)
		, reasoner(NULL)
		, sig(NULL)
		, curNode(NULL)
		, Current(NULL)
		, nConcepts (0), nTries (0), nPositives (0), nNegatives (0)
		, nSearchCalls(0)
		, nSubCalls(0)
		, nNonTrivialSubCalls(0)
		, nModuleNegative(0)
		, pTaxProgress (NULL)
	{
		upDirection = false;
	}
		/// d'tor
	virtual ~IncrementalClassifier ( void ) {}

	void reclassify ( TaxonomyVertex* node, ReasoningKernel* r, const TSignature* s, bool added, bool removed );
		/// output taxonomy to a stream
	virtual void print ( std::ostream& o ) const;
}; // IncrementalClassifier

//
// IncrementalClassifier implementation
//

inline bool IncrementalClassifier :: isUnsatisfiable ( void )
{
	if ( reasoner->isSatisfiable(Current) )
		return false;

	pTax->addCurrentToSynonym(pTax->getBottomVertex());
	return true;
}

#endif // INCREMENTALCLASSIFIER_H
