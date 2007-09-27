/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2007 by Dmitry Tsarkov

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

#include "Precomplete.h"

bool
Precompletor :: performPrecompletion ( void )
{
	// init precompletion process
	initPrecompletion();

	// propagate transitive and other RIA throughout individual cloud
	propagateRIA();

	// init ToDo List with elements in every individuals' description
	initToDoList();

	// now ind. cloud is fixed (as we don't allow nominals and merging)
	// so do the R&D addition to the labels
	addRnD();

	// main cycle: expand every not-yet-expanded concept expression
	// it throws exception if it is unable to finish a single precompletion for whatever reason
	try
	{
		if ( runPrecompletion() )	// clash found
		{
			KB.setConsistency(false);
			return false;
		}
	}
	catch ( PCException )	// fail to produce precompletion
	{
		return true;
	}

	// here precompletion suceeds
	updateIndividualsFromPrecompletion();
	KB.setPrecompleted();
	return false;
}

bool
Precompletor :: runPrecompletion ( void ) throw(PCException)
{
	while ( !ToDoList.empty() )
	{
		PCToDoEntry cur = ToDoList.get();
		switch ( cur.Expr->Element().getToken() )
		{
		case TOP:	// nothing to do
			break;
		case BOTTOM:	// clash
			return true;
		case CNAME:		// expand
			addToDoEntry ( cur.Ind,
						   static_cast<const TConcept*>(cur.Expr->Element().getName())->Description );
			break;
		case AND:
			processTree ( cur.Ind, cur.Expr );
			break;
		case FORALL:
			processForall ( cur.Ind, static_cast<const TRole*>(cur.Expr->Left()->Element().getName()),
									 cur.Expr->Right() );
			break;
		case NOT:
			switch ( cur.Expr->Left()->Element().getToken() )
			{
			case FORALL:	// \exists restriction: nothing to do
				break;
			default:
				std::cerr << "Unsupported concept expression: " << cur.Expr->Left() << "\n";
				throw PCException();
			}
			break;
		default:
			std::cerr << "Unsupported concept expression: " << cur.Expr << "\n";
			throw PCException();
		}
	}

	return false;
}
