/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2007 by Dmitry Tsarkov

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

#ifndef _TINDIVIDUAL_H
#define _TINDIVIDUAL_H

#include "tConcept.h"

class DlCompletionTree;
class TRelated;

/// class to represent individuals
class TIndividual: public TConcept
{
public:		// types
		/// pointers to RELATED constructors
	typedef std::vector<TRelated*> RelatedSet;

public:		// members
		/// pointer to nominal node (works for singletons only)
	DlCompletionTree* node;

		/// index for axioms <this,C>:R
	RelatedSet RelatedIndex;

public:		// interface
		/// the only c'tor
	explicit TIndividual ( const std::string& name )
		: TConcept(name)
		, node(NULL)
		{}
		/// empty d'tor
	virtual ~TIndividual ( void ) {}

		/// check whether a concept is indeed a singleton
	virtual bool isSingleton ( void ) const { return true; }
		/// FIXME!! need this to support the next method
	virtual bool initToldSubsumers ( const DLTree* desc ) { return TConcept::initToldSubsumers(desc); }
		/// init told subsumers of the concept by it's description
	virtual void initToldSubsumers ( TConcept* top )
	{
		getTold().clear();
		if ( isRelated() )	// check if domain and range of RELATED axioms affects TS
			updateToldFromRelated();
		// normalise description if the only parent is TOP
		if ( isPrimitive() && Description && Description->Element() == TOP )
			removeDescription();

		if ( Description == NULL && getTold().empty() )
		{
			addParent(top);
			if ( !isRelated() )	// related individuals are NOT completely defined
				setCompletelyDefined(true);
		}
		else	// init (additional) told subsumers from definition
			setCompletelyDefined ( initToldSubsumers(Description) && isPrimitive() );
	}

	// related things

		/// update told subsumers from the RELATED axioms in a given range
	template<class Iterator>
	void updateTold ( Iterator begin, Iterator end )
	{
		for ( Iterator p = begin; p < end; ++p )
			SearchTSbyRoleAndSupers((*p)->getRole());
	}
		/// update told subsumers from all relevant RELATED axioms
	void updateToldFromRelated ( void );
		/// check if individual connected to something with RELATED statement
	bool isRelated ( void ) const { return !RelatedIndex.empty(); }
		/// set individual related
	void addRelated ( TRelated* p ) { RelatedIndex.push_back(p); }
}; // TIndividual

#endif

