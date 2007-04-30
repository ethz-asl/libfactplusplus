/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2005-2007 by Dmitry Tsarkov

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
#include "dlTBox.h"
#include "logging.h"

/// report about clash with dep-set DEP and given REASON and return TRUE (to simplify caller)
static bool reportClash ( const DepSet& dep, const char* reason )
{
	if ( LLM.isWritable(llCDAction) )	// level of logging
		LL << " DT-" << reason;	// inform about clash...

	DlCompletionTree::setClashSet(dep);
	return true;
}

static void logDataValueEntry ( bool pos, const char* value )
{
	if ( LLM.isWritable(llCDAction) )	// level of logging
		LL << ' ' << (pos ? '+' : '-') << value;
}

//toms code start
bool DataTypeReasoner :: addDataEntry ( const ConceptWDep& c )
{
	BipolarPointer p = c.bp();
	DagTag tagType = DLHeap[p].Type();

	switch (tagType)
	{
	case dtDataType:		// get appropriate type
	{
		DataTypeAppearance& type = getDTAbyType(getDataEntry(p));

		logDataValueEntry ( isPositive(p), getDataEntry(p)->getName() );

		if ( isPositive(p) )
			type.setPType(getDTE(c));
		else
			type.NType = getDTE(c);

		return false;	// no clash found
	}
	case dtDataValue:
		logDataValueEntry ( isPositive(p), getDataEntry(p)->getName() );
		return isNegative(p) ?
			processNegativeDV(getDTE(c)) :
			processRestriction ( /*pos=*/true, /*min=*/false, /*excl=*/false,
								 getDataEntry(p), c.getDep() ) ||
			processRestriction ( /*pos=*/true, /*min=*/true, /*excl=*/false,
								 getDataEntry(p), c.getDep() );
	case dtDataExpr:
		return processDataExpr ( isPositive(p), getDataEntry(p), c.getDep() );
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
	const DataTypeAppearance* type = NULL;

	// find a positive class
	for ( DTAVector::const_iterator p = Types.begin(), p_end = Types.end(); p < p_end; ++p )
		if ( p->hasPType() )
		{
			if ( type == NULL )
				type = &*p;
			else	// 2 different positive classes => contradiction
				return reportClash ( type->PType.second+p->PType.second, "TT" );
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
DataTypeAppearance :: update ( bool min, bool excl, const TDataEntry* value, const DepSet& dep )
{
	DTConstraint aux;
	Constraints.swap(aux);
	for ( iterator p = aux.begin(), p_end = aux.end(); p < p_end; ++p )
	{
		if ( !p->update ( min, excl, value, dep ) )
			Constraints.push_back(*p);
		if ( !hasPType() || !p->checkMinMaxClash(negValues,accDep) )
			Constraints.push_back(*p);
	}
	aux.clear();
	if ( Constraints.empty() )
		return reportClash ( accDep, "C-MM" );
	return false;
}

bool
DataTypeAppearance :: addInterval ( bool pos, const TDataInterval& Int, const DepSet& dep )
{
	if ( LLM.isWritable(llCDAction) )	// level of logging
		LL << ' ' << (pos ? '+' : '-') << Int;

	if ( pos )	// positive interval -- just call UPDATE twice
	{
		if ( Int.hasMin() )
			if ( update ( /*min=*/true, /*excl=*/Int.minExcl, Int.min, dep ) )
				return true;
		if ( Int.hasMax() )
			if ( update ( /*min=*/false, /*excl=*/Int.maxExcl, Int.max, dep ) )
				return true;
		return false;
	}

	// negative interval -- make a copies
	DTConstraint aux;
	Constraints.swap(aux);
	const_iterator p, p_end = aux.end();
	bool min, excl;
	const TDataEntry* value;

	if ( Int.hasMin() )
	{
		min = false;
		excl = !Int.minExcl;
		value = Int.min;

		for ( p = aux.begin(); p < p_end; ++p )
		{
			DepInterval temp(*p);
			if ( !temp.update ( min, excl, value, dep ) )
				Constraints.push_back(temp);
			if ( !hasPType() || !temp.checkMinMaxClash(negValues,accDep) )
				Constraints.push_back(temp);
		}
	}
	if ( Int.hasMax() )
	{
		min = true;
		excl = !Int.maxExcl;
		value = Int.max;

		for ( p = aux.begin(); p < p_end; ++p )
		{
			DepInterval temp(*p);
			if ( !temp.update ( min, excl, value, dep ) )
				Constraints.push_back(temp);
			if ( !hasPType() || !temp.checkMinMaxClash(negValues,accDep) )
				Constraints.push_back(temp);
		}
	}
	aux.clear();
	if ( Constraints.empty() )
		return reportClash ( accDep, "C-MM" );
	return false;
}

bool
DataTypeAppearance :: checkPNTypeClash ( void ) const
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
