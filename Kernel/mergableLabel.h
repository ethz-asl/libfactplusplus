/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2007 by Dmitry Tsarkov

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

#ifndef _MERGABLE_LABEL
#define _MERGABLE_LABEL

/// implementation of labels that could be compared and merged to each other
class mergableLabel
{
protected:	// members
		/// sample for all equivalent labels
	mergableLabel* pSample;

public:		// interface
		/// empty c'tor
	mergableLabel ( void ) : pSample(this) {}
		/// copy c'tor
	mergableLabel ( mergableLabel& p ) : pSample(p.resolve()) {}
		/// assignment
	mergableLabel& operator = ( mergableLabel& p ) { pSample = p.resolve(); return *this; }
		/// d'tor (does nothing)
	~mergableLabel ( void ) {}

	// general interface

		/// are 2 labels equal; works only for normalised labels
	bool operator == ( const mergableLabel& p ) const { return (pSample == p.pSample); }
		/// are 2 labels different; works only for normalised labels
	bool operator != ( const mergableLabel& p ) const { return (pSample != p.pSample); }
		/// make 2 labels equal
	void merge ( mergableLabel& p )
	{
		mergableLabel* sample = p.resolve();
		resolve();
		if ( pSample != sample )
			pSample->pSample = sample;
	}
		/// make label's depth <= 2; @return sample of the label
	mergableLabel* resolve ( void )
	{
		// check if current node is itself sample
		if ( !isSample() )
			pSample = pSample->resolve();

		return pSample;
	}
		/// is given lable a sample label
	bool isSample ( void ) const { return (pSample == this); }
}; // mergableLabel

#endif
