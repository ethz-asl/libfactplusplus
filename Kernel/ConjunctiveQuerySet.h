/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2013 by Dmitry Tsarkov

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef CONJUNCTIVEQUERYSET_H
#define CONJUNCTIVEQUERYSET_H

#include "QR.h"

// forward declarations
class VariableFactory;
class TExpressionManager;

class CQSet
{
protected:	// members
		/// pointer to an expression manager
	TExpressionManager* pEManager;
		/// pointer to a query var factory
	VariableFactory* VarFactory;
		/// queries
	std::vector<QRQuery*> queries;
		/// flag whether an ABox is original or artificial (individuals correspond class names)
	bool artificialABox;

public:		// interface
		/// init c'tor
	CQSet ( TExpressionManager* pEM, VariableFactory* VarFact, bool art )
		: pEManager(pEM)
		, VarFactory(VarFact)
		, artificialABox(art)
		{}
		/// empty d'tor
	virtual ~CQSet ( void )
	{
		for ( std::vector<QRQuery*>::iterator p = queries.begin(), p_end = queries.end(); p != p_end; ++p )
			delete *p;
	}

	// helper macros
#	undef defC
#	undef defR
#	undef defV

#	define defC(name) ReasoningKernel::TConceptExpr* name = this->pEManager->Concept(#name)
#	define defR(name) ReasoningKernel::TORoleExpr* name = this->pEManager->ObjectRole(#name)
#	define defV(name) const QRVariable* name = this->VarFactory->getNewVar(#name);

		/// @return true if ABox is artificial
	bool isArtificialABox ( void ) const { return artificialABox; }
		/// get RO query by index
	const QRQuery* operator[] ( size_t i ) const { return queries[i]; }
		/// get RW query by index
	QRQuery* operator[] ( size_t i ) { return queries[i]; }
		/// get number of queries
	size_t size ( void ) const { return queries.size(); }
}; // CQSet

#endif
