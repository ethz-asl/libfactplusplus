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

#ifndef _DUMPINTERFACE_H
#define _DUMPINTERFACE_H

#include <ostream>

#include "globaldef.h"
#include "tNamedEntry.h"
#include "dltree.h"

class TConcept;
class TRole;

/// enumeration of dump interface concept operations
enum diOp
{		// concept expressions
	diNot,
	diAnd,
	diOr,
	diExists,
	diForall,
	diGE,
	diLE,
		// role expressions
	diInv,
		// individual expressions
	diOneOf,
		// wrong operation
	diErrorOp,
		// end of the enum
	diLastOp = diErrorOp
}; // diOp

/// enumeration of dump interface axioms
enum diAx
{		// wrong axiom
	diErrorAx = diLastOp,
		// concept axioms
	diDefineC,
	diImpliesC,
	diEqualsC,
	diDisjointC,
		// role axioms
	diDefineR,
	diTransitiveR,
	diFunctionalR,
	diImpliesR,
	diEqualsR,
	diDomainR,
	diRangeR,
		// individual axioms
	diInstanceOf,
}; // diAx

/// general interface for dumping ontology to a proper format
class dumpInterface
{
protected:	// members
		/// output stream
	std::ostream& o;
		/// indentation level
	unsigned int indent;

protected:	// methods
		/// write necessary amount of TABs
	void skipIndent ( void );
		/// increase indentation level
	void incIndent ( void );
		/// decrease indentation level
	void decIndent ( void );

public:		// interface
		/// the only c'tor -- empty
	dumpInterface ( std::ostream& oo ) : o(oo), indent(0) {}
		/// empty d'tor
	virtual ~dumpInterface ( void ) {}

	// global prologue/epilogue
	virtual void prologue ( void ) {}
	virtual void epilogue ( void ) {}

	// general concept expression
	virtual void dumpTop ( void ) {}
	virtual void dumpBottom ( void ) {}
	virtual void dumpNumber ( unsigned int n ATTR_UNUSED ) {}

	virtual void startOp ( diOp Op ATTR_UNUSED ) {}
		/// start operation >=/<= with number
	virtual void startOp ( diOp Op ATTR_UNUSED, unsigned int n ATTR_UNUSED ) {}
	virtual void contOp ( diOp Op ATTR_UNUSED ) {}
	virtual void finishOp ( diOp Op ATTR_UNUSED ) {}

	virtual void startAx ( diAx Ax ATTR_UNUSED ) {}
	virtual void contAx ( diAx Ax ATTR_UNUSED ) {}
	virtual void finishAx ( diAx Ax ATTR_UNUSED ) {}

		/// obtain name by the named entry
	virtual void dumpName ( const TNamedEntry* p ) { o << p->getName(); }
		/// dump concept atom (as used in expression)
	virtual void dumpConcept ( const TConcept* p ATTR_UNUSED ) {}
		/// dump role atom (as used in expression)
	virtual void dumpRole ( const TRole* p ATTR_UNUSED ) {}
}; // dumpInterface

inline void dumpInterface :: skipIndent ( void )
{
	o << "\n";
	for ( int i = indent-1; i >= 0; --i )
		o << "  ";
}

inline void dumpInterface :: incIndent ( void )
{
	skipIndent();
	++indent;	// operands of AND-like
}

inline void dumpInterface :: decIndent ( void )
{
	--indent;
	skipIndent();
}

	// dump given concept expression
void dumpCExpression ( dumpInterface* dump, const DLTree* C );
	// dump given role expression
void dumpRExpression ( dumpInterface* dump, const DLTree* R );

#endif
