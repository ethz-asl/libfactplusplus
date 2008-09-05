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

#include "dlTBox.h"

#include <fstream>

#include "procTimer.h"
#include "logging.h"
#include "Precomplete.h"

//#define DEBUG_PREPROCESSING

#ifdef DEBUG_PREPROCESSING
#	define BEGIN_PASS(str) std::cerr << "\n" str "... "
#	define END_PASS() std::cerr << "done"
#else
#	define BEGIN_PASS(str)
#	define END_PASS()
#endif

void TBox :: Preprocess ( void )
{
	if ( verboseOutput )
		std::cerr << "\nPreprocessing...";
	TsProcTimer pt;
	pt.Start();

	// builds role hierarchy
	BEGIN_PASS("Build role hierarchy");
	RM.initAncDesc();
	END_PASS();

	if ( verboseOutput )
	{
		std::ofstream roles("Taxonomy.Roles");
		RM.getTaxonomy()->print(roles);
	}

	// all concept descriptions contains synonyms. Remove them now
	BEGIN_PASS("Replace synonyms in expressions");
	if ( countSynonyms() > 0 )
		replaceAllSynonyms();
	END_PASS();

	// preprocess Related structure (before classification tags are defined)
	BEGIN_PASS("Preprocess related axioms");
	preprocessRelated();
	END_PASS();

	// init told subsumers as they would be used soon
	BEGIN_PASS("Init told subsumers");
	initToldSubsumers();
	END_PASS();

	// locate told (definitional) cycles and transform them into synonyms
	BEGIN_PASS("Detect and replace told cycles");
	transformToldCycles();
	END_PASS();

	// absorb axioms (move some Axioms to Role and Concept Description)
	BEGIN_PASS("Perform absorption");
	AbsorbAxioms();
	END_PASS();

	// set told TOP concepts whether necessary
	BEGIN_PASS("Set told TOP");
	setToldTop();
	END_PASS();

	// perform precompletion (if possible)
	if ( usePrecompletion )
	{
		BEGIN_PASS("Build precompletion");
		performPrecompletion();
		END_PASS();
	}

	// no more axiom transformations allowed

	// fills classification tag (strictly after told cycles)
	BEGIN_PASS("Detect classification tags");
	fillsClassificationTag();
	END_PASS();

	// set up TS depth
	BEGIN_PASS("Calculate told subsumer depth");
	calculateTSDepth();
	END_PASS();

	// create DAG (concept normalisation etc)
	BEGIN_PASS("Build DAG");
	buildDAG();
	END_PASS();

	// builds Roles range and domain
	BEGIN_PASS("Build DAG for domain and range for all roles");
	initRangeDomain();
	END_PASS();

	// builds Roles functional labels
	BEGIN_PASS("Build DAG for functional roles");
	initFunctionalRoles();
	END_PASS();

	// create sorts for KB
	BEGIN_PASS("Determine sorts");
	determineSorts();
	END_PASS();

	// calculate statistic for the whole KB:
	BEGIN_PASS("Gather relevance info");
	gatherRelevanceInfo();
	END_PASS();

	// GALEN-like flag is known here, so we can set OR defaults
	BEGIN_PASS("Set defaults for OR orderings");
	DLHeap.setOrderDefaults (
		isGalenLikeTBox() ? "Fdn" : isWineLikeTBox() ? "Sdn" : "Sap",	// SAT settings
		isGalenLikeTBox() ? "Ban" : isWineLikeTBox() ? "Dap" : "Dap"	// SUB settings
		);
	END_PASS();

	// now we can gather DAG statistics (if necessary)
	BEGIN_PASS("Gather usage statistics");
	DLHeap.gatherStatistic();
	END_PASS();

	// calculate statistic on DAG and Roles
	BEGIN_PASS("Gather concept-related statistics");
	CalculateStatistic();
	END_PASS();

	// free extra memory
	BEGIN_PASS("Free unused memory");
	RemoveExtraDescriptions();
	END_PASS();

	pt.Stop();
	preprocTime = pt;
	if ( verboseOutput )
		std::cerr << " done in " << pt << " seconds\n";
}

static bool
replaceSynonymsFromTree ( DLTree* desc )
{
	if ( desc == NULL )
		return false;

	TLexeme& cur = desc->Element();	// not const
	if ( cur == CNAME )
	{
		ClassifiableEntry* entry = static_cast<ClassifiableEntry*>(cur.getName());

		if ( entry->isSynonym() )
		{
			// check for TOP/BOTTOM
			//FIXME!! may be, better use ID for TOP/BOTTOM
			if ( entry->getSynonym()->getId() == -1 )
				cur = !strcmp(entry->getSynonym()->getName(), "TOP") ? TOP : BOTTOM;
			else
				cur = TLexeme ( CNAME, entry->getSynonym() );
			return true;
		}
		else
			return false;
	}
	else
	{
		bool ret = replaceSynonymsFromTree ( desc->Left() );
		ret |= replaceSynonymsFromTree ( desc->Right() );
		return ret;
	}
}

void TBox :: replaceAllSynonyms ( void )
{
	// replace synonyms in role's domain
	for ( RoleMaster::iterator pr = RM.begin(), pr_end = RM.end(); pr < pr_end; ++pr )
		if ( !(*pr)->isSynonym() )
			replaceSynonymsFromTree ( (*pr)->getTDomain() );

	for ( c_iterator pc = c_begin(); pc != c_end(); ++pc )
		if ( replaceSynonymsFromTree ( (*pc)->Description ) )
			(*pc)->initToldSubsumers();
	for ( i_iterator pi = i_begin(); pi != i_end(); ++pi )
		if ( replaceSynonymsFromTree ( (*pi)->Description ) )
			(*pi)->initToldSubsumers();

	// replace synonyms in Different part
	for ( DifferentIndividuals::iterator di = Different.begin(); di != Different.end(); ++di )
		for ( SingletonVector::iterator sv = di->begin(); sv != di->end(); ++sv )
			if ( (*sv)->isSynonym() )
				*sv = resolveSynonym(*sv);
}

void TBox :: preprocessRelated ( void )
{
	for ( RelatedCollection::iterator q = RelatedI.begin(), q_end = RelatedI.end(); q != q_end; ++q )
		(*q)->simplify();
}

void TBox :: transformToldCycles ( void )
{
	// remember number of synonyms appeared in KB
	unsigned int nSynonyms = countSynonyms();

	clearRelevanceInfo();
	for ( c_iterator pc = c_begin(); pc != c_end(); ++pc )
		if ( !(*pc)->isSynonym() )
			checkToldCycle(*pc);

	for ( i_iterator pi = i_begin(); pi != i_end(); ++pi )
		if ( !(*pi)->isSynonym() )
			checkToldCycle(*pi);
	clearRelevanceInfo();

	// update nymber of synonyms
	nSynonyms = countSynonyms() - nSynonyms;
	if ( nSynonyms )
	{
		if ( LLM.isWritable(llAlways) )
			LL << "\nTold cycle elimination done with " << nSynonyms << " synonyms created";

		replaceAllSynonyms();
	}
}

TConcept* TBox :: checkToldCycle ( TConcept* p )
{
	assert ( p != NULL );	// safety check

		// searchable stack for the told subsumers
	static std::set<TConcept*> sStack;
		// descriptions of the all the synonyms in the cycle
	static DLTree* desc;

	// no reason to process TOP here
	if ( p == pTop )
		return NULL;

	// if we found a cycle...
	if ( sStack.find(p) != sStack.end() )
	{
//		std::cout << "Cycle with " << p->getName() << std::endl;
		return p;
	}

	if ( isRelevant(p) )
	{
//		std::cout << "Already checked: " << p->getName() << std::endl;
		return NULL;
	}

	TConcept* ret = NULL;

	// add concept in processing
	sStack.insert(p);

redo:

//	std::cout << "Start from " << p->getName() << std::endl;

	for ( ClassifiableEntry::const_iterator r = p->told_begin(); r != p->told_end(); ++r )
		// if cycle was detected
		if ( (ret = checkToldCycle(static_cast<TConcept*>(*r))) != NULL )
		{
			if ( ret == p )
			{
//				std::cout << "Fill cycle with " << p->getName() << std::endl;

				// mark the returned concept primitive (to allow addDesc to work)
				p->setPrimitive();
				p->addDesc(desc);
				desc = NULL;

				// replace all synonyms with TOP
				p->removeSelfFromDescription();

				// clear return status and continue with the same concept
				ret = NULL;
				goto redo;
			}
			else
			{
//				std::cout << "Write synonym for " << p->getName() << std::endl;

				// some concept inside a cycle: make it synonym of RET, save old desc
				desc = createSNFAnd ( desc, makeNonPrimitive ( p, getTree(ret) ) );

				// no need to continue; finish with this cycle first
				break;
			}
		}

	// remove processed concept from set
	sStack.erase(p);

	p->setRelevant(relevance);
//	std::cout << "Done with " << p->getName() << std::endl;

	return ret;
}

void
TBox :: performPrecompletion ( void )
{
	Precompletor PC(*this);
	if ( PC.performPrecompletion() )
		std::cerr << "\nPrecompletion failed";	// do nothing for now
	else
		std::cerr << "\nPrecompletion succeed";	// do nothing for now
}

void TBox :: initFunctionalRoles ( void )
{
	for ( RoleMaster::iterator p = RM.begin(), p_end = RM.end(); p < p_end; ++p )
		if ( !(*p)->isSynonym() && (*p)->isTopFunc() )
			DLHeap.initFunctionalRole(*p);
}

void TBox :: initRangeDomain ( void )
{
	RoleMaster::iterator p, p_end = RM.end();
	for ( p = RM.begin(); p < p_end; ++p )
		if ( !(*p)->isSynonym() )
		{
#		ifdef RKG_UPDATE_RND_FROM_SUPERROLES
			// add R&D from super-roles (do it AFTER axioms are transformed into R&D)
			(*p)->collectDomainFromSupers();
#		endif

			DLTree* dom = (*p)->getTDomain();
			if ( dom )
			{
				(*p)->setBPDomain(tree2dag(dom));
				GCIs.setRnD();
			}
			else
				(*p)->setBPDomain(bpTOP);
		}
}

/// determine all sorts in KB (make job only for SORTED_REASONING)
void TBox :: determineSorts ( void )
{
#ifdef RKG_USE_SORTED_REASONING
	// Related individuals does not appears in DLHeap,
	// so their sorts shall be determined explicitely
	for ( RelatedCollection::const_iterator p = RelatedI.begin(), p_end = RelatedI.end(); p < p_end; ++p, ++p )
		DLHeap.updateSorts ( (*p)->a->pName, (*p)->R, (*p)->b->pName );

	// simple rules needs the same treatement
	for ( TSimpleRules::iterator q = SimpleRules.begin()+1; q != SimpleRules.end(); ++q )
	{
		mergableLabel& lab = DLHeap[(*q)->bpHead].getSort();
		for ( ConceptVector::const_iterator r = (*q)->Body.begin(), r_end = (*q)->Body.end(); r < r_end; ++r )
			DLHeap.merge ( lab, (*r)->pName );
	}

	// create sorts for concept and/or roles
	DLHeap.determineSorts(RM);
#endif // RKG_USE_SORTED_REASONING
}

// Told staff used, so run this AFTER fillTold*()
void TBox :: CalculateStatistic ( void )
{
	unsigned int npFull = 0, nsFull = 0;		// number of completely defined concepts
	unsigned int nPC = 0, nNC = 0, nSing = 0;	// number of primitive, non-prim and singleton concepts
	unsigned int nNoTold = 0;	// number of concepts w/o told subsumers

	// calculate statistic for all concepts
	for ( c_const_iterator pc = c_begin(); pc != c_end(); ++pc )
	{
		const TConcept* n = *pc;
		// check if concept is not relevant
		if ( !isValid(n->pName) )
			continue;

		if ( n->isPrimitive() )
			++nPC;
		else if ( n->isNonPrimitive() )
			++nNC;

		if ( n->isSynonym () )
			++nsFull;

		if ( n->isCompletelyDefined() )
		{
			if ( n->isPrimitive() )
				++npFull;
		}
		else
			if ( !n->hasToldSubsumers() )
				++nNoTold;
	}
	// calculate statistic for all individuals
	for ( i_const_iterator pi = i_begin(); pi != i_end(); ++pi )
	{
		const TConcept* n = *pi;
		// check if concept is not relevant
		if ( !isValid(n->pName) )
			continue;

		++nSing;

		if ( n->isPrimitive() )
			++nPC;
		else if ( n->isNonPrimitive() )
			++nNC;

		if ( n->isSynonym () )
			++nsFull;

		if ( n->isCompletelyDefined() )
		{
			if ( n->isPrimitive() )
				++npFull;
		}
		else
			if ( !n->hasToldSubsumers() )
				++nNoTold;
	}

	// FIXME!! check if we can skip all statistic if no logging needed
	CHECK_LL_RETURN(llAlways);
	LL << "There are " << nPC << " primitive concepts used\n";
	LL << " of which " << npFull << " completely defined\n";
	LL << "      and " << nNoTold << " has no told subsumers\n";
	LL << "There are " << nNC << " non-primitive concepts used\n";
	LL << " of which " << nsFull << " synonyms\n";
	LL << "There are " << nSing << " individuals or nominals used\n";
}

void TBox::RemoveExtraDescriptions ( void )
{
	// remove DLTree* from all named concepts
	for ( c_iterator pc = c_begin(); pc != c_end(); ++pc )
		(*pc)->removeDescription ();
	for ( i_iterator pi = i_begin(); pi != i_end(); ++pi )
		(*pi)->removeDescription ();
}

