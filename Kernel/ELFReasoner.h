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

#include <stack>
#include <queue>
#include "tOntology.h"

// uncomment the following t use stack instead of a queue as a queue ;)
//#define TMP_QUEUE_AS_STACK

class ELFReasoner;

// forward declaration to allow Rule specification
class TELFConcept;

/// pattern for the rule. Contains apply() method with updates of the monitored set
class TELFRule
{
protected:	// members
		/// reasoner that is used to add actions. The number of rules = the number of axioms, so the price is not too bad memory-wise.
	ELFReasoner& ER;

public:		// interface
		/// init c'tor
	TELFRule ( ELFReasoner& er ) : ER(er) {}
		/// empty d'tor
	virtual ~TELFRule ( void ) {}
		/// apply rule with fresh class C added to watching part
	virtual void apply ( TELFConcept* addedC ATTR_UNUSED ) {}
		/// apply rule with fresh pair (C,D) added to watching part
	virtual void apply ( TELFConcept* addedC ATTR_UNUSED, TELFConcept* addedD ATTR_UNUSED ) {}
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
	void applyRules ( TELFConcept* addedC )
	{
		for ( RVec::iterator p = Rules.begin(), p_end = Rules.end(); p != p_end; ++p )
			(*p)->apply(addedC);
	}
		/// apply all rules with two arguments
	void applyRules ( TELFConcept* addedC, TELFConcept* addedD )
	{
		for ( RVec::iterator p = Rules.begin(), p_end = Rules.end(); p != p_end; ++p )
			(*p)->apply(addedC,addedD);
	}

public:		// interface
		/// empty c'tor
	TRuleSet ( void ) {}
		/// empty d'tor
	virtual ~TRuleSet ( void )
	{
		for ( RVec::iterator p = Rules.begin(), p_end = Rules.end(); p != p_end; ++p )
			delete *p;
	}
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
	void addC ( TELFConcept* C )
	{
		if ( hasSuper(C) )
			return;
		addSuper(C);
		applyRules(C);
	}
};

/// role, set R(C,D)
class TELFRole: public TRuleSet
{
public:		// types
		/// set of concepts
	typedef std::set<TELFConcept*> CSet;
		/// map between concept and it's predecessors
	typedef std::map<const TELFConcept*, CSet> CCMap;
		/// iterator over a map
	typedef CCMap::iterator iterator;

protected:	// members
		/// original role (if any)
	const TDLObjectRoleExpression* Origin;
		/// map itself
	CCMap PredMap;

protected:	// methods
		/// add (C,D) to label
	void addLabel ( TELFConcept* C, TELFConcept* D ) { PredMap[D].insert(C); }

public:		// interface
		/// empty c'tor
	TELFRole ( void ) : Origin(NULL) {}
		/// init c'tor
	TELFRole ( const TDLObjectRoleExpression* origin ) : Origin(origin) {}
		/// empty d'tor
	~TELFRole ( void ) {}

		/// get the (possibly empty) set of predecessors of given D
	CSet& getPredSet ( const TELFConcept* D ) { return PredMap[D]; }
		/// iterator begon over a map
	iterator begin ( void ) { return PredMap.begin(); }
		/// iterator end over a map
	iterator end ( void ) { return PredMap.end(); }

		/// check whether (C,D) is in the R-set
	bool hasLabel ( TELFConcept* C, const TELFConcept* D ) { return PredMap[D].count(C) > 0; }
		/// add pair (C,D) to a set
	void addR ( TELFConcept* C, TELFConcept* D )
	{
		if ( hasLabel(C,D) )
			return;
		addLabel(C,D);
		applyRules(C,D);
	}
}; // TELFRole

//-------------------------------------------------------------
// Action class
//-------------------------------------------------------------

/// single algorithm action (application of a rule)
class ELFAction
{
protected:	// members
		/// role R corresponded to R(C,D)
	TELFRole* R;
		/// concept C; to add
	TELFConcept* C;
		/// concept D; to add
	TELFConcept* D;

public:		// interface
		/// init c'tor for C action
	ELFAction ( TELFConcept* c, TELFConcept* d ) : R(NULL), C(c), D(d) {}
		/// init c'tor for R action
	ELFAction ( TELFRole* r, TELFConcept* c, TELFConcept* d ) : R(r), C(c), D(d) {}
		/// empty d'tor
	~ELFAction ( void ) {}
		/// action itself, depending on the R state
	void apply ( void )
	{
		if ( R != NULL )
			R->addR(C,D);
		else
			C->addC(D);
	}
}; // ELFAction

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
		/// typedef for map concept expression -> concept structure
	typedef std::map<const TDLConceptExpression*, TELFConcept*> ConceptMap;
		/// set or all concepts
	ConceptMap CMap;
		/// TOP Concept
	TELFConcept* CTop;
		/// BOTTOM Concept
	TELFConcept* CBot;
		/// typedef for map role expression -> role structure
	typedef std::map<const TDLObjectRoleExpression*, TELFRole*> RoleMap;
		/// map between roles and structures
	RoleMap RMap;
		/// queue of actions to perform
#ifdef TMP_QUEUE_AS_STACK
	std::stack
#else
	std::queue
#endif
			  <ELFAction*> queue;
		/// stat counters
	unsigned int nC2C, nA2C, nC2E, nE2C, nR2R, nC2R;

protected:	// methods
		/// get concept (expression) corresponding to a given DL expression
	TELFConcept* getC ( const TDLConceptExpression* p )
	{
		ConceptMap::iterator i = CMap.find(p);
		if ( i != CMap.end() )
			return i->second;
		// add new concept
		TELFConcept* ret = new TELFConcept(p);
		CMap[p] = ret;
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

	// process different normalized  axioms

		/// process axiom C [= D
	void processC2C ( TELFConcept* C, TELFConcept* D );
		/// process axiom C1 and C2 [= D
	void processA2C ( TELFConcept* C1, TELFConcept* C2, TELFConcept* D );
		/// process axiom C [= \ER.D
	void processC2E ( TELFConcept* C, TELFRole* R, TELFConcept* D );
		/// process axiom \ER.C [= D
	void processE2C ( TELFRole* R, TELFConcept* C, TELFConcept* D );
		/// process axiom R [= S
	void processR2R ( TELFRole* R, TELFRole* S );
		/// process axiom R1 o R2 [= S
	void processC2R ( TELFRole* R1, TELFRole* R2, TELFRole* S );

	// process original axioms

		/// process concept inclusion axiom into the internal structures
	void processCI ( const TDLAxiomConceptInclusion* axiom );
		/// process role inclusion axiom into the internal structures
	void processRI ( const TDLAxiomORoleSubsumption* axiom );
		/// process declaration axiom
	void processDeclaration ( const TDLAxiomDeclaration* axiom );

		/// helper that inits \bot-related rules
	void initBotRules ( void );

public:
		/// c'tor: take the ontology and init internal structures
	ELFReasoner ( TOntology& ont ) : nC2C(0), nA2C(0), nC2E(0), nE2C(0), nR2R(0), nC2R(0)
	{
		// init top- and bottom entities
		CBot = getC(ont.getExpressionManager()->Bottom());
		CTop = getC(ont.getExpressionManager()->Top());
		for ( TOntology::iterator p = ont.begin(), p_end = ont.end(); p != p_end; ++p )
			if ( (*p)->isUsed() )
			{
				if ( likely(dynamic_cast<const TDLAxiomConceptInclusion*>(*p) != NULL) )
					processCI(dynamic_cast<const TDLAxiomConceptInclusion*>(*p));
				else if ( dynamic_cast<const TDLAxiomORoleSubsumption*>(*p) != NULL )
					processRI(dynamic_cast<const TDLAxiomORoleSubsumption*>(*p));
				else
					processDeclaration(dynamic_cast<const TDLAxiomDeclaration*>(*p));
			}
		// now prepare rules for \bot with roles (if role filler is \bot, then so do domain)
		initBotRules();
		// dump statistics
		std::cout << "\nFound "
			<< nC2C << " axioms in the form C [= D\nFound "
			<< nA2C << " axioms in the form C1/\\C2 [= D\nFound "
			<< nC2E << " axioms in the form C [= ER.D\nFound "
			<< nE2C << " axioms in the form ER.C [= D\nFound "
			<< nR2R << " axioms in the form R [= S\nFound "
			<< nC2R << " axioms in the form R o S [= T\n";
	}
		/// empty d'tor
	~ELFReasoner ( void ) {}
		/// add action to a queue
	void addAction ( ELFAction* action ) { queue.push(action); }
		/// classification method
	void classify ( void )
	{
		// init all CIs
		for ( ConceptMap::iterator p = CMap.begin(), p_end = CMap.end(); p != p_end; ++p )
		{
			TELFConcept* C = p->second;
			C->addC(CTop);
			C->addC(C);
		}
		// apply all rules
		int i = 0;
		while ( !queue.empty() )
		{
			if ( i%100000 == 0 )
				std::cerr << "\n" << i << " steps done; queue size is " << queue.size();
			++i;
#ifdef TMP_QUEUE_AS_STACK
			queue.top()->apply();
			delete queue.top();
#else
			queue.front()->apply();
			delete queue.front();
#endif
			queue.pop();
		}
		std::cerr << "\n" << i << " steps ";
	}
};	// ELFReasoner

//-------------------------------------------------------------
// Rule for C [= D case; CR1
//-------------------------------------------------------------

/// the rule for C [= D case
class CSubRule: public TELFRule
{
protected:
		/// super of a concept; it would be added to S(C)
	TELFConcept* Sup;

public:		// interface
		/// init c'tor: remember D
	CSubRule ( ELFReasoner& ER, TELFConcept* D ) : TELFRule(ER), Sup(D) {}
		/// empty d'tor
	~CSubRule ( void ) {}
		/// apply a method with a given S(C)
	virtual void apply ( TELFConcept* addedC )
	{
		if ( !addedC->hasSuper(Sup) )
			ER.addAction ( new ELFAction ( addedC, Sup ) );
	}
}; // CSubRule

//-------------------------------------------------------------
// Rule for C1 and C2 [= D case; CR2
//-------------------------------------------------------------

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
	CAndSubRule ( ELFReasoner& ER, TELFConcept* C, TELFConcept* D ) : TELFRule(ER), Conj(C), Sup(D) {}
		/// empty d'tor
	~CAndSubRule ( void ) {}
		/// apply a method with a given S(C)
	virtual void apply ( TELFConcept* C )
	{
		if ( C->hasSuper(Conj) && !C->hasSuper(Sup) )
			ER.addAction ( new ELFAction ( C, Sup ) );
	}
}; // CAndSubRule

//-------------------------------------------------------------
// Rule for C [= \Er.D case; CR3
//-------------------------------------------------------------

/// the rule for C [= \ER.D case
class RAddRule: public TELFRule
{
protected:
		/// role to add the pair
	TELFRole* R;
		/// filler (D) of the existential
	TELFConcept* Filler;

public:		// interface
		/// init c'tor: remember D
	RAddRule ( ELFReasoner& ER, TELFRole* r, TELFConcept* C ) : TELFRule(ER), R(r), Filler(C) {}
		/// empty d'tor
	~RAddRule ( void ) {}
		/// apply a method with a given source S(C)
	virtual void apply ( TELFConcept* Source )
	{
//		if ( !R->hasLabel ( Source, Filler ) )
			ER.addAction ( new ELFAction ( R, Source, Filler ) );
	}
}; // RAddRule

//-------------------------------------------------------------
// Rules for \Er.C [= D case; CR4
//-------------------------------------------------------------

/// rule that checks an addition of C to S(Y) and checks whether there is X s.t. R(X,Y)
class CAddFillerRule: public TELFRule
{
protected:
		/// role to add the pair
	TELFRole* R;
		/// super (E) of the existential
	TELFConcept* Sup;

public:		// interface
		/// init c'tor: remember E
	CAddFillerRule ( ELFReasoner& ER, TELFRole* r, TELFConcept* C ) : TELFRule(ER), R(r), Sup(C) {}
		/// empty d'tor
	~CAddFillerRule ( void ) {}
		/// apply a method with a given source S(C)
	virtual void apply ( TELFConcept* Source )
	{
		TELFRole::CSet& SupSet = R->getPredSet(Source);
		if ( !SupSet.empty() )
			for ( TELFRole::CSet::iterator p = SupSet.begin(), p_end = SupSet.end(); p != p_end; ++p )
				if ( !(*p)->hasSuper(Sup) )
					ER.addAction ( new ELFAction ( *p, Sup ) );
	}
}; // CAddFillerRule

/// rule that checks the addition of (X,Y) to R and finds a C in S(Y)
class CExistSubRule: public TELFRule
{
protected:
		/// filler of an existential
	TELFConcept* Filler;
		/// super of an axiom concept; it would be added to S(C)
	TELFConcept* Sup;

public:		// interface
		/// init c'tor: remember D
	CExistSubRule ( ELFReasoner& ER, TELFConcept* filler, TELFConcept* sup ) : TELFRule(ER), Filler(filler), Sup(sup) {}
		/// empty d'tor
	~CExistSubRule ( void ) {}
		/// apply a method with an added pair (C,D)
	virtual void apply ( TELFConcept* addedC, TELFConcept* addedD )
	{
		if ( addedD->hasSuper(Filler) && !addedC->hasSuper(Sup) )
			ER.addAction ( new ELFAction ( addedC, Sup ) );
	}
}; // CExistSubRule


//-------------------------------------------------------------
// Rule for R(C,D) with \bot\in S(D) case; CR5
//-------------------------------------------------------------

// rule that checks whether for R(C,D) S(D) contains \bot
class RBotRule: public TELFRule
{
protected:
		/// remember the Bottom concept
	TELFConcept* CBot;

public:		// interface
		/// init c'tor: remember E
	RBotRule ( ELFReasoner& ER, TELFConcept* bot ) : TELFRule(ER), CBot(bot) {}
		/// empty d'tor
	~RBotRule ( void ) {}
		/// apply a method with a given new pair (C,D)
	virtual void apply ( TELFConcept* addedC, TELFConcept* addedD )
	{
		// it seems like every other pair is already processed, either via that rule or via add(\bot)
		if ( addedD->hasSuper(CBot) && !addedC->hasSuper(CBot))
			ER.addAction ( new ELFAction ( addedC, CBot ) );
	}
}; // RBotRule


//-------------------------------------------------------------
// Rule for R [= S case; CR10
//-------------------------------------------------------------

/// the rule for R [= S case
class RSubRule: public TELFRule
{
protected:
		/// role to add the pair
	TELFRole* S;

public:		// interface
		/// init c'tor: remember S
	RSubRule ( ELFReasoner& ER, TELFRole* s ) : TELFRule(ER), S(s) {}
		/// empty d'tor
	~RSubRule ( void ) {}
		/// apply a method with a given pair (C,D)
	virtual void apply ( TELFConcept* addedC, TELFConcept* addedD )
	{
//		if ( !S->hasLabel ( addedC, addedD ) )
			ER.addAction ( new ELFAction ( S, addedC, addedD ) );
	}
}; // RSubRule


//-------------------------------------------------------------
// Rules for R o S [= T case; CR11
//-------------------------------------------------------------

/// the rule for R in R o S [= T case
class RChainLRule: public TELFRule
{
protected:
		/// role to check the chain
	TELFRole* S;
		/// role to add the pair
	TELFRole* T;

public:		// interface
		/// init c'tor: remember S and T
	RChainLRule ( ELFReasoner& ER, TELFRole* s, TELFRole* t ) : TELFRule(ER), S(s), T(t) {}
		/// empty d'tor
	~RChainLRule ( void ) {}
		/// apply a method with a given pair (C,D)
	virtual void apply ( TELFConcept* addedC, TELFConcept* addedD )
	{
		// we have R(C,D); so for all E in range(S), if S(D,E) then add T(C,E)
		for ( TELFRole::iterator i = S->begin(), i_end = S->end(); i != i_end; ++i )
			if ( i->second.count(addedD) > 0 )
			{
				TELFConcept* E = const_cast<TELFConcept*>(i->first);
//				if ( !T->hasLabel ( addedC, E ) )
					ER.addAction ( new ELFAction ( T, addedC, E ) );
			}
	}
}; // RChainLRule

/// the rule for S in R o S [= T case
class RChainRRule: public TELFRule
{
protected:
		/// role to check the chain
	TELFRole* R;
		/// role to add the pair
	TELFRole* T;

public:		// interface
		/// init c'tor: remember R and T
	RChainRRule ( ELFReasoner& ER, TELFRole* r, TELFRole* t ) : TELFRule(ER), R(r), T(t) {}
		/// empty d'tor
	~RChainRRule ( void ) {}
		/// apply a method with a given pair (C,D)
	virtual void apply ( TELFConcept* addedC, TELFConcept* addedD )
	{
		// we have S(C,D); so for all E in domain(R), if R(E,C) then add T(E,D)
		TELFRole::CSet& SupSet = R->getPredSet(addedC);
		if ( !SupSet.empty() )
			for ( TELFRole::CSet::iterator p = SupSet.begin(), p_end = SupSet.end(); p != p_end; ++p )
//				if ( !T->hasLabel ( *p, addedD ) )
					ER.addAction ( new ELFAction ( T, *p, addedD ) );
	}
}; // RChainRRule



//-------------------------------------------------------------
// inline ELFReasoner implementation
//-------------------------------------------------------------

/// process axiom C [= D
inline void
ELFReasoner :: processC2C ( TELFConcept* C, TELFConcept* D )
{
	++nC2C;
	C->addRule(new CSubRule(*this,D));
}

/// process axiom C1 and C2 [= D
inline void
ELFReasoner :: processA2C ( TELFConcept* C1, TELFConcept* C2, TELFConcept* D )
{
	++nA2C;
	C1->addRule(new CAndSubRule(*this,C2,D));
	C2->addRule(new CAndSubRule(*this,C1,D));
}

/// process axiom C [= \ER.D
inline void
ELFReasoner :: processC2E ( TELFConcept* C, TELFRole* R, TELFConcept* D )
{
	++nC2E;
	C->addRule(new RAddRule(*this, R, D));
}

/// process axiom \ER.C [= D
inline void
ELFReasoner :: processE2C ( TELFRole* R, TELFConcept* C, TELFConcept* D )
{
	++nE2C;
	// C from existential will have a C-adder rule
	C->addRule(new CAddFillerRule(*this, R, D));
	// R from the existential will have a C-adder here
	R->addRule(new CExistSubRule(*this, C, D));
}

/// process axiom R [= S
inline void
ELFReasoner :: processR2R ( TELFRole* R, TELFRole* S )
{
	++nR2R;
	R->addRule(new RSubRule(*this,S));
}

/// process axiom R1 o R2 [= S
inline void
ELFReasoner :: processC2R ( TELFRole* R1, TELFRole* R2, TELFRole* S )
{
	++nC2R;
	R1->addRule(new RChainLRule(*this, R2, S));
	R2->addRule(new RChainRRule(*this, R1, S));
}

/// process concept inclusion axiom into the internal structures
inline void
ELFReasoner :: processCI ( const TDLAxiomConceptInclusion* axiom )
{
	fpp_assert ( axiom != NULL );
	// deal with existentials
	const TDLConceptObjectExists* Exists = dynamic_cast<const TDLConceptObjectExists*>(axiom->getSupC());
	if ( Exists != NULL )	// C [= \E R.D
	{
		processC2E ( getC(axiom->getSubC()), getR(Exists->getOR()), getC(Exists->getC()) );
		return;
	}
	// now RHS is a concept name or \bottom; record it
	TELFConcept* D = getC(axiom->getSupC());
	// try to check if LHS is existential
	Exists = dynamic_cast<const TDLConceptObjectExists*>(axiom->getSubC());
	if ( Exists != NULL )	// \E R.C [= D
	{
		processE2C ( getR(Exists->getOR()), getC(Exists->getC()), D );
		return;
	}
	const TDLConceptAnd* And = dynamic_cast<const TDLConceptAnd*>(axiom->getSubC());
	if ( And != NULL )	// C1 \and C2 [= D
	{
		fpp_assert ( And->size() == 2 );
		processA2C ( getC(*(And->begin())), getC(*(And->begin()+1)), D );
		return;
	}
	// the only possible thing here is C [= D
	processC2C ( getC(axiom->getSubC()), D );
}

/// process role inclusion axiom into the internal structures
inline void
ELFReasoner :: processRI ( const TDLAxiomORoleSubsumption* axiom )
{
	const TDLObjectRoleChain* Chain = dynamic_cast<const TDLObjectRoleChain*>(axiom->getSubRole());
	TELFRole* rhs = getR(axiom->getRole());
	if ( Chain != NULL )	// R o S [= T
	{
		fpp_assert ( Chain->size() == 2 );
		processC2R ( getR(*(Chain->begin())), getR(*(Chain->begin()+1)), rhs );
	}
	else
	{
		const TDLObjectRoleExpression* R = dynamic_cast<const TDLObjectRoleExpression*>(axiom->getSubRole());
		fpp_assert ( R != NULL );
		processR2R ( getR(R), rhs );
	}
}

/// process declaration axiom
inline void
ELFReasoner :: processDeclaration ( const TDLAxiomDeclaration* axiom )
{
	fpp_assert ( axiom != NULL );
	const TDLConceptExpression* C = dynamic_cast<const TDLConceptExpression*>(axiom->getDeclaration());
	if ( C != NULL )
	{
		getC(C);
		return;
	}
	const TDLObjectRoleExpression* R = dynamic_cast<const TDLObjectRoleExpression*>(axiom->getDeclaration());
	if ( R != NULL )
	{
		getR(R);
		return;
	}
	// nothing else could be here
	fpp_unreachable();
}

/// helper that inits \bot-related rules
inline void
ELFReasoner :: initBotRules ( void )
{
	for ( RoleMap::iterator i = RMap.begin(), i_end = RMap.end(); i != i_end; ++i )
	{
		// for every R add listener that checks whether for R(C,D) S(D) contains \bot
		i->second->addRule(new RBotRule(*this, CBot));
		// add rule that adds \bot for every C in R(C,D), if S(D) contains \bot
		CBot->addRule(new CAddFillerRule(*this, i->second, CBot));
	}
}

#endif
