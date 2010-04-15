/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2010 by Dmitry Tsarkov

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

#ifndef KERNEL_H
#define KERNEL_H

#include <string>

#include "fpp_assert.h"
#include "eFPPInconsistentKB.h"
#include "tNAryQueue.h"
#include "dlTBox.h"
#include "ReasonerNom.h"
#include "ifOptions.h"
#include "DLConceptTaxonomy.h"	// for getRelatives()
#include "tOntology.h"

class ReasoningKernel;
typedef ReasoningKernel TExpressionManager;

class ReasoningKernel
{
public:	// types interface
	/*
		The type system for DL expressions used in the input language:

		TExpr;
		 TConceptExpr: TExpr;
		 TIndividualExpr : TExpr;
		 TRoleExpr: TExpr;
		  TORoleComplexExpr: TRoleExpr;
		   TORoleExpr: TORoleComplexExpr;
		  TDRoleExpr: TRoleExpr;
		 TDataExpr: TExpr;
		  TDataValueExpr: TDataExpr;

		Right now all the expressions are of the type DLTree*.
		Later on the appropriate classes from tDlExpression.h

		As a transition step, the intermediate structure is introduced.
	*/

		/// general expression
	typedef DLTree TExpr;
		/// concept expression
	typedef DLTree TConceptExpr;
		/// individual expression
	typedef DLTree TIndividualExpr;
		/// role expression
	typedef DLTree TRoleExpr;
		/// object role complex expression (including role chains and projections)
	typedef DLTree TORoleComplexExpr;
		/// object role expression
	typedef DLTree TORoleExpr;
		/// data role expression
	typedef DLTree TDRoleExpr;
		/// data expression
	typedef DLTree TDataExpr;
		/// data value expression
	typedef DLTree TDataValueExpr;

	// name sets

		/// set of arbitrary named expressions
	typedef TBox::NamesVector NamesVector;
		// IndividualSet is just set of named individual expressions
	typedef NamesVector IndividualSet;

		/// typedef for intermediate instance related type
	typedef TRelatedMap::CIVec CIVec;

private:
		/// options for the kernel and all related substructures
	ifOptionSet KernelOptions;

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
		/// set of TreeNE
/*	class TreeNESet: public TNameSet<TTreeNamedEntry>
	{
	public:
			/// dirty hack for the LISP ontology printing
		template<class T>
		void fill ( T& x ) const
		{
			for ( const_iterator p = Base.begin(); p != Base.end(); ++p )
				x.recordDataRole(p->second->getName());
		}
	}; // TreeNESet
*/
protected:	// members
		/// local TBox (to be created)
	TBox* pTBox;
		/// DataType center
	DataTypeCenter DTCenter;
		/// set of queues for the n-ary expressions/commands
	TNAryQueue<DLTree> NAryQueue;
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
		/// timeout value
	unsigned long OpTimeout;

private:	// no copy
		/// no copy c'tor
	ReasoningKernel ( const ReasoningKernel& );
		/// no assignment
	ReasoningKernel& operator = ( const ReasoningKernel& );

protected:	// methods

	// register all necessary options in local option set
	bool initOptions ( void );
		/// register the pointer that is go outside the kernel
		// do nothing for now
	DLTree* regPointer ( DLTree* arg ) { return arg; }

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
	bool isCached ( const TConceptExpr* query ) const
		{ return ( cachedQuery == NULL ? false : equalTrees ( cachedQuery, query ) ); }
		/// set up cache for query, performing additional (re-)classification if necessary
	void setUpCache ( const TConceptExpr* query, cacheStatus level );
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

		/// build and set a cache for an individual I wrt role R
	CIVec buildRelatedCache ( TIndividual* I, const TRole* R );
		/// get related cache for an individual I
	const CIVec& getRelated ( TIndividual* I, const TRole* R )
	{
		if ( !I->hasRelatedCache(R) )
			I->setRelatedCache ( R, buildRelatedCache ( I, R ) );
		return I->getRelatedCache(R);
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

		/// get RW access to Object RoleMaster from TBox
	RoleMaster* getORM ( void ) { return getTBox()->getORM(); }
		/// get RO access to Object RoleMaster from TBox
	const RoleMaster* getORM ( void ) const { return getTBox()->getORM(); }
		/// get RW access to Data RoleMaster from TBox
	RoleMaster* getDRM ( void ) { return getTBox()->getDRM(); }
		/// get RO access to Data RoleMaster from TBox
	const RoleMaster* getDRM ( void ) const { return getTBox()->getDRM(); }

		/// get access to the concept hierarchy
	const Taxonomy* getCTaxonomy ( void ) const
	{
		if ( !isKBClassified() )
			throw EFaCTPlusPlus("No access to concept taxonomy: ontology not classified");
		return getTBox()->getTaxonomy();
	}
		/// get access to the object role hierarchy
	const Taxonomy* getORTaxonomy ( void ) const
	{
		if ( !isKBPreprocessed() )
			throw EFaCTPlusPlus("No access to the object role taxonomy: ontology not preprocessed");
		return getORM()->getTaxonomy();
	}
		/// get access to the data role hierarchy
	const Taxonomy* getDRTaxonomy ( void ) const
	{
		if ( !isKBPreprocessed() )
			throw EFaCTPlusPlus("No access to the data role taxonomy: ontology not preprocessed");
		return getDRM()->getTaxonomy();
	}

	// transformation methods

		/// get individual by a DLTree
	TIndividual* getIndividual ( const TIndividualExpr* i, const char* reason ATTR_UNUSED )
	{
		if ( !getTBox()->isIndividual(i) )
			throw EFaCTPlusPlus(reason);
		return static_cast<TIndividual*>(getTBox()->getCI(i));
	}
		/// get role by the DLTree
	TRole* getRole ( const TRoleExpr* r, const char* reason ) const
	{
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

	ifOptionSet* getOptions ( void ) { return &KernelOptions; }
	const ifOptionSet* getOptions ( void ) const { return &KernelOptions; }

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

		/// set timeout value to VALUE
	void setOperationTimeout ( unsigned long value )
	{
		OpTimeout = value;
		if ( pTBox != NULL )
			pTBox->setTestTimeout(value);
	}

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

		/// get access to an expression manager
	TExpressionManager* getExpressionManager ( void ) { return this; }

public:
	//******************************************
	//* KB Management
	//******************************************

		/// create new KB
	bool newKB ( void )
	{
		if ( pTBox != NULL )
			return true;

		pTBox = new TBox ( getOptions(), DTCenter );
		pTBox->setTestTimeout(OpTimeout);
		initCacheAndFlags();
#	ifdef OWLAPI3
		declare(ObjectRole("http://www.w3.org/2002/07/owl#topObjectProperty"));
		declare(ObjectRole("http://www.w3.org/2002/07/owl#bottomObjectProperty"));
		declare(DataRole("http://www.w3.org/2002/07/owl#topDataProperty"));
		declare(DataRole("http://www.w3.org/2002/07/owl#bottomDataProperty"));
#	endif
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

		return false;
	}
		/// reset current KB
	bool clearKB ( void )
	{
		if ( pTBox == NULL )
			return true;
		return releaseKB () || newKB ();
	}

	//-------------------------------------------------------------------
	//--	DL expressions. DL syntax, not DIG/OWL
	//-------------------------------------------------------------------

		/// start new argument list for n-ary concept expressions/axioms
	void openArgList ( void ) { NAryQueue.openArgList(); }
		/// add an element C to the most recent open argument list
	void addArg ( TExpr* C ) { NAryQueue.addArg(C); }

	//-------------------------------------------------------------------
	//--	Concept expressions
	//-------------------------------------------------------------------

		/// @return TOP
	TConceptExpr* Top ( void ) { return regPointer( new DLTree(TOP) ); }
		/// @return BOTTOM
	TConceptExpr* Bottom ( void ) { return regPointer ( new DLTree(BOTTOM) ); }
		/// @return concept corresponding to NAME
	TConceptExpr* Concept ( const std::string& name ) { return regPointer ( new DLTree ( TLexeme ( CNAME, getTBox()->getConcept(name) ) ) ); }
		/// @return ~C
	TConceptExpr* Not ( TConceptExpr* C ) { return regPointer(createSNFNot(C)); }
		/// @return C/\D
	TConceptExpr* And ( TConceptExpr* C, TConceptExpr* D ) { return regPointer ( createSNFAnd ( C, D ) ); }
		/// @return C1/\.../\Cn
	TConceptExpr* And ( void ) { return regPointer(getTBox()->processAnd(NAryQueue.getLastArgList())); }
		/// @return C1\/...\/Cn
	TConceptExpr* Or ( void ) { return regPointer(getTBox()->processOr(NAryQueue.getLastArgList())); }
		/// @return \E R.C
	TConceptExpr* Exists ( TORoleExpr* R, TConceptExpr* C ) { return regPointer ( createSNFExists ( R, C ) ); }
		/// @return \E R.D
//	TConceptExpr* Exists ( TDRoleExpr* R, TDataExpr* D ) { return regPointer ( createSNFExists ( R, D ) ); }
		/// @return \A R.C
	TConceptExpr* Forall ( TORoleExpr* R, TConceptExpr* C ) { return regPointer ( createSNFForall ( R, C ) ); }
		/// @return \A R.D
//	TConceptExpr* Forall ( TDRoleExpr* R, TDataExpr* D ) { return regPointer ( createSNFForall ( R, D ) ); }
		/// @return \E R.I for individual value I
	TConceptExpr* Value ( TORoleExpr* R, TIndividualExpr* I ) { return regPointer ( createSNFExists ( R, I ) ); }
		/// @return \E R.V for data value I
//	TConceptExpr* Value ( TORoleExpr* R, TDataValueExpr* V ) { return regPointer ( createSNFExists ( R, V ) ); }
		/// @return <= n R.C
	TConceptExpr* MaxCardinality ( unsigned int n, TORoleExpr* R, TConceptExpr* C ) { return regPointer ( createSNFLE ( n, R, C ) ); }
		/// @return >= n R.C
	TConceptExpr* MinCardinality ( unsigned int n, TORoleExpr* R, TConceptExpr* C ) { return regPointer ( createSNFGE ( n, R, C ) ); }
		/// @return = n R.C
	TConceptExpr* Cardinality ( unsigned int n, TORoleExpr* R, TConceptExpr* C )
	{
		DLTree* tMin = createSNFLE ( n, clone(R), clone(C) );
		DLTree* tMax = createSNFGE ( n, R, C );
		return regPointer ( createSNFAnd ( tMin, tMax ) );
	}
		/// @return <= n R.D
//	TConceptExpr* MaxCardinality ( unsigned int n, TORoleExpr* R, TDataExpr* D ) { return regPointer ( createSNFLE ( n, R, D ) ); }
		/// @return >= n R.D
//	TConceptExpr* MinCardinality ( unsigned int n, TORoleExpr* R, TDataExpr* D ) { return regPointer ( createSNFGE ( n, R, D ) ); }
		/// @return = n R.D
/*	TConceptExpr* Cardinality ( unsigned int n, TORoleExpr* R, TDataExpr* D )
	{
		DLTree* tMin = createSNFLE ( n, clone(R), clone(D) );
		DLTree* tMax = createSNFGE ( n, R, D );
		return regPointer ( createSNFAnd ( tMin, tMax ) );
	}*/
		/// @return \E R.Self
	TConceptExpr* SelfReference ( TORoleExpr* R ) { return regPointer ( new DLTree ( REFLEXIVE, R ) ); }
		/// @return one-of construction for the arguments in NAryQueue
	TConceptExpr* OneOf ( void ) { return regPointer ( getTBox()->processOneOf ( NAryQueue.getLastArgList(), /*data=*/false ) ); }
		/// @return concept {I} for the individual I
	TConceptExpr* OneOf ( TIndividualExpr* I )
	{
		getExpressionManager()->openArgList();
		getExpressionManager()->addArg(I);
		return OneOf();
	}

	//-------------------------------------------------------------------
	//--	(Data)Role expressions
	//-------------------------------------------------------------------

		/// @return object role corresponding to NAME
	TORoleExpr* ObjectRole ( const std::string& name ) { return regPointer ( new DLTree ( TLexeme ( RNAME, getORM()->ensureRoleName(name) ) ) ); }
		/// @return data role corresponding to NAME
	TDRoleExpr* DataRole ( const std::string& name ) { return regPointer ( new DLTree ( TLexeme ( DNAME, getDRM()->ensureRoleName(name) ) ) ); }
		/// @return universal role
//	ComplexRole UniversalRole ( void ) const { return new DLTree(UROLE); }
		/// @return R^-
	TORoleExpr* Inverse ( TORoleExpr* R ) { return regPointer(createInverse(R)); }
		/// @return R1*...*Rn
	TORoleComplexExpr* Compose ( void ) { return regPointer(getTBox()->processRComposition(NAryQueue.getLastArgList())); }
		/// @return project R into C
	TORoleComplexExpr* ProjectInto ( TORoleExpr* R, TConceptExpr* C ) { return regPointer ( new DLTree ( TLexeme(PROJINTO), R, C ) ); }
		/// @return project R from C
	TORoleComplexExpr* ProjectFrom ( TORoleExpr* R, TConceptExpr* C ) { return regPointer ( new DLTree ( TLexeme(PROJFROM), R, C ) ); }

	//-------------------------------------------------------------------
	//--	individual expressions
	//-------------------------------------------------------------------

		/// @return individual corresponding to NAME
	TIndividualExpr* Individual ( const std::string& name ) { return regPointer ( new DLTree ( TLexeme ( INAME, getTBox()->getIndividual(name) ) ) ); }

	//-------------------------------------------------------------------
	//--	data expressions (data values and types are obtained by DataTypeCenter
	//-------------------------------------------------------------------

		/// @return data TOP
	TDataExpr* DataTop ( void ) { return regPointer ( new DLTree(TOP) ); }
		/// @return data BOTTOM
	TDataExpr* DataBottom ( void ) { return regPointer ( new DLTree(BOTTOM) ); }
		/// @return data negation
	TDataExpr* DataNot ( TDataExpr* E ) { return regPointer(createSNFNot(E)); }
		/// @return conjunction of data expressions
	TDataExpr* DataAnd ( void ) { return regPointer(getTBox()->processAnd(NAryQueue.getLastArgList())); }
		/// @return disjunction of data expressions
	TDataExpr* DataOr ( void ) { return regPointer(getTBox()->processOr(NAryQueue.getLastArgList())); }
		/// @return {v_1,...,v_n} constructor for the data values in NAryQueue
	TDataExpr* DataOneOf ( void ) { return regPointer ( getTBox()->processOneOf ( NAryQueue.getLastArgList(), /*data=*/true ) ); }

	//----------------------------------------------------
	//	TELLS interface
	//----------------------------------------------------

	// Declaration axioms

		/// axiom declare(x)
	TDLAxiom* declare ( TExpr* C ) { return Ontology.add(new TDLAxiomDeclaration(C)); }

	// Concept axioms

		/// axiom C [= D
	TDLAxiom* impliesConcepts ( TConceptExpr* C,TConceptExpr* D )
		{ return Ontology.add ( new TDLAxiomConceptInclusion ( C, D ) ); }
		/// axiom C1 = ... = Cn
	TDLAxiom* equalConcepts ( void )
		{ return Ontology.add ( new TDLAxiomEquivalentConcepts(NAryQueue.getLastArgList()) ); }
		/// axiom C1 != ... != Cn
	TDLAxiom* disjointConcepts ( void )
		{ return Ontology.add ( new TDLAxiomDisjointConcepts(NAryQueue.getLastArgList()) ); }


	// Role axioms

		/// R = Inverse(S)
	TDLAxiom* setInverseRoles ( TORoleExpr* R, TORoleExpr* S )
		{ return Ontology.add ( new TDLAxiomRoleInverse(R,S) ); }
		/// axiom (R [= S)
	TDLAxiom* impliesORoles ( TORoleComplexExpr* R, TORoleExpr* S )
		{ return Ontology.add ( new TDLAxiomORoleSubsumption ( R, S ) ); }
		/// axiom (R [= S)
	TDLAxiom* impliesDRoles ( TDRoleExpr* R, TDRoleExpr* S )
		{ return Ontology.add ( new TDLAxiomDRoleSubsumption ( R, S ) ); }
		/// axiom R1 = R2 = ...
	TDLAxiom* equalORoles ( void )
		{ return Ontology.add ( new TDLAxiomEquivalentORoles(NAryQueue.getLastArgList()) ); }
		/// axiom R1 = R2 = ...
	TDLAxiom* equalDRoles ( void )
		{ return Ontology.add ( new TDLAxiomEquivalentDRoles(NAryQueue.getLastArgList()) ); }
		/// axiom R1 != R2 != ...
	TDLAxiom* disjointORoles ( void )
		{ return Ontology.add ( new TDLAxiomDisjointORoles(NAryQueue.getLastArgList()) ); }
		/// axiom R1 != R2 != ...
	TDLAxiom* disjointDRoles ( void )
		{ return Ontology.add ( new TDLAxiomDisjointDRoles(NAryQueue.getLastArgList()) ); }

		/// Domain (R C)
	TDLAxiom* setODomain ( TORoleExpr* R, TConceptExpr* C )
		{ return Ontology.add ( new TDLAxiomORoleDomain ( R, C ) ); }
		/// Domain (R C)
	TDLAxiom* setDDomain ( TDRoleExpr* R, TConceptExpr* C )
		{ return Ontology.add ( new TDLAxiomDRoleDomain ( R, C ) ); }
		/// Range (R C)
	TDLAxiom* setORange ( TORoleExpr* R, TConceptExpr* C )
		{ return Ontology.add ( new TDLAxiomORoleRange ( R, C ) ); }
		/// Range (R E)
	TDLAxiom* setDRange ( TDRoleExpr* R, TDataExpr* E )
		{ return Ontology.add ( new TDLAxiomDRoleRange ( R, E ) ); }

		/// Transitive (R)
	TDLAxiom* setTransitive ( TORoleExpr* R )
		{ return Ontology.add ( new TDLAxiomRoleTransitive(R) ); }
		/// Reflexive (R)
	TDLAxiom* setReflexive ( TORoleExpr* R )
		{ return Ontology.add ( new TDLAxiomRoleReflexive(R) ); }
		/// Irreflexive (R): Domain(R) = \neg ER.Self
	TDLAxiom* setIrreflexive ( TORoleExpr* R )
		{ return Ontology.add ( new TDLAxiomRoleIrreflexive(R) ); }
		/// Symmetric (R): R [= R^-
	TDLAxiom* setSymmetric ( TORoleExpr* R )
		{ return Ontology.add ( new TDLAxiomRoleSymmetric(R) ); }
		/// AntySymmetric (R): disjoint(R,R^-)
	TDLAxiom* setAntiSymmetric ( TORoleExpr* R )
		{ return Ontology.add ( new TDLAxiomRoleAntiSymmetric(R) ); }
		/// Functional (R)
	TDLAxiom* setOFunctional ( TORoleExpr* R )
		{ return Ontology.add ( new TDLAxiomORoleFunctional(R) ); }
		/// Functional (R)
	TDLAxiom* setDFunctional ( TDRoleExpr* R )
		{ return Ontology.add ( new TDLAxiomDRoleFunctional(R) ); }
		/// InverseFunctional (R)
	TDLAxiom* setInverseFunctional ( TORoleExpr* R )
		{ return Ontology.add ( new TDLAxiomRoleInverseFunctional(R) ); }


	// Individual axioms

		/// axiom I e C
	TDLAxiom* instanceOf ( TIndividualExpr* I, TConceptExpr* C )
		{ return Ontology.add ( new TDLAxiomInstanceOf(I,C) ); }
		/// axiom <I,J>:R
	TDLAxiom* relatedTo ( TIndividualExpr* I, TORoleExpr* R, TIndividualExpr* J )
		{ return Ontology.add ( new TDLAxiomRelatedTo(I,R,J) ); }
		/// axiom <I,J>:\neg R
	TDLAxiom* relatedToNot ( TIndividualExpr* I, TORoleExpr* R, TIndividualExpr* J )
		{ return Ontology.add ( new TDLAxiomRelatedToNot(I,R,J) ); }
		/// axiom (value I A V)
	TDLAxiom* valueOf ( TIndividualExpr* I, TDRoleExpr* A, TDataValueExpr* V )
		{ return Ontology.add ( new TDLAxiomValueOf(I,A,V) ); }
		/// axiom <I,V>:\neg A
	TDLAxiom* valueOfNot ( TIndividualExpr* I, TDRoleExpr* A, TDataValueExpr* V )
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
		/// try to perform the incremental reasoning on the changed ontology
	bool tryIncremental ( void );
		/// force the re-classification of the changed ontology
	void forceReclassify ( void )
	{
		delete pTBox;
		pTBox = NULL;
		newKB();
		DTCenter.clearTypes();
		realiseKB();
	}
		/// re-classification of the changed ontology
	void reclassify ( void )
	{
		if ( tryIncremental() )
			forceReclassify();
	}

	// role info retrieval

		/// @return true iff role is functional
	bool isFunctional ( const TRoleExpr* R )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		if ( isUniversalRole(R) )
			return false;	// universal role is not functional

		return getRole ( R, "Role expression expected in isFunctional()" )->isFunctional();
	}
		/// @return true iff role is inverse-functional
	bool isInverseFunctional ( const TORoleExpr* R )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		if ( isUniversalRole(R) )
			return false;	// universal role is not functional

		return getRole ( R, "Role expression expected in isInverseFunctional()" )->inverse()->isFunctional();
	}
		/// @return true iff two roles are disjoint
	bool isDisjointRoles ( const TORoleExpr* R, const TORoleExpr* S )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		// FIXME!! add check for the empty role
		if ( isUniversalRole(R) || isUniversalRole(S) )
			return false;	// universal role is not disjoint with anything
		return getTBox()->isDisjointRoles (
			getRole ( R, "Role expression expected in isDisjointRoles()" ),
			getRole ( S, "Role expression expected in isDisjointRoles()" ) );
	}
/*		/// @return true iff two roles are disjoint
	bool isDisjointRoles ( const TDRoleExpr* R, const TDRoleExpr* S )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		// FIXME!! add check for the empty role
		if ( isUniversalRole(R) || isUniversalRole(S) )
			return false;	// universal role is not disjoint with anything
		return getTBox()->isDisjointRoles (
			getRole ( R, "Role expression expected in isDisjointRoles()" ),
			getRole ( S, "Role expression expected in isDisjointRoles()" ) );
	}
*/
	// TBox info retriveal

	// apply actor.apply() to all concept names
	template<class Actor>
	void getAllConcepts ( Actor& actor )
	{
		classifyKB();	// ensure KB is ready to answer the query
		TaxonomyVertex* p = getCTaxonomy()->getTop();	// need for successful compilation
		p->getRelativesInfo</*needCurrent=*/true, /*onlyDirect=*/false, /*upDirection=*/false>(actor);
	}

	// apply actor.apply() to all object rolenames
	template<class Actor>
	void getAllORoles ( Actor& actor )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		TaxonomyVertex* p = getORTaxonomy()->getBottom();	// need for successful compilation
		p->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/false, /*upDirection=*/true>(actor);
	}

	// apply actor.apply() to all object rolenames
	template<class Actor>
	void getAllDRoles ( Actor& actor )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		TaxonomyVertex* p = getDRTaxonomy()->getBottom();	// need for successful compilation
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
	bool isSatisfiable ( const TConceptExpr* C )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		if ( isCN(C) )
			return getTBox()->isSatisfiable(getTBox()->getCI(C));

		setUpCache ( C, csSat );
		return getTBox()->isSatisfiable(cachedConcept);
	}
		/// @return true iff C [= D holds
	bool isSubsumedBy ( const TConceptExpr* C, const TConceptExpr* D )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		if ( isCN(C) && isCN(D) )
			return getTBox()->isSubHolds ( getTBox()->getCI(C), getTBox()->getCI(D) );

		return !isSatisfiable ( TreeDeleter ( And ( clone(C), Not(clone(D)) ) ) );
	}
		/// @return true iff C is disjoint with D
	bool isDisjoint ( const TConceptExpr* C, const TConceptExpr* D )
		{ return !isSatisfiable ( TreeDeleter ( And ( clone(C), clone(D) ) ) ); }
		/// @return true iff C is equivalent to D
	bool isEquivalent ( const TConceptExpr* C, const TConceptExpr* D )
		{ return isSubsumedBy ( C, D ) && isSubsumedBy ( D, C ); }

	// concept hierarchy

		/// apply actor::apply() to all direct parents of [complex] C (with possible synonyms)
	template<class Actor>
	void getParents ( const TConceptExpr* C, Actor& actor )
	{
		classifyKB();	// ensure KB is ready to answer the query
		setUpCache ( C, csClassified );
		cachedVertex->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/true,
									   /*upDirection=*/true>(actor);
	}
		/// apply Actor::apply() to all direct children of [complex] C (with possible synonyms)
	template<class Actor>
	void getChildren ( const TConceptExpr* C, Actor& actor )
	{
		classifyKB();	// ensure KB is ready to answer the query
		setUpCache ( C, csClassified );
		cachedVertex->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/true,
									   /*upDirection=*/false>(actor);
	}
		/// apply actor::apply() to all parents of [complex] C (with possible synonyms)
	template<class Actor>
	void getAncestors ( const TConceptExpr* C, Actor& actor )
	{
		classifyKB();	// ensure KB is ready to answer the query
		setUpCache ( C, csClassified );
		cachedVertex->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/false,
									   /*upDirection=*/true>(actor);
	}
		/// apply actor::apply() to all children of [complex] C (with possible synonyms)
	template<class Actor>
	void getDescendants ( const TConceptExpr* C, Actor& actor )
	{
		classifyKB();	// ensure KB is ready to answer the query
		setUpCache ( C, csClassified );
		cachedVertex->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/false,
									   /*upDirection=*/false>(actor);
	}
		/// apply actor::apply() to all synonyms of [complex] C
	template<class Actor>
	void getEquivalents ( const TConceptExpr* C, Actor& actor )
	{
		classifyKB();	// ensure KB is ready to answer the query
		setUpCache ( C, csClassified );
		actor.apply(*cachedVertex);
	}

	// role hierarchy

	// all direct parents of R (with possible synonyms)
	template<class Actor>
	void getRParents ( const TRoleExpr* r, Actor& actor )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		TRole* R = getRole ( r, "Role expression expected in getRParents()" );
		TaxonomyVertex* p = R->getTaxVertex();	// need this for compiler
		p->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/true, /*upDirection=*/true>(actor);
	}

	// all direct children of R (with possible synonyms)
	template<class Actor>
	void getRChildren ( const TRoleExpr* r, Actor& actor )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		TRole* R = getRole ( r, "Role expression expected in getRChildren()" );
		TaxonomyVertex* p = R->getTaxVertex();	// need this for compiler
		p->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/true, /*upDirection=*/false>(actor);
	}

	// all parents of R (with possible synonyms)
	template<class Actor>
	void getRAncestors ( const TRoleExpr* r, Actor& actor )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		TRole* R = getRole ( r, "Role expression expected in getRAncestors()" );
		TaxonomyVertex* p = R->getTaxVertex();	// need this for compiler
		p->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/false, /*upDirection=*/true>(actor);
	}

	// all children of R (with possible synonyms)
	template<class Actor>
	void getRDescendants ( const TRoleExpr* r, Actor& actor )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		TRole* R = getRole ( r, "Role expression expected in getRDescendants()" );
		TaxonomyVertex* p = R->getTaxVertex();	// need this for compiler
		p->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/false, /*upDirection=*/false>(actor);
	}

		/// apply actor::apply() to all synonyms of [complex] R
	template<class Actor>
	void getREquivalents ( const TRoleExpr* r, Actor& actor )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		TRole* R = getRole ( r, "Role expression expected in getREquivalents()" );
		TaxonomyVertex* p = R->getTaxVertex();	// need this for compiler
		actor.apply(*p);
	}

	// domain and range as a set of named concepts

		/// apply actor::apply() to all NC that are in the domain of [complex] R
	template<class Actor>
	void getRoleDomain ( const TRoleExpr* r, Actor& actor )
	{
		classifyKB();	// ensure KB is ready to answer the query
		setUpCache ( TreeDeleter(Exists(clone(r),Top())), csClassified );
		// gets an exact domain is named concept; otherwise, set of the most specific concepts
		cachedVertex->getRelativesInfo</*needCurrent=*/true, /*onlyDirect=*/true,
									   /*upDirection=*/true>(actor);
	}

		/// apply actor::apply() to all NC that are in the range of [complex] R
	template<class Actor>
	void getRoleRange ( const TRoleExpr* r, Actor& actor )
		{ getRoleDomain ( TreeDeleter(Inverse(clone(r))), actor ); }

	// instances

		/// apply actor::apply() to all direct instances of given [complex] C
	template<class Actor>
	void getDirectInstances ( const TConceptExpr* C, Actor& actor )
	{
		realiseKB();	// ensure KB is ready to answer the query
		setUpCache ( C, csClassified );

		// implement 1-level check by hand

		// if the root vertex contains individuals -- we are done
		if ( actor.apply(*cachedVertex) )
			return;

		// if not, just go 1 level down and apply the actor regardless of what's found
		// FIXME!! check again after bucket-method will be implemented
		for ( TaxonomyVertex::iterator p = cachedVertex->begin(/*upDirection=*/false),
				p_end = cachedVertex->end(/*upDirection=*/false); p < p_end; ++p )
			actor.apply(**p);
	}

		/// apply actor::apply() to all instances of given [complex] C
	template<class Actor>
	void getInstances ( const TConceptExpr* C, Actor& actor )
	{	// FIXME!! check for Racer's/IS approach
		realiseKB();	// ensure KB is ready to answer the query
		setUpCache ( C, csClassified );
		cachedVertex->getRelativesInfo</*needCurrent=*/true, /*onlyDirect=*/false,
									   /*upDirection=*/false>(actor);
	}

		/// apply actor::apply() to all direct concept parents of an individual I
	template<class Actor>
	void getDirectTypes ( const TIndividualExpr* I, Actor& actor )
	{
		realiseKB();	// ensure KB is ready to answer the query
		getParents ( I, actor );
	}
		/// apply actor::apply() to all concept ancestors of an individual I
	template<class Actor>
	void getTypes ( const TIndividualExpr* I, Actor& actor )
	{
		realiseKB();	// ensure KB is ready to answer the query
		getAncestors ( I, actor );
	}
		/// apply actor::apply() to all synonyms of an individual I
	template<class Actor>
	void getSameAs ( const TIndividualExpr* I, Actor& actor )
	{
		realiseKB();	// ensure KB is ready to answer the query
		getEquivalents ( I, actor );
	}
		/// @return true iff I and J refer to the same individual
	bool isSameIndividuals ( const TIndividualExpr* I, const TIndividualExpr* J )
	{
		realiseKB();
		TIndividual* i = getIndividual ( I, "Only known individuals are allowed in the isSameAs()" );
		TIndividual* j = getIndividual ( J, "Only known individuals are allowed in the isSameAs()" );
		return getTBox()->isSameIndividuals(i,j);
	}
		/// @return true iff individual I is instance of given [complex] C
	bool isInstance ( const TIndividualExpr* I, const TConceptExpr* C )
	{
		realiseKB();	// ensure KB is ready to answer the query
		getIndividual ( I, "individual name expected in the isInstance()" );
		return isSubsumedBy ( I, C );
	}
		/// @return in Rs all (DATA)-roles R s.t. (I,x):R; add inverses if NEEDI is true
	void getRelatedRoles ( const TIndividualExpr* I, NamesVector& Rs, bool data, bool needI );
		/// set RESULT into set of J's such that R(I,J)
	void getRoleFillers ( const TIndividualExpr* I, const TORoleExpr* R, IndividualSet& Result );
		/// set RESULT into set of (I,J)'s such that R(I,J)
	void getRelatedIndividuals ( const TORoleExpr* R, IndividualSet& Is, IndividualSet& Js )
	{
		realiseKB();	// ensure KB is ready to answer the query
		getTBox()->getRelatedIndividuals (
			getRole ( R, "Role expression expected in the getRelatedIndividuals()" ),
			Is, Js );
	}
		/// set RESULT into set of J's such that R(I,J)
	bool isRelated ( const TIndividualExpr* I, const TORoleExpr* R, const TIndividualExpr* J );
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
}

#endif
