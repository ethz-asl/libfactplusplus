/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2011 by Dmitry Tsarkov

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

#ifndef TSPLITEXPANSIONRULES_H
#define TSPLITEXPANSIONRULES_H

// this file contains new expansion rules corresponding to split concepts

#include <set>
#include <vector>

#include "BiPointer.h"
#include "tDLExpression.h"
#include "tNamedEntry.h"
#include "tSplitVars.h"

class DLDag;

/// all split rules: vector of rules with init and access methods
class TSplitRules
{
protected:	// types
	/// class to check whether there is a need to unsplit splitted var
	class TSplitRule
	{
	public:		// typedefs
			/// set of signature elements
		typedef std::set<const TNamedEntity*> SigSet;
	protected:	// members
			/// signature of equivalent part of the split
		SigSet eqSig;
			/// signature of subsumption part of the split
		SigSet impSig;
			/// pointer to split vertex to activate
		BipolarPointer bpSplit;
	protected:	// methods
			/// check whether set SUB contains in the set SUP
		static bool containsIn ( const SigSet& Sub, const SigSet& Sup )
			{ return includes ( Sup.begin(), Sup.end(), Sub.begin(), Sub.end() ); }
			/// check whether set S1 intersects with the set S2
		static bool intersectsWith ( const SigSet& S1, const SigSet& S2 )
		{
			SigSet::const_iterator q = S1.begin(), q_end = S1.end(), p = S2.begin(), p_end = S2.end();
			while ( p != p_end && q != q_end )
			{
				if ( *p == *q )
					return true;
				if ( *p < *q )
					++p;
				else
					++q;
			}
			return false;
		}
	public:		// interface
			/// empty c'tor
		TSplitRule ( void ) {}
			/// init c'tor
		TSplitRule ( const SigSet& es, const SigSet& is, BipolarPointer p ) : eqSig(es), impSig(is), bpSplit(p) {}
			/// copy c'tor
		TSplitRule ( const TSplitRule& copy ) : eqSig(copy.eqSig), impSig(copy.impSig), bpSplit(copy.bpSplit) {}
			/// assignment
		TSplitRule& operator= ( const TSplitRule& copy )
		{
			eqSig = copy.eqSig;
			impSig = copy.impSig;
			bpSplit = copy.bpSplit;
			return *this;
		}
			/// empty d'tor
		~TSplitRule ( void ) {}

		// access methods

			/// get bipolar pointer of the rule
		BipolarPointer bp ( void ) const { return bpSplit; }
			/// check whether signatures of a rule are related to current signature in such a way that allows rule to fire
		bool canFire ( const SigSet& CurrentSig ) const { return containsIn ( eqSig, CurrentSig ) && intersectsWith ( impSig, CurrentSig ); }
	}; // TSplitRule

		/// base typedef
	typedef std::vector<TSplitRule> BaseType;
		/// base type RW iterator
	typedef BaseType::iterator iterator;
		/// vector of a signature elements
	typedef std::vector<const TNamedEntity*> SigVec;

public:		// type interface
		/// set of signature elements
	typedef TSplitRule::SigSet SigSet;
		/// RO iterator
	typedef BaseType::const_iterator const_iterator;

protected:	// members
		/// all known rules
	BaseType Base;
		/// all entities that appears in all the splits in a set
	SigSet PossibleSignature;
		/// map between BP and TNamedEntities
	SigVec EntityMap;

protected:	// methods
		/// add new split rule
	void addSplitRule ( const SigSet& eqSig, const SigSet impSig, BipolarPointer bp )
		{ Base.push_back(TSplitRule(eqSig,impSig,bp)); }
		/// build a set out of signature SIG w/o given ENTITY
	SigSet buildSet ( const TSignature& sig, const TNamedEntity* entity );
		/// init split as a set-of-sets
	void initSplit ( TSplitVar* split );
		/// prepare start signature
	void prepareStartSig ( const std::vector<TDLAxiom*>& Module, TSignature& FinalSig, SigVec& Allowed ) const;
		/// build all the seed signatures
	void BuildAllSeedSigs ( const SigVec& Allowed, const TSignature& sig, std::vector<TDLAxiom*>& Module, std::set<TSignature>& Out ) const;
		/// calculate single entity based on a named entry ENTRY and possible signature
	const TNamedEntity* getSingleEntity ( const TNamedEntry* entry ) const
	{
		if ( likely ( entry == NULL ) )
			return NULL;
		const TNamedEntity* ret = entry->getEntity();
		// now keep only known signature concepts
		return ret;//PossibleSignature.count(ret) > 0 ? ret : NULL;
	}

public:		// interface
		/// empty c'tor
	TSplitRules ( void ) {}
		/// empty d'tor
	~TSplitRules ( void ) {}

	// fill methods

		/// create all the split rules by given split set SPLITS
	void createSplitRules ( TSplitVars* Splits )
	{
		for ( TSplitVars::iterator p = Splits->begin(), p_end = Splits->end(); p != p_end; ++p )
			initSplit(*p);
	}
		/// init entity map using given DAG. note that this should be done AFTER rule splits are created!
	void initEntityMap ( const DLDag& Dag );
		/// ensure that Map has the same size as DAG, so there would be no access violation
	void ensureDagSize ( size_t dagSize ) { EntityMap.resize(dagSize); }

	// access methods

		/// RO begin iterator
	const_iterator begin ( void ) const { return Base.begin(); }
		/// RO end iterator
	const_iterator end ( void ) const { return Base.end(); }

		/// @return named entity corresponding to a given bp
	const TNamedEntity* getEntity ( BipolarPointer bp ) const { return EntityMap[getValue(bp)]; }
}; // TSplitRules

#endif
