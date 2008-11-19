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

#ifndef _KERNEL_H
#define _KERNEL_H

#include <cassert>
#include <string>

//#define FPP_USE_AXIOMS

#include "eFaCTPlusPlus.h"
#include "tNAryQueue.h"
#include "dlTBox.h"
#include "Reasoner.h"
#include "ifOptions.h"
#include "DLConceptTaxonomy.h"	// for getRelatives()

#ifdef FPP_USE_AXIOMS
#	include "tOntology.h"
#endif

using namespace std;

class InconsistentKB : public EFaCTPlusPlus
{
public:
	InconsistentKB ( void ) throw() : EFaCTPlusPlus("FaCT++ Kernel: Inconsistent KB") {}
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
	typedef TBox::NamesVector NamesVector;

	// ConceptSet = set of synonyms (concept names are positive)
	typedef vector<ConceptList> ConceptSet;
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
		/// header of the file with internal state; defined in SaveLoad.cpp
	static const char* InternalStateFileHeader;

protected:	// types
		/// enumeration for the cache
	enum cacheStatus { csEmpty, csSat, csClassified };

protected:	// members
		/// local TBox (to be created)
	TBox* pTBox;
		/// set of queues for the n-ary expressions/commands
	TNAryQueue NAryQueue;
#ifdef FPP_USE_AXIOMS
		/// set of axioms
	TOntology Ontology;
#endif

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

		/// is TBox changed since the last classification
	bool isChanged;
		/// set if TBox throws an exception during preprocessing/classification
	bool reasoningFailed;

private:	// no copy
		/// no copy c'tor
	ReasoningKernel ( const ReasoningKernel& );
		/// no assignment
	ReasoningKernel& operator = ( const ReasoningKernel& );

protected:	// methods

	// register all necessary options in local option set
	bool initOptions ( void );

		/// get status of the KB
	KBStatus getStatus ( void ) const
	{
		if ( pTBox == NULL )
			return kbEmpty;
		return pTBox->getStatus();
	}
		/// process KB wrt STATUS
	void processKB ( KBStatus status );

		/// checks if current query is cached
	bool isCached ( DLTree* query ) const
		{ return ( cachedQuery == NULL ? false : equalTrees ( cachedQuery, query ) ); }
		/// set up cache for query, performing additional (re-)classification if necessary
	void setUpCache ( DLTree* query, cacheStatus level );
		/// clear cache and flags
	void initCacheAndFlags ( void )
	{
		cacheLevel = csEmpty;
		deleteTree(cachedQuery);
		cachedQuery = NULL;
		cachedConcept = NULL;
		cachedVertex = NULL;
		isChanged = true;
		reasoningFailed = false;
	}

	// get access to internal structures

		/// @throw an exception if no TBox found
	void checkTBox ( void ) const
	{
		if ( pTBox == NULL )
			throw EFaCTPlusPlus("FaCT++ Kernel: KB Not Initialised");
	}
		/// get RW access to TBox
	TBox* getTBox ( void ) { checkTBox(); return pTBox; }
		/// get RO access to TBox
	const TBox* getTBox ( void ) const { checkTBox(); return pTBox; }

		/// get RW access to RoleMaster from TBox
	RoleMaster* getRM ( void ) { return getTBox()->getRM(); }
		/// get RO access to RoleMaster from TBox
	const RoleMaster* getRM ( void ) const { return getTBox()->getRM(); }

		/// get access to the concept hierarchy
	const Taxonomy* getCTaxonomy ( void ) const
	{
		if ( !isKBClassified() )
			throw EFaCTPlusPlus("No access to concept taxonomy: ontology not classified");
		return getTBox()->getTaxonomy();
	}
		/// get access to the role hierarchy
	const Taxonomy* getRTaxonomy ( void ) const
	{
		if ( !isKBPreprocessed() )
			throw EFaCTPlusPlus("No access to role taxonomy: ontology not preprocessed");
		return getRM()->getTaxonomy();
	}

	// transformation methods

		/// get individual by a DLTree
	TIndividual* getIndividual ( const ComplexConcept i, const char* reason )
	{
		if ( !getTBox()->isIndividual(i) )
			throw EFaCTPlusPlus(reason);
		return static_cast<TIndividual*>(getTBox()->getCI(i));
	}
		/// get role by the DLTree
	TRole* getRole ( const ComplexRole r, const char* reason )
	{
		TRole* R = resolveRole(r);
		if ( R == NULL )
			throw EFaCTPlusPlus(reason);
		return R;
	}

	//----------------------------------------------
	//-- save/load support; implementation in SaveLoad.cpp
	//----------------------------------------------

		/// save the header of the kernel
	void SaveHeader ( std::ostream& o ) const;
		/// save the set of Kernel's options
	void SaveOptions ( std::ostream& o ) const;
		/// save the status of the KB and the appropriate part of KB
	void SaveKB ( std::ostream& o ) const;
		/// load the header for the kernel
	bool LoadHeader ( std::istream& i );
		/// load the set of Kernel's options
	void LoadOptions ( std::istream& i );
		/// load the status of the KB and the appropriate part of KB
	void LoadKB ( std::istream& i );

public:	// general staff
	ReasoningKernel ( void );
	~ReasoningKernel ( void );

	ifOptionSet* getOptions ( void ) { return pKernelOptions; }
	const ifOptionSet* getOptions ( void ) const { return pKernelOptions; }

	static const char* getVersion ( void ) { return Version; }

		/// return classification status of KB
	bool isKBPreprocessed ( void ) const { return getStatus() >= kbCChecked; }
		/// return classification status of KB
	bool isKBClassified ( void ) const { return getStatus() >= kbClassified; }
		/// return realistion status of KB
	bool isKBRealised ( void ) const { return getStatus() >= kbRealised; }

		/// set Progress monitor to control the classification process
	void setProgressMonitor ( TProgressMonitor* pMon ) { getTBox()->setProgressMonitor(pMon); }
		/// set verbose output (ie, default progress monitor, concept and role taxonomies
	void useVerboseOutput ( void ) { getTBox()->useVerboseOutput(); }

		/// dump query processing TIME, reasoning statistics and a (preprocessed) TBox
	void writeReasoningResult ( std::ostream& o, float time ) const
		{ getTBox()->writeReasoningResult ( o, time ); }

		/// @return one-of construction for the arguments in NAryQueue; data is true if data one-of is used
	DLTree* processOneOf ( bool data = false );

	//----------------------------------------------
	//-- save/load interface; implementation in SaveLoad.cpp
	//----------------------------------------------

		/// save internal state of the Kernel to a file NAME
	void Save ( std::ostream& o ) const;
		/// load internal state of the Kernel from a file NAME
	void Load ( std::istream& i );
		/// save internal state of the Kernel to a file NAME
	void Save ( const char* name ) const;
		/// load internal state of the Kernel from a file NAME
	void Load ( const char* name );

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

		/// start new argument list for n-ary concept expressions/axioms
	void openArgList ( void ) { NAryQueue.openArgList(); }
		/// add an element C to the most recent open argument list
	void addArg ( const ComplexConcept C ) { NAryQueue.addArg(C); }
		/// start new concept list for n-ary concept expressions/axioms; 1st element is C
	void openArgList ( const ComplexConcept C ) { openArgList(); addArg(C); }

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
		if ( getStatus() <= kbLoading )
			processKB(kbCChecked);
		return getTBox()->isConsistent();
	}
		/// ensure that KB is preprocessed/consistence checked
	void preprocessKB ( void )
	{
		if ( !isKBConsistent() )
			throw InconsistentKB();
	}
		/// ensure that KB is classified
	void classifyKB ( void )
	{
		if ( !isKBClassified() )
			processKB(kbClassified);
		if ( !isKBConsistent() )
			throw InconsistentKB();
	}
		/// ensure that KB is realised
	void realiseKB ( void )
	{
		if ( !isKBRealised() )
			processKB(kbRealised);
		if ( !isKBConsistent() )
			throw InconsistentKB();
	}

	// role info retrieval

		/// @return true iff role is functional
	bool isFunctional ( const ComplexRole R )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		if ( isUniversalRole(R) )
			return false;	// universal role is not functional

		return getRole ( R, "Role expression expected in isFunctional()" )->isFunctional();
	}
		/// @return true iff role is inverse-functional
	bool isInverseFunctional ( const ComplexRole R )
	{
		ComplexRole iR = Inverse(clone(R));
		bool ret = isFunctional(iR);
		deleteTree(iR);
		return ret;
	}

	// TBox info retriveal

	// apply actor.apply() to all concept names
	template<class Actor>
	void getAllConcepts ( Actor& actor )
	{
		classifyKB();	// ensure KB is ready to answer the query
		TaxonomyVertex* p = getCTaxonomy()->getTop();	// need for successful compilation
		p->getRelativesInfo</*needCurrent=*/true, /*onlyDirect=*/false, /*upDirection=*/false>(actor);
	}

	// apply actor.apply() to all rolenames
	template<class Actor>
	void getAllRoles ( Actor& actor )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		TaxonomyVertex* p = getRTaxonomy()->getBottom();	// need for successful compilation
		p->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/false, /*upDirection=*/true>(actor);
	}

	// apply actor.apply() to all individual names
	template<class Actor>
	void getAllIndividuals ( Actor& actor )
	{
		realiseKB();	// ensure KB is ready to answer the query
		TaxonomyVertex* p = getCTaxonomy()->getTop();	// need for successful compilation
		p->getRelativesInfo</*needCurrent=*/true, /*onlyDirect=*/false, /*upDirection=*/false>(actor);
	}

	// single satisfiability

		/// @return true iff C is satisfiable
	bool isSatisfiable ( const ComplexConcept C )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		if ( isCN(C) )
			return getTBox()->isSatisfiable(getTBox()->getCI(C));

		setUpCache ( C, csSat );
		return getTBox()->isSatisfiable(cachedConcept);
	}
		/// @return true iff C [= D holds
	bool isSubsumedBy ( const ComplexConcept C, const ComplexConcept D )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		if ( isCN(C) && isCN(D) )
			return getTBox()->isSubHolds ( getTBox()->getCI(C), getTBox()->getCI(D) );

		ComplexConcept Probe = And ( clone(C), Not(clone(D)) );
		bool ret = isSatisfiable(Probe);
		deleteTree(Probe);
		return !ret;
	}
		/// @return true iff C is disjoint with D
	bool isDisjoint ( const ComplexConcept C, const ComplexConcept D )
		{ return !isSubsumedBy ( C, D ) && !isSubsumedBy ( D, C ); }
		/// @return true iff C is equivalent to D
	bool isEquivalent ( const ComplexConcept C, const ComplexConcept D )
		{ return isSubsumedBy ( C, D ) && isSubsumedBy ( D, C ); }

	// concept hierarchy

		/// apply actor::apply() to all direct parents of [complex] C (with possible synonyms)
	template<class Actor>
	void getParents ( const ComplexConcept C, Actor& actor )
	{
		classifyKB();	// ensure KB is ready to answer the query
		setUpCache ( C, csClassified );
		cachedVertex->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/true,
									   /*upDirection=*/true>(actor);
	}
		/// apply Actor::apply() to all direct children of [complex] C (with possible synonyms)
	template<class Actor>
	void getChildren ( const ComplexConcept C, Actor& actor )
	{
		classifyKB();	// ensure KB is ready to answer the query
		setUpCache ( C, csClassified );
		cachedVertex->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/true,
									   /*upDirection=*/false>(actor);
	}
		/// apply actor::apply() to all parents of [complex] C (with possible synonyms)
	template<class Actor>
	void getAncestors ( const ComplexConcept C, Actor& actor )
	{
		classifyKB();	// ensure KB is ready to answer the query
		setUpCache ( C, csClassified );
		cachedVertex->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/false,
									   /*upDirection=*/true>(actor);
	}
		/// apply actor::apply() to all children of [complex] C (with possible synonyms)
	template<class Actor>
	void getDescendants ( const ComplexConcept C, Actor& actor )
	{
		classifyKB();	// ensure KB is ready to answer the query
		setUpCache ( C, csClassified );
		cachedVertex->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/false,
									   /*upDirection=*/false>(actor);
	}
		/// apply actor::apply() to all synonyms of [complex] C
	template<class Actor>
	void getEquivalents ( const ComplexConcept C, Actor& actor )
	{
		classifyKB();	// ensure KB is ready to answer the query
		setUpCache ( C, csClassified );
		actor.apply(*cachedVertex);
	}

	// role hierarchy

	// all direct parents of R (with possible synonyms)
	template<class Actor>
	void getRParents ( const ComplexRole r, Actor& actor )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		TRole* R = getRole ( r, "Role expression expected in getRParents()" );
		TaxonomyVertex* p = R->getTaxVertex();	// need this for compiler
		p->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/true, /*upDirection=*/true>(actor);
	}

	// all direct children of R (with possible synonyms)
	template<class Actor>
	void getRChildren ( const ComplexRole r, Actor& actor )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		TRole* R = getRole ( r, "Role expression expected in getRChildren()" );
		TaxonomyVertex* p = R->getTaxVertex();	// need this for compiler
		p->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/true, /*upDirection=*/false>(actor);
	}

	// all parents of R (with possible synonyms)
	template<class Actor>
	void getRAncestors ( const ComplexRole r, Actor& actor )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		TRole* R = getRole ( r, "Role expression expected in getRAncestors()" );
		TaxonomyVertex* p = R->getTaxVertex();	// need this for compiler
		p->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/false, /*upDirection=*/true>(actor);
	}

	// all children of R (with possible synonyms)
	template<class Actor>
	void getRDescendants ( const ComplexRole r, Actor& actor )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		TRole* R = getRole ( r, "Role expression expected in getRDescendants()" );
		TaxonomyVertex* p = R->getTaxVertex();	// need this for compiler
		p->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/false, /*upDirection=*/false>(actor);
	}

		/// apply actor::apply() to all synonyms of [complex] R
	template<class Actor>
	void getREquivalents ( const ComplexRole r, Actor& actor )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		TRole* R = getRole ( r, "Role expression expected in getREquivalents()" );
		TaxonomyVertex* p = R->getTaxVertex();	// need this for compiler
		actor.apply(*p);
	}

	// domain and range as a set of named concepts

		/// apply actor::apply() to all NC that are in the domain of [complex] R
	template<class Actor>
	void getRoleDomain ( const ComplexRole r, Actor& actor )
	{
		classifyKB();	// ensure KB is ready to answer the query
		DLTree* C = Exists(r,Top());
		setUpCache ( C, csClassified );
		// gets an exact domain is named concept; otherwise, set of the most specific concepts
		cachedVertex->getRelativesInfo</*needCurrent=*/true, /*onlyDirect=*/true,
									   /*upDirection=*/true>(actor);
		delete C;	// not deleteTree() to keep R
	}

		/// apply actor::apply() to all NC that are in the range of [complex] R
	template<class Actor>
	void getRoleRange ( const ComplexRole r, Actor& actor )
	{
		getRoleDomain ( Inverse(r), actor );
	}

	// instances

		/// apply actor::apply() to all direct instances of given [complex] C
	template<class Actor>
	void getDirectInstances ( const ComplexConcept C, Actor& actor )
	{
		realiseKB();	// ensure KB is ready to answer the query
		getChildren ( C, actor );
	}

		/// apply actor::apply() to all instances of given [complex] C
	template<class Actor>
	void getInstances ( const ComplexConcept C, Actor& actor )
	{	// FIXME!! check for Racer's/IS approach
		realiseKB();	// ensure KB is ready to answer the query
		getDescendants ( C, actor );
	}

		/// apply actor::apply() to all direct concept parents of an individual I
	template<class Actor>
	void getDirectTypes ( const ComplexConcept I, Actor& actor )
	{
		realiseKB();	// ensure KB is ready to answer the query
		getParents ( I, actor );
	}
		/// apply actor::apply() to all concept ancestors of an individual I
	template<class Actor>
	void getTypes ( const ComplexConcept I, Actor& actor )
	{
		realiseKB();	// ensure KB is ready to answer the query
		getAncestors ( I, actor );
	}
		/// apply actor::apply() to all synonyms of an individual I
	template<class Actor>
	void getSameAs ( const ComplexConcept I, Actor& actor )
	{
		realiseKB();	// ensure KB is ready to answer the query
		getEquivalents ( I, actor );
	}
		/// @return true iff I and J refer to the same individual
	bool isSameIndividuals ( const ComplexConcept I, const ComplexConcept J )
	{
		realiseKB();
		TIndividual* i = getIndividual ( I, "Only known individuals are allowed in the isSameAs()" );
		TIndividual* j = getIndividual ( J, "Only known individuals are allowed in the isSameAs()" );
		return getTBox()->isSameIndividuals(i,j);
	}
		/// @return true iff individual I is instance of given [complex] C
	bool isInstance ( const ComplexConcept I, const ComplexConcept C )
	{
		realiseKB();	// ensure KB is ready to answer the query
		getIndividual ( I, "individual name expected in the isInstance()" );
		return isSubsumedBy ( I, C );
	}
		/// @return in Rs all (DATA)-roles R s.t. (I,x):R; add inverses if NEEDI is true
	void getRelatedRoles ( const ComplexConcept I, NamesVector& Rs, bool data, bool needI )
	{
		realiseKB();	// ensure KB is ready to answer the query
		getTBox()->getRelatedRoles (
			getIndividual ( I, "individual name expected in the getRelatedRoles()" ),
			Rs, data, needI );
	}
		/// set RESULT into set of J's such that R(I,J)
	void getRoleFillers ( const ComplexConcept I, const ComplexRole R, IndividualSet& Result )
	{
		realiseKB();	// ensure KB is ready to answer the query
		getTBox()->getRoleFillers (
			getIndividual ( I, "Individual name expected in the getRoleFillers()" ),
			getRole ( R, "Role expression expected in the getRoleFillers()" ),
			Result );
	}
		/// set RESULT into set of (I,J)'s such that R(I,J)
	void getRelatedIndividuals ( const ComplexRole R, IndividualSet& Is, IndividualSet& Js )
	{
		realiseKB();	// ensure KB is ready to answer the query
		getTBox()->getRelatedIndividuals (
			getRole ( R, "Role expression expected in the getRelatedIndividuals()" ),
			Is, Js );
	}
		/// set RESULT into set of J's such that R(I,J)
	bool isRelated ( const ComplexConcept I, const ComplexRole R, const ComplexConcept J )
	{
		realiseKB();	// ensure KB is ready to answer the query
		TIndividual* i = getIndividual ( I, "Individual name expected in the isRelated()" );
		TRole* r = getRole ( R, "Role expression expected in the isRelated()" );
		if ( r->isDataRole() )
			return false;	// FIXME!! not implemented
		else
			return getTBox()->isRelated ( i, r, getIndividual ( J, "Individual name expected in the isRelated()" ) );
	}
	// ???
	// ??? getToldValues ( const IndividualName I, const RoleName A );	// FIXME!! unsupported

	// extra DIG operations
		/// implement absorbedPrimitiveConceptDefinitions DIG extension
	void absorbedPrimitiveConceptDefinitions ( std::ostream& o ) const
		{ getTBox()->absorbedPrimitiveConceptDefinitions(o); }
		/// implement unabsorbed DIG extension
	void unabsorbed ( std::ostream& o ) const { getTBox()->unabsorbed(o); }
}; // ReasoningKernel

//----------------------------------------------------
//	ReasoningKernel implementation
//----------------------------------------------------

inline ReasoningKernel :: ~ReasoningKernel ( void )
{
	releaseKB ();
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
	deleteTree(cachedQuery);
	cachedQuery = NULL;

#ifdef FPP_USE_AXIOMS
	Ontology.clear();
#endif

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

inline DLTree*
ReasoningKernel :: processOneOf ( bool data )
{
	return getTBox()->processOneOf ( NAryQueue.getLastArgList(), data );
}

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
ReasoningKernel :: And ( void ) { return getTBox()->processAnd(NAryQueue.getLastArgList()); }
	// C\/D
inline ReasoningKernel::ComplexConcept
ReasoningKernel :: Or ( ComplexConcept C, ComplexConcept D ) const
{
	return createSNFOr ( C, D );
}
inline ReasoningKernel::ComplexConcept
ReasoningKernel :: Or ( void ) { return getTBox()->processOr(NAryQueue.getLastArgList()); }
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
ReasoningKernel :: Compose ( void ) { return getTBox()->processRComposition(NAryQueue.getLastArgList()); }

//----------------------------------------------------
//	TELLS interface
//----------------------------------------------------

	// concept axioms

	// axiom (C [= D)
inline bool ReasoningKernel :: impliesConcepts ( const ComplexConcept C, const ComplexConcept D )
{
	isChanged = true;
#ifdef FPP_USE_AXIOMS
	Ontology.add ( new TDLAxiomConceptInclusion ( C, D ) );
	return false;
#else
	return getTBox()->addSubsumeAxiom ( C, D );
#endif
}

	// axiom (C = D)
inline bool ReasoningKernel :: equalConcepts ( const ComplexConcept C, const ComplexConcept D )
{
	isChanged = true;
#ifdef FPP_USE_AXIOMS
	Ontology.add ( new TDLAxiomConceptEquivalence ( C, D ) );
	return false;
#else
	return getTBox()->addEqualityAxiom ( C, D );
#endif
}
inline bool ReasoningKernel :: equalConcepts ( void )
{
	isChanged = true;
#ifdef FPP_USE_AXIOMS
	Ontology.add ( new TDLAxiomConceptEquivalence(NAryQueue.getLastArgList()) );
	return false;
#else
	return getTBox()->processEquivalent(NAryQueue.getLastArgList());
#endif
}

inline bool ReasoningKernel :: processDisjoint ( void )
{
	isChanged = true;
	return getTBox()->processDisjoint(NAryQueue.getLastArgList());
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
	return getTBox()->processEquivalentR(NAryQueue.getLastArgList());
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
	return getTBox()->processDisjointR(NAryQueue.getLastArgList());
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
	return getTBox()->processSame(NAryQueue.getLastArgList());
}

	// implementation stuff: same individuals
inline bool ReasoningKernel :: processDifferent ( void )
{
	isChanged = true;
	return getTBox()->processDifferent(NAryQueue.getLastArgList());
}

#endif
