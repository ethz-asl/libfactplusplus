/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2006 by Dmitry Tsarkov

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

#ifndef _DUMPDIG_H
#define _DUMPDIG_H

#include "dumpInterface.h"
#include "dlTBox.h"	//TConcept/TRole

/// class for dumping ontology to a lisp format
class dumpDIG : public dumpInterface
{
protected:	// methods
		/// dump name of the op (since it is used in both start- and finishOp)
	std::ostream& dumpOpName ( diOp Op );

public:		// interface
		/// the only c'tor -- empty
	dumpDIG ( std::ostream& oo ) : dumpInterface(oo) {}
		/// empty d'tor
	virtual ~dumpDIG ( void ) {}

	// global prologue/epilogue
	virtual void prologue ( void ) {}
	virtual void epilogue ( void ) {}

	// general concept expression
	virtual void dumpTop ( void ) { o << "<top/>"; }
	virtual void dumpBottom ( void ) { o << "<bottom/>"; }
	virtual void dumpNumber ( unsigned int n ) { o << " num=\"" << n << "\""; }

	virtual void startOp ( diOp Op )
	{
		incIndent();
		o << "<";
		dumpOpName(Op);
		o << ">";
	}
		/// start operation >=/<= with number
	virtual void startOp ( diOp Op, unsigned int n )
	{
		incIndent();
		o << "<";
		dumpOpName(Op);
		dumpNumber(n);
		o << ">";
	}
	virtual void contOp ( diOp Op ATTR_UNUSED ) {}
	virtual void finishOp ( diOp Op )
	{
		decIndent();
		o << "</";
		dumpOpName(Op);
		o << ">";
	}

	virtual void startAx ( diAx Ax );
	virtual void contAx ( diAx Ax ATTR_UNUSED ) { incIndent(); skipIndent(); }
	virtual void finishAx ( diAx Ax ATTR_UNUSED ) { o << ")\n"; }

		/// obtain name by the named entry
	virtual void dumpName ( const TNamedEntry* p )
			{ o << " name=\"" << p->getName() << "\""; }
		/// dump concept atom (as used in expression)
	virtual void dumpConcept ( const TConcept* p )
	{
		skipIndent();
		o << "<catom"; dumpName(p); o << "/>";
	}
		/// dump role atom (as used in expression)
	virtual void dumpRole ( const TRole* p )
	{
		if ( p->getId() < 0 )	// inverse
		{
			o << "<inverse> ";
			dumpRole(p->inverse());
			o << " </inverse>";
			return;
		}

		const char* code = NULL;
		if ( p->isDataRole() )
			code = "attribute";
		else if ( p->isFunctional() )
			code = "feature";
		else
			code = "ratom";

		skipIndent();
		o << "<" << code;
		dumpName(p);
		o << "/>";
	}
}; // dumpDIG

#endif
