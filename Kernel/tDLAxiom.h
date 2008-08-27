/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2008 by Dmitry Tsarkov

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

#ifndef TDLAXIOM_H
#define TDLAXIOM_H

#include "dlTBox.h"

/// base class for the DL axiom, which include T-, A- and RBox ones
class TDLAxiom
{
protected:	// members
		/// id of the axiom
	unsigned int id;

protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb ) = 0;

public:
		/// empty c'tor
	TDLAxiom ( void ) {}
		/// empty d'tor
	virtual ~TDLAxiom ( void ) {}

	// id management

		/// set the id
	void setId ( unsigned int Id ) { id = Id; }
		/// get the id
	unsigned int getId ( void ) const { return id; }

		/// load axion intho the KB taking into account ID
	void load ( TBox& kb )
	{
		kb.setAxiomId(getId());
		loadInto(kb);
	}
}; // TDLAxiom

//------------------------------------------------------------------
//	Concept inclusion axiom
//------------------------------------------------------------------

	/// axiom for Sub [= Sup
class TDLAxiomConceptInclusion: public TDLAxiom
{
protected:	// members
	DLTree* Sub;
	DLTree* Sup;

protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb ) { kb.addSubsumeAxiom ( clone(Sub), clone(Sup) ); }

public:		// interface
		/// c'tor: create an axiom
	TDLAxiomConceptInclusion ( DLTree* sub, DLTree* sup ) : Sub(sub), Sup(sup) {}
		/// d'tor
	virtual ~TDLAxiomConceptInclusion ( void ) { deleteTree(Sub); deleteTree(Sup); }
}; // TDLAxiomConceptInclusion

//------------------------------------------------------------------
//	Concept equivalence axiom
//------------------------------------------------------------------
class TDLAxiomConceptEquivalence: public TDLAxiom
{
protected:	// types
		/// base type
	typedef TBox::ConceptSet ConceptSet;
		/// RW iterator over base type
	typedef ConceptSet::iterator iterator;
		/// RO iterator over base type
	typedef ConceptSet::const_iterator const_iterator;

protected:	// members
		/// set of equivalent concept descriptions
	ConceptSet Base;

protected:	// methods
		/// load the axiom into the TBox
	virtual void loadInto ( TBox& kb ) { kb.processEquivalent(Base); }

public:		// interface
		/// c'tor: create an axiom for C = D
	TDLAxiomConceptEquivalence ( DLTree* C, DLTree* D )
	{
		Base.push_back(C);
		Base.push_back(D);
	}
		/// c'tor: create an axiom for C1 = ... = Cn
	TDLAxiomConceptEquivalence ( const ConceptSet& v )
	{
		for ( const_iterator p = v.begin(), p_end = v.end(); p < p_end; ++p )
			Base.push_back(clone(*p));
	}
		/// d'tor
	virtual ~TDLAxiomConceptEquivalence ( void ) {}
}; // TDLAxiomConceptEquivalence

#endif
