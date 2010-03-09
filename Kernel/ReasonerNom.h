/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2010 by Dmitry Tsarkov

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

#ifndef REASONERNOM_H
#define REASONERNOM_H

#include "Reasoner.h"

//-----------------------------------------------------------------------------
//--	implemenation of nominal reasoner-related parts of TBox
//-----------------------------------------------------------------------------

inline void
TBox :: initReasoner ( void )
{
	if ( stdReasoner == NULL )	// 1st action
	{
		fpp_assert ( nomReasoner == NULL );

		GCIs.setReflexive(ORM.hasReflexiveRoles());

		stdReasoner = new DlSatTester ( *this, pOptions );
		if ( NCFeatures.hasSingletons() )
		{
			nomReasoner = new DlSatTester ( *this, pOptions );
			nomReasoner->initNominalVector();
		}
	}
}

#endif
