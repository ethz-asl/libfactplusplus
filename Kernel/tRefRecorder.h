/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2008-2009 by Dmitry Tsarkov

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

#ifndef TREFRECORDER_H
#define TREFRECORDER_H

// define REF_RECORDER_TRACING to get traces of registering/deleting pointers
#include <vector>

#ifdef REF_RECORDER_TRACING
#	include <iostream>
#endif

#include "dltree.h"

/// record Tree pointers and delete them when became useless (to reduce memory leaks)
class TRefRecorder
{
protected:	// types
		/// repository type
	typedef std::vector<DLTree*> RefVector;

protected:	// members
		/// repository of references
	RefVector refs;

public:		// interface
		/// empty c'tor
	TRefRecorder ( void ) {}
		/// d'tor
	~TRefRecorder ( void ) { clear(); }

		/// add reference to a repository
	void add ( DLTree* p )
	{
#	ifdef REF_RECORDER_TRACING
		std::cerr << "Registering (" << (void*)p << ")" << p << "\n";
#	endif
		refs.push_back(p);
	}
		/// check whether P is in the repository
	bool in ( DLTree* p ) const { return std::find ( refs.begin(), refs.end(), p ) != refs.end(); }
		/// clear repository, free all memory
	void clear ( void )
	{
		for ( RefVector::iterator p = refs.begin(), p_end = refs.end(); p < p_end; ++p )
		{
#		ifdef REF_RECORDER_TRACING
			std::cerr << "Deleting (" << (void*)(*p) << ")" << *p << "\n";
#		endif
			deleteTree(*p);
		}
		refs.clear();
	}
}; // TRefRecorder

#endif
