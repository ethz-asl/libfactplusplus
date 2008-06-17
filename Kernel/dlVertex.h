/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2008 by Dmitry Tsarkov

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

#ifndef _DLVERTEX_H
#define _DLVERTEX_H

#include <vector>
#include <cassert>
#include <iostream>

#include "globaldef.h"
#include "BiPointer.h"
#include "tLabeller.h"
#include "modelCacheInterface.h"
#include "mergableLabel.h"	// for sort inferences

class DLDag;
class TRole;
class TNamedEntry;

/// different Concept Expression tags
enum DagTag {
	// illegal entry
	dtBad = 0,
	// operations
	dtTop,
	dtAnd,
	dtCollection,
	dtForall,
	dtLE,
	dtUAll,		// \dall U.C
	dtIrr,		// \neg\exists R.Self

	// ID's
	dtPConcept,	// primitive concept
	dtNConcept,			// non-primitive concept
	dtPSingleton,
	dtNSingleton,
	dtDataType,
	dtDataValue,
	dtDataExpr,		// data type with restrictions
};

	/// check whether given DagTag is a primitive named concept-like entity
inline bool isPNameTag ( DagTag tag ) { return (tag == dtPConcept || tag == dtPSingleton); }
	/// check whether given DagTag is a non-primitive named concept-like entity
inline bool isNNameTag ( DagTag tag ) { return (tag == dtNConcept || tag == dtNSingleton); }
	/// check whether given DagTag is a named concept-like entity
inline bool isCNameTag ( DagTag tag ) { return isPNameTag(tag) || isNNameTag(tag); }

// define complex switch labels
#define dtConcept dtPConcept: case dtNConcept
#define dtSingleton dtPSingleton: case dtNSingleton
#define dtPrimName dtPConcept: case dtPSingleton
#define dtNonPrimName dtNConcept: case dtNSingleton
#define dtName dtConcept: case dtSingleton
#define dtData dtDataType: case dtDataValue: case dtDataExpr

/// interface for the cache of DLVertex
class DLVertexCache
{
protected:	// members
		/// cache for the positive entry
	const modelCacheInterface* pCache;
		/// cache for the negative entry
	const modelCacheInterface* nCache;

public:		// interface
		/// empty c'tor
	DLVertexCache ( void ) : pCache(NULL), nCache(NULL) {}
		/// d'tor
	virtual ~DLVertexCache ( void ) { delete pCache; delete nCache; }

	// cache interface

		/// return cache wrt positive flag
	const modelCacheInterface* getCache ( bool pos ) const { return pos ? pCache : nCache; }
		/// set cache wrt positive flag; note that cache is set up only once
	void setCache ( bool pos, const modelCacheInterface* p )
	{
		if ( pos )
			pCache = p;
		else
			nCache = p;
	}
}; // DLVertexCache

/// interface for the 'used' value of DLVertex
class DLVertexUsed
{
protected:	// members
		/// usage for the positive and negative entries
	TLabeller::LabType pUsed, nUsed;

public:		// interface
		/// empty c'tor
	DLVertexUsed ( void ) : pUsed(0), nUsed(0) {}
		/// d'tor
	virtual ~DLVertexUsed ( void ) {}

		/// return used value wrt positive flag
	bool isUsed ( bool pos, const TLabeller& lab ) const
		{ return lab.isLabelled(pos ? pUsed : nUsed); }
		/// set cache wrt positive flag
	void setUsed ( bool pos, const TLabeller& lab )
		{ lab.set(pos ? pUsed : nUsed); }
}; // DLVertexUsed

class DLVertexStatistic
{
public:		// types
		/// type for a statistic
	typedef unsigned short int StatType;

protected:	// members
		/// maximal depth, size and frequency of reference of the expression
	StatType stat[10];

public:		// static methods
		/// get access to statistic by the depth of a concept
	static unsigned int getStatIndexDepth ( bool pos ) { return (pos ? 0 : 1); }
		/// get access to statistic by the size of a concept
	static unsigned int getStatIndexSize ( bool pos ) { return (pos ? 2 : 3); }
		/// get access to statistic by the # of branching rules of a concept
	static unsigned int getStatIndexBranch ( bool pos ) { return (pos ? 4 : 5); }
		/// get access to statistic by the # of generating rules of a concept
	static unsigned int getStatIndexGener ( bool pos ) { return (pos ? 6 : 7); }
		/// get access to statistic by the freq of a concept
	static unsigned int getStatIndexFreq ( bool pos ) { return (pos ? 8 : 9); }

public:		// interface
		/// default c'tor
	DLVertexStatistic ( void )
	{
		setStatValues ( 0, 0, 0, 0, /*pos=*/true );
		setStatValues ( 0, 0, 0, 0, /*pos=*/false );
	}
		/// empty d'tor
	virtual ~DLVertexStatistic ( void ) {}

	// set methods

		/// set all values at once
	void setStatValues ( StatType d, StatType s, StatType b, StatType g, bool pos )
	{
		stat[getStatIndexDepth(pos)] = d;
		stat[getStatIndexSize(pos)] = s;
		stat[getStatIndexBranch(pos)] = b;
		stat[getStatIndexGener(pos)] = g;
		stat[getStatIndexFreq(pos)] = 0;
	}
		/// increment frequency value
	void incFreqValue ( bool pos ) { ++stat[getStatIndexFreq(pos)]; }

	// get methods

		/// general access to a stat value by index
	StatType getStat ( unsigned int i ) const { return stat[i]; }
		/// general access to a stat value by index
	StatType getDepth ( bool pos ) const { return stat[getStatIndexDepth(pos)]; }
		/// general access to a stat value by index
	StatType getSize ( bool pos ) const { return stat[getStatIndexSize(pos)]; }
		/// general access to a stat value by index
	StatType getBranch ( bool pos ) const { return stat[getStatIndexBranch(pos)]; }
		/// general access to a stat value by index
	StatType getGener ( bool pos ) const { return stat[getStatIndexGener(pos)]; }
		/// general access to a stat value by index
	StatType getFreq ( bool pos ) const { return stat[getStatIndexFreq(pos)]; }
}; // DLVertexStatistic

/// tag of the vertex and bits and code for efficient DFS algorithms
class DLVertexTagDFS
{
protected:	// members
		/// main operation in concept expression
		// WARNING: the Visual Studio C++ compiler treat this as a signed integer,
		// so I've added extra bit to stay in the unsigned field
	DagTag Op : 5;	// 15 types
		/// aux field for DFS in presence of cycles
	bool VisitedPos : 1;
		/// aux field for DFS in presence of cycles
	bool ProcessedPos : 1;
		/// true iff node is involved in cycle
	bool inCyclePos : 1;
		/// aux field for DFS in presence of cycles
	bool VisitedNeg : 1;
		/// aux field for DFS in presence of cycles
	bool ProcessedNeg : 1;
		/// true iff node is involved in cycle
	bool inCycleNeg : 1;
		/// padding
	unsigned unused : 5;

public:		// interface
		/// default c'tor
	DLVertexTagDFS ( DagTag op )
		: Op(op)
		, VisitedPos(false)
		, ProcessedPos(false)
		, inCyclePos(false)
		, VisitedNeg(false)
		, ProcessedNeg(false)
		, inCycleNeg(false)
		{}
		/// empty d'tor
	virtual ~DLVertexTagDFS ( void ) {}

	// tag access

		/// return tag of the CE
	DagTag Type ( void ) const { return Op; }

	// DFS-related method

		/// check whether current Vertex is being visited
	bool isVisited ( bool pos ) const { return (pos ? VisitedPos : VisitedNeg); }
		/// check whether current Vertex is processed
	bool isProcessed ( bool pos ) const { return (pos ? ProcessedPos : ProcessedNeg); }
		/// set that the node is being visited
	void setVisited ( bool pos ) { if ( pos ) VisitedPos = true; else VisitedNeg = true; }
		/// set that the node' DFS processing is completed
	void setProcessed ( bool pos )
	{
		if ( pos )
		{
			ProcessedPos = true;
			VisitedPos = false;
		}
		else
		{
			ProcessedNeg = true;
			VisitedNeg = false;
		}
	}
		/// clear DFS flags
	void clearDFS ( void ) { ProcessedPos = VisitedPos = ProcessedNeg = VisitedNeg = false; }
		/// check whether concept is in cycle
	bool isInCycle ( bool pos ) const { return (pos ? inCyclePos : inCycleNeg); }
		/// set concept is in cycle
	void setInCycle ( bool pos ) { if ( pos ) inCyclePos = true; else inCycleNeg = true; }
}; // DLVertexTagDFS

/// usage of the particulare vertex during reasoning
class DLVertexUsage
{
public:		// types
		/// type for a statistic
	typedef unsigned long UsageType;

protected:	// members
		/// usage statistic for pos- and neg occurences of a vertex
	UsageType posUsage, negUsage;

public:		// interface
		/// empty c'tor
	DLVertexUsage ( void ) : posUsage(0), negUsage(0) {}
		/// empty d'tor
	virtual ~DLVertexUsage ( void ) {}

		/// get access to a usage wrt POS
	UsageType getUsage ( bool pos ) const { return pos ? posUsage : negUsage; }
		/// increment usage of the node
	void incUsage ( bool pos ) { if ( pos ) ++posUsage; else ++negUsage; }
}; // DLVertexUsage

class DLVertexSort
{
protected:	// members
		/// maximal depth, size and frequency of reference of the expression
	mergableLabel Sort;

public:		// interface
		/// default c'tor
	DLVertexSort ( void ) {}
		/// empty d'tor
	virtual ~DLVertexSort ( void ) {}

	// label access methods

		/// get RW access to the label
	mergableLabel& getSort ( void ) { return Sort; }
		/// get RO access to the label
	const mergableLabel& getSort ( void ) const { return Sort; }
		/// merge local label to label LABEL
	void merge ( mergableLabel& label ) { Sort.merge(label); }
}; // DLVertexSort

/// Class for normalised Concept Expressions
class DLVertex
	: public DLVertexCache
	, public DLVertexUsed
	, public DLVertexStatistic
#ifdef RKG_PRINT_DAG_USAGE
	, public DLVertexUsage
#endif
	, public DLVertexTagDFS
#ifdef RKG_USE_SORTED_REASONING
	, public DLVertexSort
#endif
{
private:	// prevent copying
		// no copy c'tor
	DLVertex ( const DLVertex& v );
		/// no assignment
	DLVertex& operator = ( const DLVertex& v );

protected:	// typedefs
		/// base type for array of BPs
	typedef std::vector<BipolarPointer> BaseType;

public:		// typedefs
		/// RO access to the elements of node
	typedef BaseType::const_iterator const_iterator;
		/// RO access to the elements of node in reverse order
	typedef BaseType::const_reverse_iterator const_reverse_iterator;

protected:	// members
		/// set of arguments (CEs, numbers for NR)
	BaseType Child;
		/// pointer to concept-like entry (for PConcept, etc)
	TNamedEntry* Concept;
		/// pointer to role (for E\A, NR)
	const TRole* Role;

public:		// interface
		/// empty c'tor (do nothing)
	DLVertex ( void )
		: DLVertexTagDFS(dtBad)
		, Concept(NULL)
		, Role(NULL)
		{}
		/// c'tor for Top/CN/And (before adding any operands)
	explicit DLVertex ( DagTag op )
		: DLVertexTagDFS(op)
		, Concept(NULL)
		, Role(NULL)
		{}
		/// c'tor for Refl/Irr
	DLVertex ( DagTag op, const TRole* R )
		: DLVertexTagDFS(op)
		, Concept(NULL)
		, Role(R)
		{}
		/// c'tor for CN/DE; C is an operand
	DLVertex ( DagTag op, BipolarPointer C )
		: DLVertexTagDFS(op)
		, Concept(NULL)
		, Role(NULL)
		{ Child.push_back(C); }
		/// c'tor for <= n R_C; and for \A R{n}_C; Note order C, n, R->pointer
	DLVertex ( DagTag op, BipolarPointer n, const TRole* R, BipolarPointer C )
		: DLVertexTagDFS(op)
		, Concept(NULL)
		, Role(R)
	{
		Child.push_back(C);
		Child.push_back(n);
	}
		/// d'tor (empty)
	virtual ~DLVertex ( void ) {}

		/// compare 2 CEs
	bool operator == ( const DLVertex& v ) const
	{
		return (Type() == v.Type()) &&
			   (Role == v.Role) &&
			   (Child == v.Child);
	}
		/// return TRUE iff CE is functional restriction for some role ( in the form (<= 1 R [TOP]))
	bool isFunctional ( void ) const { return ( Type() == dtLE && getNumberLE() == 1 && getC() == bpTOP ); }
		/// return C for concepts/quantifiers/NR verteces
	BipolarPointer getC ( void ) const { return Child[0]; }
		/// return N for the (<= n R) vertex
	unsigned int getNumberLE ( void ) const { return Child[1]; }
		/// return N for the (>= n R) vertex
	unsigned int getNumberGE ( void ) const { return Child[1]+1; }
		/// return STATE for the (\all R{state}.C) vertex
	unsigned int getState ( void ) const { return Child[1]; }

		/// return pointer to the first concept name of the entry
	const_iterator begin ( void ) const { return Child.begin(); }
		/// return pointer after the last concept name of the entry
	const_iterator end ( void ) const { return (Type() == dtLE || Type() == dtForall) ? Child.begin()+1 : Child.end(); }

		/// return pointer to the last concept name of the entry; WARNING!! works for AND only
	const_reverse_iterator rbegin ( void ) const { return Child.rbegin(); }
		/// return pointer before the first concept name of the entry; WARNING!! works for AND only
	const_reverse_iterator rend ( void ) const { return Child.rend(); }

		/// return pointer to Role for the Role-like verteces
	const TRole* getRole ( void ) const { return Role; }
		/// get (RW) TConcept for concept-like fields
	TNamedEntry* getConcept ( void ) { return Concept; }
		/// get (RO) TConcept for concept-like fields
	const TNamedEntry* getConcept ( void ) const { return Concept; }

 		/// set TConcept value to entry
	void setConcept ( TNamedEntry* p ) { Concept = p; }
		/// set a concept (child) to Name-like vertex
	void setChild ( BipolarPointer p ) { Child.push_back(p); }
		/// adds a child to 'AND' vertex; returns TRUE if contradiction found
	bool addChild ( BipolarPointer p );

	// methods for choosing ordering in the OR fields

		/// whether statistic's gathering should be omitted due to the type of a vertex
	bool omitStat ( bool pos ) const;
		/// gather statistic for the subgraph starting from given node
	void gatherStat ( const DLDag& dag, bool pos );
		/// init statistic for given node based on the stat of the children
	void initStat ( const DLDag& dag, bool pos );
		/// increment frequency of the sub-tree starting from the current node
	void incFreq ( DLDag& dag, bool pos );
		/// sort entry using DAG's compare method
	void sortEntry ( const DLDag& dag );

	// output

		/// get text name for CE tag
	const char* getTagName ( void ) const;
		/// print the whole node
	void Print ( std::ostream& o ) const;
};	// DLVertex


/// whether statistic's gathering should be omitted due to the type of a vertex
inline bool
DLVertex :: omitStat ( bool pos ) const
{
	switch ( Type() )
	{
	case dtDataType:
	case dtDataValue:
	case dtDataExpr:
	case dtBad:
	case dtTop:
		return true;
	case dtPConcept:
	case dtPSingleton:
	case dtCollection:
		return !pos;
	default:
		return false;
	}
}

/**
 *	returns true iff corresponding NRs may clash.
 *	Clash may appears for (>= n R) and (<= m R) if n > m.
 *	Since \neg (<= n R) represents (>= (n+1) R), so
 *	comparison became (n+1) > m, or n >= m
 */
inline bool mayClashNR ( unsigned int geNR, unsigned int leNR )
{
	return geNR >= leNR;
}

#endif
