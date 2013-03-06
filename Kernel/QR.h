/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2011-2013 by Dmitry Tsarkov

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

#ifndef QR_H
#define QR_H

#include <set>

#include "tDLExpression.h"

//---------------------------------------------------------
// this header contains an interface for queries and rules
//---------------------------------------------------------

//---------------------------------------------------------
// i-objects (vars and individuals)
//---------------------------------------------------------

/// i-object (from SWRL proposal), which is variable or an individual
class QRiObject
{
public:		// interface
		/// empty d'tor
	virtual ~QRiObject ( void ) {}
}; // QRiObject

/// QR variable replacing the individual
class QRVariable: public QRiObject
{
protected:	// members
		/// name of a var
	std::string Name;

public:		// interface
		/// empty c'tor
	QRVariable ( void ) {}
		/// init c'tor
	QRVariable ( const std::string& name ) : Name(name) {}
		/// empty d'tor
	virtual ~QRVariable ( void ) {}

	// access methods

		/// get the name
	const std::string& getName ( void ) const { return Name; }
}; // QRVariable

/// individual in a query
class QRIndividual: public QRiObject
{
protected:	// members
		/// original individual from Expression Manager
	const TDLIndividualName* Ind;

public:		// interface
		/// init c'tor
	QRIndividual ( const TDLIndividualName* ind ) : Ind(ind) {}
		/// empty d'tor
	virtual ~QRIndividual ( void ) {}

	// access methods

		/// get the name
	const TDLIndividualName* getIndividual ( void ) const { return Ind; }
}; // QRIndividual

//---------------------------------------------------------
// var factory
//---------------------------------------------------------

class VariableFactory
{
protected:	// members
		/// class for the base
	typedef std::vector<QRVariable*> BaseClass;
		/// base itself
	BaseClass Base;

public:		// interface
		/// empty c'tor
	VariableFactory ( void ) {}
		/// d'tor: delete all registered vars
	~VariableFactory ( void )
	{
		for ( BaseClass::iterator p = Base.begin(), p_end = Base.end(); p != p_end; ++p )
			delete *p;
	}

	// access

		/// get fresh variable
	const QRVariable* getNewVar ( const std::string& name )
	{
		QRVariable* ret = new QRVariable(name);
		Base.push_back(ret);
		return ret;
	}
}; // VariableFactory

//---------------------------------------------------------
// atoms in the query
//---------------------------------------------------------

/// general atom interface
class QRAtom
{
public:		// interface
		/// empty d'tor
	virtual ~QRAtom ( void ) {}

		/// clone method
	virtual QRAtom* clone ( void ) const = 0;
}; // QRAtom

/// concept atom: C(x)
class QRConceptAtom: public QRAtom
{
protected:	// members
		/// pointer to a concept (named one atm)
	const TDLConceptExpression* Concept;
		/// argument
	const QRiObject* Arg;

public:		// interface
		/// init c'tor
	QRConceptAtom ( const TDLConceptExpression* C, const QRiObject* A ) : Concept(C), Arg(A) {}
		/// copy c'tor
	QRConceptAtom ( const QRConceptAtom& atom ) : QRAtom(), Concept(atom.Concept), Arg(atom.Arg) {}
		/// empty d'tor
	~QRConceptAtom ( void ) {}

	// access

		/// get concept expression
	const TDLConceptExpression* getConcept ( void ) const { return Concept; }
		/// get i-object
	const QRiObject* getArg ( void ) const { return Arg; }

		/// clone method
	virtual QRAtom* clone ( void ) const { return new QRConceptAtom(*this); }
}; // QRConceptAtom

/// interface for general 2-arg atom
class QR2ArgAtom: public QRAtom
{
protected:	// members
		/// argument 1
	const QRiObject* Arg1;
		/// argument 2
	const QRiObject* Arg2;

public:		// interface
		/// init c'tor
	QR2ArgAtom ( const QRiObject* A1, const QRiObject* A2 ) : Arg1(A1), Arg2(A2) {}
		/// empty d'tor
	~QR2ArgAtom ( void ) {}

	// access

		/// get first i-object
	const QRiObject* getArg1 ( void ) const { return Arg1; }
		/// get second i-object
	const QRiObject* getArg2 ( void ) const { return Arg2; }
}; // QR2ArgAtom

/// role atom R(x,y)
class QRRoleAtom: public QR2ArgAtom
{
protected:	// members
		/// role between two i-objects
	const TDLObjectRoleExpression* Role;

public:		// interface
		/// init c'tor
	QRRoleAtom ( const TDLObjectRoleExpression* R, const QRiObject* A1, const QRiObject* A2 ) : QR2ArgAtom ( A1, A2 ), Role(R) {}
		/// copy c'tor
	QRRoleAtom ( const QRRoleAtom& atom ) : QR2ArgAtom ( atom.getArg1(), atom.getArg2() ), Role(atom.Role) {}

		/// empty d'tor
	~QRRoleAtom ( void ) {}

	// access

		/// get role expression
	const TDLObjectRoleExpression* getRole ( void ) const { return Role; }

		/// clone method
	virtual QRAtom* clone ( void ) const { return new QRRoleAtom(*this); }
};

/// equality atom x=y
class QREqAtom: public QR2ArgAtom
{
public:		// interface
		/// init c'tor
	QREqAtom ( const QRiObject* A1, const QRiObject* A2 ) : QR2ArgAtom ( A1, A2 ) {}
		/// empty d'tor
	~QREqAtom ( void ) {}
}; // QREqAtom

/// inequality atom x!=y
class QRNeqAtom: public QR2ArgAtom
{
public:		// interface
		/// init c'tor
	QRNeqAtom ( const QRiObject* A1, const QRiObject* A2 ) : QR2ArgAtom ( A1, A2 ) {}
		/// empty d'tor
	~QRNeqAtom ( void ) {}
}; // QRNeqAtom

/// general QR conjunctions of atoms
class QRSetAtoms
{
private:	// prevent copy
		/// no assignment
	QRSetAtoms& operator= ( const QRSetAtoms& );

protected:	// members
		/// set of atoms itself
	std::vector<QRAtom*> Base;
		/// typedef for RW iterator
	typedef std::vector<QRAtom*>::iterator iterator;

public:		// interface
		/// type for a constant iterator
	typedef std::vector<QRAtom*>::const_iterator const_iterator;
		/// empty c'tor
	QRSetAtoms ( void ) {}
		/// copy c'tor
	QRSetAtoms ( const QRSetAtoms& Set )
	{
		for ( const_iterator p = Set.Base.begin(), p_end = Set.Base.end(); p != p_end; ++p )
			Base.push_back((*p)->clone());
	}
		/// d'tor: delete all atoms
	~QRSetAtoms ( void )
	{
		for ( iterator p = Base.begin(), p_end = Base.end(); p != p_end; ++p )
			delete *p;
	}
		/// add atom to a set
	void addAtom ( QRAtom* atom ) { Base.push_back(atom); }
		/// replace an atom at a position P with NEWATOM; @return a replaced atom
    QRAtom* replaceAtom ( const_iterator p, QRAtom* newAtom )
    {
    	iterator q = Base.begin()+(p-Base.begin());
    	QRAtom* ret = *q;
    	*q = newAtom;
    	return ret;
    }
		/// RO iterator begin
	const_iterator begin ( void ) const { return Base.begin(); }
		/// RO iterator end
	const_iterator end ( void ) const { return Base.end(); }
};

/// class for the queries
class QRQuery
{
public:		// members
		/// query as a set of atoms
	QRSetAtoms Body;
		/// set of free variables
	std::set<const QRVariable*> FreeVars;

public:		// interface
		/// empty c'tor
	QRQuery ( void ) {}
		/// copy c'tor
	QRQuery ( const QRQuery& query ) : Body(query.Body), FreeVars(query.FreeVars) {}
		/// empty d'tor
	~QRQuery ( void ) {}

	// fill the query

		/// add atom to a query body
	void addAtom ( QRAtom* atom ) { Body.addAtom(atom); }
		/// mark a variable as a free one
	void setVarFree ( const QRVariable* var ) { FreeVars.insert(var); }
		/// @return true if VAR is a free var
	bool isFreeVar ( const QRVariable* var ) const { return FreeVars.count(var) > 0; }
}; // QRQuery

/// rule in a general form body -> head
class QRRule
{
protected:	// members
}; // QRRule

#endif
