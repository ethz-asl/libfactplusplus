/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2008 by Dmitry Tsarkov

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef _CWDARRAY_H
#define _CWDARRAY_H

#include <ostream>

#include "globaldef.h"
#include "growingArray.h"
#include "ConceptWithDep.h"

enum addConceptResult { acrClash, acrExist, acrDone };
class TRestorer;

/// array of concepts with dep-set, which may be viewed as a label of a completion-graph
class CWDArray
{
protected:	// internal typedefs
	typedef growingArray<ConceptWDep> ConceptSet;
		/// RW iterator
	typedef ConceptSet::iterator iterator;

		/// restorer for the merge
	friend class UnMerge;

public:		// type interface
		/// class for saving one label
	class SaveState
	{
	public:
			/// end pointer of the label
		unsigned int ep;

	public:		// interface
			/// empty c'tor
		SaveState ( void ) {}
			/// copy c'tor
		SaveState ( const SaveState& node ) : ep(node.ep) {}
			/// empty d'tor
		~SaveState ( void ) {}
	}; // SaveState

		/// const iterator on label
	typedef ConceptSet::const_iterator const_iterator;

public:		// global vars
		/// contains clash set if clash is encountered in a node label
	static DepSet clashSet;
		/// statistic for number of lookups in a node label
	static unsigned int nLookups;

protected:	// members
		/// array of concepts together with dep-sets
	ConceptSet Base;

public:		// interface
		/// init/clear label with given size
	void init ( unsigned int size )
	{
		Base.reset(size);
		Base.clear();
	}
		/// empty c'tor
	CWDArray ( void ) {}
		/// copy c'tor
	CWDArray ( const CWDArray& copy ) : Base(copy.Base) {}
		/// assignment
	CWDArray& operator = ( const CWDArray& copy ) { Base = copy.Base; return *this; }
		/// empty d'tor
	~CWDArray ( void ) {}


	//----------------------------------------------
	// Label access interface
	//----------------------------------------------

	// label iterators

		/// begin() iterator
	const_iterator begin ( void ) const { return Base.begin(); }
		/// end() iterator
	const_iterator end ( void ) const { return Base.end(); }

	// add concept

		/// check if it is possible to add a concept to a label
	addConceptResult checkAddedConcept ( const BipolarPointer p, const DepSet& dep ) const;
		/// check if it is possible to add a concept to a label; ~P can't appear in the label
	addConceptResult checkAddedConceptP ( const BipolarPointer p ) const;
		/// check if it is possible to add a concept to a label; P can't appear in the label
	addConceptResult checkAddedConceptN ( const BipolarPointer p, const DepSet& dep ) const;
		/// adds concept P to a label
	void add ( const ConceptWDep& p ) { Base.add(p); }
		/// update concept BP with a dep-set DEP; @return the appropriate restorer
	TRestorer* updateDepSet ( BipolarPointer bp, const DepSet& dep );

	// access concepts

		/// check whether label contains BP (ignoring dep-set)
	bool contains ( BipolarPointer bp ) const;
		/// get the concept by given index in the node's label
	const ConceptWDep& getConcept ( int n ) const { return Base[n]; }

	//----------------------------------------------
	// Blocking support
	//----------------------------------------------

		/// check whether LABEL is a superset of a current one
	bool operator <= ( const CWDArray& label ) const;
		/// check whether LABEL is a subset of a current one
	bool operator >= ( const CWDArray& label ) const { return label <= *this; }
		/// check whether LABEL is the same as a current one
	bool operator == ( const CWDArray& label ) const
		{ return (*this <= label) && (label <= *this); }

	//----------------------------------------------
	// Save/restore interface
	//----------------------------------------------

		/// save label using given SS
	void save ( SaveState& ss ) const { ss.ep = Base.size(); }
		/// restore label to given LEVEL using given SS
	void restore ( const SaveState& ss, unsigned int level );

	//----------------------------------------------
	// Output
	//----------------------------------------------

		/// print the whole label
	void print ( std::ostream& o ) const;
}; // CWDArray

//----------------------------------------------
// Blocking support
//----------------------------------------------

inline bool
CWDArray :: contains ( BipolarPointer bp ) const
{
	for ( const_iterator p = begin(), p_end = end(); p < p_end; ++p )
		if ( p->bp() == bp )
			return true;

	return false;
}

inline bool
CWDArray :: operator <= ( const CWDArray& label ) const
{
	for ( const_iterator p = begin(), p_end = end(); p < p_end; ++p )
		if ( !label.contains(p->bp()) )
			return false;

	return true;
}

#endif
