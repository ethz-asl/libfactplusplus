/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2004 by Dmitry Tsarkov

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

#ifndef _MERGABLE_LABEL
#define _MERGABLE_LABEL

/// implementation of labels that could be compared and merged to each other
class mergableLabel
{
protected:	// members
		/// sample for all equivalent labels
	mergableLabel* pSample;

protected:	// methods
		/// return sample of given label
	mergableLabel* resolve ( void );

public:		// interface
		/// empty c'tor
	mergableLabel ( void ) : pSample(NULL) {}
		/// copy c'tor
	mergableLabel ( mergableLabel& p ) : pSample(p.resolve()) {}
		/// assignment
	mergableLabel& operator = ( mergableLabel& p ) { pSample = p.resolve(); return *this; }
		/// d'tor (does nothing)
	~mergableLabel ( void ) {}

	// general interface

		/// are 2 labels equal
	bool operator == ( mergableLabel& p ) { return (resolve() == p.resolve()); }
		/// are 2 labels different
	bool operator != ( mergableLabel& p ) { return !(*this == p); }
		/// make 2 labels equal
	void merge ( mergableLabel& p )
	{
		mergableLabel* p1 = resolve();
		mergableLabel* p2 = p.resolve();
		if ( p1 != p2 )
			p1->pSample = p2;
	}
		/// is given lable a sample label
	bool isSample ( void ) const { return (pSample == NULL); }
}; // mergableLabel

inline
mergableLabel*
mergableLabel :: resolve ( void )
{
	// check if current node is itself sample
	if ( isSample() )
		return this;

	// find a sample of given label
	while ( !pSample->isSample () )
		pSample = pSample->pSample;

	return pSample;
}

#endif
