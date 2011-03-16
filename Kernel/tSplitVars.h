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

#ifndef TSPLITVARS_H
#define TSPLITVARS_H

#include <vector>
#include <set>

#include "tSignature.h"

// forward declarations
class TDLConceptName;
class TConcept;

/// this is to keep the track of new vars/axioms for C >< C0, C1, ..., Cn
struct TSplitVar
{
	typedef std::vector<const TDLConceptName*> DLNameVector;
	typedef std::vector<TConcept*> CNameVector;

	const TDLConceptName* oldName;
	DLNameVector splitNames;
	TConcept* C;	// concept on DLTree level
	CNameVector Ci;	// split concepts
	std::vector<TSignature> Sigs;
}; // TSplitVar

/// set of all known var splits with access by name
class TSplitVars
{
public:		// type interface
	typedef std::vector<TSplitVar*>::iterator iterator;

protected:	// members
	std::vector<TSplitVar*> Base;
	std::map<const TDLConceptName*, TSplitVar*> Index;

public:		// interface
		/// empty c'tor
	TSplitVars ( void ) {}
		/// d'tor: delete all split vars
	~TSplitVars ( void )
	{
		for ( iterator p = Base.begin(), p_end = Base.end(); p != p_end; ++p )
			delete *p;
	}

		/// @return true iff the NAME has split in the set
	bool hasCN ( const TDLConceptName* name ) const { return Index.find(name) != Index.end(); }
		/// @return split corresponding to  given name; only correct for known names
	TSplitVar* get ( const TDLConceptName* name ) { return Index[name]; }
		/// put SPLIT into the set corresponding to NAME
	void set ( const TDLConceptName* name, TSplitVar* split )
	{
		Index[name] = split;
		Base.push_back(split);
	}

		/// access to the 1st element
	iterator begin ( void ) { return Base.begin(); }
		/// access to the last element
	iterator end ( void ) { return Base.end(); }
}; // TSplitVars

#endif
