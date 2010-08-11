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

#include "RAutomaton.h"
#include "tRole.h"

void
RATransition::Print ( std::ostream& o, RAState from ) const
{
	o << "\n" << from << " -- ";
	if ( empty() )
		o << "e";
	else
	{
		const_iterator p = label.begin();
		o << '"' << (*p)->getName() << '"';

		for ( ++p; p != label.end(); ++p )
			o << ",\"" << (*p)->getName() << '"';
	}
	o << " -> " << final();
}

RoleAutomaton :: ~RoleAutomaton ( void )
{
	for ( iterator p = Base.begin(); p != Base.end(); ++p )
		for ( trans_iterator q = p->begin(); q != p->end(); ++q )
			delete *q;
}

void
RoleAutomaton :: addTransition ( RAState state, RATransition* trans )
{
	for ( trans_iterator q = Base[state].begin(); q != Base[state].end(); ++q )
		if ( (*q)->final() == trans->final() )
		{
			// merge transitions
			(*q)->add(*trans);
			delete trans;
			return;
		}

	// no such transition found -- insert this one
	Base[state].push_back(trans);
}

void
RoleAutomaton :: addTransition ( RAState from, RAState to, const TRole* r )
{
	for ( trans_iterator q = Base[from].begin(); q != Base[from].end(); ++q )
		if ( (*q)->final() == to )
		{
			(*q)->add(r);
			return;
		}

	// no such transition found -- insert this one
	Base[from].push_back(new RATransition ( to, r ));
}

void
RoleAutomaton :: addCopy ( const RoleAutomaton& RA )
{
	for ( RAState i = 0; i < RA.size(); ++i )
		for ( const_trans_iterator p = RA.begin(i), p_end = RA.end(i); p != p_end; ++p )
		{
			RATransition* trans = new RATransition(map[(*p)->final()]);
			trans->add(**p);
			Base[map[i]].push_back(trans);
		}
}

/// init internal map according to RA size, with new initial state from chainState and final (FRA) states
void
RoleAutomaton :: initMap ( unsigned int RASize, RAState fRA )
{
	map.resize(RASize);
	// new state in the automaton
	RAState newState = size()-1;

	// fill initial state; it is always known in the automata
	map[0] = chainState;

	// fills the final state; if it is not known -- adjust newState
	if ( fRA >= size() )
	{
		fpp_assert ( fRA == size() );
		++newState;
	}
	map[1] = chainState = fRA;

	// fills the rest of map
	for ( int i = 2; i < RASize; ++i )
		map[i] = ++newState;

	// reserve enough space for the new automaton
	ensureState(newState);
}

void
RoleAutomaton :: printTransitions ( std::ostream& o, RAState state ) const
{
	const TTransitions& trans = Base[state];

	for ( TTransitions::const_iterator p = trans.begin(); p != trans.end(); ++p )
		(*p)->Print ( o, state );
}

void
RoleAutomaton :: Print ( std::ostream& o ) const
{
	for ( RAState state = 0; state < Base.size(); ++state )
		printTransitions ( o, state );
}
