/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2012 by Dmitry Tsarkov

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
#include "dlTBox.h"
#include "ifOptions.h"
#include "DLConceptTaxonomy.h"	// for getRelatives()
#include "tExpressionTranslator.h"
#include "tOntology.h"
#include "tDag2Interface.h"
#include "tOntologyAtom.h"	// types for AD
#include "Modularity.h"		// ModuleType

class AtomicDecomposer;

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
	typedef const TDLExpression TExpr;
		/// concept expression
	typedef const TDLConceptExpression TConceptExpr;
		/// individual expression
	typedef const TDLIndividualExpression TIndividualExpr;
		/// role expression
	typedef const TDLRoleExpression TRoleExpr;
		/// object role complex expression (including role chains and projections)
	typedef const TDLObjectRoleComplexExpression TORoleComplexExpr;
		/// object role expression
	typedef const TDLObjectRoleExpression TORoleExpr;
		/// data role expression
	typedef const TDLDataRoleExpression TDRoleExpr;
		/// data expression
	typedef const TDLDataExpression TDataExpr;
		/// data type expression
	typedef TDLDataTypeExpression TDataTypeExpr;
		/// data value expression
	typedef const TDLDataValue TDataValueExpr;
		/// data facet expression
	typedef const TDLFacetExpression TFacetExpr;

	// name sets

		/// set of arbitrary named expressions
	typedef std::vector<const TNamedEntry*> NamesVector;
		// IndividualSet is just set of named individual expressions
	typedef NamesVector IndividualSet;

		/// typedef for intermediate instance related type
	typedef TRelatedMap::CIVec CIVec;

	// types for knowledge exploration

		/// type for the node in the completion graph
	typedef const DlCompletionTree TCGNode;
		/// type for the node vector
	typedef std::vector<TCGNode*> TCGNodeVec;
		/// type for a set of role expressions (used in KE to return stuff)
	typedef std::set<TRoleExpr*> TCGRoleSet;
		/// type for a vector of data/concept expressions (used in KE to return stuff)
	typedef std::vector<TExpr*> TCGItemVec;

private:
		/// options for the kernel and all related substructures
	ifOptionSet KernelOptions;

private:
	static const char* Version;
	static const char* SupportedDL;
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
		/// set of axioms
	TOntology Ontology;
		/// expression translator to work with queries
	TExpressionTranslator* pET;
		/// trace vector for the last operation (set from the TBox trace-sets)
	AxiomVec TraceVec;
		/// dag-2-interface translator used in knowledge exploration
	TDag2Interface* D2I;
		/// atomic decomposer
	AtomicDecomposer* AD;

	// Top/Bottom role names: if set, they will appear in all hierarchy-related output

		/// top object role name
	std::string TopORoleName;
		/// bottom object role name
	std::string BotORoleName;
		/// top data role name
	std::string TopDRoleName;
		/// bottom data role name
	std::string BotDRoleName;

	// values to propagate to the new KB in case of clearance

		/// progress monitor (if any)
	TProgressMonitor* pMonitor;
		/// timeout value
	unsigned long OpTimeout;
		/// tell reasoner to use verbose output
	bool verboseOutput;
		/// allow reasoner to use undefined names in queries
	bool useUndefinedNames;

	// reasoning cache

		/// cache level
	enum cacheStatus cacheLevel;
		/// cached query input description
	TConceptExpr* cachedQuery;
		/// cached query concept description
	DLTree* cachedQueryTree;
		/// cached concept (either defConcept or existing one)
	TConcept* cachedConcept;
		/// cached query result (taxonomy position)
	TaxonomyVertex* cachedVertex;

	// internal flags

		/// set if TBox throws an exception during preprocessing/classification
	bool reasoningFailed;
		/// flag to gather trace information for the next reasoner's call
	bool NeedTracing;
		/// whether axiom splitting should be used
	bool useAxiomSplitting;
		/// whether EL polynomial reasoner should be used
	bool useELReasoner;

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
		// if the ontology is changed, it needs to be reclassified
		if ( Ontology.isChanged() )
			return kbLoading;
		return pTBox->getStatus();
	}
		/// process KB wrt STATUS
	void processKB ( KBStatus status );

		/// get DLTree corresponding to an expression EXPR
	DLTree* e ( const TExpr* expr )
	{
		fpp_assert ( pET != NULL );
		expr->accept(*pET);
		return *pET;
	}
		/// get fresh filled depending of a type of R
	DLTree* getFreshFiller ( const DLTree* R )
	{
		if ( resolveRole(R)->isDataRole() )
			return getTBox()->getDataTypeCenter().getFreshDataType();
		else
			return getTBox()->getFreshConcept();
	}

	//-----------------------------------------------------------------------------
	//--		internal query cache manipulation
	//-----------------------------------------------------------------------------

		/// clear query cache
	void clearQueryCache ( void ) { cachedQuery = NULL; deleteTree(cachedQueryTree); cachedQueryTree = NULL; }
		/// set query cache value to QUERY
	void setQueryCache ( TConceptExpr* query ) { clearQueryCache(); cachedQuery = query; }
		/// set query cache value to QUERY
	void setQueryCache ( DLTree* query ) { clearQueryCache(); cachedQueryTree = query; }
		/// check whether query cache is the same as QUERY
	bool checkQueryCache ( TConceptExpr* query ) const { return cachedQuery == query; }
		/// check whether query cache is the same as QUERY
	bool checkQueryCache ( DLTree* query ) const { return equalTrees ( cachedQueryTree, query ); }
		/// classify query; cache is ready at the point. NAMED means whether concept is just a name
	void classifyQuery ( bool named );
		/// set up cache for query, performing additional (re-)classification if necessary
	void setUpCache ( DLTree* query, cacheStatus level );
		/// set up cache for query, performing additional (re-)classification if necessary
	void setUpCache ( TConceptExpr* query, cacheStatus level );
		/// clear cache and flags
	void initCacheAndFlags ( void )
	{
		cacheLevel = csEmpty;
		clearQueryCache();
		cachedConcept = NULL;
		cachedVertex = NULL;
		reasoningFailed = false;
		NeedTracing = false;
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

		/// get role expression based on the R
	TDLRoleExpression* Role ( const TRole* R )
	{
		if ( unlikely(R->isDataRole()) )
			return getExpressionManager()->DataRole(R->getName());
		else
			return getExpressionManager()->ObjectRole(R->getName());
	}

	//-----------------------------------------------------------------------------
	//--		internal reasoning methods
	//-----------------------------------------------------------------------------

		/// @return true iff C is satisfiable
	bool checkSatTree ( DLTree* C )
	{
		setUpCache ( C, csSat );
		return getTBox()->isSatisfiable(cachedConcept);
	}
		/// @return true iff C is satisfiable
	bool checkSat ( const TConceptExpr* C )
	{
		setUpCache ( C, csSat );
		return getTBox()->isSatisfiable(cachedConcept);
	}
		/// @return true iff C [= D holds
	bool checkSub ( TConcept* C, TConcept* D );
		/// helper; @return true iff C is either named concept of Top/Bot
	static bool isNameOrConst ( const TConceptExpr* C )
	{
		return	likely ( dynamic_cast<const TDLConceptName*>(C) != NULL ) ||
				unlikely ( dynamic_cast<const TDLConceptTop*>(C) != NULL ) ||
				unlikely ( dynamic_cast<const TDLConceptBottom*>(C) != NULL );
	}

	// helper methods to query properties of roles

		/// @return true if R is functional wrt ontology
	bool checkFunctionality ( DLTree* R )
	{
		// R is transitive iff \ER.C and \ER.\not C is unsatisfiable
		DLTree* tmp = createSNFExists ( clone(R), createSNFNot(getFreshFiller(R)) );
		tmp = createSNFAnd ( tmp, createSNFExists ( R, getFreshFiller(R) ) );
		return !checkSatTree(tmp);
	}
		/// @return true if R is functional; set the value for R if necessary
	bool getFunctionality ( TRole* R )
	{
		if ( !R->isFunctionalityKnown() )	// calculate functionality
			R->setFunctional ( checkFunctionality ( new DLTree ( TLexeme ( R->isDataRole() ? DNAME : RNAME, R ) ) ) );
		return R->isFunctional();
	}
		/// @return true if R is transitive wrt ontology
	bool checkTransitivity ( DLTree* R )
	{
		// R is transitive iff \ER.\ER.C and \AR.\not C is unsatisfiable
		DLTree* tmp = createSNFExists ( clone(R), createSNFNot(getTBox()->getFreshConcept()) );
		tmp = createSNFExists ( clone(R), tmp );
		tmp = createSNFAnd ( tmp, createSNFForall ( R, getTBox()->getFreshConcept() ) );
		return !checkSatTree(tmp);
	}
		/// @return true if R is symmetric wrt ontology
	bool checkSymmetry ( DLTree* R )
	{
		// R is symmetric iff C and \ER.\AR.(not C) is unsatisfiable
		DLTree* tmp = createSNFForall ( clone(R), createSNFNot(getTBox()->getFreshConcept()) );
		tmp = createSNFAnd ( getTBox()->getFreshConcept(), createSNFExists ( R, tmp ) );
		return !checkSatTree(tmp);
	}
		/// @return true if R is reflexive wrt ontology
	bool checkReflexivity ( DLTree* R )
	{
		// R is reflexive iff C and \AR.(not C) is unsatisfiable
		DLTree* tmp = createSNFForall ( R, createSNFNot(getTBox()->getFreshConcept()) );
		tmp = createSNFAnd ( getTBox()->getFreshConcept(), tmp );
		return !checkSatTree(tmp);
	}
		/// @return true if R [= S wrt ontology
	bool checkRoleSubsumption ( DLTree* R, DLTree* S )
	{
		if ( resolveRole(R)->isDataRole() != resolveRole(S)->isDataRole() )
			return false;
		// R [= S iff \ER.C and \AS.(not C) is unsatisfiable
		DLTree* tmp = createSNFForall ( S, createSNFNot(getFreshFiller(S)) );
		tmp = createSNFAnd ( createSNFExists ( R, getFreshFiller(R) ), tmp );
		return !checkSatTree(tmp);
	}
		/// @return true iff the chain contained in the arg-list is a sub-property of R
	bool checkSubChain ( TRole* R )
	{
		// retrieve a role chain
		typedef const std::vector<const TDLExpression*> TExprVec;
		TExprVec Chain = getExpressionManager()->getArgList();

		// R1 o ... o Rn [= R iff \ER1.\ER2....\ERn.(notC) and AR.C is unsatisfiable
		DLTree* tmp = createSNFNot(getTBox()->getFreshConcept());
		for ( TExprVec::const_reverse_iterator p = Chain.rbegin(), p_end = Chain.rend(); p != p_end; ++p )
		{
			TORoleExpr* Ri = dynamic_cast<TORoleExpr*>(*p);
			if ( Ri == NULL )
				throw EFaCTPlusPlus("Role expression expected in the role chain construct");
			tmp = createSNFExists ( e(Ri), tmp );
		}
		tmp = createSNFAnd ( tmp, createSNFForall ( new DLTree(TLexeme(RNAME,R)), getTBox()->getFreshConcept() ) );
		return !checkSatTree(tmp);
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
		/// clear TBox and related structures; keep ontology in place
	void clearTBox ( void )
	{
		delete pTBox;
		pTBox = NULL;
		delete pET;
		pET = NULL;
		delete D2I;
		D2I = NULL;
	}

		/// get RW access to Object RoleMaster from TBox
	RoleMaster* getORM ( void ) { return getTBox()->getORM(); }
		/// get RO access to Object RoleMaster from TBox
	const RoleMaster* getORM ( void ) const { return getTBox()->getORM(); }
		/// get RW access to Data RoleMaster from TBox
	RoleMaster* getDRM ( void ) { return getTBox()->getDRM(); }
		/// get RO access to Data RoleMaster from TBox
	const RoleMaster* getDRM ( void ) const { return getTBox()->getDRM(); }

		/// get access to the concept hierarchy
	Taxonomy* getCTaxonomy ( void )
	{
		if ( !isKBClassified() )
			throw EFaCTPlusPlus("No access to concept taxonomy: ontology not classified");
		return getTBox()->getTaxonomy();
	}
		/// get access to the object role hierarchy
	Taxonomy* getORTaxonomy ( void )
	{
		if ( !isKBPreprocessed() )
			throw EFaCTPlusPlus("No access to the object role taxonomy: ontology not preprocessed");
		return getORM()->getTaxonomy();
	}
		/// get access to the data role hierarchy
	Taxonomy* getDRTaxonomy ( void )
	{
		if ( !isKBPreprocessed() )
			throw EFaCTPlusPlus("No access to the data role taxonomy: ontology not preprocessed");
		return getDRM()->getTaxonomy();
	}

	// transformation methods

		/// get individual by the TIndividualExpr
	TIndividual* getIndividual ( const TIndividualExpr* i, const char* reason )
	{
		try { return static_cast<TIndividual*>(getTBox()->getCI(e(i))); }
		catch(...) { throw EFaCTPlusPlus(reason); }
	}
		/// get role by the TRoleExpr
	TRole* getRole ( const TRoleExpr* r, const char* reason )
	{
		try { return resolveRole(TreeDeleter(e(r))); }
		catch(...) { throw EFaCTPlusPlus(reason); }
	}

		/// get taxonomy of the property wrt it's name
	Taxonomy* getTaxonomy ( TRole* R )
	{
		return R->isDataRole() ? getDRTaxonomy() : getORTaxonomy();
	}
		/// get taxonomy vertex of the property wrt it's name
	TaxonomyVertex* getTaxVertex ( TRole* R )
	{
		return R->getTaxVertex();
	}

		/// try to perform the incremental reasoning on the changed ontology
	bool tryIncremental ( void );
		/// force the re-classification of the changed ontology
	void forceReload ( void );


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

	//----------------------------------------------------------------------------------
	// knowledge exploration queries
	//----------------------------------------------------------------------------------

		/// add the role R and all its supers to a set RESULT
	void addRoleWithSupers ( const TRole* R, TCGRoleSet& Result );

public:	// general staff
		/// default c'tor
	ReasoningKernel ( void );
		/// d'tor
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
	void setProgressMonitor ( TProgressMonitor* pMon )
	{
		delete pMonitor;
		pMonitor = pMon;
		if ( pTBox != NULL )
			pTBox->setProgressMonitor(pMon);
	}
		/// set verbose output (ie, concept and role taxonomies) wrt given VALUE
	void setVerboseOutput ( bool value )
	{
		verboseOutput = value;
		if ( pTBox != NULL )
			pTBox->setVerboseOutput(value);
	}
		/// (dis-)allow reasoner to use the undefined names in queries
	void setUseUndefinedNames ( bool value )
	{
		useUndefinedNames = value;
		if ( pTBox != NULL )
			pTBox->setUseUndefinedNames(value);
	}

		/// set top/bottom role names to use them in the related output
	void setTopBottomRoleNames ( const char* topORoleName, const char* botORoleName, const char* topDRoleName, const char* botDRoleName )
	{
		TopORoleName = topORoleName;
		BotORoleName = botORoleName;
		TopDRoleName = topDRoleName;
		BotDRoleName = botDRoleName;
		// make sure expression manager knows the top/bot names
		Ontology.getExpressionManager()->setTopBottomRoles ( topORoleName, botORoleName,topDRoleName, botDRoleName );
	}

		/// dump query processing TIME, reasoning statistics and a (preprocessed) TBox
	void writeReasoningResult ( std::ostream& o, float time )
	{
		getTBox()->clearQueryConcept();	// get rid of the query leftovers
		getTBox()->writeReasoningResult ( o, time );
	}

		/// set timeout value to VALUE
	void setOperationTimeout ( unsigned long value )
	{
		OpTimeout = value;
		if ( pTBox != NULL )
			pTBox->setTestTimeout(value);
	}
		/// choose whether axiom splitting should be used
	void setAxiomSplitting ( bool value ) { useAxiomSplitting = value; }
		/// set the signature of the expression translator
	void setSignature ( const TSignature* sig ) { if ( pET != NULL ) pET->setSignature(sig); }

	//----------------------------------------------
	//-- Tracing support
	//----------------------------------------------

		/// tells reasoner that the next reasoning operation shall gather the trace
	void needTracing ( void ) { NeedTracing = true; }
		/// @return the trace-set of the last reasoning operation
	const AxiomVec& getTrace ( void )
	{
		TraceVec.clear();
		return TraceVec;
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

		/// get access to an expression manager
	TExpressionManager* getExpressionManager ( void ) { return Ontology.getExpressionManager(); }
		/// get RW access to the ontology
	TOntology& getOntology ( void ) { return Ontology; }
		/// get RO access to the ontology
	const TOntology& getOntology ( void ) const { return Ontology; }

public:
	//******************************************
	//* KB Management
	//******************************************

		/// create new KB
	bool newKB ( void )
	{
		if ( pTBox != NULL )
			return true;

		pTBox = new TBox ( getOptions(), TopORoleName, BotORoleName, TopDRoleName, BotDRoleName );
		pTBox->setTestTimeout(OpTimeout);
		pTBox->setProgressMonitor(pMonitor);
		pTBox->setVerboseOutput(verboseOutput);
		pTBox->setUseUndefinedNames(useUndefinedNames);
		pET = new TExpressionTranslator(*pTBox);
		initCacheAndFlags();
		return false;
	}
		/// delete existed KB
	bool releaseKB ( void )
	{
		clearTBox();
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

	//----------------------------------------------------
	//	TELLS interface
	//----------------------------------------------------

	// Declaration axioms

		/// axiom declare(x)
	TDLAxiom* declare ( TExpr* C ) { return Ontology.add(new TDLAxiomDeclaration(C)); }

	// Concept axioms

		/// axiom C [= D
	TDLAxiom* impliesConcepts ( TConceptExpr* C, TConceptExpr* D )
		{ return Ontology.add ( new TDLAxiomConceptInclusion ( C, D ) ); }
		/// axiom C1 = ... = Cn
	TDLAxiom* equalConcepts ( void )
		{ return Ontology.add ( new TDLAxiomEquivalentConcepts(getExpressionManager()->getArgList()) ); }
		/// axiom C1 != ... != Cn
	TDLAxiom* disjointConcepts ( void )
		{ return Ontology.add ( new TDLAxiomDisjointConcepts(getExpressionManager()->getArgList()) ); }
		/// axiom C = C1 or ... or Cn; C1 != ... != Cn
	TDLAxiom* disjointUnion ( TConceptExpr* C )
		{ return Ontology.add ( new TDLAxiomDisjointUnion ( C, getExpressionManager()->getArgList() ) ); }


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
		{ return Ontology.add ( new TDLAxiomEquivalentORoles(getExpressionManager()->getArgList()) ); }
		/// axiom R1 = R2 = ...
	TDLAxiom* equalDRoles ( void )
		{ return Ontology.add ( new TDLAxiomEquivalentDRoles(getExpressionManager()->getArgList()) ); }
		/// axiom R1 != R2 != ...
	TDLAxiom* disjointORoles ( void )
		{ return Ontology.add ( new TDLAxiomDisjointORoles(getExpressionManager()->getArgList()) ); }
		/// axiom R1 != R2 != ...
	TDLAxiom* disjointDRoles ( void )
		{ return Ontology.add ( new TDLAxiomDisjointDRoles(getExpressionManager()->getArgList()) ); }

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
		/// Asymmetric (R): disjoint(R,R^-)
	TDLAxiom* setAsymmetric ( TORoleExpr* R )
		{ return Ontology.add ( new TDLAxiomRoleAsymmetric(R) ); }
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
		{ return Ontology.add ( new TDLAxiomSameIndividuals(getExpressionManager()->getArgList()) ); }
		/// different individuals
	TDLAxiom* processDifferent ( void )
		{ return Ontology.add ( new TDLAxiomDifferentIndividuals(getExpressionManager()->getArgList()) ); }
		/// let all concept expressions in the ArgQueue to be fairness constraints
	TDLAxiom* setFairnessConstraint ( void )
		{ return Ontology.add ( new TDLAxiomFairnessConstraint(getExpressionManager()->getArgList()) ); }

		/// retract an axiom
	void retract ( TDLAxiom* axiom ) { Ontology.retract(axiom); }

	//******************************************
	//* ASK part
	//******************************************

	/*
	 * Before execution of any query the Kernel make sure that the KB is in an appropriate
	 * state: Preprocessed, Classified or Realised. If the ontology was changed between asks,
	 * incremental classification is performed and the corrected result is returned.
	 */

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

		/// @return true iff object role is functional
	bool isFunctional ( const TORoleExpr* R )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		if ( getExpressionManager()->isUniversalRole(R) )
			return false;	// universal role is not functional
		if ( getExpressionManager()->isEmptyRole(R) )
			return true;	// empty role is functional

		return getFunctionality ( getRole ( R, "Role expression expected in isFunctional()" ) );
	}
		/// @return true iff data role is functional
	bool isFunctional ( const TDRoleExpr* R )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		if ( getExpressionManager()->isUniversalRole(R) )
			return false;	// universal role is not functional
		if ( getExpressionManager()->isEmptyRole(R) )
			return true;	// empty role is functional

		return getFunctionality ( getRole ( R, "Role expression expected in isFunctional()" ) );
	}
		/// @return true iff role is inverse-functional
	bool isInverseFunctional ( const TORoleExpr* R )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		if ( getExpressionManager()->isUniversalRole(R) )
			return false;	// universal role is not functional
		if ( getExpressionManager()->isEmptyRole(R) )
			return true;	// empty role is functional

		return getFunctionality ( getRole ( R, "Role expression expected in isInverseFunctional()" )->inverse() );
	}
		/// @return true iff role is transitive
	bool isTransitive ( const TORoleExpr* R )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		if ( getExpressionManager()->isUniversalRole(R) )
			return true;	// universal role is transitive
		if ( getExpressionManager()->isEmptyRole(R) )
			return true;	// empty role is transitive

		TRole* r = getRole ( R, "Role expression expected in isTransitive()" );
		if ( !r->isTransitivityKnown() )	// calculate transitivity
			r->setTransitive(checkTransitivity(e(R)));
		return r->isTransitive();
	}
		/// @return true iff role is symmetric
	bool isSymmetric ( const TORoleExpr* R )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		if ( getExpressionManager()->isUniversalRole(R) )
			return true;	// universal role is symmetric
		if ( getExpressionManager()->isEmptyRole(R) )
			return true;	// empty role is symmetric

		TRole* r = getRole ( R, "Role expression expected in isSymmetric()" );
		if ( !r->isSymmetryKnown() )	// calculate symmetry
			r->setSymmetric(checkSymmetry(e(R)));
		return r->isSymmetric();
	}
		/// @return true iff role is asymmetric
	bool isAsymmetric ( const TORoleExpr* R )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		if ( getExpressionManager()->isUniversalRole(R) )
			return false;	// universal role is not asymmetric
		if ( getExpressionManager()->isEmptyRole(R) )
			return true;	// empty role is asymmetric

		TRole* r = getRole ( R, "Role expression expected in isAsymmetric()" );
		if ( !r->isAsymmetryKnown() )	// calculate asymmetry
			r->setAsymmetric(getTBox()->isDisjointRoles(r,r->inverse()));
		return r->isAsymmetric();
	}
		/// @return true iff role is reflexive
	bool isReflexive ( const TORoleExpr* R )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		if ( getExpressionManager()->isUniversalRole(R) )
			return true;	// universal role is reflexive
		if ( getExpressionManager()->isEmptyRole(R) )
			return false;	// empty role is not reflexive

		TRole* r = getRole ( R, "Role expression expected in isReflexive()" );
		if ( !r->isReflexivityKnown() )	// calculate reflexivity
			r->setReflexive(checkReflexivity(e(R)));
		return r->isReflexive();
	}
		/// @return true iff role is irreflexive
	bool isIrreflexive ( const TORoleExpr* R )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		if ( getExpressionManager()->isUniversalRole(R) )
			return false;	// universal role is not irreflexive
		if ( getExpressionManager()->isEmptyRole(R) )
			return true;	// empty role is irreflexive

		TRole* r = getRole ( R, "Role expression expected in isIrreflexive()" );
		if ( !r->isIrreflexivityKnown() )	// calculate irreflexivity
			r->setIrreflexive(getTBox()->isIrreflexive(r));
		return r->isIrreflexive();
	}
		/// @return true if R is a sub-role of S
	bool isSubRoles ( const TORoleExpr* R, const TORoleExpr* S )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		if ( getExpressionManager()->isEmptyRole(R) || getExpressionManager()->isUniversalRole(S) )
			return true;	// \bot [= X [= \top
		if ( getExpressionManager()->isUniversalRole(R) && getExpressionManager()->isEmptyRole(S) )
			return false;	// as \top [= \bot leads to inconsistent ontology

		// told case first
		if ( *getRole ( R, "Role expression expected in isSubRoles()" ) <=
			 *getRole ( S, "Role expression expected in isSubRoles()" ) )
			return true;
		// check the general case
		// FIXME!! cache it later
		DLTree* r = e(R), *s = e(S);
		return checkRoleSubsumption ( r, s );
	}
		/// @return true if R is a sub-role of S
	bool isSubRoles ( const TDRoleExpr* R, const TDRoleExpr* S )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		if ( getExpressionManager()->isEmptyRole(R) || getExpressionManager()->isUniversalRole(S) )
			return true;	// \bot [= X [= \top
		if ( getExpressionManager()->isUniversalRole(R) && getExpressionManager()->isEmptyRole(S) )
			return false;	// as \top [= \bot leads to inconsistent ontology

		// told case first
		if ( *getRole ( R, "Role expression expected in isSubRoles()" ) <=
			 *getRole ( S, "Role expression expected in isSubRoles()" ) )
			return true;
		// check the general case
		// FIXME!! cache it later
		DLTree* r = e(R), *s = e(S);
		return checkRoleSubsumption ( r, s );
	}
		/// @return true iff two roles are disjoint
	bool isDisjointRoles ( const TORoleExpr* R, const TORoleExpr* S )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		if ( getExpressionManager()->isUniversalRole(R) || getExpressionManager()->isUniversalRole(S) )
			return false;	// universal role is not disjoint with anything
		if ( getExpressionManager()->isEmptyRole(R) || getExpressionManager()->isEmptyRole(S) )
			return true;	// empty role is disjoint with everything
		return getTBox()->isDisjointRoles (
			getRole ( R, "Role expression expected in isDisjointRoles()" ),
			getRole ( S, "Role expression expected in isDisjointRoles()" ) );
	}
		/// @return true iff two roles are disjoint
	bool isDisjointRoles ( const TDRoleExpr* R, const TDRoleExpr* S )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		if ( getExpressionManager()->isUniversalRole(R) || getExpressionManager()->isUniversalRole(S) )
			return false;	// universal role is not disjoint with anything
		if ( getExpressionManager()->isEmptyRole(R) || getExpressionManager()->isEmptyRole(S) )
			return true;	// empty role is disjoint with everything
		return getTBox()->isDisjointRoles (
			getRole ( R, "Role expression expected in isDisjointRoles()" ),
			getRole ( S, "Role expression expected in isDisjointRoles()" ) );
	}
		/// @return true iff all the roles in a arg-list are pairwise disjoint
	bool isDisjointRoles ( void );
		/// @return true if R is a super-role of a chain holding in the args
	bool isSubChain ( const TORoleExpr* R )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		if ( getExpressionManager()->isUniversalRole(R) )
			return true;	// universal role is a super of any chain
		if ( getExpressionManager()->isEmptyRole(R) )	// FIXME!! true in case empty role is in chain
			return false;	// empty role is not a super of any chain
		return checkSubChain ( getRole ( R, "Role expression expected in isSubChain()" ) );
	}

	// single satisfiability

		/// @return true iff C is satisfiable
	bool isSatisfiable ( const TConceptExpr* C )
	{
		preprocessKB();
		try { return checkSat(C); }
		catch ( const EFPPCantRegName& crn )
		{
			if ( dynamic_cast<const TDLConceptName*>(C) != NULL )	// this is an unknown concept
				return true;
			// complex expression, involving unknown names
			throw crn;
		}
	}
		/// @return true iff C [= D holds
	bool isSubsumedBy ( const TConceptExpr* C, const TConceptExpr* D )
	{
		preprocessKB();
		if ( isNameOrConst(D) && likely(isNameOrConst(C)) )
			return checkSub ( getTBox()->getCI(TreeDeleter(e(C))), getTBox()->getCI(TreeDeleter(e(D))) );

		return !checkSat ( getExpressionManager()->And ( C, getExpressionManager()->Not(D) ) );
	}
		/// @return true iff C is disjoint with D; that is, (C and D) is unsatisfiable
	bool isDisjoint ( const TConceptExpr* C, const TConceptExpr* D ) { return !isSatisfiable(getExpressionManager()->And(C,D)); }
		/// @return true iff C is equivalent to D
	bool isEquivalent ( const TConceptExpr* C, const TConceptExpr* D )
	{
		if ( C == D )	// easy case
			return true;
		preprocessKB();
		if ( isKBClassified() )
		{	// try to detect C=D wrt named concepts
			if ( isNameOrConst(D) && likely(isNameOrConst(C)) )
			{
				const TaxonomyVertex* cV = getTBox()->getCI(TreeDeleter(e(C)))->getTaxVertex();
				const TaxonomyVertex* dV = getTBox()->getCI(TreeDeleter(e(D)))->getTaxVertex();
				if ( unlikely(cV == NULL) && unlikely(dV == NULL) )
					return false;	// 2 different fresh names
				return cV == dV;
			}
		}
		// not classified or not named constants
		return isSubsumedBy ( C, D ) && isSubsumedBy ( D, C );
	}
	// concept hierarchy

		/// apply actor::apply() to all DIRECT super-concepts of [complex] C
	template<class Actor>
	void getSupConcepts ( const TConceptExpr* C, bool direct, Actor& actor )
	{
		classifyKB();	// ensure KB is ready to answer the query
		setUpCache ( C, csClassified );
		Taxonomy* tax = getCTaxonomy();
		if ( direct )
			tax->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/true, /*upDirection=*/true> ( cachedVertex, actor );
		else
			tax->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/false, /*upDirection=*/true> ( cachedVertex, actor );
	}
		/// apply actor::apply() to all DIRECT sub-concepts of [complex] C
	template<class Actor>
	void getSubConcepts ( const TConceptExpr* C, bool direct, Actor& actor )
	{
		classifyKB();	// ensure KB is ready to answer the query
		setUpCache ( C, csClassified );
		Taxonomy* tax = getCTaxonomy();
		if ( direct )
			tax->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/true, /*upDirection=*/false> ( cachedVertex, actor );
		else
			tax->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/false, /*upDirection=*/false> ( cachedVertex, actor );
	}
		/// apply actor::apply() to all synonyms of [complex] C
	template<class Actor>
	void getEquivalentConcepts ( const TConceptExpr* C, Actor& actor )
	{
		classifyKB();	// ensure KB is ready to answer the query
		setUpCache ( C, csClassified );
		actor.apply(*cachedVertex);
	}
		/// apply actor::apply() to all named concepts disjoint with [complex] C
	template<class Actor>
	void getDisjointConcepts ( const TConceptExpr* C, Actor& actor )
	{
		classifyKB();	// ensure KB is ready to answer the query
		setUpCache ( getExpressionManager()->Not(C), csClassified );
		Taxonomy* tax = getCTaxonomy();
		// we are looking for all sub-concepts of (not C) (including synonyms to it)
		tax->getRelativesInfo</*needCurrent=*/true, /*onlyDirect=*/false, /*upDirection=*/false> ( cachedVertex, actor );
	}

	// role hierarchy

		/// apply actor::apply() to all DIRECT super-roles of [complex] R
	template<class Actor>
	void getSupRoles ( const TRoleExpr* r, bool direct, Actor& actor )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		TRole* R = getRole ( r, "Role expression expected in getSupRoles()" );
		Taxonomy* tax = getTaxonomy(R);
		if ( direct )
			tax->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/true, /*upDirection=*/true> ( getTaxVertex(R), actor );
		else
			tax->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/false, /*upDirection=*/true> ( getTaxVertex(R), actor );
	}
		/// apply actor::apply() to all DIRECT sub-roles of [complex] R
	template<class Actor>
	void getSubRoles ( const TRoleExpr* r, bool direct, Actor& actor )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		TRole* R = getRole ( r, "Role expression expected in getSubRoles()" );
		Taxonomy* tax = getTaxonomy(R);
		if ( direct )
			tax->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/true, /*upDirection=*/false> ( getTaxVertex(R), actor );
		else
			tax->getRelativesInfo</*needCurrent=*/false, /*onlyDirect=*/false, /*upDirection=*/false> ( getTaxVertex(R), actor );
	}
		/// apply actor::apply() to all synonyms of [complex] R
	template<class Actor>
	void getEquivalentRoles ( const TRoleExpr* r, Actor& actor )
	{
		preprocessKB();	// ensure KB is ready to answer the query
		TRole* R = getRole ( r, "Role expression expected in getEquivalentRoles()" );
		actor.apply(*getTaxVertex(R));
	}

	// domain and range as a set of named concepts

		/// apply actor::apply() to all DIRECT NC that are in the domain of [complex] object role R
	template<class Actor>
	void getORoleDomain ( const TORoleExpr* r, bool direct, Actor& actor )
	{
		classifyKB();	// ensure KB is ready to answer the query
		setUpCache ( getExpressionManager()->Exists ( r, getExpressionManager()->Top() ), csClassified );
		Taxonomy* tax = getCTaxonomy();
		if ( direct )	// gets an exact domain is named concept; otherwise, set of the most specific concepts
			tax->getRelativesInfo</*needCurrent=*/true, /*onlyDirect=*/true, /*upDirection=*/true> ( cachedVertex, actor );
		else			// gets all named classes that are in the domain of a role
			tax->getRelativesInfo</*needCurrent=*/true, /*onlyDirect=*/false, /*upDirection=*/true> ( cachedVertex, actor );
	}
		/// apply actor::apply() to all DIRECT NC that are in the domain of data role R
	template<class Actor>
	void getDRoleDomain ( const TDRoleExpr* r, bool direct, Actor& actor )
	{
		classifyKB();	// ensure KB is ready to answer the query
		setUpCache ( getExpressionManager()->Exists ( r, getExpressionManager()->DataTop() ), csClassified );
		Taxonomy* tax = getCTaxonomy();
		if ( direct )	// gets an exact domain is named concept; otherwise, set of the most specific concepts
			tax->getRelativesInfo</*needCurrent=*/true, /*onlyDirect=*/true, /*upDirection=*/true> ( cachedVertex, actor );
		else			// gets all named classes that are in the domain of a role
			tax->getRelativesInfo</*needCurrent=*/true, /*onlyDirect=*/false, /*upDirection=*/true> ( cachedVertex, actor );
	}
		/// apply actor::apply() to all DIRECT NC that are in the range of [complex] R
	template<class Actor>
	void getRoleRange ( const TORoleExpr* r, bool direct, Actor& actor )
		{ getORoleDomain ( getExpressionManager()->Inverse(r), direct, actor ); }

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
		Taxonomy* tax = getCTaxonomy();
		tax->getRelativesInfo</*needCurrent=*/true, /*onlyDirect=*/false, /*upDirection=*/false> ( cachedVertex, actor );
	}

		/// apply actor::apply() to all DIRECT concepts that are types of an individual I
	template<class Actor>
	void getTypes ( const TIndividualExpr* I, bool direct, Actor& actor )
	{
		realiseKB();	// ensure KB is ready to answer the query
		setUpCache ( getExpressionManager()->OneOf(I), csClassified );
		Taxonomy* tax = getCTaxonomy();
		if ( direct )
			tax->getRelativesInfo</*needCurrent=*/true, /*onlyDirect=*/true, /*upDirection=*/true> ( cachedVertex, actor );
		else
			tax->getRelativesInfo</*needCurrent=*/true, /*onlyDirect=*/false, /*upDirection=*/true> ( cachedVertex, actor );
	}
		/// apply actor::apply() to all synonyms of an individual I
	template<class Actor>
	void getSameAs ( const TIndividualExpr* I, Actor& actor )
	{
		realiseKB();	// ensure KB is ready to answer the query
		getEquivalentConcepts ( getExpressionManager()->OneOf(I), actor );
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
		// FIXME!! this way a new concept is created; could be done more optimal
		return isSubsumedBy ( getExpressionManager()->OneOf(I), C );
	}
		/// @return in Rs all (DATA)-roles R s.t. (I,x):R; add inverses if NEEDI is true
	void getRelatedRoles ( const TIndividualExpr* I, NamesVector& Rs, bool data, bool needI );
		/// set RESULT into set of J's such that R(I,J)
	void getRoleFillers ( const TIndividualExpr* I, const TORoleExpr* R, IndividualSet& Result );
		/// set RESULT into set of J's such that R(I,J)
	bool isRelated ( const TIndividualExpr* I, const TORoleExpr* R, const TIndividualExpr* J );

	//----------------------------------------------------------------------------------
	// knowledge exploration queries
	//----------------------------------------------------------------------------------

		/// build a completion tree for a concept expression C (no caching as it breaks the idea of KE). @return the root node
	const TCGNode* buildCompletionTree ( const TConceptExpr* C )
	{
		preprocessKB();
		setUpCache ( C, csSat );
		return getTBox()->buildCompletionTree(cachedConcept);
	}
		/// build the set of data neighbours of a NODE, put the set of data roles into the RESULT variable
	void getDataRoles ( const TCGNode* node, TCGRoleSet& Result, bool onlyDet );
		/// build the set of object neighbours of a NODE, put the set of object roles and inverses into the RESULT variable
	void getObjectRoles ( const TCGNode* node, TCGRoleSet& Result, bool onlyDet, bool needIncoming );
		/// build the set of neighbours of a NODE via role ROLE; put the resulting list into RESULT
	void getNeighbours ( const TCGNode* node, TRoleExpr* role, TCGNodeVec& Result );
		/// put into RESULT all the expressions from the NODE label; if ONLYDET is true, return only deterministic elements
	void getLabel ( const TCGNode* node, TCGItemVec& Result, bool onlyDet );

	//----------------------------------------------------------------------------------
	// atomic decomposition queries
	//----------------------------------------------------------------------------------

		/// create new atomic decomposition of the loaded ontology using TYPE. @return size of the AD
	unsigned int getAtomicDecompositionSize ( ModuleType type );
		/// get a set of axioms that corresponds to the atom with the id INDEX
	const TOntologyAtom::AxiomSet& getAtomAxioms ( unsigned int index ) const;
		/// get a set of atoms on which atom with index INDEX depends
	const TOntologyAtom::AtomSet& getAtomDependents ( unsigned int index ) const;
}; // ReasoningKernel

#endif
