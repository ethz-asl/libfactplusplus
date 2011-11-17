/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2006-2011 by Dmitry Tsarkov

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

#ifndef ACTOR_H
#define ACTOR_H

#include "taxVertex.h"

/// class for acting with concept taxonomy
class Actor
{
protected:	// types
		/// array of TNEs
	typedef std::vector<const ClassifiableEntry*> SynVector;
		/// array for a set of taxonomy verteces
	typedef std::vector<SynVector> SetOfNodes;

protected:	// members
		/// 2D array to return
	SetOfNodes acc;
		/// 1D array to return
	SynVector plain;
		/// temporary vector to keep synonyms
	SynVector syn;
		/// flag to look at concept-like or role-like entities
	bool isRole;
		/// flag to look at concepts or object roles
	bool isStandard;
		/// flag to throw exception at the 1st found
	bool interrupt;

protected:	// methods
		/// check whether actor is applicable to the ENTRY
	bool applicable ( const ClassifiableEntry* entry );
		/// try current entry
	void tryEntry ( const ClassifiableEntry* p )
	{
		if ( p->isSystem() )
			return;
		if ( applicable(p) )
			syn.push_back(p);
/*		if ( unlikely(interrupt) )
		{
			plain.push_back(p);

		}*/
	}
		/// build the NULL-terminated array of names of entries
	const char** buildArray ( const SynVector& vec ) const
	{
		const char** ret = new const char*[vec.size()+1];
		for ( size_t i = 0; i < vec.size(); ++i )
			ret[i] = vec[i]->getName();
		ret[vec.size()] = NULL;
		return ret;
	}

public:		// interface
		/// empty c'tor
	Actor ( void ) {}
		/// empty d'tor
	virtual ~Actor ( void ) {}

	// flags setup

		/// set the actor to look for classes
	void needConcepts ( void ) { isRole = false; isStandard = true; }
		/// set the actor to look for individuals
	void needIndividuals ( void ) { isRole = false; isStandard = false; }
		/// set the actor to look for object properties
	void needObjectRoles ( void ) { isRole = true; isStandard = true; }
		/// set the actor to look for individuals
	void needDataRoles ( void ) { isRole = true; isStandard = false; }
		/// set the interrupt parameter to VALUE
	void setInterruptAfterFirstFound ( bool value ) { interrupt = value; }

	// return values
		/// get 1-d NULL-terminated array of synonyms of the 1st entry(necessary for Equivalents, for example)
	const char** getSynonyms ( void ) const { return buildArray ( acc.empty() ? SynVector() : acc[0] ); }
		/// get NULL-terminated 2D array of all required elements of the taxonomy
	const char*** getElements2D ( void ) const
	{
		const char*** ret = new const char**[acc.size()+1];
		for ( size_t i = 0; i < acc.size(); ++i )
			ret[i] = buildArray(acc[i]);
		ret[acc.size()] = NULL;
		return ret;
	}
		/// get NULL-terminated 1D array of all required elements of the taxonomy
	const char** getElements1D ( void ) const
	{
		SynVector vec;
		for ( SetOfNodes::const_iterator p = acc.begin(), p_end = acc.end(); p != p_end; ++p )
			vec.insert ( vec.end(), p->begin(), p->end() );
		return buildArray(vec);
	}

		/// taxonomy walking method.
		/// @return true if node was processed, and there is no need to go further
		/// @return false if node can not be processed in current settings
	bool apply ( const TaxonomyVertex& v );
}; // Actor

#endif
