/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2014 by Dmitry Tsarkov

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

#include <iomanip>

#include "tConcept.h"
#include "TimeMetricsHelper.h"

/// init c'tor: read setup, create dirs, open files
TimeMetricsHelper :: TimeMetricsHelper ( void )
{

}

/// d'tor: flush streams, close files
TimeMetricsHelper :: ~TimeMetricsHelper ( void )
{

}

// output

	/// print the nano-time out of a timeval
inline std::ostream&
operator << ( std::ostream& o, const timeval& tv )
{
	o << "," << tv.tv_sec << std::setw(6) << tv.tv_usec << "000";
	return o;
}
	/// print the concept name
inline std::ostream&
operator << ( std::ostream& o, const TConcept* pC )
{
	if ( unlikely(pC->isTop()) )
		o << "owl:Thing,";
	else if ( unlikely(pC->isBottom()) )
		o << "owl:Nothing,";
	else
		o << pC->getName() << ",";
	return o;
}

/// print the subsumption test results
void
TimeMetricsHelper :: printST ( std::ostream& o, const SubTest& st ) const
{
	o << reasonerId.c_str() << "," << st.subConcept << st.supConcept << "fullsat,"
	  << (st.result?"true":"false") << st.start << st.finish << "\n";
}

// interface calls

/// move pCur to the next available slot
void
TimeMetricsHelper :: progressCur ( void )
{

}


