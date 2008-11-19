/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2005 by Dmitry Tsarkov

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

#include "dumpDIG.h"

std::ostream& dumpDIG :: dumpOpName ( diOp Op )
{
	switch (Op)
	{
	case diErrorOp:
	default:
		assert (0);
		// concept expressions
	case diNot:
		o << "not";
		break;
	case diAnd:
		o << "and";
		break;
	case diOr:
		o << "or";
		break;
	case diExists:
		o << "some";
		break;
	case diForall:
		o << "all";
		break;
	case diGE:
		o << "atleast";
		break;
	case diLE:
		o << "atmost";
		break;
	}

	return o;
}

void dumpDIG :: startAx ( diAx Ax )
{
	o << "<";

	switch (Ax)
	{
	case diErrorAx:
	default:
		assert (0);
		// concept axioms
	case diDefineC:
		o << "defconcept";
		break;
	case diImpliesC:
		o << "impliesc";
		break;
	case diEqualsC:
		o << "equalc";
		break;
		// role axioms
	case diDefineR:
		o << "defrole";
		break;
	case diTransitiveR:
		o << "transitive";
		break;
	case diFunctionalR:
		o << "functional";
		break;
	case diImpliesR:
		o << "impliesr";
		break;
	case diEqualsR:
		o << "equalr";
		break;
	case diDomainR:
		o << "domain";
		break;
	case diRangeR:
		o << "range";
		break;
	};

	contAx(Ax);
}
