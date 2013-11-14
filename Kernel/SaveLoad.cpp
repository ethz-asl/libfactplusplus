/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2008-2013 by Dmitry Tsarkov

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

//-------------------------------------------------------
//-- Saving/restoring internal state of the FaCT++
//-------------------------------------------------------

#include "Kernel.h"
#include "ReasonerNom.h"	// for initReasoner()
#include "SaveLoadManager.h"

using namespace std;

const char* ReasoningKernel :: InternalStateFileHeader = "FaCT++InternalStateDump1.0";

const int bytesInInt = sizeof(int);

#if 0
// FIXME!! try to avoid recursion later on
static inline
void saveUIntAux ( ostream& o, unsigned int n, const int rest )
{
	if ( rest > 1 )
		saveUIntAux ( o, n/256, rest-1 );
	m.o() << (unsigned char)n%256;
}

static inline
void saveUInt ( ostream& o, unsigned int n )
{
	saveUIntAux ( o, n, bytesInInt );
}

static inline
void saveSInt ( ostream& o, int n ) { saveUInt(o,n); }

static inline
unsigned int loadUInt ( istream& i )
{
	static unsigned int ret = 0;
	unsigned char byte;
	for ( int j = bytesInInt-1; j >= 0; --j )
	{
		i >> byte;
		ret *= 256;
		ret += byte;
	}
	return ret;
}

static inline
int loadSInt ( istream& i ) { return (int)loadUInt(i); }
#endif	// 0

//----------------------------------------------------------
//-- Global structures used for the methods below
//----------------------------------------------------------

template<class Pointer>
class PointerMap
{
protected:	// types
		/// map int->pointer type
	typedef std::vector<Pointer> I2PMap;
		/// map pointer->int type
	typedef std::map<Pointer, unsigned int> P2IMap;

protected:	// members
		/// map i -> pointer
	I2PMap i2p;
		/// map pointer -> i
	P2IMap p2i;
		/// ID of the last recorded NE
	unsigned int last;

protected:	// methods
		/// @return true if given pointer present in the map
	bool in ( const Pointer p ) const { return p2i.find(p) != p2i.end(); }
		/// @return true if given index present in the map
	bool in ( unsigned int i ) const { return i < last; }
		/// @throw an exception if P is not registered
	void ensure ( const Pointer p ) const { if ( !in(p) ) throw EFPPSaveLoad("Cannot save unregistered pointer"); }
		/// @throw an exception if I is not registered
	void ensure ( unsigned int i ) const { if ( !in(i) ) throw EFPPSaveLoad("Cannot load unregistered index"); }

public:		// interface
		/// empty c'tor
	PointerMap ( void ) : last(0) {}
		/// empty d'tor
	~PointerMap ( void ) { clear(); }
		/// clear the maps
	void clear ( void )
	{
		i2p.clear();
		p2i.clear();
		last = 0;
	}

	// populate the map

		/// add an entry
	void add ( Pointer p )
	{
		if ( in(p) )
			return;
		i2p.push_back(p);
		p2i[p] = last++;
	}
		/// add entries from the [begin,end) range
	template<class Iterator>
	void add ( Iterator begin, Iterator end )
	{
		for ( ; begin != end; ++begin )
			add(*begin);
	}

	// access to the mapped element

		/// get the NE by index I
	Pointer getP ( unsigned int i ) { ensure(i); return i2p[i]; }
		/// get the index by NE P
	unsigned int getI ( const Pointer p ) { ensure(p); return p2i[p]; }
}; // PointerMap


// int -> named entity map for the current taxonomy
PointerMap<TNamedEntity*> eMap;
// int -> named entry map for the current taxonomy
PointerMap<TNamedEntry*> neMap;
// uint -> TaxonomyVertex map to update the taxonomy
PointerMap<TaxonomyVertex*> tvMap;

inline void regPointer ( const TNamedEntry* p )
{
	neMap.add(const_cast<TNamedEntry*>(p));
	if ( p->getEntity() != NULL )
		eMap.add(const_cast<TNamedEntity*>(p->getEntity()));
}

//----------------------------------------------------------
//-- Implementation of the Kernel methods (Kernel.h)
//----------------------------------------------------------

#undef CHECK_FILE_STATE
#define CHECK_FILE_STATE() if ( !m.o().good() ) throw(EFPPSaveLoad(name,/*save=*/true))

void
ReasoningKernel :: Save ( SaveLoadManager& m, const char* name ) const
{
	TsProcTimer t;
	t.Start();
	CHECK_FILE_STATE();
	SaveHeader(m);
	CHECK_FILE_STATE();
	SaveOptions(m);
	CHECK_FILE_STATE();
	SaveKB(m);
	CHECK_FILE_STATE();
	SaveIncremental(m);
	CHECK_FILE_STATE();
	t.Stop();
	std::cout << "Reasoner internal state saved in " << t << " sec" << std::endl;
}

void
ReasoningKernel :: Save ( void )
{
	fpp_assert ( pSLManager != NULL );
	pSLManager->prepare(/*input=*/false);
	Save(*pSLManager);
}

#undef CHECK_FILE_STATE
#define CHECK_FILE_STATE() if ( !m.i().good() ) throw(EFPPSaveLoad(name,/*save=*/false))

void
ReasoningKernel :: Load ( SaveLoadManager& m, const char* name )
{
	TsProcTimer t;
	t.Start();
	CHECK_FILE_STATE();
//	releaseKB();	// we'll start a new one if necessary
	if ( LoadHeader(m) )
		throw(EFPPSaveLoad(name,/*save=*/false));
	CHECK_FILE_STATE();
	LoadOptions(m);
	CHECK_FILE_STATE();
	LoadKB(m);
	CHECK_FILE_STATE();
	LoadIncremental(m);
	CHECK_FILE_STATE();
	t.Stop();
	std::cout << "Reasoner internal state loaded in " << t << " sec" << std::endl;
}

void
ReasoningKernel :: Load ( void )
{
	fpp_assert ( pSLManager != NULL );
	pSLManager->prepare(/*input=*/true);
	Load(*pSLManager);
}

//-- save/load header (Kernel.h)

void
ReasoningKernel :: SaveHeader ( SaveLoadManager& m ) const
{
	m.o() << InternalStateFileHeader << "\n" << Version << "\n" << bytesInInt << "\n";
}

bool
ReasoningKernel :: LoadHeader ( SaveLoadManager& m )
{
	string str;
	m.i() >> str;
	if ( str != InternalStateFileHeader )
		return true;
	m.i() >> str;
	// FIXME!! we don't check version equivalence for now
//	if ( str != Version )
//		return true;
	int n;
	m.i() >> n;
	if ( n != bytesInInt )
		return true;
	return false;
}

//-- save/load options (Kernel.h)

void
ReasoningKernel :: SaveOptions ( SaveLoadManager& m ) const
{
	m.o() << "Options\n";
}

void
ReasoningKernel :: LoadOptions ( SaveLoadManager& m )
{
	std::string options;
	m.i() >> options;
}

//-- save/load KB (Kernel.h)

void
ReasoningKernel :: SaveKB ( SaveLoadManager& m ) const
{
	m.saveUInt((unsigned int)getStatus());
	switch ( getStatus() )
	{
	case kbEmpty:	// nothing to do
		return;
	case kbLoading:
		throw EFPPSaveLoad("Can't save internal state of the unclassified reasoner");
	default:
		getTBox()->Save(m);
		break;
	}
}

void
ReasoningKernel :: LoadKB ( SaveLoadManager& m )
{
	KBStatus status = (KBStatus)m.loadUInt();
//	initCacheAndFlags();	// will be done
	// no classification => no need to monitor
	pMonitor = NULL;
	if ( status == kbEmpty )
		return;
//	newKB();
	getTBox()->Load(m,status);
}

//----------------------------------------------------------
//-- Helpers: Save/Load of class TNECollection
//----------------------------------------------------------

/// Save all the objects in the collection
template<class T>
static void
SaveTNECollection ( const TNECollection<T>& collection, SaveLoadManager& m, const std::set<const TNamedEntry*>& excluded )
{
	typename TNECollection<T>::const_iterator p, p_beg = collection.begin(), p_end = collection.end();
	// get the max length of the identifier in the collection
	unsigned int maxLength = 0, curLength;

	for ( p = p_beg; p < p_end; ++p )
		if ( /*excluded.count(*p) == 0 && */maxLength < (curLength = strlen((*p)->getName())) )
			maxLength = curLength;

	// save number of entries and max length of the entry
	m.saveUInt(collection.size());
	m.saveUInt(maxLength);

	// save names of all entries
	for ( p = p_beg; p < p_end; ++p )
	{
		// register all entries in the global map
		regPointer(*p);
//		if ( excluded.count(*p) == 0 )
			m.o() << (*p)->getName() << "\n";
	}

	// save the entries itself
//	for ( p = p_beg; p < p_end; ++p )
//		(*p)->Save(o);
}
/// Load all the objects into the collection
template<class T>
static void
LoadTNECollection ( TNECollection<T>& collection, SaveLoadManager& m )
{
	// sanity check: Load shall be done for the empty collection and only once
//	fpp_assert ( size() == 0 );

	unsigned int collSize, maxLength;
	collSize = m.loadUInt();
	maxLength = m.loadUInt();
	++maxLength;
	char* name = new char[maxLength];

	// register all the named entries
	for ( unsigned int j = 0; j < collSize; ++j )
	{
		m.i().getline ( name, maxLength, '\n' );
		regPointer(collection.get(name));
	}

	delete [] name;

	// load all the named entries
//	for ( iterator p = begin(); p < end(); ++p )
//		(*p)->Load(i);
}

//----------------------------------------------------------
//-- Helpers: Save/Load of class RoleMaster
//----------------------------------------------------------

static void
SaveRoleMaster ( const RoleMaster& RM, SaveLoadManager& m )
{
	RoleMaster::const_iterator p, p_beg = RM.begin(), p_end = RM.end();
	// get the max length of the identifier in the collection
	unsigned int maxLength = 0, curLength, size = 0;

	for ( p = p_beg; p != p_end; p += 2, size++ )
		if ( maxLength < (curLength = strlen((*p)->getName())) )
			maxLength = curLength;

	// save number of entries and max length of the entry
	m.saveUInt(size);
	m.saveUInt(maxLength);

	// register const entries in the global map
	regPointer(RM.getBotRole());
	regPointer(RM.getTopRole());

	// save names of all (non-inverse) entries
	for ( p = p_beg; p != p_end; p += 2 )
	{
		regPointer(*p);
		m.o() << (*p)->getName() << "\n";
	}

//	// save the entries itself
//	for ( p = p_beg; p < p_end; ++p )
//		(*p)->Save(o);
//
//	// save the rest of the RM
//	o << "\nRT";
//	pTax->Save(o);
}

static void
LoadRoleMaster ( RoleMaster& RM, SaveLoadManager& m )
{
	// sanity check: Load shall be done for the empty collection and only once
//	fpp_assert ( size() == 0 );

	unsigned int RMSize, maxLength;
	RMSize = m.loadUInt();
	maxLength = m.loadUInt();
	++maxLength;
	char* name = new char[maxLength];

	// register const entries in the global map
	regPointer(RM.getBotRole());
	regPointer(RM.getTopRole());

	// register all the named entries
	for ( unsigned int j = 0; j < RMSize; ++j )
	{
		m.i().getline ( name, maxLength, '\n' );
		regPointer(RM.ensureRoleName(name));
	}

	delete [] name;

//	// load all the named entries
//	for ( iterator p = begin(); p < end(); ++p )
//		(*p)->Load(i);
//
//	// load the rest of the RM
//	expectChar(i,'R');
//	expectChar(i,'T');
//	pTax = new Taxonomy ( &universalRole, &emptyRole );
//	pTax->Load(i);
//	useUndefinedNames = false;	// no names
}

//----------------------------------------------------------
//-- Implementation of the DLDag methods (dlDag.h)
//----------------------------------------------------------

void
DLDag :: Save ( SaveLoadManager& m ) const
{
	m.saveUInt(finalDagSize);
	m.o() << "\n";
	// skip fake vertex and TOP
	for ( unsigned int i = 2; i < finalDagSize; ++i )
		Heap[i]->Save(m);
}

void
DLDag :: Load ( SaveLoadManager& m )
{
	unsigned int j, size;
	size = m.loadUInt();
	for ( j = 2; j < size; ++j )
	{
		DagTag tag = static_cast<DagTag>(m.loadUInt());
		DLVertex* v = new DLVertex(tag);
		v->Load(m);
		directAdd(v);
	}

	// only reasoning now -- no cache
	setFinalSize();
}

/// @return true if the DAG in the SL structure is the same that is loaded
bool
DLDag :: Verify ( SaveLoadManager& m ) const
{
	unsigned int j, size;
	size = m.loadUInt();

	if ( size != finalDagSize )
	{
		std::cout << "DAG verification fail: size " << size << ", expected " << finalDagSize << "\n";
		return false;
	}

	for ( j = 2; j < size; ++j )
	{
		DagTag tag = static_cast<DagTag>(m.loadUInt());
		DLVertex* v = new DLVertex(tag);
		v->Load(m);
		if ( *v != *(Heap[j]) )
		{
			std::cout << "DAG verification fail: dag entry at " << j << " is ";
			v->Print(std::cout);
			std::cout << ", expected ";
			Heap[j]->Print(std::cout);
			std::cout << "\n";
			delete v;
			return false;
		}
		delete v;
	}

	return true;
}

static void
SaveSingleCache ( SaveLoadManager& m, BipolarPointer bp, const modelCacheInterface* cache )
{
	if ( cache == NULL )
		return;
	m.saveSInt(bp);
	m.saveUInt(cache->getCacheType());
	switch ( cache->getCacheType() )
	{
	case modelCacheInterface::mctConst:
		m.saveUInt(cache->getState() == csValid);
		break;

	case modelCacheInterface::mctSingleton:
		m.saveSInt(dynamic_cast<const modelCacheSingleton*>(cache)->getValue());
		break;

	case modelCacheInterface::mctIan:
		dynamic_cast<const modelCacheIan*>(cache)->Save(m);
		break;

	default:
		fpp_unreachable();
	}
	m.o() << "\n";
}

static const modelCacheInterface*
LoadSingleCache ( SaveLoadManager& m )
{
	modelCacheState state = (modelCacheState)m.loadUInt();
	switch ( state )
	{
	case modelCacheInterface::mctConst:
		return new modelCacheConst ( m.loadUInt() != 0 );
	case modelCacheInterface::mctSingleton:
		return new modelCacheSingleton(m.loadSInt());
	case modelCacheInterface::mctIan:
	{
		bool hasNominals = bool(m.loadUInt());
		unsigned int nC = m.loadUInt();
		unsigned int nR = m.loadUInt();
		modelCacheIan* cache = new modelCacheIan ( hasNominals, nC, nR );
		cache->Load(m);
		return cache;
	}

	default:
		fpp_unreachable();
	}
}

void
DLDag :: SaveCache ( SaveLoadManager& m ) const
{
	m.o() << "\nDC";	// dag cache
	for ( unsigned int i = 2; i < finalDagSize; ++i )
	{
		DLVertex* v = Heap[i];
		SaveSingleCache ( m, i, v->getCache(true) );
		SaveSingleCache ( m, -i, v->getCache(false) );
	}
	m.saveUInt(0);
}

void
DLDag :: LoadCache ( SaveLoadManager& m )
{
	m.expectChar('D');
	m.expectChar('C');
	BipolarPointer bp = m.loadSInt();
	while ( bp != 0 )
	{
		setCache ( bp, LoadSingleCache(m) );
		bp = m.loadSInt();
	}
}


//----------------------------------------------------------
//-- Implementation of the TBox methods (dlTBox.h)
//----------------------------------------------------------

void
TBox :: Save ( SaveLoadManager& m ) const
{
	tvMap.clear();
	neMap.clear();
	neMap.add(pBottom);
	neMap.add(pTop);
	neMap.add(pTemp);
	neMap.add(pQuery);
	// datatypes
//	TreeDeleter Bool(TreeDeleter(DTCenter.getBoolType()));
	neMap.add(DTCenter.getStringType()->Element().getNE());
	neMap.add(DTCenter.getNumberType()->Element().getNE());
	neMap.add(DTCenter.getRealType()->Element().getNE());
	neMap.add(DTCenter.getBoolType()->Element().getNE());
	neMap.add(DTCenter.getTimeType()->Element().getNE());
	neMap.add(DTCenter.getFreshDataType()->Element().getNE());
	m.o() << "\nC";
	std::set<const TNamedEntry*> empty;
	SaveTNECollection(Concepts,m,empty);
	m.o() << "\nI";
	SaveTNECollection(Individuals,m,empty);
	m.o() << "\nOR";
	SaveRoleMaster(ORM,m);
	m.o() << "\nDR";
	SaveRoleMaster(DRM,m);
	m.o() << "\nD";
	DLHeap.Save(m);
	if ( Status > kbCChecked )
	{
		m.o() << "\nCT";
		pTax->Save(m,empty);
	}
	DLHeap.SaveCache(m);
}

void
TBox :: Load ( SaveLoadManager& m, KBStatus status )
{
	Status = status;
	tvMap.clear();
	neMap.clear();
	neMap.add(pBottom);
	neMap.add(pTop);
	neMap.add(pTemp);
	neMap.add(pQuery);
	// datatypes
	neMap.add(DTCenter.getStringType()->Element().getNE());
	neMap.add(DTCenter.getNumberType()->Element().getNE());
	neMap.add(DTCenter.getRealType()->Element().getNE());
	neMap.add(DTCenter.getBoolType()->Element().getNE());
	neMap.add(DTCenter.getTimeType()->Element().getNE());
	neMap.add(DTCenter.getFreshDataType()->Element().getNE());
	m.expectChar('C');
	LoadTNECollection(Concepts,m);
	m.expectChar('I');
	LoadTNECollection(Individuals,m);
	m.expectChar('O');
	m.expectChar('R');
	LoadRoleMaster(ORM,m);
	m.expectChar('D');
	m.expectChar('R');
	LoadRoleMaster(DRM,m);
	m.expectChar('D');
//	DLHeap.Load(m);
	if ( !DLHeap.Verify(m) )
		throw EFPPSaveLoad("DAG verification failed");
//	initReasoner();
	if ( Status > kbCChecked )
	{
		initTaxonomy();
		pTaxCreator->setBottomUp(GCIs);
		m.expectChar('C');
		m.expectChar('T');
		pTax->Load(m);
	}
	DLHeap.LoadCache(m);
}

void
TBox :: SaveTaxonomy ( SaveLoadManager& m, const std::set<const TNamedEntry*>& excluded )
{
	tvMap.clear();
	neMap.clear();
	neMap.add(pBottom);
	neMap.add(pTop);
	neMap.add(pTemp);
	neMap.add(pQuery);
	m.o() << "\nC";
	SaveTNECollection(Concepts,m,excluded);
	m.o() << "\nI";
	SaveTNECollection(Individuals,m,excluded);
	m.o() << "\nCT";
	pTax->Save(m,excluded);
}

void
TBox :: LoadTaxonomy ( SaveLoadManager& m )
{
	tvMap.clear();
	neMap.clear();
	neMap.add(pBottom);
	neMap.add(pTop);
	neMap.add(pTemp);
	neMap.add(pQuery);
	m.expectChar('C');
	LoadTNECollection(Concepts,m);
	m.expectChar('I');
	LoadTNECollection(Individuals,m);
	initTaxonomy();
	pTaxCreator->setBottomUp(GCIs);
	m.expectChar('C');
	m.expectChar('T');
	pTax->Load(m);
}

//----------------------------------------------------------
//-- Save/Load incremental structures (Kernel.h)
//----------------------------------------------------------

void
ReasoningKernel :: SaveIncremental ( SaveLoadManager& m ) const
{
	if ( !useIncrementalReasoning )
		return;
	m.o() << "\nQ";
	m.saveUInt(Name2Sig.size());
	for ( NameSigMap::const_iterator p = Name2Sig.begin(), p_end = Name2Sig.end(); p != p_end; ++p )
	{
		m.saveUInt(eMap.getI(const_cast<TNamedEntity*>(p->first)));
		m.saveUInt(p->second->size());

		for ( TSignature::iterator q = p->second->begin(), q_end = p->second->end(); q != q_end; ++q )
			m.saveUInt(eMap.getI(const_cast<TNamedEntity*>(*q)));
	}
}

void
ReasoningKernel :: LoadIncremental ( SaveLoadManager& m )
{
	if ( !useIncrementalReasoning )
		return;
	m.expectChar('Q');
	Name2Sig.clear();
	unsigned int size = m.loadUInt();
	for ( unsigned int j = 0; j < size; j++ )
	{
		TNamedEntity* entity = eMap.getP(m.loadUInt());
		unsigned int sigSize = m.loadUInt();
		TSignature* sig = new TSignature();
		for ( unsigned int k = 0; k < sigSize; k++ )
			sig->add(eMap.getP(m.loadUInt()));
		Name2Sig[entity] = sig;
	}
}

//----------------------------------------------------------
//-- Implementation of the TNamedEntry methods (tNamedEntry.h)
//----------------------------------------------------------

void
TNamedEntry :: Save ( SaveLoadManager& m ) const
{
	m.saveUInt(getAllFlags());
}

void
TNamedEntry :: Load ( SaveLoadManager& m )
{
	setAllFlags(m.loadUInt());
}

//----------------------------------------------------------
//-- Implementation of the TConcept methods (tConcept.h)
//----------------------------------------------------------

void
TConcept :: Save ( SaveLoadManager& m ) const
{
	ClassifiableEntry::Save(m);
	m.saveUInt((unsigned int)classTag);
	m.saveUInt(tsDepth);
	m.saveSInt(pName);
	m.saveSInt(pBody);
	m.saveUInt(posFeatures.getAllFlags());
	m.saveUInt(negFeatures.getAllFlags());
//	ERSet.Save(m);
}

void
TConcept :: Load ( SaveLoadManager& m )
{
	ClassifiableEntry::Load(m);
	classTag = CTTag(m.loadUInt());
	tsDepth = m.loadUInt();
	pName = m.loadSInt();
	pBody = m.loadSInt();
	posFeatures.setAllFlags(m.loadUInt());
	negFeatures.setAllFlags(m.loadUInt());
//	ERSet.Load(m);
}

//----------------------------------------------------------
//-- Implementation of the TIndividual methods (tIndividual.h)
//----------------------------------------------------------

void
TIndividual :: Save ( SaveLoadManager& m ) const
{
	TConcept::Save(m);
//	RelatedIndex.Save(m);
}

void
TIndividual :: Load ( SaveLoadManager& m )
{
	TConcept::Load(m);
//	RelatedIndex.Load(m);
}

//----------------------------------------------------------
//-- Implementation of the TRole methods (tRole.h)
//----------------------------------------------------------

void
TRole :: Save ( SaveLoadManager& m ) const
{
	ClassifiableEntry::Save(m);
	// FIXME!! think about automaton
}

void
TRole :: Load ( SaveLoadManager& m )
{
	ClassifiableEntry::Load(m);
	// FIXME!! think about automaton
}

//----------------------------------------------------------
//-- Implementation of the TaxonomyVertex methods (taxVertex.h)
//----------------------------------------------------------

void
TaxonomyVertex :: SaveLabel ( SaveLoadManager& m ) const
{
	m.saveUInt(neMap.getI(const_cast<ClassifiableEntry*>(sample)));
	m.saveUInt(synonyms.size());
	for ( syn_iterator p = begin_syn(), p_end = end_syn(); p < p_end; ++p )
		m.saveUInt(neMap.getI(const_cast<ClassifiableEntry*>(*p)));
	m.o() << "\n";
}

void
TaxonomyVertex :: LoadLabel ( SaveLoadManager& m )
{
	// note that sample is already loaded
	unsigned int size = m.loadUInt();
	for ( unsigned int j = 0; j < size; ++j )
		addSynonym(static_cast<ClassifiableEntry*>(neMap.getP(m.loadUInt())));
}

void
TaxonomyVertex :: SaveNeighbours ( SaveLoadManager& m ) const
{
	const_iterator p, p_end;
	m.saveUInt(neigh(true).size());
	for ( p = begin(true), p_end = end(true); p != p_end; ++p )
		m.saveUInt(tvMap.getI(*p));
	m.saveUInt(neigh(false).size());
	for ( p = begin(false), p_end = end(false); p != p_end; ++p )
		m.saveUInt(tvMap.getI(*p));
	m.o() << "\n";
}

void
TaxonomyVertex :: LoadNeighbours ( SaveLoadManager& m )
{
	unsigned int j, size;
	size = m.loadUInt();
	for ( j = 0; j < size; ++j )
		addNeighbour ( true, tvMap.getP(m.loadUInt()) );
	size = m.loadUInt();
	for ( j = 0; j < size; ++j )
		addNeighbour ( false, tvMap.getP(m.loadUInt()) );
}

//----------------------------------------------------------
//-- Implementation of the Taxonomy methods (Taxonomy.h)
//----------------------------------------------------------

void
Taxonomy :: Save ( SaveLoadManager& m, const std::set<const TNamedEntry*>& excluded ) const
{
	TaxVertexVec::const_iterator p, p_beg = Graph.begin(), p_end = Graph.end();
	tvMap.clear();	// it would be it's own map for every taxonomy
	tvMap.add ( p_beg, p_end );

	// save number of taxonomy elements
	m.saveUInt(Graph.size()/*-excluded.size()*/);
	m.o() << "\n";

	// save labels for all verteces of the taxonomy
	for ( p = p_beg; p != p_end; ++p )
//		if ( excluded.count((*p)->getPrimer()) == 0 )
			(*p)->SaveLabel(m);

	// save the taxonomys hierarchy
	for ( p = p_beg; p != p_end; ++p )
//		if ( excluded.count((*p)->getPrimer()) == 0 )
			(*p)->SaveNeighbours(m);
}

void
Taxonomy :: Load ( SaveLoadManager& m )
{
	unsigned int size = m.loadUInt();
	tvMap.clear();
	Graph.clear();	// both TOP and BOTTOM elements would be load;

	// create all the verteces and load their labels
	for ( unsigned int j = 0; j < size; ++j )
	{
		ClassifiableEntry* p = static_cast<ClassifiableEntry*>(neMap.getP(m.loadUInt()));
		TaxonomyVertex* v = new TaxonomyVertex(p);
		Graph.push_back(v);
		v->LoadLabel(m);
		tvMap.add(v);
	}

	// load the hierarchy
	for ( TaxVertexVec::iterator p = Graph.begin(), p_end = Graph.end(); p < p_end; ++p )
		(*p)->LoadNeighbours(m);
}

//----------------------------------------------------------
//-- Implementation of the modelCacheIan methods (modelCacheIan.h)
//----------------------------------------------------------

static void
SaveIndexSet ( SaveLoadManager& m, const TSetAsTree& Set )
{
	for ( TSetAsTree::const_iterator p = Set.begin(), p_end = Set.end(); p != p_end; ++p )
		m.saveUInt(*p);
	m.saveUInt(0);
}

static void
LoadIndexSet ( SaveLoadManager& m, TSetAsTree& Set )
{
	unsigned int n = m.loadUInt();
	while ( n != 0 )
	{
		Set.insert(n);
		n = m.loadUInt();
	}
}

void
modelCacheIan :: Save ( SaveLoadManager& m ) const
{
	// header: hasNominals, nC, nR
	m.saveUInt(hasNominalNode);
	m.saveUInt(posDConcepts.maxSize());
	m.saveUInt(existsRoles.maxSize());
	// the body that will be loaded
	SaveIndexSet(m,posDConcepts);
	SaveIndexSet(m,posNConcepts);
	SaveIndexSet(m,negDConcepts);
	SaveIndexSet(m,negNConcepts);
#ifdef RKG_USE_SIMPLE_RULES
	SaveIndexSet(o,extraDConcepts);
	SaveIndexSet(o,extraNConcepts);
#endif
	SaveIndexSet(m,existsRoles);
	SaveIndexSet(m,forallRoles);
	SaveIndexSet(m,funcRoles);
	m.saveUInt(curState);
}

void
modelCacheIan :: Load ( SaveLoadManager& m )
{
	// note that nominals, nC and nR already read, and all sets are created
	LoadIndexSet(m,posDConcepts);
	LoadIndexSet(m,posNConcepts);
	LoadIndexSet(m,negDConcepts);
	LoadIndexSet(m,negNConcepts);
#ifdef RKG_USE_SIMPLE_RULES
	LoadIndexSet(m,extraDConcepts);
	LoadIndexSet(m,extraNConcepts);
#endif
	LoadIndexSet(m,existsRoles);
	LoadIndexSet(m,forallRoles);
	LoadIndexSet(m,funcRoles);
	curState = (modelCacheState) m.loadUInt();
}

//----------------------------------------------------------
//-- Implementation of the DLVertex methods (dlVertex.h)
//----------------------------------------------------------

void
DLVertex :: Save ( SaveLoadManager& m ) const
{
	m.saveUInt(static_cast<unsigned int>(Type()));

	switch ( Type() )
	{
	case dtBad:
	case dtTop:		// can't be S/L
	default:
		fpp_unreachable();
		break;

	case dtAnd:
		m.saveUInt(Child.size());
		for ( const_iterator p = begin(); p != end(); ++p )
			m.saveSInt(*p);
		break;

	case dtLE:
		m.saveUInt(neMap.getI(const_cast<TRole*>(Role)));
		m.saveSInt(getC());
		m.saveUInt(getNumberLE());
		break;

	case dtForall:	// n here is for the automaton state
		m.saveUInt(neMap.getI(const_cast<TRole*>(Role)));
		m.saveSInt(getC());
		m.saveUInt(getNumberLE());
		break;

	case dtIrr:
		m.saveUInt(neMap.getI(const_cast<TRole*>(Role)));
		break;

	case dtPConcept:
	case dtNConcept:
	case dtPSingleton:
	case dtNSingleton:
		m.saveUInt(neMap.getI(static_cast<TConcept*>(Concept)));
		m.saveSInt(getC());
		break;

	case dtNN:	// nothing to do
		break;

	case dtDataType:
	case dtDataValue:
		m.saveUInt(neMap.getI(Concept));
		m.saveSInt(getC());
		break;

	case dtDataExpr:
		break;	// FIXME!! for now
	}
	m.o() << "\n";
}

void
DLVertex :: Load ( SaveLoadManager& m )
{
	// now OP is already loaded
	switch ( Type() )
	{
	case dtBad:
	case dtTop:		// can't be S/L
	default:
		fpp_unreachable();
		break;

	case dtAnd:
	{
		unsigned int size = m.loadUInt();
		for ( unsigned int j = 0; j < size; ++j )
			Child.push_back(m.loadSInt());
		break;
	}

	case dtLE:
		Role = static_cast<const TRole*>(neMap.getP(m.loadUInt()));
		setChild(m.loadSInt());
		n = m.loadUInt();
		break;

	case dtForall:
		Role = static_cast<const TRole*>(neMap.getP(m.loadUInt()));
		setChild(m.loadSInt());
		n = m.loadUInt();
		break;

	case dtIrr:
		Role = static_cast<const TRole*>(neMap.getP(m.loadUInt()));
		break;

	case dtPConcept:
	case dtNConcept:
	case dtPSingleton:
	case dtNSingleton:
		setConcept(neMap.getP(m.loadUInt()));
		setChild(m.loadSInt());
		break;

	case dtNN:	// nothing to do
		break;

	case dtDataType:
	case dtDataValue:
		setConcept(neMap.getP(m.loadUInt()));
		setChild(m.loadSInt());
		break;

	case dtDataExpr:
		break;	// FIXME!! for now
	}
}
