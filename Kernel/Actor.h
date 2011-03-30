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

public:		// interface
		/// empty c'tor
	Actor ( void ) {}
		/// empty d'tor
	~Actor ( void ) {}

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
/*
		/// get single vector of synonyms (necessary for Equivalents, for example)
	jobjectArray getSynonyms ( void ) const { return getArray ( acc.empty() ? SynVector() : acc[0] ); }
		/// get 2D array of all required elements of the taxonomy
	jobjectArray getElements ( void ) const
	{
		if ( AccessPolicy::needPlain() )
			return getArray(plain);
		jclass ArrayClassID = env->FindClass(AccessPolicy::getIDs().ArrayClassName);
		jobjectArray ret = env->NewObjectArray ( acc.size(), ArrayClassID, NULL );
		for ( unsigned int i = 0; i < acc.size(); ++i )
			env->SetObjectArrayElement ( ret, i, getArray(acc[i]) );
		return ret;
	}

	*/
		/// taxonomy walking method.
		/// @return true if node was processed, and there is no need to go further
		/// @return false if node can not be processed in current settings
	bool apply ( const TaxonomyVertex& v );
}; // Actor

#endif
