/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2015 by Dmitry Tsarkov

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

#ifndef DUMPLISP_H
#define DUMPLISP_H

#include "dumpInterface.h"
#include "dlTBox.h"	//TConcept/TRole

/// class for dumping ontology to a lisp format
class dumpLisp : public dumpInterface
{
public:		// interface
		/// the only c'tor -- empty
	dumpLisp ( std::ostream& oo ) : dumpInterface(oo) {}
		/// empty d'tor
	virtual ~dumpLisp ( void ) {}

	// global prologue/epilogue
	virtual void prologue ( void ) {}
	virtual void epilogue ( void ) {}

	// general concept expression
	virtual void dumpTop ( void ) { o << "*TOP*"; }
	virtual void dumpBottom ( void ) { o << "*BOTTOM*"; }
	virtual void dumpNumber ( unsigned int n ) { o << n << " "; }

	virtual void startOp ( diOp Op );
		/// start operation >=/<= with number
	virtual void startOp ( diOp Op, unsigned int n ) { startOp(Op); dumpNumber(n); }
	virtual void contOp ( diOp Op )
	{
		if ( Op == diAnd || Op == diOr )
			skipIndent();
		else
			o << " ";
	}
	virtual void finishOp ( diOp Op )
	{
		if ( Op == diAnd || Op == diOr )
			decIndent();
		o << ")";
	}

	virtual void startAx ( diAx Ax );
	virtual void contAx ( diAx ) { o << " "; }
	virtual void finishAx ( diAx ) { o << ")\n"; }

		/// obtain name by the named entry
	virtual void dumpName ( const TNamedEntry* p ) { o << "|" << p->getName() << "|"; }
		/// dump concept atom (as used in expression)
	virtual void dumpConcept ( const TConcept* p ) { dumpName(p); }
		/// dump role atom (as used in expression)
	virtual void dumpRole ( const TRole* p )
	{
		if ( p->getId() < 0 )	// inverse
		{
			o << "(inv ";
			dumpName(p->inverse());
			o << ")";
		}
		else
			dumpName(p);
	}
}; // dumpLisp

#endif
