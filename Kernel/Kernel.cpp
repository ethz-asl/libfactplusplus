/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2012 by Dmitry Tsarkov

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

#include "Kernel.h"
#include "tOntologyLoader.h"
#include "tOntologyPrinterLISP.h"
#include "AxiomSplitter.h"
#include "AtomicDecomposer.h"

const char* ReasoningKernel :: Version = "1.5.3";
const char* ReasoningKernel :: SupportedDL = "SROIQ(D)";
const char* ReasoningKernel :: Copyright =
	"Copyright (C) Dmitry Tsarkov, 2002-2011";
const char* ReasoningKernel :: ReleaseDate = "7 December 2011";

// print the FaCT++ information only once
static bool KernelFirstRun = true;

// debug related individual/values switch
//#define FPP_DEBUG_PRINT_RELATED_PROGRESS

// dump loaded ontology in LISP format to the stdout
//#define FPP_DEBUG_DUMP_LISP_ONTOLOGY

ReasoningKernel :: ReasoningKernel ( void )
	: pTBox (NULL)
	, pET(NULL)
	, KE(NULL)
	, AD(NULL)
	, pMonitor(NULL)
	, OpTimeout(0)
	, verboseOutput(false)
	, useUndefinedNames(true)
	, cachedQuery(NULL)
	, cachedQueryTree(NULL)
	, useAxiomSplitting(false)
{
	// Intro
	if ( KernelFirstRun )
	{
		std::cerr << "FaCT++.Kernel: Reasoner for the " << SupportedDL << " Description Logic, " << 8*sizeof(void*) << "-bit\n"
				  << Copyright << ". Version " << Version << " (" << ReleaseDate << ")\n";
		KernelFirstRun = false;
	}

	initCacheAndFlags();

	// init option set (fill with options):
	if ( initOptions () )
		throw EFaCTPlusPlus("FaCT++ kernel: Cannot init options");
}

/// d'tor
ReasoningKernel :: ~ReasoningKernel ( void )
{
	clearTBox();
	delete pMonitor;
	delete KE;
	delete AD;
}

bool
ReasoningKernel :: tryIncremental ( void )
{
	// if no TBox known -- reload
	if ( pTBox == NULL )
		return true;
	// if ontology wasn't change -- no need to reload
	if ( !Ontology.isChanged() )
		return false;
	// else -- check whether incremental is possible
	// FOR NOW!! switch off incremental reasoning while the JNI is unstable (as it is the only user ATM)
	return true;
}

/// force the re-classification of the changed ontology
void
ReasoningKernel :: forceReload ( void )
{
	// reset TBox
	clearTBox();
	newKB();

	// Protege (as the only user of non-trivial monitors with reload) does not accept multiple usage of a monitor
	// so switch it off after the 1st usage
	pMonitor = NULL;

	// split ontological axioms
	if ( useAxiomSplitting )
	{
		TAxiomSplitter AxiomSplitter(&Ontology);
		AxiomSplitter.buildSplit();
	}

	// (re)load ontology
	TOntologyLoader OntologyLoader(*getTBox());
	OntologyLoader.visitOntology(Ontology);

#ifdef FPP_DEBUG_DUMP_LISP_ONTOLOGY
	TLISPOntologyPrinter OntologyPrinter(std::cout);
//	DRoles.fill(OntologyPrinter);
	OntologyPrinter.visitOntology(Ontology);
#endif

	// after loading ontology became processed completely
	Ontology.setProcessed();
}

//-------------------------------------------------
// Prepare reasoning/query
//-------------------------------------------------

void
ReasoningKernel :: processKB ( KBStatus status )
{
	fpp_assert ( status >= kbCChecked );

	// check whether reasoning was failed
	if ( reasoningFailed )
		throw EFaCTPlusPlus("Can't classify KB because of previous errors");

	// check if something have to be done
	if ( getStatus() >= status )
	{	// nothing to do; but make sure that we are consistent
		if ( !isKBConsistent() )
			throw EFPPInconsistentKB();
		return;
	}

	// here we have to do something: let's decide what to do
	switch ( getStatus() )
	{
	case kbEmpty:
	case kbLoading:		break;	// need to do the whole cycle -- just after the switch
	case kbCChecked:	goto Classify;	// do classification
	case kbClassified:	goto Realise;	// do realisation
	default:	// nothing should be here
		fpp_unreachable();
	}

	// start with loading and preprocessing -- here might be a failures
	reasoningFailed = true;

	// load the axioms from the ontology to the TBox
	if ( tryIncremental() )
		forceReload();

	// do the consistency check
	pTBox->isConsistent();

	// if there were no exception thrown -- clear the failure status
	reasoningFailed = false;

	if ( status == kbCChecked )
		return;

Classify:	// do classification

	// don't do classification twice
	if ( status == kbRealised )
		goto Realise;

	if ( !pTBox->isConsistent() )
		return;

	pTBox->performClassification();
	return;

Realise:	// do realisation

	if ( !pTBox->isConsistent() )
		return;

	pTBox->performRealisation();
}

//-----------------------------------------------------------------------------
//--		query caching support
//-----------------------------------------------------------------------------

/// classify query; cache is ready at the point. NAMED means whether concept is just a name
void
ReasoningKernel :: classifyQuery ( bool named )
{
	// make sure KB is classified
	classifyKB();

	if ( !named )	// general expression: classify query concept
		getTBox()->classifyQueryConcept();

	cachedVertex = cachedConcept->getTaxVertex();

	if ( unlikely(cachedVertex == NULL) )	// fresh concept
		cachedVertex = getCTaxonomy()->getFreshVertex(cachedConcept);
}

void
ReasoningKernel :: setUpCache ( DLTree* query, cacheStatus level )
{
	// if KB was changed since it was classified,
	// we should catch it before
	fpp_assert ( !Ontology.isChanged() );

	// check if the query is already cached
	if ( checkQueryCache(query) )
	{	// ... with the same level -- nothing to do
		deleteTree(query);
		if ( level <= cacheLevel )
			return;
		else
		{	// concept was defined but not classified yet
			fpp_assert ( level == csClassified && cacheLevel != csClassified );
			if ( cacheLevel == csSat )	// already check satisfiability
			{
				classifyQuery(isCN(cachedQueryTree));
				return;
			}
		}
	}
	else	// change current query
		setQueryCache(query);

	// clean cached info
	cachedVertex = NULL;
	cacheLevel = level;

	// check if concept-to-cache is defined in ontology
	if ( isCN(cachedQueryTree) )
		cachedConcept = getTBox()->getCI(cachedQueryTree);
	else	// case of complex query
	{
		getTBox()->clearQueryConcept();
		cachedConcept = getTBox()->createQueryConcept(cachedQueryTree);
	}

	fpp_assert ( cachedConcept != NULL );
	// preprocess concept is necessary (fresh concept in query or complex one)
	if ( !isValid(cachedConcept->pName) )
		getTBox()->preprocessQueryConcept(cachedConcept);

	if ( level == csClassified )
		classifyQuery(isCN(cachedQueryTree));
}

void
ReasoningKernel :: setUpCache ( TConceptExpr* query, cacheStatus level )
{
	// if KB was changed since it was classified,
	// we should catch it before
	fpp_assert ( !Ontology.isChanged() );

	// check if the query is already cached
	if ( checkQueryCache(query) )
	{	// ... with the same level -- nothing to do
		if ( level <= cacheLevel )
			return;
		else
		{	// concept was defined but not classified yet
			fpp_assert ( level == csClassified && cacheLevel != csClassified );
			if ( cacheLevel == csSat )	// already check satisfiability
			{
				classifyQuery(isNameOrConst(cachedQuery));
				return;
			}
		}
	}
	else	// change current query
		setQueryCache(query);

	// clean cached info
	cachedVertex = NULL;
	cacheLevel = level;

	// check if concept-to-cache is defined in ontology
	if ( isNameOrConst(cachedQuery) )
		cachedConcept = getTBox()->getCI(TreeDeleter(e(cachedQuery)));
	else	// case of complex query
	{
		// need to clear the query before transform it into DLTree
		getTBox()->clearQueryConcept();
		// ... as if fresh names appears there, they would be cleaned up
		cachedConcept = getTBox()->createQueryConcept(TreeDeleter(e(cachedQuery)));
	}

	fpp_assert ( cachedConcept != NULL );
	// preprocess concept is necessary (fresh concept in query or complex one)
	if ( !isValid(cachedConcept->pName) )
		getTBox()->preprocessQueryConcept(cachedConcept);

	if ( level == csClassified )
		classifyQuery(isNameOrConst(cachedQuery));
}

//-------------------------------------------------
// concept subsumption query implementation
//-------------------------------------------------

/// class for exploring concept taxonomy to find super classes
class SupConceptActor
{
protected:
	const ClassifiableEntry* pe;
	void entry ( const ClassifiableEntry* q ) { if ( pe == q ) throw std::exception(); }
public:
	SupConceptActor ( ClassifiableEntry* q ) :pe(q) {}
	bool apply ( const TaxonomyVertex& v )
	{
		entry(v.getPrimer());
		for ( TaxonomyVertex::syn_iterator p = v.begin_syn(), p_end=v.end_syn(); p != p_end; ++p )
			entry(*p);
		return true;
	}
}; // SupConceptActor

/// @return true iff C [= D holds
bool
ReasoningKernel :: checkSub ( TConcept* C, TConcept* D )
{
	// check whether a concept is fresh
	if ( unlikely(!isValid(D->pName)) )	// D is fresh
	{
		if ( unlikely(!isValid(C->pName)) )	// C is fresh
			return C == D;	// 2 fresh concepts subsumes one another iff they are the same
		else	// C is known
			return !getTBox()->isSatisfiable(C);	// C [= D iff C=\bottom
	}
	else	// D is known
		if ( unlikely(!isValid(C->pName)) )	// C is fresh
			// C [= D iff D = \top, or ~D = \bottom
			return !checkSatTree(createSNFNot(getTBox()->getTree(C)));
	// here C and D are known (not fresh)
	if ( getStatus() < kbClassified )	// unclassified => do via SAT test
		return getTBox()->isSubHolds ( C, D );
	// classified => do the taxonomy traversal
	SupConceptActor actor(D);
	Taxonomy* tax = getCTaxonomy();
	try { tax->getRelativesInfo</*needCurrent=*/true, /*onlyDirect=*/false, /*upDirection=*/true> ( C->getTaxVertex(), actor ); return false; }
	catch (...) { tax->clearCheckedLabel(); return true; }
}

//-------------------------------------------------
// all-disjoint query implementation
//-------------------------------------------------

bool
ReasoningKernel :: isDisjointRoles ( void )
{
	// grab all roles from the arg-list
	typedef const std::vector<const TDLExpression*> TExprVec;
	typedef std::vector<const TRole*> TRoleVec;
	TExprVec Disj = getExpressionManager()->getArgList();
	TRoleVec Roles;
	Roles.reserve(Disj.size());

	for ( TExprVec::const_iterator p = Disj.begin(), p_end = Disj.end(); p != p_end; ++p )
	{
		TORoleExpr* ORole = dynamic_cast<TORoleExpr*>(*p);
		if ( ORole != NULL )
		{
			if ( getExpressionManager()->isUniversalRole(ORole) )
				return false;	// universal role is not disjoint with anything
			if ( getExpressionManager()->isEmptyRole(ORole) )
				continue;		// empty role is disjoint with everything

			Roles.push_back ( getRole ( ORole, "Role expression expected in isDisjointRoles()" ) );
		}
		else
		{
			TDRoleExpr* DRole = dynamic_cast<TDRoleExpr*>(*p);
			if ( DRole == NULL )
				throw EFaCTPlusPlus("Role expression expected in isDisjointRoles()");

			if ( getExpressionManager()->isUniversalRole(DRole) )
				return false;	// universal role is not disjoint with anything
			if ( getExpressionManager()->isEmptyRole(DRole) )
				continue;		// empty role is disjoint with everything

			Roles.push_back ( getRole ( DRole, "Role expression expected in isDisjointRoles()" ) );
		}
	}

	// test pair-wise disjointness
	TRoleVec::const_iterator q = Roles.begin(), q_end = Roles.end(), s;
	for ( ; q != q_end; ++q )
		for ( s = q+1; s != q_end; ++s )
			if ( !getTBox()->isDisjointRoles(*q,*s) )
				return false;

	return true;
}

//-------------------------------------------------
// related individuals implementation
//-------------------------------------------------

class RIActor
{
protected:
	ReasoningKernel::CIVec acc;

		/// process single entry in a vertex label
	bool tryEntry ( const ClassifiableEntry* p )
	{
		// check the applicability
		if ( p->isSystem() || !static_cast<const TConcept*>(p)->isSingleton() )
			return false;

		// print the concept
		acc.push_back(static_cast<const TIndividual*>(p));
		return true;
	}

public:
	RIActor ( void ) {}
	~RIActor ( void ) {}

	bool apply ( const TaxonomyVertex& v )
	{
		bool ret = tryEntry(v.getPrimer());

		for ( TaxonomyVertex::syn_iterator p = v.begin_syn(), p_end = v.end_syn(); p != p_end; ++p )
			ret |= tryEntry(*p);

		return ret;
	}
	const ReasoningKernel::CIVec& getAcc ( void ) const { return acc; }
}; // RIActor

ReasoningKernel::CIVec
ReasoningKernel :: buildRelatedCache ( TIndividual* I, const TRole* R )
{
#ifdef FPP_DEBUG_PRINT_RELATED_PROGRESS
	std::cout << "Related for " << I->getName() << " via property " << R->getName() << "\n";
#endif

	// for synonyms: use the representative's cache
	if ( R->isSynonym() )
		return getRelated ( I, resolveSynonym(R) );

	// FIXME!! return an empty set for data roles
	if ( R->isDataRole() )
		return CIVec();

	// empty role has no fillers
	if ( R->isBottom() )
		return CIVec();

	// now fills the query
	RIActor actor;
	// ask for instances of \exists R^-.{i}
	TORoleExpr* InvR = R->getId() > 0
		? getExpressionManager()->Inverse(getExpressionManager()->ObjectRole(R->getName()))
		: getExpressionManager()->ObjectRole(R->inverse()->getName());
	TConceptExpr* query =
		R->isTop() ? getExpressionManager()->Top() : 	// universal role has all the named individuals as a filler
		getExpressionManager()->Value ( InvR, getExpressionManager()->Individual(I->getName()) );
	getInstances ( query, actor );
	return actor.getAcc();
}

/// @return in Rs all (DATA)-roles R s.t. (I,x):R; add inverses if NEEDI is true
void
ReasoningKernel :: getRelatedRoles ( const TIndividualExpr* I, NamesVector& Rs, bool data, bool needI )
{
	realiseKB();	// ensure KB is ready to answer the query
	Rs.clear();

	TIndividual* i = getIndividual ( I, "individual name expected in the getRelatedRoles()" );
	RoleMaster* RM = data ? getDRM() : getORM();
	for ( RoleMaster::iterator p = RM->begin(), p_end = RM->end(); p < p_end; ++p )
	{
		const TRole* R = *p;
		if ( ( R->getId() > 0 || needI ) && !getRelated(i,R).empty() )
			Rs.push_back(R);
	}
}

void
ReasoningKernel :: getRoleFillers ( const TIndividualExpr* I, const TORoleExpr* R, IndividualSet& Result )
{
	realiseKB();	// ensure KB is ready to answer the query
	CIVec vec = getRelated ( getIndividual ( I, "Individual name expected in the getRoleFillers()" ),
							 getRole ( R, "Role expression expected in the getRoleFillers()" ) );
	for ( CIVec::iterator p = vec.begin(), p_end = vec.end(); p < p_end; ++p )
		Result.push_back(const_cast<TIndividual*>(*p));
}

/// set RESULT into set of J's such that R(I,J)
bool
ReasoningKernel :: isRelated ( const TIndividualExpr* I, const TORoleExpr* R, const TIndividualExpr* J )
{
	realiseKB();	// ensure KB is ready to answer the query
	TIndividual* i = getIndividual ( I, "Individual name expected in the isRelated()" );
	TRole* r = getRole ( R, "Role expression expected in the isRelated()" );
	if ( r->isDataRole() )
		return false;	// FIXME!! not implemented

	TIndividual* j = getIndividual ( J, "Individual name expected in the isRelated()" );
	CIVec vec = getRelated ( i, r );
	for ( CIVec::iterator p = vec.begin(), p_end = vec.end(); p < p_end; ++p )
		if ( j == (*p) )
			return true;

	return false;
}

//----------------------------------------------------------------------------------
// atomic decomposition queries
//----------------------------------------------------------------------------------

	/// create new atomic decomposition of the loaded ontology using TYPE. @return size of the AD
unsigned int
ReasoningKernel :: getAtomicDecompositionSize ( ModuleType type )
{
	// init AD field
	if ( AD == NULL )
		AD = new AtomicDecomposer();
	return AD->getAOS ( &Ontology, type )->size();
}
	/// get a set of axioms that corresponds to the atom with the id INDEX
const TOntologyAtom::AxiomSet&
ReasoningKernel :: getAtomAxioms ( unsigned int index ) const
{
	return (*AD->getAOS())[index]->getAtomAxioms();
}
	/// get a set of axioms that corresponds to the module of the atom with the id INDEX
const TOntologyAtom::AxiomSet&
ReasoningKernel :: getAtomModule ( unsigned int index ) const
{
	return (*AD->getAOS())[index]->getModule();
}
	/// get a set of atoms on which atom with index INDEX depends
const TOntologyAtom::AtomSet&
ReasoningKernel :: getAtomDependents ( unsigned int index ) const
{
	return (*AD->getAOS())[index]->getDepAtoms();
}

//******************************************
//* Initialization
//******************************************

bool ReasoningKernel :: initOptions ( void )
{
	// register all possible options used in FaCT++ Kernel

	// options for TBox

	// register "useRelevantOnly" option
	if ( KernelOptions.RegisterOption (
		"useRelevantOnly",
		"Option 'useRelevantOnly' is used when creating internal DAG representation for externally given TBox. "
		"If true, DAG contains only concepts, relevant to query. It is safe to leave this option false.",
		ifOption::iotBool,
		"false"
		) )
		return true;

	// register "dumpQuery" option -- 11-08-04
	if ( KernelOptions.RegisterOption (
		"dumpQuery",
		"Option 'dumpQuery' dumps sub-TBox relevant to given satisfiability/subsumption query.",
		ifOption::iotBool,
		"false"
		) )
		return true;

	// register "absorptionFlags" option (04/05/2005)
	if ( KernelOptions.RegisterOption (
		"absorptionFlags",
		"Option 'absorptionFlags' sets up absorption process for general axioms. "
		"It text field of arbitrary length; every symbol means the absorption action: "
		"(B)ottom Absorption), (T)op absorption, (E)quivalent concepts replacement, (C)oncept absorption, "
		"(N)egated concept absorption, (F)orall expression replacement, (R)ole absorption, (S)plit",
		ifOption::iotText,
		"BTECFSR"
		) )
		return true;

	// register "alwaysPreferEquals" option (26/01/2006)
	if ( KernelOptions.RegisterOption (
		"alwaysPreferEquals",
		"Option 'alwaysPreferEquals' allows user to enforce usage of C=D definition instead of C[=D "
		"during absorption, even if implication appeares earlier in stream of axioms.",
		ifOption::iotBool,
		"true"
		) )
		return true;

	// register "useSpecialDomains" option (25/10/2011)
	if ( KernelOptions.RegisterOption (
		"useSpecialDomains",
		"Option 'useSpecialDomains' (development) controls the special processing of R&D for non-simple roles. "
		"Should always be set to true.",
		ifOption::iotBool,
		"true"
		) )
		return true;

	// options for DLDag

	// register "orSortSub" option (20/12/2004)
	if ( KernelOptions.RegisterOption (
		"orSortSub",
		"Option 'orSortSub' define the sorting order of OR vertices in the DAG used in subsumption tests. "
		"Option has form of string 'Mop', where 'M' is a sort field (could be 'D' for depth, 'S' for size, 'F' "
		"for frequency, and '0' for no sorting), 'o' is a order field (could be 'a' for ascending and 'd' "
		"for descending mode), and 'p' is a preference field (could be 'p' for preferencing non-generating "
		"rules and 'n' for not doing so).",
		ifOption::iotText,
		"0"
		) )
		return true;

	// register "orSortSat" option (20/12/2004)
	if ( KernelOptions.RegisterOption (
		"orSortSat",
		"Option 'orSortSat' define the sorting order of OR vertices in the DAG used in satisfiability tests "
		"(used mostly in caching). Option has form of string 'Mop', see orSortSub for details.",
		ifOption::iotText,
		"0"
		) )
		return true;

	// options for ToDoTable

	// register "IAOEFLG" option
	if ( KernelOptions.RegisterOption (
		"IAOEFLG",
		"Option 'IAOEFLG' define the priorities of different operations in TODO list. Possible values are "
		"7-digit strings with ony possible digit are 0-6. The digits on the places 1, 2, ..., 7 are for "
		"priority of Id, And, Or, Exists, Forall, LE and GE operations respectively. The smaller number means "
		"the higher priority. All other constructions (TOP, BOTTOM, etc) has priority 0.",
		ifOption::iotText,
		"1263005"
		) )
		return true;

	// options for Reasoner

	// register "useSemanticBranching" option
	if ( KernelOptions.RegisterOption (
		"useSemanticBranching",
		"Option 'useSemanticBranching' switch semantic branching on and off. The usage of semantic branching "
		"usually leads to faster reasoning, but sometime could give small overhead.",
		ifOption::iotBool,
		"true"
		) )
		return true;

	// register "useBackjumping" option
	if ( KernelOptions.RegisterOption (
		"useBackjumping",
		"Option 'useBackjumping' switch backjumping on and off. The usage of backjumping "
		"usually leads to much faster reasoning.",
		ifOption::iotBool,
		"true"
		) )
		return true;

	// register "testTimeout" option -- 21/08/09
	if ( KernelOptions.RegisterOption (
		"testTimeout",
		"Option 'testTimeout' sets timeout for a single reasoning test in milliseconds.",
		ifOption::iotInt,
		"0"
		) )
		return true;

	// options for Blocking

	// register "useLazyBlocking" option -- 08-03-04
	if ( KernelOptions.RegisterOption (
		"useLazyBlocking",
		"Option 'useLazyBlocking' makes checking of blocking status as small as possible. This greatly "
		"increase speed of reasoning.",
		ifOption::iotBool,
		"true"
		) )
		return true;

	// register "useAnywhereBlocking" option (18/08/2008)
	if ( KernelOptions.RegisterOption (
		"useAnywhereBlocking",
		"Option 'useAnywhereBlocking' allow user to choose between Anywhere and Ancestor blocking.",
		ifOption::iotBool,
		"true"
		) )
		return true;

	// register "skipBeforeBlock" option (28/02/2009)
	if ( KernelOptions.RegisterOption (
		"skipBeforeBlock",
		"Internal use only. Option 'skipBeforeBlock' allow user to skip given number of nodes before make a block.",
		ifOption::iotInt,
		"0"
		) )
		return true;

	// options for Taxonomy

	// register "useCompletelyDefined" option
	if ( KernelOptions.RegisterOption (
		"useCompletelyDefined",
		"Option 'useCompletelyDefined' leads to simpler Taxonomy creation if TBox contains no non-primitive "
		"concepts. Unfortunately, it is quite rare case.",
		ifOption::iotBool,
		"true"
		) )
		return true;

	// options for testing

	// register "checkAD" option (24/02/2012)
	if ( KernelOptions.RegisterOption (
		"checkAD",
		"Option 'checkAD' forces FaCT++ to create the AD and exit instead of performing classification",
		ifOption::iotBool,
		"false"
		) )
		return true;

	// register "useELReasoner" option (24/02/2012)
	if ( KernelOptions.RegisterOption (
		"useELReasoner",
		"Option 'useELReasoner' forces FaCT++ to use the EL reasoner and exit instead of performing classification",
		ifOption::iotBool,
		"false"
		) )
		return true;

	// all was registered OK
	return false;
}

