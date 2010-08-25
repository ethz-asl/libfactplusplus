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

#include "fpp_assert.h"

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

/// class to represent transitions from a single state in an automaton
class RAStateTransitions
{
protected:	// types
		/// keep all the transitions
	typedef std::vector<RATransition*> RTBase;
		/// RW iterators
	typedef RTBase::iterator iterator;

public:		// type interface
		/// RO iterators
	typedef RTBase::const_iterator const_iterator;

protected:	// members
		/// all transitions
	RTBase Base;
		/// check whether there is an empty transition going from this state
	bool EmptyTransition;

protected:	// methods
		/// RW begin
	iterator begin ( void ) { return Base.begin(); }
		/// RW end
	iterator end ( void ) { return Base.end(); }

public:		// interface
		/// empty c'tor
	RAStateTransitions ( void ) : EmptyTransition(false) {}
		/// copy c'tor
	RAStateTransitions ( const RAStateTransitions& trans ) : EmptyTransition(trans.EmptyTransition)
	{
		for ( const_iterator p = trans.begin(), p_end = trans.end(); p != p_end; ++p )
			Base.push_back(new RATransition(**p));
	}
		/// assignment
	RAStateTransitions& operator = ( const RAStateTransitions& trans )
	{
		for ( const_iterator p = trans.begin(), p_end = trans.end(); p != p_end; ++p )
			Base.push_back(new RATransition(**p));
		EmptyTransition = trans.EmptyTransition;
		return *this;
	}
		/// d'tor: delete all transitions
	~RAStateTransitions ( void )
	{
		for ( iterator p = begin(), p_end = end(); p != p_end; ++p )
			delete *p;
	}

		/// add a transition from a given state
	void add ( RATransition* trans )
	{
		Base.push_back(trans);
		if ( trans->empty() )
			EmptyTransition = true;
	}
		/// add information from TRANS to existing transition between the same states. @return false if no such transition found
	bool addToExisting ( const RATransition* trans )
	{
		RAState to = trans->final();
		bool tEmpty = trans->empty();
		for ( iterator p = Base.begin(), p_end = Base.end(); p != p_end; ++p )
			if ( (*p)->final() == to && (*p)->empty() == tEmpty )
			{	// found existing transition
				(*p)->add(*trans);
				return true;
			}
		// no transition from->to found
		return false;
	}
		/// @return true iff there are no transitions from this state
	bool empty ( void ) const { return Base.empty(); }
		/// @return true iff there is an empty transition from the state
	bool hasEmptyTransition ( void ) const { return EmptyTransition; }

	// RO access

		/// RO begin
	const_iterator begin ( void ) const { return Base.begin(); }
		/// RO end
	const_iterator end ( void ) const { return Base.end(); }

		/// print all the transitions starting from the state FROM
	void Print ( std::ostream& o, RAState from ) const
	{
		for ( const_iterator p = begin(), p_end = end(); p != p_end; ++p )
			(*p)->Print ( o, from );
	}
}; // RAStateTransitions

/// automaton for the role in RIQ-like languages
class RoleAutomaton
{
protected:	// members
		/// all transitions of the automaton, groupped by a starting state
	std::vector<RAStateTransitions> Base;
		/// maps original automata state into the new ones (used in copyRA)
	std::vector<unsigned int> map;
		/// initial state of the next automaton in chain
	RAState iRA;
		/// flag whether automaton is input safe
	bool ISafe;
		/// flag whether automaton is output safe
	bool OSafe;
		/// flag for the automaton to be completed
	bool Complete;

protected:	// methods
		/// make sure that STATE exists in the automaton (update ton's size)
	void ensureState ( RAState state )
	{
		if ( state >= Base.size() )
			Base.resize(state+1);
	}

		/// state that the automaton is i-unsafe
	void setIUnsafe ( void ) { ISafe = false; }
		/// state that the automaton is o-unsafe
	void setOUnsafe ( void ) { OSafe = false; }
		/// check whether transition between FROM and TO breaks safety
	void checkTransition ( RAState from, RAState to )
	{
		if ( from == final() )
			setOUnsafe();
		if ( to == initial() )
			setIUnsafe();
	}

		/// add TRANSition leading from a state FROM; all states are known to fit the ton
	void addTransition ( RAState from, RATransition* trans )
	{
		checkTransition ( from, trans->final() );
		Base[from].add(trans);
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

public:		// interface
		/// empty c'tor
	RoleAutomaton ( void )
		: iRA(0)
		, ISafe(true)
		, OSafe(true)
		, Complete(false)
	{
		ensureState(1);
	}
		/// copy c'tor
	RoleAutomaton ( const RoleAutomaton& RA );
		/// assignment
	RoleAutomaton& operator= ( const RoleAutomaton& RA );
		/// empty d'tor
	~RoleAutomaton ( void ) {}

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

		/// get access to the transitions starting from STATE
	const RAStateTransitions& operator [] ( RAState state ) const { return Base[state]; }

	// automaton's construction

		/// add TRANSition leading from a given STATE; check whether all states are correct
	void addTransitionSafe ( RAState state, RATransition* trans )
	{
		ensureState(state);
		ensureState(trans->final());
		addTransition ( state, trans );
	}

	// chain automaton creation

		/// make the beginning of the chain
	void initChain ( RAState from ) { iRA = from; }
		/// add an Automaton to the chain that would start from the iRA; OSAFE shows the safety of a previous automaton in a chain
	bool addToChain ( const RoleAutomaton& RA, bool oSafe, RAState fRA );
		/// add an Automaton to the chain with a default final state
	bool addToChain ( const RoleAutomaton& RA, bool oSafe ) { return addToChain ( RA, oSafe, size()+1 ); }

	// i/o safety

		/// get the i-safe value
	bool isISafe ( void ) const { return ISafe; }
		/// get the o-safe value
	bool isOSafe ( void ) const { return OSafe; }

	// add single RA

		/// add RA from simple subrole to given one
	void addSimpleRA ( const RoleAutomaton& RA )
	{
		bool ok = Base[initial()].addToExisting(*RA[initial()].begin());
		fpp_assert(ok);
	}
		/// add RA from a subrole to given one
	void addRA ( const RoleAutomaton& RA )
	{
		initChain(initial());
		addToChain ( RA, /*oSafe=*/false, final() );
	}

		/// return number of distinct states
	unsigned int size ( void ) const { return Base.size(); }

	// completeness of an automaton

		/// check whether automaton is complete
	bool isComplete ( void ) const { return Complete; }
		/// complete automaton
	void complete ( void ) { Complete = true; }

		/// print an automaton
	void Print ( std::ostream& o ) const
	{
		for ( RAState state = 0; state < Base.size(); ++state )
			Base[state].Print ( o, state );
	}
}; // RoleAutomaton

#endif

