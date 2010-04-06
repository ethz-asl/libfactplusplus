/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2010 by Dmitry Tsarkov

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

#include "KernelFactory.h"
#include "Kernel.h"

#include <sstream>
#include <iomanip>
#include <time.h>

// print current time to the stream
void printCurrentTime ( std::ostream& o )
{
	time_t curTime = time(NULL);
	struct tm* p = gmtime (&curTime);
	o << p->tm_hour+1 << '-' << p->tm_min << '-' << p->tm_sec << '_'
	  << p->tm_mday << '-' << p->tm_mon+1 << '-' << (1900+p->tm_year);
}

/// create new unique name (using count and date+time)
void KernelFactory :: createName ( std::string& id )
{
	const char KBPrefix[] = "http://dl.kr.org/dig/FaCTpp-";
	const unsigned long maxKBNumber = 1000;
	const unsigned KBNumberLength = 3;

	static unsigned int count;

	std::stringstream s;
	s << KBPrefix << std::setw(KBNumberLength) << std::setfill('0') << count++ << "_";

	// clear the counter
	if ( count >= maxKBNumber )
		count = 0;

	// add date-time to URI
	printCurrentTime (s);

	// copy result to return
	id = s.str();
}

/// D'tor: delete all kernels
KernelFactory :: ~KernelFactory ( void )
{
	for ( KernelCollection::iterator i = Factory.begin(); i != Factory.end(); ++i )
		delete i->second;
}

/// create a new KB, with a given name. @return true if there exist such a name
bool KernelFactory :: createKB ( const std::string& id )
{
	// error if such kernel exists
	if ( Factory.find(id) != Factory.end() )
		return true;

	ReasoningKernel* p = new ReasoningKernel;
	p->newKB ();		// init new kernel
	Factory[id] = p;	// add new kernel under a given name

	return false;
}

/// release existing KB with given name. @return true if unable to proceed
bool KernelFactory :: releaseKB ( const std::string& id )
{
	KernelCollection::iterator i = Factory.find (id);

	// error if there are no such kernel
	if ( i == Factory.end () )
		return true;

	Factory.erase (i);
	return false;
}

/** get access to KB with given id.
 *  @return 'default' KB for empty id.
 *  @return NULL for incorrect (not registered) ID
 */
ReasoningKernel* KernelFactory :: getKernel ( const std::string& id ) const
{
	// special case -- empty string in a name
	if ( id == "" )
		return getKernel ();

	KernelCollection::const_iterator i = Factory.find (id);

	// error if there are no such kernel
	if ( i == Factory.end () )
		return NULL;

	return i->second;
}
