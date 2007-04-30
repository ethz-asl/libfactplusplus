/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2006 by Dmitry Tsarkov

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

#ifndef __IFQUERY_H
#define __IFQUERY_H

#include <cassert>
#include <iostream>

#include "dltree.h"

/// class for querying DL Kernel (with reasoning tasks)
class ifQuery
{
protected:
		/// first and second target concepts
	DLTree* Target [2];

public:
		/// C'tor
	ifQuery ( void ) { Target[0] = Target[1] = NULL; }
		/// D'tor
	~ifQuery ( void ) { delete Target[0]; delete Target[1]; }

		/// check if query is correct
	bool isCorrect ( void ) const { return Target[0] != NULL || Target[1] == NULL; }

	// type of query

		/// whether query is SAT test
	bool isSat ( void ) const { return Target[0] != NULL && Target[1] == NULL; }
		/// whether query is SUB test
	bool isSub ( void ) const { return Target[0] != NULL && Target[1] != NULL; }
		/// whether query is classification
	bool isClassification ( void ) const { return Target[0] == NULL; }

	// targets of query

		/// get target Concept description
	DLTree* getTarget ( unsigned int index ) const
	{
		assert ( index < 2 );
		return Target[index];
	}
		/// set target Concept description
	void setTarget ( unsigned int index, DLTree* target )
	{
		assert ( index < 2 );
		Target[index] = target;
	}

		/// write query
	void writeQuery ( std::ostream& o ) const
	{
		assert ( isCorrect() );	// sanity check

		if ( isClassification() )
			o << "classification";
		else if ( isSat() )
			o << "satisfiability of" << Target[0];
		else if ( isSub() )
			o << "subsumption" << Target[0] << " [=" << Target[1];
	}
}; // ifQuery

#endif
