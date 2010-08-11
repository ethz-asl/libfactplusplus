/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2006-2010 by Dmitry Tsarkov

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

#ifndef RAUTOMATON_H
#define RAUTOMATON_H

#include <vector>
#include <iostream>

class TRole;

/// state of the role automaton
typedef unsigned int RAState;

/// transition in the automaton for the role in RIQ-like languages
class RATransition
{
protected:	// typedefs
		/// set or roles that labels transition
	typedef std::vector<const TRole*> TLabel;

public:		// typedefs
		/// iterator over roles
	typedef TLabel::const_iterator const_iterator;

protected:	// members
		/// set of roles that may affect the transition
	TLabel label;
		/// final state of the transition
	RAState state;

public:		// interface
		/// create a transition to given state
	RATransition ( RAState st ) : state(st) {}
		/// create a transition with a given label R to given state ST
	RATransition ( RAState st, const TRole* R ) : state(st) { add(R); }
		/// copy c'tor
	RATransition ( const RATransition& trans ) : label(trans.label), state(trans.state) {}
		/// assignment
	RATransition& operator = ( const RATransition& trans )
	{
		label = trans.label;
		state = trans.state;
		return *this;
	}
		/// d'tor
	~RATransition ( void ) {}

	// update the transition

		/// add role R to transition's label
	void add ( const TRole* R ) { label.push_back(R); }
		/// add label of transition TRANS to transition's label
	void add ( const RATransition& trans )
		{ label.insert ( label.end(), trans.label.begin(), trans.label.end() ); }

	// query the transition

		/// get the 1st role in (multi-)transition
	const_iterator begin ( void ) const { return label.begin(); }
		/// get the last role in (multi-)transition
	const_iterator end ( void ) const { return label.end(); }

		/// give a final point of the transition
	RAState final ( void ) const { return state; }
		/// check whether transition is applicable wrt role R
	bool applicable ( const TRole* R ) const
	{
		for ( const_iterator p = label.begin(), p_end = label.end(); p < p_end; ++p )
			if ( *p == R )
				return true;

		return false;
	}
		/// check whether transition is empty
	bool empty ( void ) const { return label.empty(); }
		/// print the transition starting from FROM
	void Print ( std::ostream& o, RAState from ) const;
}; // RATransition

/// automaton for the role in RIQ-like languages
class RoleAutomaton
{
public:		// types interface
	typedef std::vector<RATransition*> TTransitions;
	typedef TTransitions::iterator trans_iterator;
	typedef TTransitions::const_iterator const_trans_iterator;

	typedef std::vector<TTransitions> AutoBase;
	typedef AutoBase::iterator iterator;
	typedef AutoBase::const_iterator const_iterator;

protected:	// members
		/// all transitions of the automaton, groupped by a starting state
	AutoBase Base;
		/// maps original automata state into the new ones (used in copyRA)
	std::vector<unsigned int> map;
		/// initial state of the next automaton in chain
	RAState iRA;
		/// flag for the automaton to be completed
	bool Complete;

protected:	// methods
		/// make sure that STATE exists in the automaton (update ton's size)
	void ensureState ( RAState state )
	{
		if ( state >= Base.size() )
			Base.resize(state+1);
	}

		/// find the existing transition between FROM and TO; @return end() if none found
	trans_iterator findTransition ( RAState from, RAState to )
	{
		trans_iterator p = Base[from].begin(), p_end= Base[from].end();
		for ( ; p != p_end; ++p )
			if ( (*p)->final() == to )
				return p;
		return p_end;
	}
		/// add TRANSition leading from a state FROM; all states are known to fit the ton
	void addTransition ( RAState from, RATransition* trans )
	{
		TTransitions& T = Base[from];
		trans_iterator p = findTransition ( from, trans->final() );
		if ( p == T.end() )
			T.push_back(trans);
		else	// merge transitions
		{
			(*p)->add(*trans);
			delete trans;
		}
	}
		/// add transition from a state FROM to a state TO labelled with R
	void addTransition ( RAState from, RAState to, const TRole* r )
	{
		TTransitions& T = Base[from];
		trans_iterator p = findTransition ( from, to );
		if ( p == T.end() )
			T.push_back(new RATransition ( to, r ));
		else	// merge transitions
			(*p)->add(r);
	}

		/// make the internal chain transition (between chainState and TO)
	void nextChainTransition ( RAState to )
	{
		addTransition ( iRA, new RATransition(to) );
		iRA = to;
	}

		/// add copy of the RA to given one; use internal MAP to renumber the states
	void addCopy ( const RoleAutomaton& RA );
		/// init internal map according to RA size and final (FRA) states
	void initMap ( unsigned int RASize, RAState fRA );

		/// print all transitions from a single STATE
	void printTransitions ( std::ostream& o, RAState state ) const;

public:		// interface
		/// empty c'tor
	RoleAutomaton ( void )
		: iRA(0)
		, Complete(false)
	{
		ensureState(1);
	}
		/// copy c'tor
	RoleAutomaton ( const RoleAutomaton& RA );
		/// assignment
	RoleAutomaton& operator= ( const RoleAutomaton& RA );
		/// d'tor
	~RoleAutomaton ( void );

	// access to states

		/// get the initial state
	RAState initial ( void ) const { return 0; }
		/// get the final state
	RAState final ( void ) const { return 1; }
		/// create new state
	RAState newState ( void )
	{
		RAState ret = Base.size();
		ensureState(ret);
		return ret;
	}

	// iterators

		/// get the 1st (multi-)transition starting in STATE
	const_trans_iterator begin ( RAState state ) const { return Base[state].begin(); }
		/// get the last (multi-)transition starting in STATE
	const_trans_iterator end ( RAState state ) const { return Base[state].end(); }

	// automaton's construction

		/// add TRANSition leading from a given STATE; check whether all states are correct
	void addTransitionSafe ( RAState state, RATransition* trans )
	{
		ensureState(state);
		ensureState(trans->final());
		addTransition ( state, trans );
	}
		/// add transition from a state FROM to a state TO labelled with R; check whether all states are correct
	void addTransitionSafe ( RAState from, RAState to, const TRole* r )
	{
		ensureState(from);
		ensureState(to);
		addTransition ( from, to, r );
	}

	// chain automaton creation

		/// make the beginning of the chain
	void initChain ( RAState from ) { iRA = from; }
		/// add an Automaton to the chain that would start from the iRA
	void addToChain ( const RoleAutomaton& RA, RAState fRA )
	{
		bool needFinalTrans = ( fRA < size() );
		nextChainTransition(newState());
		initMap ( RA.size(), size() );	// right now we need the last transition anyway
		addCopy(RA);
		if ( needFinalTrans )
			nextChainTransition(fRA);
	}
		/// add an Automaton to the chain with a default final state
	void addToChain ( const RoleAutomaton& RA ) { addToChain ( RA, size()+1 ); }

	// add single RA

		/// add RA from simple subrole to given one
	void addSimpleRA ( const RoleAutomaton& RA ) { (*begin(0))->add(**RA.begin(0)); }
		/// add RA from a subrole to given one
	void addRA ( const RoleAutomaton& RA )
	{
		initChain(initial());
		addToChain ( RA, final() );
	}

		/// return number of distinct states
	unsigned int size ( void ) const { return Base.size(); }

	// completeness of an automaton

		/// check whether automaton is complete
	bool isComplete ( void ) const { return Complete; }
		/// complete automaton
	void complete ( void ) { Complete = true; }

		/// print an automaton
	void Print ( std::ostream& o ) const;
}; // RoleAutomaton

#endif

