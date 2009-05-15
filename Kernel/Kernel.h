/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2009 by Dmitry Tsarkov

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

#ifndef _KERNEL_H
#define _KERNEL_H

#include <cassert>
#include <string>

#include "eFPPInconsistentKB.h"
#include "tNAryQueue.h"
#include "dlTBox.h"
#include "Reasoner.h"
#include "ifOptions.h"
#include "DLConceptTaxonomy.h"	// for getRelatives()
#include "tOntology.h"

using namespace std;

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
		/// NameSets for the named entities
	TNameSet<TTreeNamedEntry>
		Concepts,
		ORoles,
		DRoles,
		Individuals;
		/// DataType center
	DataTypeCenter DTCenter;
		/// set of queues for the n-ary expressions/commands
	TNAryQueue NAryQueue;
		/// set of axioms
	TOntology Ontology;

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
		checkDefined(i);
		if ( !getTBox()->isIndividual(i) )
			throw EFaCTPlusPlus(reason);
		return static_cast<TIndividual*>(getTBox()->getCI(i));
	}
		/// get role by the DLTree
	TRole* getRole ( const ComplexRole r, const char* reason ) const
	{
		checkDefined(r);
		try { return resolveRole(r); }
		catch ( EFaCTPlusPlus e ) { throw EFaCTPlusPlus(reason); }
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
	DLTree* processOneOf ( bool data = false ) { return getTBox()->processOneOf ( NAryQueue.getLastArgList(), data ); }

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
	DataTypeCenter& getDataTypeCenter ( void ) { return DTCenter; }
		/// get RO access to a DT center
	const DataTypeCenter& getDataTypeCenter ( void ) const { return DTCenter; }

	////////////////////////////////////////////
	//  ensure* declarations
	////////////////////////////////////////////

	// ensure that given names are of the apropriate types

		/// register NAME as a concept
	ComplexConcept ensureConceptName ( const std::string& name )
		{ return new DLTree ( TLexeme ( CNAME, Concepts.insert(name) ) ); }
		/// register NAME as an individual
	ComplexConcept ensureSingletonName ( const std::string& name )
		{ return new DLTree ( TLexeme ( INAME, Individuals.insert(name) ) ); }
		/// register NAME as a data role (data property)
	ComplexRole ensureDataRoleName ( const std::string& name )
		{ return new DLTree ( TLexeme ( DNAME, DRoles.insert(name) ) ); }
		/// register NAME as a role (object property)
	ComplexRole ensureRoleName ( const std::string& name )
	{
		// kludge for LISP interface
		if ( DRoles.get(name) != NULL )
			return ensureDataRoleName(name);
		return new DLTree ( TLexeme ( RNAME, ORoles.insert(name) ) );
	}

		/// check whether every named entry of E is defined in the KB
	static void checkDefined ( const DLTree* E ) throw(EFPPCantRegName)
	{
		if ( E == NULL )
			return;
		switch ( E->Element().getToken() )
		{
		case CNAME:
			if ( E->Element().getNE() == NULL )
				throw EFPPCantRegName ( E->Element().getName(), "concept" );
			break;
		case INAME:
			if ( E->Element().getNE() == NULL )
				throw EFPPCantRegName ( E->Element().getName(), "individual" );
			break;
		case RNAME:
			if ( E->Element().getNE() == NULL )
				throw EFPPCantRegName ( E->Element().getName(), "role" );
			break;
		case DNAME:
			if ( E->Element().getNE() == NULL )
				throw EFPPCantRegName ( E->Element().getName(), "data role" );
			break;
		default:
			checkDefined(E->Left());
			checkDefined(E->Right());
			break;
		};
	}

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
	bool newKB ( void )
	{
		if ( pTBox != NULL )
			return true;

		pTBox = new TBox ( getOptions(), DTCenter );
		initCacheAndFlags();
		return false;
	}
		/// delete existed KB
	bool releaseKB ( void )
	{
		delete pTBox;
		pTBox = NULL;
		deleteTree(cachedQuery);
		cachedQuery = NULL;
		Ontology.clear();
		Concepts.clear();
		Individuals.clear();
		ORoles.clear();
		DRoles.clear();

		return false;
	}
		/// reset current KB
	bool clearKB ( void )
	{
		if ( pTBox == NULL )
			return true;
		return releaseKB () || newKB ();
	}

	//******************************************
	//* Concept/Role expressions. DL syntax, not DIG/OWL
	//******************************************

		/// @return TOP
	ComplexConcept Top ( void ) const { return new DLTree(TOP); }
		/// @return BOTTOM
	ComplexConcept Bottom ( void ) const { return new DLTree(BOTTOM); }
		/// @return ~C
	ComplexConcept Not ( ComplexConcept C ) const { return createSNFNot(C); }
		/// @return C/\D
	ComplexConcept And ( ComplexConcept C, ComplexConcept D ) const { return createSNFAnd ( C, D ); }
		/// @return C1/\.../\Cn
	ComplexConcept And ( void ) { return getTBox()->processAnd(NAryQueue.getLastArgList()); }
		/// @return C\/D
	ComplexConcept Or ( ComplexConcept C, ComplexConcept D ) const { return createSNFOr ( C, D ); }
		/// @return C1\/...\/Cn
	ComplexConcept Or ( void ) { return getTBox()->processOr(NAryQueue.getLastArgList()); }

		/// start new argument list for n-ary concept expressions/axioms
	void openArgList ( void ) { NAryQueue.openArgList(); }
		/// add an element C to the most recent open argument list
	void addArg ( const ComplexConcept C ) { NAryQueue.addArg(C); }

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
	ComplexConcept Exists ( ComplexRole R, ComplexConcept C ) const { return createSNFExists ( R, C ); }
		/// @return \A R.C
	ComplexConcept Forall ( ComplexRole R, ComplexConcept C ) const { return createSNFForall ( R, C ); }
		/// @return \E R.I for individual/data value I
	ComplexConcept Value ( ComplexRole R, ComplexConcept I ) const { return createSNFExists ( R, I ); }
		/// @return <= n R.C
	ComplexConcept MaxCardinality ( unsigned int n, ComplexRole R, ComplexConcept C ) const { return createSNFLE ( n, R, C ); }
		/// @return >= n R.C
	ComplexConcept MinCardinality ( unsigned int n, ComplexRole R, ComplexConcept C ) const { return createSNFGE ( n, R, C ); }
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
	ComplexRole Inverse ( ComplexRole R ) const { return createInverse(R); }
		/// @return R*S
	ComplexRole Compose ( ComplexRole R, ComplexRole S ) const { return new DLTree ( TLexeme(RCOMPOSITION), R, S ); }
		/// @return R1*...*Rn
	ComplexRole Compose ( void ) { return getTBox()->processRComposition(NAryQueue.getLastArgList()); }

	//----------------------------------------------------
	//	TELLS interface
	//----------------------------------------------------

	// Declaration axioms

		/// axiom declare(x)
	TDLAxiom* declare ( const ComplexConcept C ) { return Ontology.add(new TDLAxiomDeclaration(C)); }

	// Concept axioms

		/// axiom C [= D
	TDLAxiom* impliesConcepts ( const ComplexConcept C, const ComplexConcept D )
		{ return Ontology.add ( new TDLAxiomConceptInclusion ( C, D ) ); }
		/// axiom C1 = ... = Cn
	TDLAxiom* equalConcepts ( void )
		{ return Ontology.add ( new TDLAxiomEquivalentConcepts(NAryQueue.getLastArgList()) ); }
		/// axiom C = D
	TDLAxiom* equalConcepts ( const ComplexConcept C, const ComplexConcept D )
		{ return Ontology.add ( new TDLAxiomEquivalentConcepts ( C, D ) ); }
		/// axiom C1 != ... != Cn
	TDLAxiom* disjointConcepts ( void )
		{ return Ontology.add ( new TDLAxiomDisjointConcepts(NAryQueue.getLastArgList()) ); }


	// Role axioms

		/// axiom (R [= S)
	TDLAxiom* impliesRoles ( ComplexRole R, ComplexRole S )
		{ return Ontology.add ( new TDLAxiomRoleSubsumption ( R, S ) ); }
		/// axiom R1 = R2 = ...
	TDLAxiom* equalRoles ( void )
		{ return Ontology.add ( new TDLAxiomEquivalentRoles(NAryQueue.getLastArgList()) ); }
		/// axiom (R = S)
	TDLAxiom* equalRoles ( const ComplexRole R, const ComplexRole S )
		{ return Ontology.add ( new TDLAxiomEquivalentRoles ( R, S ) ); }
		/// axiom R1 != R2 != ...
	TDLAxiom* disjointRoles ( void )
		{ return Ontology.add ( new TDLAxiomDisjointRoles(NAryQueue.getLastArgList()) ); }
		/// axiom (R != S)
	TDLAxiom* disjointRoles ( const ComplexRole R, const ComplexRole S )
		{ return Ontology.add ( new TDLAxiomDisjointRoles ( R, S ) ); }

		/// Domain (R C)
	TDLAxiom* setDomain ( ComplexRole R, const ComplexConcept C )
		{ return Ontology.add ( new TDLAxiomRoleDomain ( R, C ) ); }
		/// Range (R C)
	TDLAxiom* setRange ( ComplexRole R, const ComplexConcept C )
		{ return Ontology.add ( new TDLAxiomRoleRange ( R, C ) ); }

		/// Transitive (R)
	TDLAxiom* setTransitive ( ComplexRole R )
		{ return Ontology.add ( new TDLAxiomRoleTransitive(R) ); }
		/// Reflexive (R)
	TDLAxiom* setReflexive ( ComplexRole R )
		{ return Ontology.add ( new TDLAxiomRoleReflexive(R) ); }
		/// Irreflexive (R): Domain(R) = \neg ER.Self
	TDLAxiom* setIrreflexive ( ComplexRole R )
		{ return Ontology.add ( new TDLAxiomRoleIrreflexive(R) ); }
		/// Symmetric (R): R [= R^-
	TDLAxiom* setSymmetric ( ComplexRole R )
		{ return Ontology.add ( new TDLAxiomRoleSymmetric(R) ); }
		/// AntySymmetric (R): disjoint(R,R^-)
	TDLAxiom* setAntiSymmetric ( ComplexRole R )
		{ return Ontology.add ( new TDLAxiomRoleAntiSymmetric(R) ); }
		/// Functional (R)
	TDLAxiom* setFunctional ( ComplexRole R )
		{ return Ontology.add ( new TDLAxiomRoleFunctional(R) ); }


	// Individual axioms

		/// axiom I e C
	TDLAxiom* instanceOf ( const ComplexConcept I, const ComplexConcept C )
		{ return Ontology.add ( new TDLAxiomInstanceOf(I,C) ); }
		/// axiom <I,J>:R
	TDLAxiom* relatedTo ( const ComplexConcept I, const ComplexRole R, const ComplexConcept J )
		{ return Ontology.add ( new TDLAxiomRelatedTo(I,R,J) ); }
		/// axiom <I,J>:\neg R
	TDLAxiom* relatedToNot ( const ComplexConcept I, const ComplexRole R, const ComplexConcept J )
		{ return Ontology.add ( new TDLAxiomRelatedToNot(I,R,J) ); }
		/// axiom (value I A V)
	TDLAxiom* valueOf ( const ComplexConcept I, const ComplexRole A, const ComplexConcept V )
		{ return Ontology.add ( new TDLAxiomValueOf(I,A,V) ); }
		/// axiom <I,V>:\neg A
	TDLAxiom* valueOfNot ( const ComplexConcept I, const ComplexRole A, const ComplexConcept V )
		{ return Ontology.add ( new TDLAxiomValueOfNot(I,A,V) ); }
		/// same individuals
	TDLAxiom* processSame ( void )
		{ return Ontology.add ( new TDLAxiomSameIndividuals(NAryQueue.getLastArgList()) ); }
		/// different individuals
	TDLAxiom* processDifferent ( void )
		{ return Ontology.add ( new TDLAxiomDifferentIndividuals(NAryQueue.getLastArgList()) ); }
		/// let all concept expressions in the ArgQueue to be fairness constraints
	TDLAxiom* setFairnessConstraint ( void )
		{ return Ontology.add ( new TDLAxiomFairnessConstraint(NAryQueue.getLastArgList()) ); }

		/// retract an axiom
	void retract ( TDLAxiom* axiom ) { axiom->setUsed(false); }

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
			throw EFPPInconsistentKB();
	}
		/// ensure that KB is classified
	void classifyKB ( void )
	{
		if ( !isKBClassified() )
			processKB(kbClassified);
		if ( !isKBConsistent() )
			throw EFPPInconsistentKB();
	}
		/// ensure that KB is realised
	void realiseKB ( void )
	{
		if ( !isKBRealised() )
			processKB(kbRealised);
		if ( !isKBConsistent() )
			throw EFPPInconsistentKB();
	}

	// role info retrieval

		/// @return true iff role is functional
	bool isFunctional ( const ComplexRole R )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		checkDefined(R);
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
		checkDefined(C);
		if ( isCN(C) )
			return getTBox()->isSatisfiable(getTBox()->getCI(C));

		setUpCache ( C, csSat );
		return getTBox()->isSatisfiable(cachedConcept);
	}
		/// @return true iff C [= D holds
	bool isSubsumedBy ( const ComplexConcept C, const ComplexConcept D )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		checkDefined(C);
		checkDefined(D);
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
		setUpCache ( C, csClassified );
		cachedVertex->getRelativesInfo</*needCurrent=*/true, /*onlyDirect=*/true,
									   /*upDirection=*/false>(actor);
	}

		/// apply actor::apply() to all instances of given [complex] C
	template<class Actor>
	void getInstances ( const ComplexConcept C, Actor& actor )
	{	// FIXME!! check for Racer's/IS approach
		realiseKB();	// ensure KB is ready to answer the query
		setUpCache ( C, csClassified );
		cachedVertex->getRelativesInfo</*needCurrent=*/true, /*onlyDirect=*/false,
									   /*upDirection=*/false>(actor);
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

#endif
