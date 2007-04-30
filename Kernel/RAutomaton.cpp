/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2006 by Dmitry Tsarkov

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

RAState
RoleAutomaton :: addCopy ( const RoleAutomaton& RA )
{
	RAState oldSize = size();
	ensureState(oldSize+RA.size()-1);	// enough to fill RA

	for ( RAState i = 0; i < RA.size(); ++i )
		for ( const_trans_iterator p = RA.begin(i); p != RA.end(i); ++p )
		{
			RATransition* trans = new RATransition((*p)->final()+oldSize);
			trans->add(**p);
			Base[i+oldSize].push_back(trans);
		}

	return oldSize;
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
