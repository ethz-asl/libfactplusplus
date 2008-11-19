/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2004 by Dmitry Tsarkov

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

#ifndef _TLABELLER_H
#define _TLABELLER_H

#include <cassert>
#include "tCounter.h"

/** define class that implements support for labelling entries with
 *  cheap 'unselect' operation. An external entity is 'marked' iff
 *  it's value equal to the internal counter.
 */
class TLabeller
{
private:	// internal type definition
		/// type of a counter
	typedef TCounter<unsigned int> LabCounter;

public:		// type interface
		/// define integral type of a label
	typedef LabCounter::IntType LabType;

protected:	// members
		/// counter
	LabCounter counter;

public:		// interface
		/// init c'tor
	TLabeller ( void ) : counter(1) {}
		/// copy c'tor
	TLabeller ( const TLabeller& copy ) : counter(copy.counter) {}
		/// assignment
	TLabeller& operator= ( const TLabeller& copy )
	{
		counter = copy.counter;
		return *this;
	}
		/// d'tor
	~TLabeller ( void ) {}

	// operations with Labeller

		/// create a new label value
	void newLab ( void )
	{
		counter.inc();
		assert ( counter.val() != 0 );
	}

	// operations with Labels

		/// set given label's value to the counter's one
	void set ( LabType& lab ) const { lab = counter.val(); }
		/// clear given label's value
	void clear ( LabType& lab ) const { lab = 0; }
		/// check if given label is labelled
	bool isLabelled ( const LabType& lab ) const { return (lab == counter.val()); }
}; // TLabeller

#endif
