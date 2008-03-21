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

#ifndef _DLTBOX_H
#define _DLTBOX_H

#include <string>
#include <vector>

#include "tConcept.h"
#include "tIndividual.h"
#include "RoleMaster.h"
#include "LogicFeature.h"
#include "dlDag.h"
#include "ifOptions.h"
#include "tRelated.h"
#include "tNECollection.h"
#include "tAxiomSet.h"
#include "DataTypeCenter.h"
#include "tProgressMonitor.h"
#include "tKBFlags.h"

class DlSatTester;
class DLConceptTaxonomy;
class dumpInterface;
class modelCacheSingleton;

class TBox
{
	friend class Precompletor;
	friend class DlSatTester;
	friend class ReasoningKernel;

public:	// interface
		/// type for DISJOINT-like statements
	typedef std::vector<DLTree*> ConceptSet;
		/// set of concepts together with creation methods
	typedef TNECollection<TConcept> ConceptCollection;
		/// vector of CONCEPT-like elements
	typedef std::vector<TConcept*> ConceptVector;
		/// vector of SINGLETON-like elements
	typedef std::vector<TIndividual*> SingletonVector;
		/// type for the array of Related elements
	typedef std::vector<TRelated*> RelatedCollection;
		/// type for a collection of DIFFERENT individuals
	typedef std::vector<SingletonVector> DifferentIndividuals;
		/// return type for a set of names
	typedef std::vector<TNamedEntry*> NamesVector;

protected:	// types
		/// collection of individuals
	class IndividualCollection: public TNECollection<TIndividual>
	{
	protected:	// methods
			/// virtual method for additional tuning of newly created element
		virtual void registerNew ( TIndividual* p ATTR_UNUSED ) {}

	public:		// interface
			/// c'tor: clear 0-th element
		IndividualCollection ( void ) : TNECollection<TIndividual>("individual") {}
			/// empty d'tor: all elements will be deleted in other place
		virtual ~IndividualCollection ( void ) {}
	}; // IndividualCollection

		/// class for simple rules like Ch :- Cb1, Cbi, CbN; all C are primitive named concepts
	class SimpleRule
	{
	public:		// members
			/// body of the rule
		TBox::ConceptVector Body;
			/// head of the rule
		const TConcept* Head;

	public:		// interface
			/// empty c'tor
		SimpleRule ( void ) {}
			/// create c'tor
		SimpleRule ( const TBox::ConceptVector& body, const TConcept* head ) : Body(body), Head(head) {}
			/// empty d'tor
		~SimpleRule ( void ) {}
	}; // SimpleRule

		/// all simple rules in KB
	typedef std::vector<SimpleRule> TSimpleRules;

		/// queue for n-ary operations
	class NAryQueue
	{
	protected:	// types
			/// interface with host
		typedef TBox::ConceptSet ConceptSet;
			/// type of a base storage
		typedef std::vector<ConceptSet*> BaseType;

	protected:	// members
			/// all lists of arguments for n-ary predicates/commands
		BaseType Base;
			/// pre-current index of n-ary statement
		int djLevel;

	protected:	// methods
			/// increase size of internal AUX array
		void grow ( void )
		{
			unsigned int n = Base.size();
			Base.resize(2*n);
			for ( BaseType::iterator p = Base.begin()+n, p_end = Base.end(); p < p_end; ++p )
				*p = new ConceptSet;
		}

	public:		// interface
			/// empty c'tor
		NAryQueue ( void ) : djLevel(-1) { Base.push_back(new ConceptSet); }
			/// d'tor
		~NAryQueue ( void )
		{
			for ( BaseType::iterator q = Base.begin(), q_end = Base.end(); q < q_end; ++q )
				delete *q;
		}

			/// init new n-ary queue
		void push ( void )
		{
			if ( (unsigned)++djLevel >= Base.size() )
				grow();
			Base[djLevel]->resize(0);
		}
			/// get access to the last n-ary queue
		ConceptSet* top ( void ) const { return Base[djLevel]; }
			/// get access to the last n-ary queue; remove the last queue
		const ConceptSet* pop ( void ) { return Base[djLevel--]; }
	}; // NAryQueue

protected:	// typedefs
		/// RW concept iterator
	typedef ConceptCollection::iterator c_iterator;
		/// RO concept iterator
	typedef ConceptCollection::const_iterator c_const_iterator;
		/// RW individual iterator
	typedef IndividualCollection::iterator i_iterator;
		/// RO individual iterator
	typedef IndividualCollection::const_iterator i_const_iterator;

protected:	// members
		/// aux arrays for processing n-ary statements
	NAryQueue auxConceptList;

	TLabeller relevance;

	DLDag DLHeap;

		/// reasoner for TBox-related queries w/o nominals
	DlSatTester* stdReasoner;
		/// reasoner for TBox-related queries with nominals
	DlSatTester* nomReasoner;
		/// progress monitor
	TProgressMonitor* pMonitor;

	/// taxonomy structure of a TBox
	DLConceptTaxonomy* pTax;
		/// DataType center
	DataTypeCenter DTCenter;
	/// set of reasoning options
	const ifOptionSet* pOptions;

		/// global KB features
	LogicFeatures KBFeatures;
		/// GCI features
	LogicFeatures GCIFeatures;
		/// nominal cloud features
	LogicFeatures NCFeatures;
		/// aux features
	LogicFeatures auxFeatures;
		/// pointer to current feature (in case of local ones)
	LogicFeatures* curFeature;

	// auxiliary concepts for Taxonomy
	TConcept* pTop;
	TConcept* pBottom;

	/// temporary concept
	TConcept* defConcept;

	/** all named concepts */
	ConceptCollection Concepts;
		/// all named individuals/nominals
	IndividualCollection Individuals;
	RoleMaster RM;
	TAxiomSet Axioms;
		/// given individual-individual relations
	RelatedCollection RelatedI;
		/// known disjoint sets of individuals
	DifferentIndividuals Different;
		/// all simple rules in KB
	TSimpleRules SimpleRules;

		/// internalisation of a general axioms
	BipolarPointer T_G;
		/// KB flags about GCIs
	TKBFlags GCIs;

	/////////////////////////////////////////////////////
	// Flags section
	/////////////////////////////////////////////////////
		/// flag for names skipping
	bool useAllNames;
		/// flag for full/short KB
	bool useRelevantOnly;
		/// flag for using native range and domain support
	bool useRangeDomain;
		/// flag for creating taxonomy
	bool useCompletelyDefined;
		/// flag for dumping TBox relevant to query
	bool dumpQuery;
		/// whether or not DAG cache is used
	bool useDagCache;
		/// whether or not we need classification. Set up in checkQueryNames()
	bool needClassification;
		/// shall we prefer C=D axioms to C[=E in definition of concepts
	bool alwaysPreferEquals;
		/// shall verbose output be used
	bool verboseOutput;
		/// whether we use sorted reasoning; depends on some simplifications
	bool useSortedReasoning;
		/// flag whether TBox is GALEN-like
	bool isLikeGALEN;
		/// flag whether TBox is WINE-like
	bool isLikeWINE;
		/// flag whether precompletion should be used
	bool usePrecompletion;

		/// flag whether consistency was checked
	bool consistencyChecked;
		/// whether KB is consistent; valid only if consistencyChecked is true
	bool Consistent;
		/// whether KB(ABox) is precompleted
	bool Precompleted;

		/// time spend for preprocessing
	float preprocTime;

	// other statistic

		/// number of synonyms encountered/changed
	unsigned int nSynonyms;

protected:	// methods
		/// init all flags using given set of options
	void readConfig ( const ifOptionSet* Options );
		/// initialise Top and Bottom internal concepts
	void initTopBottom ( void );


//-----------------------------------------------------------------------------
//--		internal iterators
//-----------------------------------------------------------------------------

		/// RW begin() for concepts
	c_iterator c_begin ( void ) { return Concepts.begin(); }
		/// RW end() for concepts
	c_iterator c_end ( void ) { return Concepts.end(); }
		/// RO begin() for concepts
	c_const_iterator c_begin ( void ) const { return Concepts.begin(); }
		/// RO end() for concepts
	c_const_iterator c_end ( void ) const { return Concepts.end(); }

		/// RW begin() for individuals
	i_iterator i_begin ( void ) { return Individuals.begin(); }
		/// RW end() for individuals
	i_iterator i_end ( void ) { return Individuals.end(); }
		/// RO begin() for individuals
	i_const_iterator i_begin ( void ) const { return Individuals.begin(); }
		/// RO end() for individuals
	i_const_iterator i_end ( void ) const { return Individuals.end(); }

//-----------------------------------------------------------------------------
//--		internal ensure*-like interface
//-----------------------------------------------------------------------------

		/// @return true if given name is registered as a concept-like structure in given TBox
	bool isRegisteredConcept ( const TNamedEntry* name ) const { return Concepts.isRegistered(name); }

		/// @return concept by given Named Entry ID
	TConcept* getConcept ( TNamedEntry* id ) { return static_cast<TConcept*>(id); }
		/// get TOP/BOTTOM/CN by the DLTree entry
	TConcept* getConcept ( const DLTree* name );
		/// @return individual by given Named Entry ID
	TIndividual* getIndividual ( TNamedEntry* id ) { return static_cast<TIndividual*>(id); }

//-----------------------------------------------------------------------------
//--		internal BP-to-concept interface
//-----------------------------------------------------------------------------

		/// set P as a concept corresponding BP
	void setBPforConcept ( BipolarPointer bp, TConcept* p )
	{
		DLHeap[bp].setConcept(p);
		p->pName = bp;
	}

		/// get concept by it's BP (non-const version)
	TConcept* getConceptByBP ( BipolarPointer bp )
	{
		TConcept* p = static_cast<TConcept*>(DLHeap[bp].getConcept());
		assert ( p != NULL );
		return p;
	}
		/// get concept by it's BP (const version)
	const TConcept* getConceptByBP ( BipolarPointer bp ) const
	{
		const TConcept* p = static_cast<const TConcept*>(DLHeap[bp].getConcept());
		assert ( p != NULL );
		return p;
	}

		/// get concept by it's BP (non-const version)
	TDataEntry* getDataEntryByBP ( BipolarPointer bp )
	{
		TDataEntry* p = static_cast<TDataEntry*>(DLHeap[bp].getConcept());
		assert ( p != NULL );
		return p;
	}
		/// get concept by it's BP (const version)
	const TDataEntry* getDataEntryByBP ( BipolarPointer bp ) const
	{
		const TDataEntry* p = static_cast<const TDataEntry*>(DLHeap[bp].getConcept());
		assert ( p != NULL );
		return p;
	}


//-----------------------------------------------------------------------------
//--		internal concept building interface
//-----------------------------------------------------------------------------

		/// add description to a concept; @return true in case of error
	bool initNonPrimitive ( TConcept* p, DLTree* desc )
	{
		if ( !p->canMakeNonPrim(desc) )
			return true;
		// delete return value in case of duplicated desc
		deleteTree(makeNonPrimitive(p,desc));
		return false;
	}
		/// make concept non-primitive; @return it's old description
	DLTree* makeNonPrimitive ( TConcept* p, DLTree* desc )
	{
		DLTree* ret = p->makeNonPrimitive(desc);
		checkEarlySynonym(p);
		return ret;
	}
	/// checks if C is defined as C=D and set Synonyms accordingly
	void checkEarlySynonym ( TConcept* p )
	{
		if ( p->isSynonym() )
			return;	// nothing to do
		if ( p->isPrimitive() )
			return;	// couldn't be a synonym
		if ( !isCN (p->Description) )
			return;	// complex expression -- not a synonym(imm.)

		p->setSynonym(getConcept(p->Description));
		p->initToldSubsumers();
		++nSynonyms;
	}

	/// remove concept from TBox by given EXTERNAL id. @return true in case of failure. WARNING!! tested only for TempConcept!!!
	bool removeConcept ( TConcept* p );

//-----------------------------------------------------------------------------
//--		support for n-ary predicates
//-----------------------------------------------------------------------------

		/// get access to the last used n-ary array
	const ConceptSet& getLastNAry ( void ) { return *auxConceptList.pop(); }

	// external-set methods for set-of-concept-names
	bool processEquivalent ( const ConceptSet& v );
	bool processDisjoint ( const ConceptSet& v );
	bool processEquivalentR ( const ConceptSet& v );
	bool processDisjointR ( const ConceptSet& v );
	bool processSame ( const ConceptSet& v );
	bool processDifferent ( const ConceptSet& v );
	DLTree* processAnd ( const ConceptSet& v );
	DLTree* processOr ( const ConceptSet& v );
	DLTree* processOneOf ( const ConceptSet& v );
	DLTree* processRComposition ( const ConceptSet& v );

		/// build a construction in the form AND (\neg q_i)
	template<class Iterator>
	DLTree* buildDisjAux ( Iterator beg, Iterator end )
	{
		DLTree* t = new DLTree(TOP);
		for ( Iterator i = beg; i < end; ++i )
			t = createSNFAnd ( t, createSNFNot(clone(*i)) );
		return t;
	}
		/// process a disjoint set [beg,end) in a usual manner
	template<class Iterator>
	bool processDisjoint ( Iterator beg, Iterator end )
	{
		for ( Iterator i = beg; i < end; ++i )
			if ( addSubsumeAxiom ( *i, buildDisjAux ( i+1, end ) ) )
				return true;
		return false;
	}
//-----------------------------------------------------------------------------
//--		internal DAG building methods
//-----------------------------------------------------------------------------

		/// build a DAG-structure for concepts and axioms
	void buildDAG ( void );

		/// translate concept P (together with definition) to DAG representation
	void addConceptToHeap ( TConcept* p );
		/// register data-related expression in the DAG; @return it's DAG index
	BipolarPointer addDataExprToHeap ( TDataEntry* p );
		/// builds DAG entry by general concept expression
	BipolarPointer tree2dag ( const DLTree* );
		/// build role (from RM) by given tree representation
	const TRole* role2dag ( const DLTree* t )
		{ TRole* r = resolveRole(t); return r ? resolveSynonym(r) : NULL; }
		/// create forall node (together with transitive sub-roles entries)
	BipolarPointer forall2dag ( const TRole* R, BipolarPointer C );
		/// create atmost node (together with NN-rule entries)
	BipolarPointer atmost2dag ( unsigned int n, const TRole* R, BipolarPointer C );
		/// create REFLEXIVE node
	BipolarPointer reflexive2dag ( const TRole* R )
	{
		// input check: only simple roles are allowed in the reflexivity construction
		if ( !R->isSimple() )
			throw EFPPNonSimpleRole(R->getName());
		return inverse ( DLHeap.add ( new DLVertex ( dtIrr, R ) ) );
	}
		/// fills AND-like vertex V with an AND-like expression T; process result
	BipolarPointer and2dag ( DLVertex* v, const DLTree* t );
		/// process AND-like expression T
	BipolarPointer and2dag ( const DLTree* t ) { return and2dag ( new DLVertex(dtAnd), t ); }
		/// add elements of T to and-like vertex V; @return true if clash occures
	bool fillANDVertex ( DLVertex* v, const DLTree* t );
		/// create forall node for data role
	BipolarPointer dataForall2dag ( const TRole* R, BipolarPointer C )
		{ return DLHeap.add ( new DLVertex ( dtForall, 0, R, C ) ); }
		/// create atmost node for data role
	BipolarPointer dataAtMost2dag ( unsigned int n, const TRole* R, BipolarPointer C )
		{ return DLHeap.add ( new DLVertex ( dtLE, n, R, C ) ); }
		/// @return a pointer to concept representation
	BipolarPointer concept2dag ( TConcept* p )
	{
		if ( p == NULL )
			return bpINVALID;
		if ( !isValid(p->pName) )
			addConceptToHeap(p);
		return p->resolveId();
	}

//-----------------------------------------------------------------------------
//--		internal parser (input) interface
//-----------------------------------------------------------------------------

		/// tries to add LEFT = RIGHT for the concept LEFT; @return true if OK
	bool addNonprimitiveDefinition ( DLTree* left, DLTree* right );
		/// tries to add LEFT = RIGHT for the concept LEFT [= X; @return true if OK
	bool switchToNonprimitive ( DLTree* left, DLTree* right );

	// for complex Concept operations
		/// try to absorb GCI C[=D; if not possible, just record this GCI
	bool processGCI ( DLTree* C, DLTree* D )
	{
		Axioms.addAxiom ( C, D );
		return false;
	}

	// recognize Range/Domain restriction in an axiom and transform it into role R&D.
	// return true if transformation was performed
	bool axiomToRangeDomain ( DLTree* l, DLTree* r );

	bool isNamedConcept ( BipolarPointer p ) const;

		/// check if TBox contains too many GCIs to switch strategy
	bool isGalenLikeTBox ( void ) const { return isLikeGALEN; }
		/// check if TBox contains too many nominals and GCIs to switch strategy
	bool isWineLikeTBox ( void ) const { return isLikeWINE; }

//-----------------------------------------------------------------------------
//--		internal preprocessing methods
//-----------------------------------------------------------------------------

		/// build a roles taxonomy and a DAG
	void Preprocess ( void );
		/// absorb all axioms and set hasGCI
	void ConvertAxioms ( void ) { GCIs.setGCI(Axioms.absorb()); }

		/// pre-process RELATED axioms: resolve synonyms, mark individuals as related
	void preprocessRelated ( void );
		/// perform precompletion;
	void performPrecompletion ( void );
		/// determine all sorts in KB (make job only for SORTED_REASONING)
	void determineSorts ( void );

		/// calculate statistic for DAG and Roles
	void CalculateStatistic ( void );
		/// Remove DLTree* from TConcept after DAG is constructed
	void RemoveExtraDescriptions ( void );

		/// init Range and Domain for all roles; sets hasGCI if R&D was found
	void initRangeDomain ( void );
		/// init functional roles with functional entries
	void initFunctionalRoles ( void );

		/// set told TOP concept whether necessary
	void initToldSubsumers ( void )
	{
		for ( c_iterator pc = c_begin(); pc != c_end(); ++pc )
			if ( !(*pc)->isSynonym() )
				(*pc)->initToldSubsumers();
		for ( i_iterator pi = i_begin(); pi != i_end(); ++pi )
			if ( !(*pi)->isSynonym() )
				(*pi)->initToldSubsumers();
	}
		/// set told TOP concept whether necessary
	void setToldTop ( void )
	{
		TConcept* top = const_cast<TConcept*>(pTop);
		for ( c_iterator pc = c_begin(); pc != c_end(); ++pc )
			(*pc)->setToldTop(top);
		for ( i_iterator pi = i_begin(); pi != i_end(); ++pi )
			(*pi)->setToldTop(top);
	}
		/// calculate TS depth for all concepts
	void calculateTSDepth ( void )
	{
		for ( c_iterator pc = c_begin(); pc != c_end(); ++pc )
			(*pc)->calculateTSDepth();
		for ( i_iterator pi = i_begin(); pi != i_end(); ++pi )
			(*pi)->calculateTSDepth();
	}

		/// find referential cycles (like A [= (and B C), B [= A) and transform them to synonyms (B=A, A[=C)
	void transformToldCycles ( void );
		/// check if P appears in referential cycle;
		/// @return concept which creates cycle, NULL if no such concept exists.
	TConcept* checkToldCycle ( TConcept* p );

		/// replace all synonyms in concept descriptions with their definitions
	void replaceAllSynonyms ( void );
		/// replace synonyms in concept expression with their definitions; @return true if DESC has been changed
	bool replaceSynonymsFromTree ( DLTree* desc );

		/// init Extra Rule field in concepts given by a vector V with a given INDEX
	inline void
	initRuleFields ( const ConceptVector& v, BipolarPointer index ) const
	{
		for ( ConceptVector::const_iterator q = v.begin(), q_end = v.end(); q < q_end; ++q )
			(*q)->addExtraRule(index);
	}
		/// mark all concepts wrt their classification tag
	void fillsClassificationTag ( void )
	{
		for ( c_const_iterator pc = c_begin(); pc != c_end(); ++pc )
			(*pc)->getClassTag();
		for ( i_const_iterator pi = i_begin(); pi != i_end(); ++pi )
			(*pi)->getClassTag();
	}

//-----------------------------------------------------------------------------
//--		internal reasoner-related interface
//-----------------------------------------------------------------------------

		/// get RW reasoner wrt nominal case
	DlSatTester* getReasoner ( void )
	{
		assert ( curFeature != NULL );
		if ( curFeature->hasSingletons() )
			return nomReasoner;
		else
			return stdReasoner;
	}
		/// prepare the reasoner to the new session
	void clearReasoner ( void );			// implemented in Reasoner.h
		/// set ToDo priorities using OPTIONS
	void setToDoPriorities ( void );		// implemented in Reasoner.h
		/// check whether KB is consistent; @return true if it is
	bool performConsistencyCheck ( void );	// implemented in Reasoner.h

//-----------------------------------------------------------------------------
//--		internal reasoning interface
//-----------------------------------------------------------------------------

		/// init cache for all named concepts
	void buildNamedConceptCache ( void );
		/// init reasoning service: create reasoner(s)
	void initReasoner ( void );				// implemented in Reasoner.h
		/// init priorities in order to do subsumption tests
	void prepareSubReasoning ( void )
	{
		DLHeap.setSubOrder();
		clearReasoner();
	}
		/// creating taxonomy for given TBox; include individuals if necessary
	void createTaxonomy ( bool needIndividuals );
		/// classify all concepts from given COLLECTION with given CD value
	void classifyConcepts ( const ConceptVector& collection, bool curCompletelyDefined, const char* type );

//-----------------------------------------------------------------------------
//--		internal cache-related methods
//-----------------------------------------------------------------------------

		/// init [singleton] cache for given concept implementation
	void initSingletonCache ( BipolarPointer p );	// implemented in Reasoner.h
		/// create cache for ~C where C is a primitive concept (as it is simple)
	void buildSimpleCache ( void )
	{
		/// set cache for TOP and BOTTOM entries
		initSingletonCache(bpBOTTOM);
		initSingletonCache(bpTOP);

		// inapplicable if KB contains CGIs in any form
		if ( GCIs.isGCI() || GCIs.isReflexive() )
			return;

		for ( c_const_iterator c = c_begin(), cend = c_end(); c < cend; ++c )
			if ( (*c)->isPrimitive() )
				initSingletonCache(inverse((*c)->pName));

		for ( i_const_iterator i = i_begin(), iend = i_end(); i < iend; ++i )
			if ( (*i)->isPrimitive() )
				initSingletonCache(inverse((*i)->pName));
	}

//-----------------------------------------------------------------------------
//--		internal output helper methods
//-----------------------------------------------------------------------------

	void PrintDagEntry ( std::ostream& o, BipolarPointer p ) const;
	void PrintDagEntrySR ( std::ostream& o, const TRole* p ) const;
	void PrintRoles ( std::ostream& o ) const;
		/// print one concept-like entry
	void PrintConcept ( std::ostream& o, const TConcept* p ) const;
		/// print all registered concepts
	void PrintConcepts ( std::ostream& o ) const
	{
		o << "Concepts (" << Concepts.size()-1 << "): \n";
		for ( c_const_iterator pc = c_begin(); pc != c_end(); ++pc )
			PrintConcept(o,*pc);
	}
		/// print all registered individuals
	void PrintIndividuals ( std::ostream& o ) const
	{
		o << "Individuals (" << Individuals.size()-1 << "): \n";
		for ( i_const_iterator pi = i_begin(); pi != i_end(); ++pi )
			PrintConcept(o,*pi);
	}
	void PrintSimpleRules ( std::ostream& o ) const
	{
		if ( SimpleRules.size() == 1 )
			return;
		o << "Simple rules (" << SimpleRules.size() << "): \n";
		for ( TSimpleRules::const_iterator p = SimpleRules.begin()+1; p != SimpleRules.end(); ++p )
		{
			ConceptVector::const_iterator q = p->Body.begin(), q_end = p->Body.end();
			o << "(" << (*q)->getName();
			for ( ++q; q < q_end; ++q )
				o << ", " << (*q)->getName();
			o << ") => " << p->Head->getName() << "\n";
		}
	}
	void PrintAxioms ( std::ostream& o ) const;

//-----------------------------------------------------------------------------
//--		internal relevance helper methods
//-----------------------------------------------------------------------------
		/// is given concept relevant wrt current TBox
	bool isRelevant ( const TConcept* p ) const { return p->isRelevant(relevance); }
		/// set given concept relevant wrt current TBox
	void setRelevant1 ( TConcept* p );
		/// set given concept relevant wrt current TBox if not checked yet
	void setRelevant ( TConcept* p ) { if ( !isRelevant(p) ) setRelevant1(p); }

		/// is given role relevant wrt current TBox
	bool isRelevant ( const TRole* p ) const { return p->isRelevant(relevance); }
		/// set given role relevant wrt current TBox
	void setRelevant1 ( TRole* p );
		/// set given role relevant wrt current TBox if not checked yet
	void setRelevant ( TRole* p ) { if ( !isRelevant(p) ) setRelevant1(p); }

		/// set given DAG entry relevant wrt current TBox
	void setRelevant ( BipolarPointer p );

		/// gather information about logical features of relevant concept
	void collectLogicFeature ( const TConcept* p ) const
	{
		if ( curFeature )
			curFeature->fillConceptData(p);
	}
		/// gather information about logical features of relevant role
	void collectLogicFeature ( const TRole* p ) const
	{
		if ( curFeature )	// update features w.r.t. current concept
			curFeature->fillRoleData ( p, isRelevant(p->inverse()) );
	}
		/// gather information about logical features of relevant DAG entry
	void collectLogicFeature ( const DLVertex& v, bool pos ) const
	{
		if ( curFeature )
			curFeature->fillDAGData ( v, pos );
	}
		/// mark all active GCIs relevant
	void markGCIsRelevant ( void ) { setRelevant(T_G); }

//-----------------------------------------------------------------------------
//--		internal relevance interface
//-----------------------------------------------------------------------------
		/// set all TBox content (namely, concepts and GCIs) relevant
	void markAllRelevant ( void )
	{
		for ( c_iterator pc = c_begin(); pc != c_end(); ++pc )
			setRelevant(*pc);
		for ( i_iterator pi = i_begin(); pi != i_end(); ++pi )
			setRelevant(*pi);

		markGCIsRelevant();
	}
		/// mark chosen part of TBox (P, Q and GCIs) relevant
	void calculateRelevant ( TConcept* p, TConcept* q )
	{
		setRelevant(p);
		if ( q != NULL )
			setRelevant(q);
		markGCIsRelevant();
	}
		/// clear all relevance info
	void clearRelevanceInfo ( void ) { relevance.newLab(); }
		/// gather relevance statistic for the whole KB
	void gatherRelevanceInfo ( void );
		/// put relevance information to a concept's data
	void setConceptRelevant ( TConcept* p )
	{
		curFeature = &p->posFeatures;
		setRelevant(p->pBody);
		KBFeatures |= p->posFeatures;
		collectLogicFeature(p);
		clearRelevanceInfo();

		// nothing to do for neg-prim concepts
		if ( p->isPrimitive() )
			return;

		curFeature = &p->negFeatures;
		setRelevant(inverse(p->pBody));
		KBFeatures |= p->negFeatures;
		clearRelevanceInfo();
	}
		/// update AUX features with the given one; update roles if necessary
	void updateAuxFeatures ( const LogicFeatures& lf )
	{
		if ( !lf.empty() )
		{
			auxFeatures |= lf;
			auxFeatures.mergeRoles();
		}
	}
		/// prepare features for SAT(P), or SUB(P,Q) test
	void prepareFeatures ( const TConcept* pConcept, const TConcept* qConcept )
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
		DlCompletionTree::setBlockingMethod(isIRinQuery());
	}
		/// clear current features
	void clearFeatures ( void ) { curFeature = NULL; }

//-----------------------------------------------------------------------------
//--		internal dump output interface
//-----------------------------------------------------------------------------
		/// dump concept-like essence using given dump method
	void dumpConcept ( dumpInterface* dump, const TConcept* p ) const;
		/// dump role-like essence using given dump method
	void dumpRole ( dumpInterface* dump, const TRole* p ) const;
		/// dump general concept expression using given dump method
	void dumpExpression ( dumpInterface* dump, BipolarPointer p ) const;
		/// dump all (relevant) roles
	void dumpAllRoles ( dumpInterface* dump ) const;

public:
	/// C'tor
	TBox ( const ifOptionSet* Options );
	/// D'tor
	~TBox ( void );

		/// get RW access to used Role Master
	RoleMaster* getRM ( void ) { return &RM; }
		/// get RO access to used Role Master
	const RoleMaster* getRM ( void ) const { return &RM; }

//-----------------------------------------------------------------------------
//--		public parser ensure* interface
//-----------------------------------------------------------------------------

		/// return registered concept by given NAME; @return NULL if can't register
	TConcept* getConcept ( const std::string& name ) { return Concepts.get(name); }
		/// return registered individual by given NAME; @return NULL if can't register
	TIndividual* getIndividual ( const std::string& name ) { return Individuals.get(name); }

	/** Ensure that given name is nominal. Register name if it is undefined.
		@return true if name could not be defined as a nominal; @return false if OK */
	bool ensureNominal ( TNamedEntry* name ) { return !Individuals.isRegistered(name); }
		/// ensure that given concept expression is a nominal; @return false if OK
	bool ensureNominal ( const DLTree* entry )
	{
		if ( entry->Element().getToken() != INAME )
			return true;
		return ensureNominal(entry->Element().getName());
	}

		/// get unique aux concept
	TConcept* getAuxConcept ( void );

//-----------------------------------------------------------------------------
//--		public parser (input) interface
//-----------------------------------------------------------------------------

		/// set the flag that forbid usage of undefined names for concepts/roles; @return old value
	bool setForbidUndefinedNames ( bool val )
	{
		RM.setUndefinedNames(!val);
		DTCenter.setLocked(val);
		Individuals.setLocked(val);
		return Concepts.setLocked(val);
	}

	bool RegisterInstance ( TNamedEntry* name, DLTree* Desc )
		{ return ensureNominal(name) || addSubsumeAxiom ( name, Desc ); }
	bool RegisterIndividualRelation ( TNamedEntry* a, TNamedEntry* R, TNamedEntry* b )
	{
		if ( ensureNominal(a) || ensureNominal(b) )
			return true;
		RelatedI.push_back ( new
			TRelated ( static_cast<TIndividual*>(a),
					   static_cast<TIndividual*>(b),
					   static_cast<TRole*>(R) ) );
		RelatedI.push_back ( new
			TRelated ( static_cast<TIndividual*>(b),
					   static_cast<TIndividual*>(a),
					   static_cast<TRole*>(R)->inverse() ) );
		return false;
	}

	bool addSubsumeAxiom ( DLTree* left, DLTree* right );
	bool addSubsumeAxiom ( TNamedEntry* C, DLTree* right );	// special form: CN [= D
	bool addEqualityAxiom ( DLTree* left, DLTree* right );

	// different ConceptList methods
	bool openConceptList ( void )
	{
		auxConceptList.push();
		return false;
	}

	bool contConceptList ( DLTree* name )
	{
		if ( name == NULL )
			return true;
		auxConceptList.top()->push_back(name);
		return false;
	}

	// internal-set versions of the same methods
	bool processEquivalent ( void ) { return processEquivalent(getLastNAry()); }
	bool processDisjoint ( void ) { return processDisjoint(getLastNAry()); }
	bool processEquivalentR ( void ) { return processEquivalentR(getLastNAry()); }
	bool processDisjointR ( void ) { return processDisjointR(getLastNAry()); }
	bool processSame ( void ) { return processSame(getLastNAry()); }
	bool processDifferent ( void ) { return processDifferent(getLastNAry()); }
	DLTree* processAnd ( void ) { return processAnd(getLastNAry()); }
	DLTree* processOr ( void ) { return processOr(getLastNAry()); }
	DLTree* processOneOf ( void ) { return processOneOf(getLastNAry()); }
	DLTree* processRComposition ( void ) { return processRComposition(getLastNAry()); }

//-----------------------------------------------------------------------------
//--		public access interface
//-----------------------------------------------------------------------------

		/// GCI Axioms access
	BipolarPointer getTG ( void ) const { return T_G; }
		/// get head if the INDEX'th simple rule
	BipolarPointer getExtraRuleHead ( BipolarPointer index ) const { return SimpleRules[index].Head->pName; }

		/// check if the relevant part of KB contains inverse roles.
	bool isIRinQuery ( void ) const
	{
		if ( curFeature != NULL )
			return curFeature->hasInverseRole();
		else
			return KBFeatures.hasInverseRole();
	}
		/// check if the relevant part of KB contains singletons
	bool testHasNominals ( void ) const
	{
		if ( curFeature != NULL )
			return curFeature->hasSingletons();
		else
			return KBFeatures.hasSingletons();
	}
		/// check if Sorted Reasoning is applicable
	bool canUseSortedReasoning ( void ) const
		{ return useSortedReasoning && !GCIs.isGCI() && !GCIs.isReflexive(); }

//-----------------------------------------------------------------------------
//--		public reasoning interface
//-----------------------------------------------------------------------------
		/// prepare to reasoning
	void prepareReasoning ( void );
		/// perform classification (assuming KB is consistent)
	void performClassification ( void ) { createTaxonomy ( /*needIndividuals=*/false ); }
		/// perform realisation (assuming KB is consistent)
	void performRealisation ( void ) { createTaxonomy ( /*needIndividuals=*/true ); }

	/// get (READ-WRITE) access to internal Taxonomy of concepts
	DLConceptTaxonomy* getTaxonomy ( void ) { return pTax; }
	/// get (READ-ONLY) access to internal Taxonomy of concepts
	const DLConceptTaxonomy* getTaxonomy ( void ) const { return pTax; }

		/// get (RW) access to the DataType center
	DataTypeCenter& getDataTypeCenter ( void ) { return DTCenter; }
		/// get (RO) access to the DataType center
	const DataTypeCenter& getDataTypeCenter ( void ) const { return DTCenter; }

		/// set given structure as a progress monitor
	void setProgressMonitor ( TProgressMonitor* pMon ) { delete pMonitor; pMonitor = pMon; }
		/// check that reasoning progress was cancelled by external application
	bool isCancelled ( void ) const { return pMonitor != NULL && pMonitor->isCancelled(); }
		/// set verbose output (ie, default progress monitor, concept and role taxonomies
	void useVerboseOutput ( void ) { verboseOutput = true; }

	/// create (and DAG-ify) temporary concept via its definition
	TConcept* createTempConcept ( const DLTree* desc );
	/// classify temporary concept
	bool classifyTempConcept ( void );

//-----------------------------------------------------------------------------
//--		public reasoning interface
//-----------------------------------------------------------------------------

		/// inform that KB is precompleted
	void setPrecompleted ( void ) { Precompleted = true; }
		/// if KB is precompleted
	bool isPrecompleted ( void ) const { return Precompleted; }
		/// set consistency flag
	void setConsistency ( bool val )
	{
		consistencyChecked = true;
		Consistent = val;
	}
		/// check if the ontology is consistent
	bool isConsistent ( void )
	{
		if ( !consistencyChecked )
			setConsistency(performConsistencyCheck());
		return Consistent;
	}
		/// check if a subsumption C [= D holds
	bool isSubHolds ( const TConcept* C, const TConcept* D );
		/// check if a concept C is satisfiable
	bool isSatisfiable ( const TConcept* C );

		/// fills cache entry for given concept; set up SAT flag to a concept
	const modelCacheInterface* initCache ( TConcept* pConcept );

		/// test if 2 concept non-subsumption can be determined by cache merging
	enum modelCacheState testCachedNonSubsumption ( const TConcept* p, const TConcept* q );
		/// test if 2 concept non-subsumption can be determined by sorts checking
	bool testSortedNonSubsumption ( const TConcept* p, const TConcept* q )
	{
		// sorted reasoning doesn't work in presence of GCIs
		if ( !canUseSortedReasoning() )
			return false;
		// doesn't work for the SAT tests
		if ( q == NULL )
			return false;
		return !DLHeap.haveSameSort ( p->pName, q->pName );
	}


//-----------------------------------------------------------------------------
//--		public output interface
//-----------------------------------------------------------------------------

		/// dump query processing TIME, reasoning statistics and a (preprocessed) TBox
	void writeReasoningResult ( std::ostream& o, float time, bool isConsistent ) const;
	std::ostream& Print ( std::ostream& o ) const;
	std::ostream& PrintStat ( std::ostream& o ) const;

		/// create dump of relevant part of query using given method
	void dump ( dumpInterface* dump ) const;

		/// implement DIG-like roleFillers query; @return in Js all J st (I,J):R
	void getRoleFillers ( TIndividual* I, TRole* R, NamesVector& Js ) const;
		/// implement DIG-like RelatedIndividuals query; @return Is and Js st (I,J):R
	void getRelatedIndividuals ( TRole* R, NamesVector& Is, NamesVector& Js ) const;
		/// implement absorbedPrimitiveConceptDefinitions DIG extension
	void absorbedPrimitiveConceptDefinitions ( std::ostream& o ) const;
		/// implement unabsorbed DIG extension
	void unabsorbed ( std::ostream& o ) const;
}; // TBox

/**
  * Implementation of TBox class
  */

inline TBox :: TBox ( const ifOptionSet* Options )
	: DLHeap(Options)
	, stdReasoner(NULL)
	, nomReasoner(NULL)
	, pMonitor(NULL)
	, pTax (NULL)
	, pOptions (Options)
	, curFeature(NULL)
	, defConcept (NULL)
	, Concepts("concept")
	, Axioms(*this)
	, T_G(bpTOP)	// initialise GCA's concept with Top
	, useSortedReasoning(true)
	, isLikeGALEN(false)	// just in case Relevance part would be omited
	, isLikeWINE(false)
	, consistencyChecked(false)
	, Precompleted(false)
	, preprocTime(0)
	, nSynonyms(0)
{
	readConfig ( Options );
	initTopBottom ();
	SimpleRules.push_back(SimpleRule());	// dummy for 0th element

	setForbidUndefinedNames(false);
}

//---------------------------------------------------------------
//--		Implementation of ensure*-like stuff
//---------------------------------------------------------------

inline TConcept* TBox :: getConcept ( const DLTree* name )
{
	if ( name->Element() == TOP )
		return pTop;
	if ( name->Element() == BOTTOM )
		return pBottom;

	if ( !isName(name) )
		return NULL;

	return getConcept(name->Element().getName());
}

//---------------------------------------------------------------
//--		access to concept by BiPointer
//---------------------------------------------------------------

inline bool TBox :: isNamedConcept ( BipolarPointer bp ) const
{
	DagTag tag = DLHeap[bp].Type();
	return isCNameTag(tag) || tag == dtDataType || tag == dtDataValue;
}

//---------------------------------------------------------------
//--		misc inline implementation
//---------------------------------------------------------------

inline std::ostream& TBox :: Print ( std::ostream& o ) const
{
	DLHeap.PrintStat (o);
	RM.Print (o);
	PrintConcepts (o);
	PrintIndividuals(o);
	PrintSimpleRules(o);
	PrintAxioms (o);
	DLHeap.Print(o);
	return o;
}

#endif
