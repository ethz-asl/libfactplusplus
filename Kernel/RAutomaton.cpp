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

/// set up state transitions: no more additions to the structure
void
RAStateTransitions :: setup ( RAState state, unsigned int nRoles, bool data )
{
	from = state;
	DataRole = data;
	ApplicableRoles.ensureMaxSetSize(nRoles);
	// fills the set of recognisable roles
	for ( const_iterator p = begin(), p_end = end(); p != p_end; ++p )
		for ( RATransition::const_iterator q = (*p)->begin(), q_end = (*p)->end(); q != q_end; ++q )
			ApplicableRoles.add((*q)->getIndex());
}

/// add information from TRANS to existing transition between the same states. @return false if no such transition found
bool
RAStateTransitions :: addToExisting ( const RATransition* trans )
{
	RAState to = trans->final();
	bool tEmpty = trans->empty();
	for ( iterator p = Base.begin(), p_end = Base.end(); p != p_end; ++p )
		if ( (*p)->final() == to && (*p)->empty() == tEmpty )
		{	// found existing transition
			(*p)->addIfNew(*trans);
			return true;
		}
	// no transition from->to found
	return false;
}

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

void
RoleAutomaton :: addCopy ( const RoleAutomaton& RA )
{
	for ( RAState i = 0; i < RA.size(); ++i )
	{
		RAState from = map[i];
		RAStateTransitions& RST = Base[from];
		const RAStateTransitions& RSTOrig = RA[i];

		if ( RSTOrig.empty() )
			continue;

		for ( RAStateTransitions::const_iterator p = RSTOrig.begin(), p_end = RSTOrig.end(); p != p_end; ++p )
		{
			RAState to = (*p)->final();
			RATransition* trans = new RATransition(map[to]);
			checkTransition ( from, trans->final() );
			trans->add(**p);

			// try to merge transitions going to the original final state
			if ( to == 1 && RST.addToExisting(trans) )
				delete trans;
			else
				RST.add(trans);
		}
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
		fRA = size();	// make sure we don't create an extra unused state
		++newState;
	}
	map[1] = fRA;

	// check transitions as it may turns out to be a single transition
	checkTransition ( iRA, fRA );

	// set new initial state
	iRA = fRA;

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
	bool needFinalTrans = ( fRA < size() && !RA.isOSafe() );
	// we can skip transition if chaining automata are i- and o-safe
	if ( !oSafe && !RA.isISafe() )
		nextChainTransition(newState());
	// check whether we need an output transition
	initMap ( RA.size(), needFinalTrans ? size() : fRA );
	addCopy(RA);
	if ( needFinalTrans )
		nextChainTransition(fRA);

	return RA.isOSafe();
}
