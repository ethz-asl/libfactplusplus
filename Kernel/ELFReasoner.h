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

#ifndef ELFNREASONER_H
#define ELFNREASONER_H

#include <queue>
#include "tOntology.h"

class ELFReasoner;

/// single algorithm action (application of a rule)
class TELFAction
{
public:		// interface
		/// main function
	virtual void apply ( ELFReasoner& r ) {}
		/// destructor does all the job
	virtual ~TELFAction ( void ) {}
}; // TELFAction

// forward declaration to allow Rule specification
class TELFConcept;

/// pattern for the rule. Contains apply() method with updates of the monitored set
class TELFRule
{
public:		// interface
		/// empty d'tor
	virtual ~TELFRule ( void ) {}
		/// apply rule with fresh class C added to watching part
	virtual TELFAction* apply ( TELFConcept* C ATTR_UNUSED ) { return NULL; }
		/// apply rule with fresh pair (C,D) added to watching part
	virtual TELFAction* apply ( TELFConcept* C ATTR_UNUSED, TELFConcept* D ATTR_UNUSED ) { return NULL; }
}; // TELFRule

//-------------------------------------------------------------
// Concepts and roles, i.e. S(C) and R(C,D)
//-------------------------------------------------------------

/// aux class to support set of rules and rule applications
class TRuleSet
{
protected:	// typedefs
		/// vector of rules
	typedef std::vector<TELFRule*> RVec;

protected:	// members
		/// set of rules to apply on change
	RVec Rules;

protected:	// methods
		/// apply all rules with a single argument
	void applyRules ( ELFReasoner& ER, TELFConcept* C );
		/// apply all rules with two arguments
	void applyRules ( ELFReasoner& ER, TELFConcept* C, TELFConcept* D );

public:		// interface
		/// empty c'tor
	TRuleSet ( void ) {}
		/// empty d'tor
	virtual ~TRuleSet ( void ) {}
		/// add rule to a set
	void addRule ( TELFRule* rule ) { Rules.push_back(rule); }
}; // TRuleSet

/// concept, set S(C) and aux things
class TELFConcept: public TRuleSet
{
public:		// typedefs
	typedef std::set<const TELFConcept*> CVec;

protected:	// members
		/// original concept (if any)
	const TDLConceptExpression* Origin;
		/// set of supers (built during classification)
	CVec Supers;

protected:	// methods
		/// add C to supers
	void addSuper ( TELFConcept* C ) { Supers.insert(C); }

public:		// interface
		/// empty c'tor
	TELFConcept ( void ) : Origin(NULL) {}
		/// init c'tor
	TELFConcept ( const TDLConceptExpression* origin ) : Origin(origin) {}
		/// empty d'tor
	~TELFConcept ( void ) {}

		/// check whether concept C is contained in supers
	bool hasSuper ( TELFConcept* C ) const { return Supers.count(C) > 0; }
		/// add an super concept
	void addC ( ELFReasoner& ER, TELFConcept* C )
	{
		if ( hasSuper(C) )
			return;
		addSuper(C);
		applyRules(ER,C);
	}
};

/// role, set R(C,D)
class TELFRole: public TRuleSet
{
protected:	// members
		/// original role (if any)
	const TDLObjectRoleExpression* Origin;
		/// set of pairs
	std::set<std::pair<const TELFConcept*, const TELFConcept*> > PairSet;

protected:	// methods
		/// add (C,D) to label
	void addLabel ( TELFConcept* C, TELFConcept* D ) { PairSet.insert(std::make_pair(C,D)); }

public:		// interface
		/// empty c'tor
	TELFRole ( void ) : Origin(NULL) {}
		/// init c'tor
	TELFRole ( const TDLObjectRoleExpression* origin ) : Origin(origin) {}
		/// empty d'tor
	~TELFRole ( void ) {}

		/// check whether (C,D) is in the R-set
	bool hasLabel ( TELFConcept* C, TELFConcept* D ) const { return PairSet.count(std::make_pair(C,D)) > 0; }
		/// add pair (C,D) to a set
	void addR ( ELFReasoner& ER, TELFConcept* C, TELFConcept* D )
	{
		if ( hasLabel(C,D) )
			return;
		addLabel(C,D);
		applyRules(ER,C,D);
	}
}; // TELFRole

//-------------------------------------------------------------
// Reasoner class
//-------------------------------------------------------------

/// EL reasoner
class ELFReasoner
{
protected:	// typedefs
		/// S(C) structure
	typedef std::vector<TELFConcept*> CVec;

protected:	// members
	std::map<const TDLConceptExpression*, TELFConcept*> CMap;
		/// set or all concepts
	CVec Concepts;
		/// map between roles and structures
	std::map<const TDLObjectRoleExpression*, TELFRole*> RMap;
		/// queue of actions to perform
	std::queue<TELFAction*> queue;
		/// stat counters
	unsigned int nE1, nE2, nA, nC, nR, nCh;

protected:	// methods
		/// get concept (expression) corresponding to a given DL expression
	TELFConcept* getC ( const TDLConceptExpression* p )
	{
		if ( CMap.find(p) != CMap.end() )
			return CMap[p];
		// add new concept
		TELFConcept* ret = new TELFConcept(p);
		CMap[p] = ret;
		Concepts.push_back(ret);
		return ret;
	}
		/// get role (expression, but actually just a name)
	TELFRole* getR ( const TDLObjectRoleExpression* p )
	{
		if ( RMap.find(p) != RMap.end() )
			return RMap[p];
		TELFRole* ret = new TELFRole(p);
		RMap[p] = ret;
		return ret;
	}
		/// process concept inclusion axiom into the internal structures
	void processCI ( const TDLAxiomConceptInclusion* axiom );
		/// process role inclusion axiom into the internal structures
	void processRI ( const TDLAxiomORoleSubsumption* axiom );

public:
		/// c'tor: take the ontology and init internal structures
	ELFReasoner ( TOntology& ont ) : nE1(0), nE2(0), nA(0), nC(0), nR(0), nCh(0)
	{
		// init top- and bottom entities
		getC(ont.getExpressionManager()->Bottom());
		getC(ont.getExpressionManager()->Top());
		for ( TOntology::iterator p = ont.begin(), p_end = ont.end(); p != p_end; ++p )
			if ( (*p)->isUsed() )
			{
				if ( likely(dynamic_cast<const TDLAxiomConceptInclusion*>(*p) != NULL) )
					processCI(dynamic_cast<const TDLAxiomConceptInclusion*>(*p));
				else
					processRI(dynamic_cast<const TDLAxiomORoleSubsumption*>(*p));
				// FIXME!! later -- process declarations
			}
		std::cout << "\nFound " << nC << " axioms in the form C [= D\nFound " << nA
				  << " axioms in the form C1/\\C2 [= D\nFound " << nE1
				  << " axioms in the form ER.C [= D\nFound " << nE2
				  << " axioms in the form C [= ER.D\nFound " << nR
				  << " axioms in the form R [= S\nFound " << nCh
				  << " axioms in the form R o S [= T\n";
	}
		/// empty d'tor
	~ELFReasoner ( void ) {}
		/// add action to a queue
	void addAction ( TELFAction* action ) { queue.push(action); }
		/// classification method
	void classify ( void )
	{
		TELFConcept* Top = Concepts[1];
		// init all CIs
		for ( CVec::iterator p = Concepts.begin(), p_end = Concepts.end(); p != p_end; ++p )
		{
			(*p)->addC ( *this, Top );
			(*p)->addC ( *this, *p );
		}
		// apply all rules
		int i = 0;
		while ( !queue.empty() )
		{
			if ( i%100000 == 0 )
				std::cerr << "\n" << i << " steps done; queue size is " << queue.size();
			++i;
			queue.front()->apply(*this);
			delete queue.front();
			queue.pop();
		}
		std::cerr << "\n" << i << " steps ";
	}
};	// ELFReasoner

void
TRuleSet :: applyRules ( ELFReasoner& ER, TELFConcept* C )
{
	for ( RVec::iterator p = Rules.begin(), p_end = Rules.end(); p != p_end; ++p )
		ER.addAction((*p)->apply(C));
}

void
TRuleSet :: applyRules ( ELFReasoner& ER, TELFConcept* C, TELFConcept* D )
{
	for ( RVec::iterator p = Rules.begin(), p_end = Rules.end(); p != p_end; ++p )
		ER.addAction((*p)->apply(C,D));
}

//-------------------------------------------------------------
// Rule and action for C [= D case; CR1
//-------------------------------------------------------------

/// action corresponding to addition of D to S(C)
class CAddAction: public TELFAction
{
protected:	// members
		/// concept C corresponded to S(C)
	TELFConcept* C;
		/// concept D; would be added to S(C)
	TELFConcept* D;

public:		// interface
		/// init c'tor
	CAddAction ( TELFConcept* c, TELFConcept* d ) : C(c), D(d) {}
		/// empty d'tor
	virtual ~CAddAction ( void ) {}
		/// action
	virtual void apply ( ELFReasoner& r ) { C->addC ( r, D ); }
}; // CAddAction

/// the rule for C [= D case
class CSubRule: public TELFRule
{
protected:
		/// super of a concept; it would be added to S(C)
	TELFConcept* Sup;

public:		// interface
		/// init c'tor: remember D
	CSubRule ( TELFConcept* D ) : Sup(D) {}
		/// empty d'tor
	~CSubRule ( void ) {}
		/// apply a method with a given S(C)
	virtual TELFAction* apply ( TELFConcept* C ) { return new CAddAction ( C, Sup ); }
		/// accept
}; // CSubRule

//-------------------------------------------------------------
// Rule and action for C1 and C2 [= D case; CR2
//-------------------------------------------------------------

/// action corresponding to addition of D to S(C)
class CAddConjAction: public TELFAction
{
protected:	// members
		/// concept C corresponded to S(C)
	TELFConcept* C;
		/// concept Conj; should be in S(C)
	TELFConcept* Conj;
		/// concept D; would be added to S(C)
	TELFConcept* D;

public:		// interface
		/// init c'tor
	CAddConjAction ( TELFConcept* c, TELFConcept* conj, TELFConcept* d ) : C(c), Conj(conj), D(d) {}
		/// empty d'tor
	virtual ~CAddConjAction ( void ) {}
		/// action
	virtual void apply ( ELFReasoner& r )
	{
		if ( C->hasSuper(Conj) )
			C->addC ( r, D );
	}
}; // CAddConjAction

/// the rule for C1 and C2 [= D case
class CAndSubRule: public TELFRule
{
protected:
		/// concept to find in order to fire a rule
	TELFConcept* Conj;
		/// super of a concept; it would be added to S(C)
	TELFConcept* Sup;

public:		// interface
		/// init c'tor: remember D
	CAndSubRule ( TELFConcept* C, TELFConcept* D ) : Conj(C), Sup(D) {}
		/// empty d'tor
	~CAndSubRule ( void ) {}
		/// apply a method with a given S(C)
	virtual TELFAction* apply ( TELFConcept* C ) { return new CAddConjAction ( C, Conj, Sup ); }
		/// accept
}; // CAndSubRule

//-------------------------------------------------------------
// Rule and action for C [= \Er.D case; CR3
//-------------------------------------------------------------

//-------------------------------------------------------------
// Rule and action for \Er.C [= D case; CR4
//-------------------------------------------------------------

//-------------------------------------------------------------
// Rule and action for R(C,D) with \bot\in S(D) case; CR5
//-------------------------------------------------------------

//-------------------------------------------------------------
// Rule and action for C [= \Er.D case; CR3
//-------------------------------------------------------------

//-------------------------------------------------------------
// Rule and action for R [= S case; CR10
//-------------------------------------------------------------

//-------------------------------------------------------------
// Rule and action for R o S [= T case; CR11
//-------------------------------------------------------------


//-------------------------------------------------------------
// inline ELFReasoner implementation
//-------------------------------------------------------------

/// process concept inclusion axiom into the internal structures
inline void
ELFReasoner :: processCI ( const TDLAxiomConceptInclusion* axiom )
{
	fpp_assert ( axiom != NULL );
	// deal with existentials
	const TDLConceptObjectExists* Exists = dynamic_cast<const TDLConceptObjectExists*>(axiom->getSupC());
	if ( Exists != NULL )	// C [= \E R.D
	{
		// do something with axiom->Sub and Exists
		++nE2;
		return;
	}
	// now RHS is a concept name or \bottom; record it
	TELFConcept* D = getC(axiom->getSupC());
	// try to check if LHS is existential
	Exists = dynamic_cast<const TDLConceptObjectExists*>(axiom->getSubC());
	if ( Exists != NULL )	// \E R.C [= D
	{
		// do something with subE and axiom->Sup
		++nE1;
		return;
	}
	const TDLConceptAnd* And = dynamic_cast<const TDLConceptAnd*>(axiom->getSubC());
	if ( And != NULL )	// C1 \and C2 [= D
	{
		TELFConcept* C1 = getC(*(And->begin()));
		TELFConcept* C2 = getC(*(And->begin()+1));
		C1->addRule(new CAndSubRule(C2,D));
		C2->addRule(new CAndSubRule(C1,D));
		++nA;
		return;
	}
	// the only possible thing here is C [= D
	++nC;
	getC(axiom->getSubC())->addRule(new CSubRule(D));
}

/// process role inclusion axiom into the internal structures
inline void
ELFReasoner :: processRI ( const TDLAxiomORoleSubsumption* axiom )
{
	if ( axiom == NULL )
		return;
	const TDLObjectRoleChain* Chain = dynamic_cast<const TDLObjectRoleChain*>(axiom->getSubRole());
	TELFRole* rhs = getR(axiom->getRole());
	if ( Chain != NULL )	// R o S [= T
	{
		++nCh;
	}
	else
	{
		++nR;
	}
}

#endif
