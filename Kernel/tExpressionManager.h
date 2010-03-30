/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2010 by Dmitry Tsarkov

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

#ifndef TEXPRESSIONMANAGER_H
#define TEXPRESSIONMANAGER_H

#include "tDLExpression.h"

/// manager to work with all DL expressions in the kernel
class TExpressionManager
{
protected:	// members
		/// TOP concept
	TDLConceptTop* CTop;
		/// BOTTOM concept
	TDLConceptBottom* CBottom;
		/// TOP object role
	TDLObjectRoleTop* ORTop;
		/// BOTTOM object role
	TDLObjectRoleBottom* ORBottom;
		/// TOP data role
	TDLDataRoleTop* DRTop;
		/// BOTTOM data role
	TDLDataRoleBottom* DRBottom;
		/// TOP data element
	TDLDataTop* DTop;
		/// BOTTOM data element
	TDLDataBottom* DBottom;

public:		// interface
		/// empty c'tor
	TExpressionManager ( void );
		/// d'tor
	~TExpressionManager ( void );

		/// clear the ontology
	void clear ( void );

	// create expressions methods

	// concepts

		/// get TOP concept
	TDLConceptExpression* getConceptTop ( void ) const { return CTop; }
		/// get BOTTOM concept
	TDLConceptExpression* getConceptBottom ( void ) const { return CBottom; }

	// object roles

		/// get TOP object role
	TDLObjectRoleExpression* getObjectRoleTop ( void ) const { return ORTop; }
		/// get BOTTOM object role
	TDLObjectRoleExpression* getObjectRoleBottom ( void ) const { return ORBottom; }

	// data roles

		/// get TOP data role
	TDLDataRoleExpression* getDataRoleTop ( void ) const { return DRTop; }
		/// get BOTTOM data role
	TDLDataRoleExpression* getDataRoleBottom ( void ) const { return DRBottom; }

	// data expressions

		/// get TOP data element
	TDLDataExpression* getDataTop ( void ) const { return DTop; }
		/// get BOTTOM data element
	TDLDataExpression* getDataBottom ( void ) const { return DBottom; }
}; // TExpressionManager

#endif
