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
	map[0] = iRA;

	// fills the final state; if it is not known -- adjust newState
	if ( fRA >= size() )
	{
		fpp_assert ( fRA == size() );
		++newState;
	}
	map[1] = iRA = fRA;

	// fills the rest of map
	for ( unsigned int i = 2; i < RASize; ++i )
		map[i] = ++newState;

	// reserve enough space for the new automaton
	ensureState(newState);
}

/// add an Automaton to the chain that would start from the iRA; OSAFE shows the safety of a previous automaton in a chain
bool
RoleAutomaton :: addToChain ( const RoleAutomaton& RA, bool oSafe, RAState fRA )
{
	bool needFinalTrans = ( fRA < size() );
	// we can skip transition if chaining automata are i- and o-safe
	nextChainTransition(newState());
	initMap ( RA.size(), size() );	// right now we need the last transition anyway
	addCopy(RA);
	if ( needFinalTrans )
		nextChainTransition(fRA);

	return RA.isOSafe();
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
