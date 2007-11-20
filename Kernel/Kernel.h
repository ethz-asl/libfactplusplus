/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2007 by Dmitry Tsarkov

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

#ifndef _KERNEL_H
#define _KERNEL_H

#include <cassert>
#include <string>

#include "dlTBox.h"
#include "Reasoner.h"
#include "ifOptions.h"
#include "DLConceptTaxonomy.h"	// for getRelatives()

using namespace std;

class UnInitException : public exception
{
public:
	UnInitException ( void ) throw() {}
	~UnInitException ( void ) throw() {}
	const char* what ( void ) const throw() { return "FaCT++ Kernel: KB Not Initialised"; }
}; // UnInitException

class InconsistentKB : public exception
{
public:
	InconsistentKB ( void ) throw() {}
	~InconsistentKB ( void ) throw() {}
	const char* what ( void ) const throw() { return "FaCT++ Kernel: Inconsistent KB"; }
}; // InconsistentKB

class ReasoningKernel
{
public:	// types interface
	// single concept name is just its index
	typedef TNamedEntry* ConceptName;
	// role name can bi direct or inverted
	typedef TNamedEntry* RoleName;
	// datatype and/or datavalue name
	typedef TNamedEntry* DataName;
	// individual name is just a concept name
	typedef ConceptName IndividualName;
	// complex concept is DLTree for it
	typedef DLTree* ComplexConcept;
	// complex role expression is DLTree for it
	typedef DLTree* ComplexRole;
	// individual expression is DLTree for it
	typedef DLTree* IndividualExpression;

	// name sets
	typedef TaxonomyVertex::EqualNames ConceptList;
	typedef vector<ConceptName> ConceptNameSet;
	typedef vector<RoleName> RoleNameSet;
	typedef vector<ComplexConcept> ConceptExpressionList;

	// ConceptSet = set of synonyms (concept names are positive)
	typedef TaxonomyVertex::SetOfTaxElements ConceptSet;
	// RoleSet = set of synonyms (role may be inverse)
	typedef vector<RoleNameSet> RoleSet;
	// IndividualSet is just set of pos. names
	typedef ConceptNameSet IndividualSet;

private:
	ifOptionSet* pKernelOptions;

private:
	static const char* Version;
	static const char* ProductName;
	static const char* Copyright;
	static const char* ReleaseDate;

protected:	// types
		/// enumeration for the cache
	enum cacheStatus { csEmpty, csSat, csClassified };
		/// enumeration for the reasoner status
	enum KernelStatus { ksLoading, ksCChecked, ksClassified, ksRealised };

protected:	// members
		/// local TBox (to be created)
	TBox* pTBox;

	// reasoning cache

		/// cache level
	enum cacheStatus cacheLevel;
		/// cached query concept description
	DLTree* cachedQuery;
		/// cached concept (either defConcept or existing one)
	TConcept* cachedConcept;
		/// cached query result (taxonomy position)
	TaxonomyVertex* cachedVertex;

	// internal flags

		/// KB status
	enum KernelStatus Status;
		/// is KB consistent or not
	bool isConsistent;
		/// is TBox changed since the last classification
	bool isChanged;

protected:	// methods

	// register all necessary options in local option set
	bool initOptions ( void );

		/// process KB wrt STATUS
	void processKB ( KernelStatus status );

		/// checks if current query is cached
	bool isCached ( DLTree* query ) const
		{ return ( cachedQuery == NULL ? false : equalTrees ( cachedQuery, query ) ); }
		/// set up cache for query, performing additional (re-)classification if necessary
	bool setUpCache ( DLTree* query, cacheStatus level );
		/// clear cache and flags
	void initCacheAndFlags ( void )
	{
		cacheLevel = csEmpty;
		deleteTree(cachedQuery);
		cachedQuery = NULL;
		cachedConcept = NULL;
		cachedVertex = NULL;
		Status = ksLoading;
		isConsistent = true;
		isChanged = true;
	}

	// get access to internal structures

		/// get RW access to TBox
	TBox* getTBox ( void )
	{
		if ( pTBox == NULL )
			throw UnInitException();
		return pTBox;
	}
		/// get RO access to TBox
	const TBox* getTBox ( void ) const
	{
		if ( pTBox == NULL )
			throw UnInitException();
		return pTBox;
	}

		/// get RW access to RoleMaster from TBox
	RoleMaster* getRM ( void ) { return getTBox()->getRM(); }
		/// get RO access to RoleMaster from TBox
	const RoleMaster* getRM ( void ) const { return getTBox()->getRM(); }

		/// get access to the concept hierarchy
	const Taxonomy* getCTaxonomy ( void ) const { return isKBClassified() ? pTBox->getTaxonomy() : NULL; }
		/// get access to the role hierarchy
	const Taxonomy* getRTaxonomy ( void ) const { return isKBPreprocessed() ? getRM()->getTaxonomy() : NULL; }

public:	// general staff
	ReasoningKernel ( void );
	~ReasoningKernel ( void );

	ifOptionSet* getOptions ( void ) { return pKernelOptions; }
	const ifOptionSet* getOptions ( void ) const { return pKernelOptions; }

	static const char* getVersion ( void ) { return Version; }

		/// return classification status of KB
	bool isKBPreprocessed ( void ) const { return Status >= ksCChecked; }
		/// return classification status of KB
	bool isKBClassified ( void ) const { return Status >= ksClassified; }
		/// return realistion status of KB
	bool isKBRealised ( void ) const { return Status >= ksRealised; }

		/// set Progress monitor to control the classification process
	void setProgressMonitor ( TProgressMonitor* pMon ) { getTBox()->setProgressMonitor(pMon); }
		/// set verbose output (ie, default progress monitor, concept and role taxonomies
	void useVerboseOutput ( void ) { getTBox()->useVerboseOutput(); }

		/// dump query processing TIME, reasoning statistics and a (preprocessed) TBox
	void writeReasoningResult ( std::ostream& o, float time ) const
		{ getTBox()->writeReasoningResult ( o, time, isConsistent ); }

	// aux methods -- for parser
	DLTree* processOneOf ( void );

	//******************************************
	//* DataTypes access
	//******************************************

		/// get RW access to a DT center
	DataTypeCenter& getDataTypeCenter ( void ) { return getTBox()->getDataTypeCenter(); }
		/// get RO access to a DT center
	const DataTypeCenter& getDataTypeCenter ( void ) const { return getTBox()->getDataTypeCenter(); }

	////////////////////////////////////////////
	//  ensure* declarations
	////////////////////////////////////////////

	// ensure that given names are of the apropriate types

		/// register NAME as a concept
	ComplexConcept ensureConceptName ( const std::string& name ) throw(EFPPCantRegName)
		{ return new DLTree ( TLexeme ( CNAME, getTBox()->getConcept(name) ) ); }
		/// register NAME as an individual
	ComplexConcept ensureSingletonName ( const std::string& name ) throw(EFPPCantRegName)
		{ return new DLTree ( TLexeme ( INAME, getTBox()->getIndividual(name) ) ); }
		/// register NAME as a role (object property)
	ComplexRole ensureRoleName ( const std::string& name ) throw(EFPPCantRegName)
		{ return new DLTree ( TLexeme ( RNAME, getRM()->ensureRoleName(name,/*isDataRole=*/false) ) ); }
		/// register NAME as a data role (data property)
	ComplexRole ensureDataRoleName ( const std::string& name ) throw(EFPPCantRegName)
		{ return new DLTree ( TLexeme ( RNAME, getRM()->ensureRoleName(name,/*isDataRole=*/true) ) ); }

public:
	//******************************************
	//*****  interface (DIG 1.0 for now)  ******
	//******************************************

	// all methods returns true if error was there

	//******************************************
	//* Identification report
	//******************************************
//	bool getIdentification ( ostream& o ) const;

	//******************************************
	//* KB Management
	//******************************************

		/// create new KB
	bool newKB ( void );
		/// delete existed KB
	bool releaseKB ( void );
		/// reset current KB
	bool clearKB ( void );

	//******************************************
	//* Concept/Role expressions. DL syntax, not DIG/OWL
	//******************************************

		/// @return TOP
	ComplexConcept Top ( void ) const { return new DLTree(TOP); }
		/// @return BOTTOM
	ComplexConcept Bottom ( void ) const { return new DLTree(BOTTOM); }
		/// @return ~C
	ComplexConcept Not ( ComplexConcept C ) const;
		/// @return C/\D
	ComplexConcept And ( ComplexConcept C, ComplexConcept D ) const;
		/// @return C1/\.../\Cn
	ComplexConcept And ( void );
		/// @return C\/D
	ComplexConcept Or ( ComplexConcept C, ComplexConcept D ) const;
		/// @return C1\/...\/Cn
	ComplexConcept Or ( void );

		/// start new concept list for n-ary concept expressions/axioms
	bool openConceptList ( void ) { return getTBox()->openConceptList(); }
		/// start new concept list for n-ary concept expressions/axioms; 1st element is C
	bool openConceptList ( const ComplexConcept C )
		{ return openConceptList() || contConceptList(C); }
		/// add an element C to the most recent open concept list
	bool contConceptList ( const ComplexConcept C ) { return getTBox()->contConceptList(C); }

		/// simple concept expression
	ComplexConcept SimpleExpression ( Token t, ComplexConcept C, ComplexConcept D ) const
	{
		assert ( t == AND || t == OR );
		if ( t == AND )
			return And ( C, D );
		else
			return Or ( C, D );
	}
		/// @return \E R.C
	ComplexConcept Exists ( ComplexRole R, ComplexConcept C ) const;
		/// @return \A R.C
	ComplexConcept Forall ( ComplexRole R, ComplexConcept C ) const;
		/// @return \E R.I for individual/data value I
	ComplexConcept Value ( ComplexRole R, ComplexConcept I ) const;
		/// @return <= n R.C
	ComplexConcept MaxCardinality ( unsigned int n, ComplexRole R, ComplexConcept C ) const;
		/// @return >= n R.C
	ComplexConcept MinCardinality ( unsigned int n, ComplexRole R, ComplexConcept C ) const;
		/// @return = n R.C
	ComplexConcept Cardinality ( unsigned int n, ComplexRole R, ComplexConcept C ) const
	{
		ComplexConcept temp = MinCardinality ( n, clone(R), clone(C) );
		return And ( temp, MaxCardinality ( n, R, C ) );
	}
		/// complex concept expression
	ComplexConcept ComplexExpression ( Token t, unsigned int n, ComplexRole R, ComplexConcept C ) const
	{
		switch(t)
		{
		case LE:
			return MaxCardinality ( n, R, C );
		case GE:
			return MinCardinality ( n, R, C );
		case EXISTS:
			return Exists ( R, C );
		case FORALL:
			return Forall ( R, C );
		default:
			assert (0);
			return NULL;
		}
	}
		/// @return \E R.Self
	ComplexConcept SelfReference ( ComplexRole R ) const { return new DLTree ( REFLEXIVE, R ); }

	// role-related expressions

		/// @return universal role
	ComplexRole UniversalRole ( void ) const { return new DLTree(UROLE); }
		/// @return R^-
	ComplexRole Inverse ( ComplexRole R ) const;
		/// @return R*S
	ComplexRole Compose ( ComplexRole R, ComplexRole S ) const;
		/// @return R1*...*Rn
	ComplexRole Compose ( void );

	// data-related expressions -- to be done

	//******************************************
	//* TELL part
	//******************************************

	// concept axioms

	// axiom (C [= D)
	bool impliesConcepts ( const ComplexConcept C, const ComplexConcept D );

	// axiom (C = D)
	bool equalConcepts ( const ComplexConcept C, const ComplexConcept D );
		/// axiom C1 = ... = Cn
	bool equalConcepts ( void );

		/// axiom C1 != ... != Cn
	bool processDisjoint ( void );

	// Role axioms

	// axiom (R [= S)
	bool impliesRoles ( const ComplexRole R, const ComplexRole S );

	// axiom (R = S)
	bool equalRoles ( const ComplexRole R, const ComplexRole S );
		/// axiom R1 = ... = Rn
	bool equalRoles ( void );

	// axiom (R != S)
	bool disjointRoles ( const ComplexRole R, const ComplexRole S );
		/// axiom R1 != ... != Rn
	bool disjointRoles ( void );

	// Domain (R C)
	bool setDomain ( const ComplexRole R, const ComplexConcept C );

	// Range (R C)
	bool setRange ( const ComplexRole R, const ComplexConcept C );

	// Transitive (R)
	bool setTransitive ( const ComplexRole R );

	// Reflexive (R)
	bool setReflexive ( const ComplexRole R );

	// Irreflexive (R)
	bool setIrreflexive ( const ComplexRole R );

	// Symmetric (R)
	bool setSymmetric ( const ComplexRole R );

	// AntiSymmetric (R)
	bool setAntiSymmetric ( const ComplexRole R );

	// Functional (R)
	bool setFunctional ( const ComplexRole R );

	// Individual axioms

	// axiom I e C
	bool instanceOf ( const ComplexConcept I, const ComplexConcept C );

	// axiom (related I R J)
	bool relatedTo ( const ComplexConcept I, const ComplexRole R, const ComplexConcept J );

	// axiom <I,J>:\neg R
	bool relatedToNot ( const ComplexConcept I, const ComplexRole R, const ComplexConcept J );

	// axiom (value I A V)
	bool valueOf ( const ComplexConcept I, const ComplexRole A, const ComplexConcept V );

	// axiom <I,V>:\neg A
	bool valueOfNot ( const ComplexConcept I, const ComplexRole A, const ComplexConcept V );

	// implementation stuff: same individuals
	bool processSame ( void );

	// implementation stuff: same individuals
	bool processDifferent ( void );

	//******************************************
	//* ASK part
	//******************************************

		/// return consistency status of KB
	bool isKBConsistent ( void )
	{
		if ( Status == ksLoading )
			processKB(ksCChecked);
		return isConsistent;
	}
		/// ensure that KB is preprocessed/consistence checked
	void preprocessKB ( void )
	{
		if ( !isKBPreprocessed() )
			processKB(ksCChecked);
		if ( !isConsistent )
			throw InconsistentKB();
	}
		/// ensure that KB is classified
	void classifyKB ( void )
	{
		if ( !isKBClassified() )
			processKB(ksClassified);
		if ( !isConsistent )
			throw InconsistentKB();
	}
		/// ensure that KB is realised
	void realiseKB ( void )
	{
		if ( !isKBRealised() )
			processKB(ksRealised);
		if ( !isConsistent )
			throw InconsistentKB();
	}

	// role info retrieval

		/// RESULT is true iff role is functional
	bool isFunctional ( const ComplexRole R, bool& Result );
		/// RESULT is true iff role is inverse-functional
	bool isInverseFunctional ( const ComplexRole R, bool& Result );

	// TBox info retriveal

	// apply actor.apply() to all concept names
	template<class Actor>
	bool getAllConcepts ( Actor& actor )
	{
		classifyKB();	// ensure KB is ready to answer the query
		if ( getCTaxonomy() == NULL )
			return true;
		TaxonomyVertex* p = getCTaxonomy()->getTop();	// need for successful compilation
		p->getRelativesInfo</*needCurrent=*/true, /*onlyDirect=*/false, /*upDirection=*/false>(actor);
		return false;
	}

	// apply actor.apply() to all rolenames
	template<class Actor>
	bool getAllRoles ( Actor& actor )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		if ( getRTaxonomy() == NULL )
			return true;
		TaxonomyVertex* p = getRTaxonomy()->getBottom();	// need for successful compilation
		p->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/false, /*upDirection=*/true>(actor);
		return false;
	}

	// apply actor.apply() to all individual names
	template<class Actor>
	bool getAllIndividuals ( Actor& actor )
	{
		realiseKB();	// ensure KB is ready to answer the query
		if ( getCTaxonomy() == NULL )
			return true;
		TaxonomyVertex* p = getCTaxonomy()->getTop();	// need for successful compilation
		p->getRelativesInfo</*needCurrent=*/true, /*onlyDirect=*/false, /*upDirection=*/false>(actor);
		return false;
	}

	// single satisfiability

		/// is C satisfiable
	bool isSatisfiable ( const ComplexConcept C, bool& Result );
		/// does C [= D holds
	bool isSubsumedBy ( const ComplexConcept C, const ComplexConcept D, bool& Result );
		/// is C disjoint with D
	bool isDisjoint ( const ComplexConcept C, const ComplexConcept D, bool& Result );
		/// is C equivalent to D
	bool isEquivalent ( const ComplexConcept C, const ComplexConcept D, bool& Result );

	// concept hierarchy

		/// apply actor::apply() to all direct parents of [complex] C (with possible synonyms)
	template<class Actor>
	bool getParents ( const ComplexConcept C, Actor& actor )
	{
		classifyKB();	// ensure KB is ready to answer the query
		if ( setUpCache ( C, csClassified ) )	// cache result
			return true;	// FIXME!! later
		cachedVertex->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/true,
									   /*upDirection=*/true>(actor);
		return false;
	}
		/// apply Actor::apply() to all direct children of [complex] C (with possible synonyms)
	template<class Actor>
	bool getChildren ( const ComplexConcept C, Actor& actor )
	{
		classifyKB();	// ensure KB is ready to answer the query
		if ( setUpCache ( C, csClassified ) )	// cache result
			return true;	// FIXME!! later
		cachedVertex->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/true,
									   /*upDirection=*/false>(actor);
		return false;
	}
		/// apply actor::apply() to all parents of [complex] C (with possible synonyms)
	template<class Actor>
	bool getAncestors ( const ComplexConcept C, Actor& actor )
	{
		classifyKB();	// ensure KB is ready to answer the query
		if ( setUpCache ( C, csClassified ) )	// cache result
			return true;	// FIXME!! later
		cachedVertex->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/false,
									   /*upDirection=*/true>(actor);
		return false;
	}
		/// apply actor::apply() to all children of [complex] C (with possible synonyms)
	template<class Actor>
	bool getDescendants ( const ComplexConcept C, Actor& actor )
	{
		classifyKB();	// ensure KB is ready to answer the query
		if ( setUpCache ( C, csClassified ) )	// cache result
			return true;	// FIXME!! later
		cachedVertex->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/false,
									   /*upDirection=*/false>(actor);
		return false;
	}
		/// apply actor::apply() to all synonyms of [complex] C
	template<class Actor>
	bool getEquivalents ( const ComplexConcept C, Actor& actor )
	{
		classifyKB();	// ensure KB is ready to answer the query
		if ( setUpCache ( C, csClassified ) )	// cache result
			return true;	// FIXME!! later
		return !actor.apply(*cachedVertex);
	}

	// role hierarchy

	// all direct parents of R (with possible synonyms)
	template<class Actor>
	bool getRParents ( const ComplexRole r, Actor& actor )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		TRole* R = resolveRole(r);
		if ( R == NULL )
			return true;
		TaxonomyVertex* p = R->getTaxVertex();	// need this for compiler
		if ( p == NULL )
			return true;
		p->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/true, /*upDirection=*/true>(actor);
		return false;
	}

	// all direct children of R (with possible synonyms)
	template<class Actor>
	bool getRChildren ( const ComplexRole r, Actor& actor )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		TRole* R = resolveRole(r);
		if ( R == NULL )
			return true;
		TaxonomyVertex* p = R->getTaxVertex();	// need this for compiler
		if ( p == NULL )
			return true;
		p->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/true, /*upDirection=*/false>(actor);
		return false;
	}

	// all parents of R (with possible synonyms)
	template<class Actor>
	bool getRAncestors ( const ComplexRole r, Actor& actor )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		TRole* R = resolveRole(r);
		if ( R == NULL )
			return true;
		TaxonomyVertex* p = R->getTaxVertex();	// need this for compiler
		if ( p == NULL )
			return true;
		p->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/false, /*upDirection=*/true>(actor);
		return false;
	}

	// all children of R (with possible synonyms)
	template<class Actor>
	bool getRDescendants ( const ComplexRole r, Actor& actor )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		TRole* R = resolveRole(r);
		if ( R == NULL )
			return true;
		TaxonomyVertex* p = R->getTaxVertex();	// need this for compiler
		if ( p == NULL )
			return true;
		p->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/false, /*upDirection=*/false>(actor);
		return false;
	}

		/// apply actor::apply() to all synonyms of [complex] R
	template<class Actor>
	bool getREquivalents ( const ComplexRole r, Actor& actor )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		TRole* R = resolveRole(r);
		if ( R == NULL )
			return true;
		TaxonomyVertex* p = R->getTaxVertex();	// need this for compiler
		if ( p == NULL )
			return true;
		return !actor.apply(*p);
	}

	// instances

	// get all instances of given [complex] C
		/// apply actor::apply() to all instances of given [complex] C
	template<class Actor>
	bool getInstances ( const ComplexConcept C, Actor& actor )
	{	// FIXME!! check for Racer's/IS approach
		realiseKB();	// ensure KB is ready to answer the query
		return getDescendants ( C, actor );
	}

		/// apply actor::apply() to all direct concept parents of an individual I
	template<class Actor>
	bool getDirectTypes ( const ComplexConcept I, Actor& actor )
	{
		realiseKB();	// ensure KB is ready to answer the query
		return getParents ( I, actor );
	}
		/// apply actor::apply() to all concept ancestors of an individual I
	template<class Actor>
	bool getTypes ( const ComplexConcept I, Actor& actor )
	{
		realiseKB();	// ensure KB is ready to answer the query
		return getAncestors ( I, actor );
	}
		/// apply actor::apply() to all synonyms of an individual I
	template<class Actor>
	bool getSameAs ( const ComplexConcept I, Actor& actor )
	{
		realiseKB();	// ensure KB is ready to answer the query
		return getEquivalents ( I, actor );
	}

	// if I is instance of given [complex] C
	bool isInstance ( const ComplexConcept I, const ComplexConcept C, bool& Result );

	// set of J such that (I R J)
	bool getRoleFillers ( const ComplexConcept I, const ComplexRole R, IndividualSet& Result );

	// set of (I,J) such that (I R J)
	bool getRelatedIndividuals ( const ComplexRole R, IndividualSet& Is, IndividualSet& Js );

	// ???
	// bool getToldValues ( const IndividualName I, const RoleName A, ??? Result );	// FIXME!! unsupported

	// extra DIG operations
		/// implement absorbedPrimitiveConceptDefinitions DIG extension
	void absorbedPrimitiveConceptDefinitions ( std::ostream& o ) const;
		/// implement unabsorbed DIG extension
	void unabsorbed ( std::ostream& o ) const;
}; // ReasoningKernel

//----------------------------------------------------
//	ReasoningKernel implementation
//----------------------------------------------------

inline ReasoningKernel :: ~ReasoningKernel ( void )
{
	releaseKB ();
	deleteTree(cachedQuery);
	delete pKernelOptions;
}

// create new KB
inline bool ReasoningKernel :: newKB ( void )
{
	if ( pTBox != NULL )
		return true;

	pTBox = new TBox ( getOptions () );
	initCacheAndFlags();
	return false;
}

// delete current KB
inline bool ReasoningKernel :: releaseKB ( void )
{
	delete pTBox;
	pTBox = NULL;

	return false;
}

// clear current KB
inline bool ReasoningKernel :: clearKB ( void )
{
	if ( pTBox == NULL )
		return true;
	return releaseKB () || newKB ();
}

// some aux methods

inline DLTree* ReasoningKernel :: processOneOf ( void ) { return getTBox()->processOneOf(); }

//----------------------------------------------------
//	concept expression interface
//----------------------------------------------------

	// ~C
inline ReasoningKernel::ComplexConcept
ReasoningKernel :: Not ( ComplexConcept C ) const
{
	return createSNFNot(C);
}
	// C/\D
inline ReasoningKernel::ComplexConcept
ReasoningKernel :: And ( ComplexConcept C, ComplexConcept D ) const
{
	return createSNFAnd ( C, D );
}
inline ReasoningKernel::ComplexConcept
ReasoningKernel :: And ( void ) { return getTBox()->processAnd(); }
	// C\/D
inline ReasoningKernel::ComplexConcept
ReasoningKernel :: Or ( ComplexConcept C, ComplexConcept D ) const
{
	return createSNFOr ( C, D );
}
inline ReasoningKernel::ComplexConcept
ReasoningKernel :: Or ( void ) { return getTBox()->processOr(); }
	// \E R.C
inline ReasoningKernel::ComplexConcept
ReasoningKernel :: Exists ( ComplexRole R, ComplexConcept C ) const
{
	return createSNFExists ( R, C );
}
	// \A R.C
inline ReasoningKernel::ComplexConcept
ReasoningKernel :: Forall ( ComplexRole R, ComplexConcept C ) const
{
	return createSNFForall ( R, C );
}
	// \E R.I for individual/data value I
inline ReasoningKernel::ComplexConcept
ReasoningKernel :: Value ( ComplexRole R, ComplexConcept I ) const
{
	return createSNFExists ( R, I );
}
	// <= n R.C
inline ReasoningKernel::ComplexConcept
ReasoningKernel :: MaxCardinality ( unsigned int n, ComplexRole R, ComplexConcept C ) const
{
	return createSNFLE ( n, R, C );
}
	// >= n R.C
inline ReasoningKernel::ComplexConcept
ReasoningKernel :: MinCardinality ( unsigned int n, ComplexRole R, ComplexConcept C ) const
{
	return createSNFGE ( n, R, C );
}

	// R^-
inline ReasoningKernel::ComplexRole
ReasoningKernel :: Inverse ( ComplexRole R ) const
{
	return createInverse(R);
}

	// R*S
inline ReasoningKernel::ComplexRole
ReasoningKernel :: Compose ( ComplexRole R, ComplexRole S ) const
{
	return new DLTree ( TLexeme(RCOMPOSITION), R, S );
}
inline ReasoningKernel::ComplexRole
ReasoningKernel :: Compose ( void ) { return getTBox()->processRComposition(); }

//----------------------------------------------------
//	TELLS interface
//----------------------------------------------------

	// concept axioms

	// axiom (C [= D)
inline bool ReasoningKernel :: impliesConcepts ( const ComplexConcept C, const ComplexConcept D )
{
	isChanged = true;
	return getTBox()->addSubsumeAxiom ( C, D );
}

	// axiom (C = D)
inline bool ReasoningKernel :: equalConcepts ( const ComplexConcept C, const ComplexConcept D )
{
	isChanged = true;
	return getTBox()->addEqualityAxiom ( C, D );
}
inline bool ReasoningKernel :: equalConcepts ( void )
{
	isChanged = true;
	return getTBox()->processEquivalent();
}

inline bool ReasoningKernel :: processDisjoint ( void )
{
	isChanged = true;
	return getTBox()->processDisjoint ();
}



	// Role axioms

	// axiom (R [= S)
inline bool ReasoningKernel :: impliesRoles ( const ComplexRole R, const ComplexRole S )
{
	isChanged = true;
	return getRM()->addRoleParent ( R, resolveRole(S) );
}

	// axiom (R = S)
inline bool ReasoningKernel :: equalRoles ( const ComplexRole R, const ComplexRole S )
{
	isChanged = true;
	return getRM()->addRoleSynonym ( resolveRole(R), resolveRole(S) );
}
inline bool ReasoningKernel :: equalRoles ( void )
{
	isChanged = true;
	return getTBox()->processEquivalentR();
}

	// axiom (R != S)
inline bool ReasoningKernel :: disjointRoles ( const ComplexRole R, const ComplexRole S )
{
	isChanged = true;

	if ( isUniversalRole(R) || isUniversalRole(S) )
		return true;

	TRole* r = resolveRole(R);
	TRole* s = resolveRole(S);
	if ( r == NULL || s == NULL )
		return true;
	getRM()->addDisjointRoles(r,s);
	return false;
}
inline bool ReasoningKernel :: disjointRoles ( void )
{
	isChanged = true;
	return getTBox()->processDisjointR();
}

	// Domain (R C)
inline bool ReasoningKernel :: setDomain ( const ComplexRole R, const ComplexConcept C )
{
	isChanged = true;
	TRole* r = resolveRole(R);
	if ( r == NULL )
		return true;
	r->setDomain(C);
	return false;
}

	// Range (R C)
inline bool ReasoningKernel :: setRange ( const ComplexRole R, const ComplexConcept C )
{
	isChanged = true;
	TRole* r = resolveRole(R);
	if ( r == NULL )
		return true;
	r->setRange(C);
	return false;
}

	// Transitive (R)
inline bool ReasoningKernel :: setTransitive ( const ComplexRole R )
{
	isChanged = true;
	TRole* r = resolveRole(R);
	if ( r == NULL )
		return true;
	r->setBothTransitive();
	return false;
}

	// Reflexive (R)
inline bool ReasoningKernel :: setReflexive ( const ComplexRole R )
{
	isChanged = true;
	TRole* r = resolveRole(R);
	if ( r == NULL )
		return true;
	r->setBothReflexive();
	return false;
}

	// Irreflexive (R): Domain(R) = \neg ER.Self
inline bool ReasoningKernel :: setIrreflexive ( const ComplexRole R )
{
	return setDomain ( R, Not(SelfReference(clone(R))) );
}

	// Symmetric (R): R [= R^-
inline bool ReasoningKernel :: setSymmetric ( const ComplexRole R )
{
	if ( isUniversalRole(R) )	// nothing to do
		return false;

	return impliesRoles ( R, Inverse(clone(R)) );
}

	// AntySymmetric (R): disjoint(R,R^-)
inline bool ReasoningKernel :: setAntiSymmetric ( const ComplexRole R )
{
	if ( isUniversalRole(R) )	// not possible
		return true;

	return disjointRoles ( R, Inverse(clone(R)) );
}

	// Functional (R)
inline bool ReasoningKernel :: setFunctional ( const ComplexRole R )
{
	TRole* r = resolveRole(R);
	if ( r == NULL )
		return true;
	if ( r->isFunctional() )
		return false;

	isChanged = true;
	r->setFunctional();
	return false;
}

	// Individual axioms

	// axiom I e C
inline bool ReasoningKernel :: instanceOf ( const ComplexConcept I, const ComplexConcept C )
{
	isChanged = true;
	if ( I->Element().getToken() != INAME )
		return true;
	return getTBox()->RegisterInstance ( I->Element().getName(), C );
}

	// axiom (related I R J)
inline bool ReasoningKernel :: relatedTo ( const ComplexConcept I, const ComplexRole R, const ComplexConcept J )
{
	isChanged = true;
	if ( I->Element().getToken() != INAME || J->Element().getToken() != INAME )
		return true;
	if ( isUniversalRole(R) )	// nothing to do
		return false;
	return getTBox()->RegisterIndividualRelation (
		I->Element().getName(),
		resolveRole(R),
		J->Element().getName() );
}

	// axiom <I,J>:\neg R
inline bool ReasoningKernel :: relatedToNot ( const ComplexConcept I, const ComplexRole R, const ComplexConcept J )
{
	isChanged = true;
	if ( I->Element().getToken() != INAME || J->Element().getToken() != INAME )
		return true;
	if ( isUniversalRole(R) )	// nothing to do
		return false;
	// change to i:\AR.\neg{j}
	return instanceOf ( I, Forall ( R, Not(J) ) );
}

	// axiom (value I A V)
inline bool ReasoningKernel :: valueOf ( const ComplexConcept I, const ComplexRole A, const ComplexConcept V )
{
	isChanged = true;
	return instanceOf ( I, Exists ( A, V ) );
}

	// axiom <I,V>:\neg A
inline bool ReasoningKernel :: valueOfNot ( const ComplexConcept I, const ComplexRole A, const ComplexConcept V )
{
	isChanged = true;
	return instanceOf ( I, Forall ( A, Not(V) ) );
}

	// implementation stuff: same individuals
inline bool ReasoningKernel :: processSame ( void )
{
	isChanged = true;
	return getTBox()->processSame ();
}

	// implementation stuff: same individuals
inline bool ReasoningKernel :: processDifferent ( void )
{
	isChanged = true;
	return getTBox()->processDifferent ();
}

//----------------------------------------------------
//	ASKS implementation
//----------------------------------------------------

/// is R functional
inline bool
ReasoningKernel :: isFunctional ( const ComplexRole R, bool& Result )
{
	preprocessKB();	// ensure KB is ready to answer the query

	// universal role is not functional
	if ( isUniversalRole(R) )
	{
		Result = false;
		return false;
	}

	TRole* r = resolveRole(R);
	if ( r == NULL )
		return true;

	Result = r->isFunctional();
	return false;
}

/// is R inverse-functional
inline bool
ReasoningKernel :: isInverseFunctional ( const ComplexRole R, bool& Result )
{
	ComplexRole iR = Inverse(clone(R));
	bool ret = isFunctional ( iR, Result );
	deleteTree(iR);
	return ret;
}

	// is C satisfiable
inline bool
ReasoningKernel :: isSatisfiable ( const ComplexConcept C, bool& Result )
{
	preprocessKB();	// ensure KB is ready to answer the query
	if ( isCN(C) )
	{
		Result = getTBox()->isSatisfiable(getTBox()->getConcept(C));
		return false;
	}

	if ( setUpCache ( C, csSat ) )	// cache result
		return true;

	// sanity check
	if ( cachedConcept == NULL || !isCorrect(cachedConcept->pBody) )
		return true;

	Result = cachedConcept->isSatisfiable();
	return false;
}

	// is C [= D holds
inline bool
ReasoningKernel :: isSubsumedBy ( const ComplexConcept C, const ComplexConcept D, bool& Result )
{
	preprocessKB();	// ensure KB is ready to answer the query
	if ( isCN(C) && isCN(D) )
	{
		Result = getTBox()->isSubHolds ( getTBox()->getConcept(C), getTBox()->getConcept(D) );
		return false;
	}

	ComplexConcept Probe = And ( clone(C), Not(clone(D)) );
	bool ret = isSatisfiable ( Probe, Result );
	deleteTree(Probe);
	Result = !Result;
	return ret;
}

	// is C&D disjoint
inline bool
ReasoningKernel :: isDisjoint ( const ComplexConcept C, const ComplexConcept D, bool& Result )
{
	preprocessKB();	// ensure KB is ready to answer the query
	bool sub = false, fail;

	// check if one of subsumption holds
	fail = isSubsumedBy ( C, D, sub );

	if ( fail )
		return true;

	// check second one if 1st subsumption not holds
	if ( !sub )
		fail = isSubsumedBy ( D, C, sub );

	if ( !fail )
		Result = !sub;

	return fail;
}

// are C and D equivalent
inline bool
ReasoningKernel :: isEquivalent ( const ComplexConcept C, const ComplexConcept D, bool& Result )
{
	preprocessKB();	// ensure KB is ready to answer the query
	bool sub = false, fail;

	// check if one of subsumption holds
	fail = isSubsumedBy ( C, D, sub );

	if ( fail )
		return true;

	// check second one if 1st subsumption not holds
	if ( sub )
		fail = isSubsumedBy ( D, C, sub );

	if ( !fail )
		Result = sub;

	return fail;
}

	// isInstance test: simulation via concept subsumption
inline bool ReasoningKernel :: isInstance ( const ComplexConcept I, const ComplexConcept C, bool& Result )
{
	realiseKB();	// ensure KB is ready to answer the query
	return isSubsumedBy ( I, C, Result );
}

	// set of J such that (I R J)
inline bool
ReasoningKernel :: getRoleFillers ( const ComplexConcept I, const ComplexRole R, IndividualSet& Result )
{
	preprocessKB();	// only told information here
	if ( I->Element().getToken() != INAME || resolveRole(R) == NULL )
		return true;
	getTBox()->getRoleFillers (
		static_cast<TIndividual*>(I->Element().getName()), resolveRole(R), Result );
	return false;
}

	// set of (I,J) such that (I R J)
inline bool
ReasoningKernel :: getRelatedIndividuals ( const ComplexRole R, IndividualSet& Is, IndividualSet& Js )
{
	preprocessKB();	// only told information here
	if ( resolveRole(R) == NULL )
		return true;
	getTBox()->getRelatedIndividuals ( resolveRole(R), Is, Js );
	return false;
}


//----------------------------------------------------
//	extra ASKS implementation
//----------------------------------------------------

	/// implement absorbedPrimitiveConceptDefinitions DIG extension
inline void ReasoningKernel :: absorbedPrimitiveConceptDefinitions ( std::ostream& o ) const
{
	getTBox()->absorbedPrimitiveConceptDefinitions(o);
}
	/// implement unabsorbed DIG extension
inline void ReasoningKernel :: unabsorbed ( std::ostream& o ) const
{
	getTBox()->unabsorbed(o);
}

#endif
