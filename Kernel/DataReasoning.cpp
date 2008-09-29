/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2005-2008 by Dmitry Tsarkov

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

#include "DataReasoning.h"

//toms code start
bool DataTypeReasoner :: addDataEntry ( BipolarPointer p, const DepSet& dep )
{
	const DLVertex& v = DLHeap[p];

	switch ( v.Type() )
	{
	case dtDataType:		// get appropriate type
	{
		DataTypeAppearance* type = getDTAbyType(getDataEntry(p));

		if ( LLM.isWritable(llCDAction) )	// level of logging
			LL << ' ' << (isPositive(p) ? '+' : '-') << getDataEntry(p)->getName();

		if ( isPositive(p) )
			type->setPType(getDTE(p,dep));
		else
			type->NType = getDTE(p,dep);

		return false;	// no clash found
	}
	case dtDataValue:
		return isNegative(p) ?
			processNegativeDV(getDTE(p,dep)) :
			processDataValue ( /*pos=*/true, getDataEntry(p), dep );
	case dtDataExpr:
		return processDataExpr ( isPositive(p), getDataEntry(p), dep );
	default:
		assert(false);
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

				clashDep = type->PType.second+(*p)->PType.second;
				return true;
			}
		}

	// check the case where an interval /min..max/ is completed with !min,...,!max
	return type ? type->checkPNTypeClash() : false;
}

// ---------- Processing different alternatives

bool
DataTypeAppearance::DepInterval :: checkMinMaxClash ( const DataTypeAppearance::SingleValues& values, DepSet& dep ) const
{
	// we are interested in a NEG intervals iff a PType is set
	if ( !Constraints.closed() )
		return false;

	const ComparableDT& Min = Constraints.min->getComp();
	const ComparableDT& Max = Constraints.max->getComp();

	// normal interval
	if ( Min < Max )
		return false;

	// >?x and <?y leads to clash for y < x
	// >5 and <5, >=5 and <5, >5 and <=5 leads to clash
	if ( Max < Min || Constraints.minExcl || Constraints.maxExcl )
	{
		dep += minDep;
		dep += maxDep;
		return true;
	}

	// here we have an interval [X,X]
	// if X is in negValues, then clash occurs
	DepSet loc;
	if ( values.find ( Min, loc ) )
	{
		dep += minDep;
		dep += maxDep;
		dep += loc;
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

bool
DataTypeAppearance :: checkPNTypeClash ( void )
{
	// check if NType is already present
	if ( hasNType() )
		return reportClash ( PType.second+NType.second, "TNT" );

	DepSet acc(accDep), dep;
	for ( const_iterator p = Constraints.begin(), p_end = Constraints.end(); p < p_end; ++p )
	{
		dep.clear();
		if ( p->isCovered ( negValues, dep ) )
			acc += dep;
		else
			return false;
	}

	// NType can be inferred
	return reportClash ( PType.second+acc, "NI" );
}

bool
DataTypeAppearance::DepInterval :: isCovered ( const DataTypeAppearance::SingleValues& values,
											   DepSet& dep ) const
{
	// not a closed interval -- can't infer NType
	//FIXME!! check this if/when enumeration types will appear
	if ( !Constraints.closed() )
		return false;

	/*
	 * check if you can infer NType from all negative constraints
	 * Examples include not(<5) && not(5) && not(6) && not(>=7)
	 */
	const ComparableDT& Min = Constraints.min->getComp();
	const ComparableDT& Max = Constraints.max->getComp();

	if ( !Min.hasDiscreteType() )
		return false;

	long startInt = Min.getLongIntValue() + Constraints.minExcl;
	long finalInt = Max.getLongIntValue() - Constraints.maxExcl;

	dep += minDep;
	dep += maxDep;
	return values.covers ( startInt, finalInt, dep );
}

bool
DataTypeAppearance :: SingleValues :: find ( const ComparableDT& value, DepSet& dep ) const
{
	for ( DEVector::const_iterator i = Base.begin(), i_end = Base.end(); i < i_end; ++i )
		if ( value == i->first->getComp() )
		{
			dep += i->second;
			return true;
		}

	return false;
}

bool
DataTypeAppearance :: SingleValues :: covers ( long from, long to, DepSet& dep ) const
{

	for ( long l = from; l <= to; ++l )
		if ( !find ( ComparableDT(l), dep ) )
			return false;

	// all interval is covered
	return true;
}
