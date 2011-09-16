/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2005-2011 by Dmitry Tsarkov

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

#include "DataReasoning.h"

//toms code start
bool DataTypeReasoner :: addDataEntry ( BipolarPointer p, const DepSet& dep )
{
	switch ( DLHeap[p].Type() )
	{
	case dtDataType:		// get appropriate type
	{
		DataTypeAppearance* type = getDTAbyType(getDataEntry(p));

		if ( LLM.isWritable(llCDAction) )	// level of logging
			LL << ' ' << (isPositive(p) ? '+' : '-') << getDataEntry(p)->getName();

		type->setTypePresence ( isPositive(p), dep );
		return type->checkPNTypeClash();
	}
	case dtDataValue:
		return processDataValue ( isPositive(p), getDataEntry(p), dep );
	case dtDataExpr:
		return processDataExpr ( isPositive(p), getDataEntry(p), dep );
	case dtAnd:		// processed by general reasoning
		return false;
	default:
		fpp_unreachable();
		return true;
	}
}

	// try to find contradiction:

	// -- if we have 2 same elements or direct contradiction (like "p" and "(not p)")
	//    then addConcept() will eliminate this;
	// => negations are not interesting also (p & ~p are eliminated; ~p means "all except p").
	// -- all cases with 2 different values of the same class are found in previous search;
	// -- The remaining problems are
	//   - check if there are 2 different positive classes
	//   - check if some value is present together with negation of its class
	//   - check if some value is present together with the other class
	//   - check if two values of different classes are present at the same time

bool DataTypeReasoner :: checkClash ( void )
{
	DataTypeAppearance* type = NULL;

	// find a positive class
	for ( DTAVector::const_iterator p = Types.begin(), p_end = Types.end(); p < p_end; ++p )
		if ( (*p)->hasPType() )
		{
			if ( type == NULL )
				type = *p;
			else	// 2 different positive classes => contradiction
			{
				if ( LLM.isWritable(llCDAction) )	// level of logging
					LL << " DT-TT";					// inform about clash...

				clashDep = *(type->PType);
				clashDep += *((*p)->PType);
				return true;
			}
		}

	// no clash like (PType, NType) can happen as this will be checked earlier
	fpp_assert ( !type || !type->checkPNTypeClash() );
	return false;
}

// ---------- Processing different alternatives

bool
DataTypeAppearance::DepInterval :: checkMinMaxClash ( DepSet& dep ) const
{
	// we are interested in a NEG intervals iff a PType is set
	if ( !Constraints.closed() )
		return false;

	const ComparableDT& Min = Constraints.min;
	const ComparableDT& Max = Constraints.max;

	// normal interval
	if ( Min < Max )
		return false;

	// >?x and <?y leads to clash for y < x
	// >5 and <5, >=5 and <5, >5 and <=5 leads to clash
	if ( Max < Min || Constraints.minExcl || Constraints.maxExcl )
	{
		dep += locDep;
		return true;
	}

	return false;
}

bool
DataTypeAppearance :: addPosInterval ( const TDataInterval& Int, const DepSet& dep )
{
	DTConstraint aux;
	if ( Int.hasMin() )
	{
		Constraints.swap(aux);
		setLocal ( /*min=*/true, /*excl=*/Int.minExcl, Int.min, dep );
		if ( addIntervals ( aux.begin(), aux.end() ) )
			return true;
		aux.clear();
	}
	if ( Int.hasMax() )
	{
		Constraints.swap(aux);
		setLocal ( /*min=*/false, /*excl=*/Int.maxExcl, Int.max, dep );
		if ( addIntervals ( aux.begin(), aux.end() ) )
			return true;
		aux.clear();
	}
	if ( Constraints.empty() )
		return reportClash ( accDep, "C-MM" );
	return false;
}

bool
DataTypeAppearance :: addNegInterval ( const TDataInterval& Int, const DepSet& dep )
{
	// negative interval -- make a copies
	DTConstraint aux;
	Constraints.swap(aux);

	if ( Int.hasMin() )
	{
		setLocal ( /*min=*/false, /*excl=*/!Int.minExcl, Int.min, dep );
		if ( addIntervals ( aux.begin(), aux.end() ) )
			return true;
	}
	if ( Int.hasMax() )
	{
		setLocal ( /*min=*/ true, /*excl=*/!Int.maxExcl, Int.max, dep );
		if ( addIntervals ( aux.begin(), aux.end() ) )
			return true;
	}
	aux.clear();
	if ( Constraints.empty() )
		return reportClash ( accDep, "C-MM" );
	return false;
}
