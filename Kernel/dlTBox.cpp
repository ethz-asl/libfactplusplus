/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2010 by Dmitry Tsarkov

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

#include "dlTBox.h"

#include <sstream>
#include "dltree.h"
#include "cppi.h"

#include "globaldef.h"
#include "ReasonerNom.h"
#include "DLConceptTaxonomy.h"
#include "procTimer.h"
#include "dumpLisp.h"
#include "logging.h"

// uncomment the following line to print currently checking subsumption
//#define FPP_DEBUG_PRINT_CURRENT_SUBSUMPTION

TBox :: TBox ( const ifOptionSet* Options, const std::string& TopORoleName, const std::string& BotORoleName, const std::string& TopDRoleName, const std::string& BotDRoleName )
	: DLHeap(Options)
	, stdReasoner(NULL)
	, nomReasoner(NULL)
	, pMonitor(NULL)
	, pTax (NULL)
	, pOptions (Options)
	, Status(kbLoading)
	, curFeature(NULL)
	, defConcept (NULL)
	, Concepts("concept")
	, Individuals("individual")
	, ORM ( /*data=*/false, TopORoleName, BotORoleName )
	, DRM ( /*data=*/true, TopDRoleName, BotDRoleName )
	, Axioms(*this)
	, T_G(bpTOP)	// initialise GCA's concept with Top
	, RCCache(NULL)
	, auxConceptID(0)
	, Fairness(NULL)
	, useSortedReasoning(true)
	, isLikeGALEN(false)	// just in case Relevance part would be omited
	, isLikeWINE(false)
	, Consistent(true)
	, Precompleted(false)
	, preprocTime(0)
	, consistTime(0)
{
	readConfig ( Options );
	initTopBottom ();

	setForbidUndefinedNames(false);
}

TBox :: ~TBox ( void )
{
	// remove all RELATED structures
	for ( RelatedCollection::iterator p = RelatedI.begin(), p_end = RelatedI.end(); p < p_end; ++p )
		delete *p;

	// remove all simple rules
	for ( TSimpleRules::iterator q = SimpleRules.begin(), q_end = SimpleRules.end(); q < q_end; ++q )
		delete *q;

	// remove all concepts
	delete pTop;
	delete pBottom;
	delete pTemp;

	// right now -- to delete the appropriate tax-vertex; TODO: clear all newly introduced ones
	if ( defConcept != NULL )
		removeConcept(defConcept);

	// remove aux structures
	delete stdReasoner;
	delete nomReasoner;
	delete pTax;
	delete RCCache;
}

/// get unique aux concept
TConcept* TBox :: getAuxConcept ( void )
{
	std::stringstream name;
	name << " aux" << ++auxConceptID;
	TConcept* C = getConcept(name.str());
	C->setSystem();
	C->setNonClassifiable();
	C->initToldSubsumers();	// it is created after this is done centrally
	return C;
}

/// replace (AR:C) with X such that C [= AR^-:X for fresh X. @return X
TConcept*
TBox :: replaceForall ( DLTree* R, DLTree* C )
{
	// init cache if necessary
	if ( RCCache == NULL )
		RCCache = new TRCCache((ORM.end()-ORM.begin())/2);

	// check whether we already did this before for given R,C
	const TRole* r = resolveRole(R);
	TConcept* X = RCCache->get(r,C);

	if ( X != NULL )
		return X;

	// see R and C at the first time
	X = getAuxConcept();
	DLTree* c = clone(C);
	// create ax axiom C [= AR^-.X
	addSubsumeAxiom ( C, createSNFForall ( createInverse(R), getTree(X) ) );
	// save cache for R,C
	RCCache->add ( r, c, X );

	return X;
}

void TBox :: initTopBottom ( void )
{
	// create BOTTOM concept
	TConcept* p = new TConcept ("BOTTOM");
	p->setId(-1);
	p->pName = p->pBody = bpBOTTOM;
	pBottom = p;

	// create TOP concept
	p = new TConcept ("TOP");
	p->setId(-1);
	p->pName = p->pBody = bpTOP;
	p->tsDepth=1;
	p->classTag = cttTrueCompletelyDefined;
	pTop = p;

	p = new TConcept (" ");
	p->setId(-1);
	p->tsDepth=1;
	p->classTag = cttTrueCompletelyDefined;
	pTemp = p;
}

void TBox :: prepareReasoning ( void )
{
	// do the preprocessing
	Preprocess();

	// init reasoner (if not exist)
	initReasoner();

	// disallow to add new names into DL
	setForbidUndefinedNames(true);

	// check if it is necessary to dump relevant part TBox
	if ( dumpQuery )
	{
		// set up relevance info
		markAllRelevant();
		std::ofstream of ( "tbox" );
		fpp_assert ( of.good() );
		dumpLisp lDump(of);
		dump(&lDump);
		clearRelevanceInfo();
	}

	// init values for SAT tests -- either cache, or consistency check
	DLHeap.setSatOrder();
	setToDoPriorities();
}

/// prepare features for SAT(P), or SUB(P,Q) test
void TBox :: prepareFeatures ( const TConcept* pConcept, const TConcept* qConcept )
{
	auxFeatures = GCIFeatures;
	if ( pConcept != NULL )
		updateAuxFeatures(pConcept->posFeatures);
	if ( qConcept != NULL )
		updateAuxFeatures(qConcept->negFeatures);
	if ( auxFeatures.hasSingletons() )
		updateAuxFeatures(NCFeatures);
	curFeature = &auxFeatures;

	// set blocking method for the current reasoning session
	getReasoner()->setBlockingMethod ( isIRinQuery(), isNRinQuery() );
}

void
TBox :: buildSimpleCache ( void )
{
	// set cache for BOTTOM entry
	initSingletonCache(bpBOTTOM);

	// set all the caches for the temp concept
	initSingletonCache(pTemp->pName);
	initSingletonCache(inverse(pTemp->pName));

	// inapplicable if KB contains CGIs in any form
	if ( GCIs.isGCI() || GCIs.isReflexive() )
		return;

	// it is now safe to make a TOP cache
	initSingletonCache(bpTOP);

	for ( c_const_iterator c = c_begin(), cend = c_end(); c < cend; ++c )
		if ( (*c)->isPrimitive() )
			initSingletonCache(inverse((*c)->pName));

	for ( i_const_iterator i = i_begin(), iend = i_end(); i < iend; ++i )
		if ( (*i)->isPrimitive() )
			initSingletonCache(inverse((*i)->pName));
}

/// check if the ontology is consistent
bool
TBox :: performConsistencyCheck ( void )
{
	if ( verboseOutput )
		std::cerr << "Consistency checking...";
	TsProcTimer pt;
	pt.Start();

	buildSimpleCache();

	TConcept* test = ( NCFeatures.hasSingletons() ? *i_begin() : NULL );
	prepareFeatures ( test, NULL );
//	DlSatTester* Reasoner = getReasoner();
	bool ret = false;

	if ( test )
	{
		// make a cache for TOP if it is not there
		if ( DLHeap.getCache(bpTOP) == NULL )
			initSingletonCache(bpTOP);

		ret = nomReasoner->consistentNominalCloud();
	}
	else
		ret = isSatisfiable(pTop);

	// setup cache for GCI
	if ( GCIs.isGCI() )
	{
		// there is no much win to have it together with special-domains-as-GCIs ATM.
//		DLHeap.setCache ( T_G, Reasoner->buildCacheByCGraph(ret) );
		DLHeap.setCache ( inverse(T_G), new modelCacheConst(/*sat=*/false) );
	}

	pt.Stop();
	consistTime = pt;
	if ( verboseOutput )
		std::cerr << " done in " << pt << " seconds\n";

	return ret;
}

bool
TBox :: isSatisfiable ( const TConcept* pConcept )
{
	fpp_assert ( pConcept != NULL );

	// check whether we already does the test
	const modelCacheInterface* cache = DLHeap.getCache(pConcept->pName);
	if ( cache != NULL )
		return ( cache->getState() != csInvalid );

	// logging the startpoint
	if ( LLM.isWritable(llBegSat) )
		LL << "\n--------------------------------------------\n"
			  "Checking satisfiability of '" << pConcept->getName() << "':";
	if ( LLM.isWritable(llGTA) )
		LL << "\n";

	// perform reasoning with a proper logical features
	prepareFeatures ( pConcept, NULL );
	bool result = getReasoner()->runSat ( pConcept->resolveId(), bpTOP );
	// save cache
	cache = getReasoner()->buildCacheByCGraph(result);
	DLHeap.setCache ( pConcept->pName, cache );
	clearFeatures();

	CHECK_LL_RETURN_VALUE(llSatResult,result);

#if 1
	LL << "\n";		// usual checking -- time is extra info
#else
	LL << " ";		// time tests -- time is necessary info
#endif

	LL << "The '" << pConcept->getName() << "' concept is ";
	if ( !result )
		LL << "un";
	LL << "satisfiable w.r.t. TBox";

	return result;
}

bool
TBox :: isSubHolds ( const TConcept* pConcept, const TConcept* qConcept )
{
	fpp_assert ( pConcept != NULL && qConcept != NULL );

#ifdef FPP_DEBUG_PRINT_CURRENT_SUBSUMPTION
	std::cerr << "Checking '" << pConcept->getName() << "' [= '" << qConcept->getName() << "'...";
#endif

	// logging the startpoint
	if ( LLM.isWritable(llBegSat) )
		LL << "\n--------------------------------------------\nChecking subsumption '"
		   << pConcept->getName() << " [= " << qConcept->getName() << "':";
	if ( LLM.isWritable(llGTA) )
		LL << "\n";

	// perform reasoning with a proper logical features
	prepareFeatures ( pConcept, qConcept );
	bool result = !getReasoner()->runSat ( pConcept->resolveId(), (qConcept ? inverse(qConcept->resolveId()) : bpTOP) );
	clearFeatures();

#ifdef FPP_DEBUG_PRINT_CURRENT_SUBSUMPTION
	std::cerr << " done\n";
#endif

	CHECK_LL_RETURN_VALUE(llSatResult,result);

#if 1
	LL << "\n";		// usual checking -- time is extra info
#else
	LL << " ";		// time tests -- time is necessary info
#endif

	LL << "The '" << pConcept->getName() << " [= " << qConcept->getName() << "' subsumption";
	if (!result)
			LL << " NOT";
	LL << " holds w.r.t. TBox";

	return result;
}

/// check that 2 individuals are the same
bool TBox :: isSameIndividuals ( const TIndividual* a, const TIndividual* b )
{
	if ( !isIndividual(a) || !isIndividual(b) )
		throw EFaCTPlusPlus("Individuals are expected in the isSameIndividuals() query");
	if ( a->node == NULL || b->node == NULL )
		throw EFaCTPlusPlus("isSameIndividuals() query with non-realised ontology");
	return a->node->resolvePBlocker() == b->node->resolvePBlocker();
}

/// check if 2 roles are disjoint
bool
TBox :: isDisjointRoles ( const TRole* R, const TRole* S )
{
	fpp_assert ( R != NULL && S != NULL );

	// object roles are disjoint with data roles
	if ( R->isDataRole() != S->isDataRole() )
		return true;
	// prepare feature that are KB features
	// FIXME!! overkill, but fine for now as it is sound
	curFeature = &KBFeatures;
	getReasoner()->setBlockingMethod ( isIRinQuery(), isNRinQuery() );
	bool result = getReasoner()->checkDisjointRoles ( R, S );
	clearFeatures();
	return result;
}

/// check if the role R is irreflexive
bool
TBox :: isIrreflexive ( const TRole* R )
{
	fpp_assert ( R != NULL );

	// data roles are irreflexive
	if ( R->isDataRole() )
		return true;
	// prepare feature that are KB features
	// FIXME!! overkill, but fine for now as it is sound
	curFeature = &KBFeatures;
	getReasoner()->setBlockingMethod ( isIRinQuery(), isNRinQuery() );
	bool result = getReasoner()->checkIrreflexivity(R);
	clearFeatures();
	return result;
}

// load init values from config file
void TBox :: readConfig ( const ifOptionSet* Options )
{
	fpp_assert ( Options != NULL );	// safety check

// define a macro for registering boolean option
#	define addBoolOption(name)				\
	name = Options->getBool ( #name );		\
	if ( LLM.isWritable(llAlways) )			\
		LL << "Init " #name " = " << name << "\n"

	addBoolOption(useCompletelyDefined);
	addBoolOption(useRelevantOnly);
	addBoolOption(dumpQuery);
	addBoolOption(useRangeDomain);
	addBoolOption(alwaysPreferEquals);
	addBoolOption(usePrecompletion);

	if ( Axioms.initAbsorptionFlags(Options->getText("absorptionFlags"))
		 || !Axioms.isAbsorptionFlagsCorrect(useRangeDomain) )
		throw EFaCTPlusPlus ( "Incorrect absorption flags given" );

	verboseOutput = false;
#undef addBoolOption
}

/// remove concept from TBox by given EXTERNAL id. WARNING!! tested only for TempConcept!!!
void
TBox :: removeConcept ( TConcept* p )
{
	fpp_assert ( p == defConcept);

	// clear DAG and name indeces (if necessary)
	if ( isCorrect(p->pName) )
		DLHeap.removeAfter(p->pName);

	// delete associated taxonomy vertex as it is not done using destructor
	delete p->getTaxVertex();

	if ( Concepts.Remove(p) )
		fpp_unreachable();	// can't remove non-last concept
}

void
TBox :: clearQueryConcept ( void )
{
	removeConcept(defConcept);
}

/// create (and DAG-ify) query concept via its definition
TConcept*
TBox :: createQueryConcept ( const DLTree* desc )
{
	static const char* const defConceptName = "FaCT++.default";

	// clear the default concept def=desc
	if ( defConcept != NULL )
		clearQueryConcept();
//	else
	{
		// we have to add this concept in any cases. So change undefined names mode
		bool old = setForbidUndefinedNames(false);
		defConcept = getConcept(defConceptName);
		setForbidUndefinedNames(old);
	}

//	std::cerr << "Create new temp concept with description =" << desc << "\n";
	fpp_assert ( defConcept != NULL );

	// create description
	deleteTree ( makeNonPrimitive ( defConcept, clone(desc) ) );
	defConcept->setSystem();

	// build DAG entries for the default concept
	DLHeap.setExpressionCache(false);
	addConceptToHeap ( defConcept );

	// gather statistics about the concept
	setConceptRelevant(defConcept);

	// DEBUG_ONLY: print the DAG info
//	std::ofstream debugPrint ( defConceptName, std::ios::app|std::ios::out );
//	Print (debugPrint);
//	debugPrint << std::endl;

	// check satisfiability of the concept
	initCache ( defConcept );

	return defConcept;
}

/// classify query concept
void
TBox :: classifyQueryConcept ( void )
{
	// prepare told subsumers for classification; as it is non-primitive, it is not CD
	defConcept->initToldSubsumers();

	fpp_assert ( pTax != NULL );

	// setup taxonomy behaviour flags
	pTax->setCompletelyDefined ( false );	// non-primitive concept
	pTax->setInsertIntoTaxonomy ( false );	// just classify
	pTax->setProgressIndicator(NULL);		// switch off the progress monitor

	// classify the concept
	pTax->classifyEntry (defConcept);
}


/// dump QUERY processing time, reasoning statistics and a (preprocessed) TBox
void
TBox :: writeReasoningResult ( std::ostream& o, float time ) const
{
	if ( nomReasoner )
	{
		o << "Query processing reasoning statistic: Nominals";
		nomReasoner->writeTotalStatistic(o);
	}
	o << "Query processing reasoning statistic: Standard";
	stdReasoner->writeTotalStatistic(o);

	// we know here whether KB is consistent
	fpp_assert ( getStatus() >= kbCChecked );
	if ( Consistent )
		o << "Required";
	else
		o << "KB is inconsistent. Query is NOT processed\nConsistency";

	float sum = preprocTime + consistTime;
	o << " check done in " << time << " seconds\nof which:\nPreproc. takes "
	  << preprocTime << " seconds\nConsist. takes " << consistTime << " seconds";
	if ( nomReasoner )
	{
		o << "\nReasoning NOM:";
		sum += nomReasoner->printReasoningTime(o);
	}
	o << "\nReasoning STD:";
	sum += stdReasoner->printReasoningTime(o);
	o << "\nThe rest takes ";
	// determine and normalize the rest
	float f = time - sum;
	if ( f < 0 )
		f = 0;
	f = ((unsigned long)(f*100))/100.f;
	o << f << " seconds\n";
	Print(o);
}

void TBox :: PrintDagEntry ( std::ostream& o, BipolarPointer p ) const
{
	fpp_assert ( isValid (p) );

	// primitive ones -- check first
	if ( p == bpTOP )
	{
		o << " *TOP*";
		return;
	}
	else if ( p == bpBOTTOM )
	{
		o << " *BOTTOM*";
		return;
	}

	// checks inversion
	if ( isNegative(p) )
	{
		o << " (not";
		PrintDagEntry ( o, inverse(p) );
		o << ")";
		return;
	}

	const DLVertex& v = DLHeap [getValue(p)];

	switch ( v.Type() )
	{
	case dtTop:
		o << " *TOP*";
		return;

	case dtName:
	case dtDataType:
	case dtDataValue:
		o << ' ' << v.getConcept()->getName();
		return;

	case dtDataExpr:
		o << ' ' << *getDataEntryByBP(p)->getFacet();
		return;

	case dtIrr:
		o << " (" << v.getTagName() << ' ' << v.getRole()->getName() << ")";
		return;

	case dtCollection:
	case dtAnd:
		o << " (" << v.getTagName();
		for ( DLVertex::const_iterator q = v.begin(); q != v.end(); ++q )
			PrintDagEntry ( o, *q );
		o << ")";
		return;

	case dtForall:
	case dtLE:
		o << " (" << v.getTagName();
		if ( v.Type() == dtLE )
			o << ' ' << v.getNumberLE();
		o << ' ' << v.getRole()->getName();
		PrintDagEntry ( o, v.getC() );
		o << ")";
		return;

	case dtProj:
		o << " (" << v.getTagName() << ' ' << v.getRole()->getName() << ' ';
		PrintDagEntry ( o, v.getC() );
		o << " => " << v.getProjRole()->getName() << ")";
		return;

	case dtNN:	// shouldn't appears in the expressions
		fpp_unreachable();

	default:	// invalid value
		std::cerr << "Error printing vertex of type " << v.getTagName() << "(" << v.Type () << ")";
		fpp_unreachable();
		return;
	}
}

void TBox :: PrintConcept ( std::ostream& o, const TConcept* p ) const
{
	// print only relevant concepts
	if ( isValid(p->pName) )
	{
		o << getCTTagName(p->getClassTag()) << '.';

		if ( p->isSingleton() )
			o << '!';

		o << p->getName() << " [" << p->tsDepth
		  << "] " << (p->isNonPrimitive() ? "=" : "[=");

		if ( isValid (p->pBody) )
			PrintDagEntry ( o, p->pBody );

		// if you want to check correctness of translation (print following info)
		// you should comment out RemoveExtraDescription() in TBox::Preprocess()
		// but check hasSynonym assignment
		if ( p->Description != NULL )
			o << (p->isNonPrimitive() ? "\n-=" : "\n-[=") << p->Description;

		o << "\n";
	}
}

void
TBox :: printFeatures ( void ) const
{
	KBFeatures.writeState();

	if ( LLM.isWritable(llAlways) )
		LL << "KB contains " << (GCIs.isGCI() ? "" : "NO ") << "GCIs\nKB contains "
		   << (GCIs.isReflexive() ? "" : "NO ") << "reflexive roles\nKB contains "
		   << (GCIs.isRnD() ? "" : "NO ") << "range and domain restrictions\n";
}
