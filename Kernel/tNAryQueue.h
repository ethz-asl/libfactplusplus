/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2008 by Dmitry Tsarkov

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

#ifndef TNARYQUEUE_H
#define TNARYQUEUE_H

#include <vector>
#include "dltree.h"

typedef std::vector<DLTree*> DLExpressionArray;

/// queue for n-ary operations
class TNAryQueue
{
protected:	// types
		/// type of a base storage
	typedef std::vector<DLExpressionArray*> BaseType;
		/// base storage iterator
	typedef BaseType::iterator iterator;

private:	// members
		/// all lists of arguments for n-ary predicates/commands
	BaseType Base;
		/// pointer to the current n-ary array
	DLExpressionArray* Current;
		/// pre-current index of n-ary statement
	int level;

private:	// methods
		/// increase size of internal AUX array
	void grow ( void )
	{
		unsigned int n = Base.size();
		Base.resize(2*n);
		for ( iterator p = Base.begin()+n, p_end = Base.end(); p < p_end; ++p )
			*p = new DLExpressionArray;
	}

public:		// interface
		/// empty c'tor
	TNAryQueue ( void ) : level(-1) { Base.push_back(new DLExpressionArray); }
		/// d'tor
	~TNAryQueue ( void )
	{
		for ( iterator q = Base.begin(), q_end = Base.end(); q < q_end; ++q )
			delete *q;
	}

	// queue interface

		/// init the next argument list
	void openArgList ( void )
	{
		if ( (unsigned)++level >= Base.size() )
			grow();
		Base[level]->clear();
	}
		/// add the next element to the current argument list
	void addArg ( DLTree* p ) { Base[level]->push_back(p); }

		/// get access to the last closed argument list
	const DLExpressionArray& getLastArgList ( void ) { return *Base[level--]; }
}; // TNAryQueue

#endif
