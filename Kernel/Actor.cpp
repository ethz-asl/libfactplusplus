/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2006-2011 by Dmitry Tsarkov

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

#include "Actor.h"
#include "tConcept.h"

	/// check whether actor is applicable to the ENTRY
bool
Actor :: applicable ( const ClassifiableEntry* entry )
{
	if ( isRole )	// object- or data-role
	{
		if ( isStandard )	// object role
			return true;
		else	// data role -- need only direct ones
			return entry->getId() > 0;
	}
	else	// concept or individual: standard are concepts
		return static_cast<const TConcept*>(entry)->isSingleton() != isStandard;
}

bool
Actor :: apply ( const TaxonomyVertex& v )
{
	syn.clear();
	tryEntry(v.getPrimer());

	for ( TaxonomyVertex::syn_iterator p = v.begin_syn(), p_end=v.end_syn(); p != p_end; ++p )
		tryEntry(*p);

	/// no applicable elements were found
	if ( syn.empty() )
		return false;

	acc.push_back(syn);
	return true;
}
