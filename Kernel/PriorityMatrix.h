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

#ifndef _PRIORITYMATRIX_H
#define _PRIORITYMATRIX_H

#include "dlVertex.h"	// DagTag
#include "logging.h"

/// number of regular options (o- and NN-rules are not included)
const unsigned int nRegularOps = 7;
/// priority index for o- and ID operations (note that these ops have the highest priority)
const unsigned int iId = nRegularOps+1;
/// priority index for <= operation in nominal node
const unsigned int iNN = nRegularOps+2;

/// Auxiliary class to get priorities on operations
class ToDoPriorMatrix
{
protected:	// members
	// regular operation indexes
	unsigned int iAnd;
	unsigned int iOr;
	unsigned int iExists;
	unsigned int iForall;
	unsigned int iLE;
	unsigned int iGE;

public:		// interface
		/// empty c'tor
	ToDoPriorMatrix ( void ) {}
		/// empty d'tor
	~ToDoPriorMatrix ( void ) {}

		/// init priorities via given string OPTIONS. @return true if couldn't.
	bool initPriorities ( const std::string& options, const char* optionName );
		/// get an index corresponding given Op, Sign and NominalNode
	unsigned int getIndex ( DagTag Op, bool Sign, bool NominalNode ) const;
}; // ToDoPriorMatrix

inline bool ToDoPriorMatrix :: initPriorities ( const std::string& options, const char* optionName )
{
	// check for correctness
	if ( options.size () < 7 )
		return true;

	// init values by symbols loaded
	iAnd	= options[1] - '0';
	iOr		= options[2] - '0';
	iExists	= options[3] - '0';
	iForall	= options[4] - '0';
	iLE		= options[5] - '0';
	iGE		= options[6] - '0';

	// correctness checking
	if ( iAnd >= nRegularOps ||
		 iOr >= nRegularOps ||
		 iExists >= nRegularOps ||
		 iForall >= nRegularOps ||
		 iGE >= nRegularOps ||
		 iLE >= nRegularOps )
		return true;

	// inform about used rules order
	if ( LLM.isWritable(llAlways) )
		LL << "\nInit " << optionName << " = " << iAnd << iOr << iExists << iForall << iLE << iGE;

	// all was inited OK
	return false;
}

inline unsigned int ToDoPriorMatrix :: getIndex ( DagTag Op, bool Sign, bool NominalNode ) const
{
	switch ( Op )
	{
	case dtAnd:
		return (Sign?iAnd:iOr);

	case dtForall:
	case dtUAll:
	case dtIrr:		// process local (ir-)reflexivity as a FORALL
		return (Sign?iForall:iExists);

	case dtLE:
		return (Sign?(NominalNode?iNN:iLE):iGE);

	case dtDataType:
	case dtDataValue:
	case dtDataExpr:
	case dtTop:			// no need to process these ops
		return nRegularOps;

	case dtPSingleton:
	case dtPConcept:	// no need to process neg of PC
		return (Sign?iId:nRegularOps);

	case dtNSingleton:
	case dtNConcept:	// both NC and neg NC are processed
		return iId;

	default:	// safety check
		assert(0);
	}
}

#endif
