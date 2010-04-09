/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2008-2010 by Dmitry Tsarkov

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

#include <istream>

#include "eFPPSaveLoad.h"
#include "Kernel.h"

using namespace std;

const char* ReasoningKernel :: InternalStateFileHeader = "FaCT++InternalStateDump1.0";

const int bytesInInt = sizeof(int);

static inline
void expectChar ( istream& i, const char C )
{
	char c;
	i >> c;
	if ( c != C )
		throw EFPPSaveLoad(C);
}

#if 0
// FIXME!! try to avoid recursion later on
static inline
void saveUIntAux ( ostream& o, unsigned int n, const int rest )
{
	if ( rest > 1 )
		saveUIntAux ( o, n/256, rest-1 );
	o << (unsigned char)n%256;
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
#else
static inline
void saveUInt ( ostream& o, unsigned int n )
{
	o << "(" << n << ")";
}

static inline
void saveSInt ( ostream& o, int n )
{
	o << "(" << n << ")";
}

static inline
unsigned int loadUInt ( istream& i )
{
	unsigned int ret;
	expectChar(i,'(');
	i >> ret;
	expectChar(i,')');
	return ret;
}

static inline
int loadSInt ( istream& i )
{
	int ret;
	expectChar(i,'(');
	i >> ret;
	expectChar(i,')');
	return ret;
}

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
	bool in ( Pointer p ) const { return p2i.find(p) != p2i.end(); }
		/// @return true if given index present in the map
	bool in ( unsigned int i ) const { return i < last; }
		/// @throw an exception if P is not registered
	void ensure ( Pointer p ) const { if ( !in(p) ) throw EFPPSaveLoad("Cannot save unregistered pointer"); }
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
	unsigned int getI ( Pointer p ) { ensure(p); return p2i[p]; }
}; // PointerMap

// int -> named entry map for the current taxonomy
PointerMap<ClassifiableEntry*> neMap;
// uint -> TaxonomyVertex map to update the taxonomy
PointerMap<TaxonomyVertex*> tvMap;

//----------------------------------------------------------
//-- Implementation of the Kernel methods (Kernel.h)
//----------------------------------------------------------

#undef CHECK_FILE_STATE
#define CHECK_FILE_STATE() if ( !o.good() ) throw(EFPPSaveLoad(name,/*save=*/true))

void
ReasoningKernel :: Save ( const char* name ) const
{
	std::ofstream o(name);
	CHECK_FILE_STATE();
	SaveHeader(o);
	CHECK_FILE_STATE();
	SaveOptions(o);
	CHECK_FILE_STATE();
	SaveKB(o);
	CHECK_FILE_STATE();
}

#undef CHECK_FILE_STATE
#define CHECK_FILE_STATE() if ( !i.good() ) throw(EFPPSaveLoad(name,/*save=*/false))

void
ReasoningKernel :: Load ( const char* name )
{
	std::ifstream i(name);
	CHECK_FILE_STATE();
	releaseKB();	// we'll start a new one if necessary
	if ( LoadHeader(i) )
		throw(EFPPSaveLoad(name,/*save=*/false));
	CHECK_FILE_STATE();
	LoadOptions(i);
	CHECK_FILE_STATE();
	LoadKB(i);
	CHECK_FILE_STATE();
}

//-- save/load header (Kernel.h)

void
ReasoningKernel :: SaveHeader ( ostream& o ) const
{
	o << InternalStateFileHeader << "\n" << Version << "\n" << bytesInInt << "\n";
}

bool
ReasoningKernel :: LoadHeader ( istream& i )
{
	string str;
	i >> str;
	if ( str != InternalStateFileHeader )
		return true;
	i >> str;
	// FIXME!! we don't check version equivalence for now
//	if ( str != Version )
//		return true;
	int n;
	i >> n;
	if ( n != bytesInInt )
		return true;
	return false;
}

//-- save/load options (Kernel.h)

void
ReasoningKernel :: SaveOptions ( ostream& o ) const
{
	o << "Options\n";
}

void
ReasoningKernel :: LoadOptions ( istream& i )
{
	std::string options;
	i >> options;
}

//-- save/load KB (Kernel.h)

void
ReasoningKernel :: SaveKB ( ostream& o ) const
{
	saveUInt(o,(unsigned int)getStatus());
	switch ( getStatus() )
	{
	case kbEmpty:	// nothing to do
		return;
	case kbLoading:
		throw EFPPSaveLoad("Can't load internal state of the unclassified reasoner");
	default:
		getTBox()->Save(o);
		break;
	}
}

void
ReasoningKernel :: LoadKB ( istream& i )
{
	KBStatus status = (KBStatus)loadUInt(i);
	initCacheAndFlags();	// will be done
	if ( status == kbEmpty )
		return;
	newKB();
	getTBox()->Load(i,status);
}

//----------------------------------------------------------
//-- Implementation of the TBox methods (dlTBox.h)
//----------------------------------------------------------

void
TBox :: Save ( ostream& o ) const
{
	tvMap.clear();
	neMap.clear();
	neMap.add(pBottom);
	neMap.add(pTop);
	o << "\nC";
	Concepts.Save(o);
	o << "\nI";
	Individuals.Save(o);
	o << "\nOR";
	ORM.Save(o);
	o << "\nDR";
	DRM.Save(o);
	o << "\nD";
	DLHeap.Save(o);
	if ( Status > kbCChecked )
	{
		o << "\nCT";
		pTax->Save(o);
	}
}

void
TBox :: Load ( istream& i, KBStatus status )
{
	Status = status;
	string KB;
	tvMap.clear();
	neMap.clear();
	neMap.add(pBottom);
	neMap.add(pTop);
	expectChar(i,'C');
	Concepts.Load(i);
	expectChar(i,'I');
	Individuals.Load(i);
	expectChar(i,'O');
	expectChar(i,'R');
	ORM.Load(i);
	expectChar(i,'D');
	expectChar(i,'R');
	DRM.Load(i);
	expectChar(i,'D');
	DLHeap.Load(i);
	if ( Status > kbCChecked )
	{
		initTaxonomy();
		expectChar(i,'C');
		expectChar(i,'T');
		pTax->Load(i);
	}
}

//----------------------------------------------------------
//-- Implementation of the TNECollection methods (tNECollection.h)
//----------------------------------------------------------

/// Save all the objects in the collection
template<class T>
void
TNECollection<T> :: Save ( ostream& o ) const
{
	const_iterator p, p_beg = begin(), p_end = end();
	// get the max length of the identifier in the collection
	unsigned int maxLength = 0, curLength;

	for ( p = p_beg; p < p_end; ++p )
		if ( maxLength < (curLength = strlen((*p)->getName())) )
			maxLength = curLength;

	// save number of entries and max length of the entry
	saveUInt(o,size());
	saveUInt(o,maxLength);

	// register all entries in the global map
	neMap.add ( p_beg, p_end );

	// save names of all entries
	for ( p = p_beg; p < p_end; ++p )
		o << (*p)->getName() << "\n";

	// save the entries itself
	for ( p = p_beg; p < p_end; ++p )
		(*p)->Save(o);
}
/// Load all the objects into the collection
template<class T>
void
TNECollection<T> :: Load ( istream& i )
{
	// sanity check: Load shall be done for the empty collection and only once
	fpp_assert ( size() == 0 );

	unsigned int collSize, maxLength;
	collSize = loadUInt(i);
	maxLength = loadUInt(i);
	++maxLength;
	char* name = new char[maxLength];

	// register all the named entries
	for ( unsigned int j = 0; j < collSize; ++j )
	{
		i.getline ( name, maxLength, '\n' );
		get(name);
	}

	delete [] name;

	// register all entries in the global map
	neMap.add ( begin(), end() );

	// load all the named entries
	for ( iterator p = begin(); p < end(); ++p )
		(*p)->Load(i);
}

//----------------------------------------------------------
//-- Implementation of the TNamedEntry methods (tNamedEntry.h)
//----------------------------------------------------------

void
TNamedEntry :: Save ( ostream& o ) const
{
	saveUInt(o,getAllFlags());
}

void
TNamedEntry :: Load ( istream& i )
{
	setAllFlags(loadUInt(i));
}

//----------------------------------------------------------
//-- Implementation of the TConcept methods (tConcept.h)
//----------------------------------------------------------

void
TConcept :: Save ( ostream& o ) const
{
	ClassifiableEntry::Save(o);
	saveUInt(o,(unsigned int)classTag);
	saveUInt(o,tsDepth);
	saveSInt(o,pName);
	saveSInt(o,pBody);
	saveUInt(o,posFeatures.getAllFlags());
	saveUInt(o,negFeatures.getAllFlags());
//	ERSet.Save(o);
}

void
TConcept :: Load ( istream& i )
{
	ClassifiableEntry::Load(i);
	classTag = CTTag(loadUInt(i));
	tsDepth = loadUInt(i);
	pName = loadSInt(i);
	pBody = loadSInt(i);
	posFeatures.setAllFlags(loadUInt(i));
	negFeatures.setAllFlags(loadUInt(i));
//	ERSet.Load(i);
}

//----------------------------------------------------------
//-- Implementation of the TIndividual methods (tIndividual.h)
//----------------------------------------------------------

void
TIndividual :: Save ( ostream& o ) const
{
	TConcept::Save(o);
//	RelatedIndex.Save(o);
}

void
TIndividual :: Load ( istream& i )
{
	TConcept::Load(i);
//	RelatedIndex.Load(i);
}

//----------------------------------------------------------
//-- Implementation of the RoleMaster methods (RoleMaster.h)
//----------------------------------------------------------

void
RoleMaster :: Save ( ostream& o ) const
{
	const_iterator p, p_beg = begin(), p_end = end();
	// get the max length of the identifier in the collection
	unsigned int maxLength = 0, curLength;

	for ( p = p_beg; p < p_end; p += 2 )
		if ( maxLength < (curLength = strlen((*p)->getName())) )
			maxLength = curLength;

	// save number of entries and max length of the entry
	saveUInt(o,size());
	saveUInt(o,maxLength);

	// register all entries in the global map
	neMap.add(const_cast<ClassifiableEntry*>((const ClassifiableEntry*)&emptyRole));
	neMap.add(const_cast<ClassifiableEntry*>((const ClassifiableEntry*)&universalRole));
	neMap.add ( p_beg, p_end );

	// save names of all (non-inverse) entries
	for ( p = p_beg; p < p_end; p += 2 )
		o << (*p)->getName() << "\n";

	// save the entries itself
	for ( p = p_beg; p < p_end; ++p )
		(*p)->Save(o);

	// save the rest of the RM
	o << "\nRT";
	pTax->Save(o);
}

void
RoleMaster :: Load ( istream& i )
{
	// sanity check: Load shall be done for the empty collection and only once
	fpp_assert ( size() == 0 );

	unsigned int RMSize, maxLength;
	RMSize = loadUInt(i);
	maxLength = loadUInt(i);
	++maxLength;
	char* name = new char[maxLength];

	// register all the named entries
	for ( unsigned int j = 0; j < RMSize; ++j )
	{
		i.getline ( name, maxLength, '\n' );
		ensureRoleName(name);
	}

	delete [] name;

	// register all entries in the global map
	neMap.add(&emptyRole);
	neMap.add(&universalRole);
	neMap.add ( begin(), end() );

	// load all the named entries
	for ( iterator p = begin(); p < end(); ++p )
		(*p)->Load(i);

	// load the rest of the RM
	expectChar(i,'R');
	expectChar(i,'T');
	pTax->Load(i);
	useUndefinedNames = false;	// no names
}

//----------------------------------------------------------
//-- Implementation of the TRole methods (tRole.h)
//----------------------------------------------------------

void
TRole :: Save ( ostream& o ) const
{
	ClassifiableEntry::Save(o);
}

void
TRole :: Load ( istream& i )
{
	ClassifiableEntry::Load(i);
}

//----------------------------------------------------------
//-- Implementation of the Taxonomy methods (Taxonomy.h)
//----------------------------------------------------------

void
Taxonomy :: Save ( ostream& o ) const
{
	const_iterator p, p_beg = begin(), p_end = end();
	tvMap.clear();	// it would be it's own map for every taxonomy
	tvMap.add ( p_beg, p_end );

	// save number of taxonomy elements
	saveUInt(o,Graph.size());
	o << "\n";

	// save labels for all verteces of the taxonomy
	for ( p = p_beg; p != p_end; ++p )
		(*p)->SaveLabel(o);

	// save the taxonomys hierarchy
	for ( p = p_beg; p != p_end; ++p )
		(*p)->SaveNeighbours(o);
}

void
Taxonomy :: Load ( istream& i )
{
	unsigned int size = loadUInt(i);
	tvMap.clear();
	Graph.clear();	// both TOP and BOTTOM elements would be load;

	// create all the verteces and load their labels
	for ( unsigned int j = 0; j < size; ++j )
	{
		ClassifiableEntry* p = neMap.getP(loadUInt(i));
		TaxonomyVertex* v = new TaxonomyVertex(p);
		Graph.push_back(v);
		v->LoadLabel(i);
		tvMap.add(v);
	}

	// load the hierarchy
	for ( iterator p = begin(), p_end = end(); p < p_end; ++p )
		(*p)->LoadNeighbours(i);
}

//----------------------------------------------------------
//-- Implementation of the TaxonomyVertex methods (taxVertex.h)
//----------------------------------------------------------

void
TaxonomyVertex :: SaveLabel ( ostream& o ) const
{
	saveUInt(o,neMap.getI(const_cast<ClassifiableEntry*>(sample)));
	saveUInt(o,synonyms.size());
	for ( syn_iterator p = begin_syn(), p_end = end_syn(); p < p_end; ++p )
		saveUInt(o,neMap.getI(const_cast<ClassifiableEntry*>(*p)));
	o << "\n";
}

void
TaxonomyVertex :: LoadLabel ( istream& i )
{
	// note that sample is already loaded
	unsigned int size = loadUInt(i);
	for ( unsigned int j = 0; j < size; ++j )
		addSynonym(neMap.getP(loadUInt(i)));
}

void
TaxonomyVertex :: SaveNeighbours ( ostream& o ) const
{
	const_iterator p, p_end;
	saveUInt(o,neigh(true).size());
	for ( p = begin(true), p_end = end(true); p < p_end; ++p )
		saveUInt(o,tvMap.getI(*p));
	saveUInt(o,neigh(false).size());
	for ( p = begin(false), p_end = end(false); p < p_end; ++p )
		saveUInt(o,tvMap.getI(*p));
	o << "\n";
}

void
TaxonomyVertex :: LoadNeighbours ( istream& i )
{
	unsigned int j, size;
	size = loadUInt(i);
	for ( j = 0; j < size; ++j )
		addNeighbour ( true, tvMap.getP(loadUInt(i)) );
	size = loadUInt(i);
	for ( j = 0; j < size; ++j )
		addNeighbour ( false, tvMap.getP(loadUInt(i)) );
}

//----------------------------------------------------------
//-- Implementation of the DLDag methods (dlDag.h)
//----------------------------------------------------------

void
DLDag :: Save ( ostream& o ) const
{
	saveUInt(o,Heap.size());
	o << "\n";
	// skip fake vertex and TOP
	for ( unsigned int i = 2; i < Heap.size(); ++i )
		Heap[i]->Save(o);
}

void
DLDag :: Load ( istream& i )
{
	unsigned int j, size;
	size = loadUInt(i);
	for ( j = 2; j < size; ++j )
	{
		DagTag tag = static_cast<DagTag>(loadUInt(i));
		DLVertex* v = new DLVertex(tag);
		v->Load(i);
		directAdd(v);	// FIXME!! think about ..AndCache
	}

	// only reasoning now -- no cache
	useDLVCache = false;
}

//----------------------------------------------------------
//-- Implementation of the DLVertex methods (dlVertex.h)
//----------------------------------------------------------

void
DLVertex :: Save ( ostream& o ) const
{
	saveUInt(o,static_cast<unsigned int>(Type()));
	// FIXME!! save Cache if applicable; this will help reasoning

	switch ( Type() )
	{
	case dtBad:
	case dtTop:		// can't be S/L
	default:
		fpp_unreachable();
		break;

	case dtAnd:
	case dtCollection:
		saveUInt(o,Child.size());
		for ( const_iterator p = begin(); p != end(); ++p )
			saveSInt(o,*p);
		break;

	case dtUAll:
		saveSInt(o,getC());
		break;

	case dtLE:
		saveUInt(o,neMap.getI(const_cast<TRole*>(Role)));
		saveSInt(o,getC());
		saveUInt(o,getNumberLE());
		break;

	case dtForall:
		saveUInt(o,neMap.getI(const_cast<TRole*>(Role)));
		saveSInt(o,getC());
		break;

	case dtIrr:
		saveUInt(o,neMap.getI(const_cast<TRole*>(Role)));
		break;

	case dtPConcept:
	case dtNConcept:
	case dtPSingleton:
	case dtNSingleton:
		saveUInt(o,neMap.getI(static_cast<TConcept*>(Concept)));
		saveSInt(o,getC());
		break;

	case dtDataType:
	case dtDataValue:
	case dtDataExpr:
		break;	// FIXME!! for now
	}
	o << "\n";
}

void
DLVertex :: Load ( istream& i )
{
	// now OP is already saved
	// FIXME!! load Cache if applicable; this will help reasoning
	switch ( Type() )
	{
	case dtBad:
	case dtTop:		// can't be S/L
	default:
		fpp_unreachable();
		break;

	case dtAnd:
	case dtCollection:
	{
		unsigned int size = loadUInt(i);
		for ( unsigned int j = 0; j < size; ++j )
			Child.push_back(loadSInt(i));
		break;
	}

	case dtUAll:
		setChild(loadSInt(i));
		break;

	case dtLE:
		Role = static_cast<const TRole*>(neMap.getP(loadUInt(i)));
		setChild(loadSInt(i));
		Child.push_back(loadUInt(i));
		break;

	case dtForall:
		Role = static_cast<const TRole*>(neMap.getP(loadUInt(i)));
		setChild(loadSInt(i));
		break;

	case dtIrr:
		Role = static_cast<const TRole*>(neMap.getP(loadUInt(i)));
		break;

	case dtPConcept:
	case dtNConcept:
	case dtPSingleton:
	case dtNSingleton:
		setConcept(neMap.getP(loadUInt(i)));
		setChild(loadSInt(i));
		break;

	case dtDataType:
	case dtDataValue:
	case dtDataExpr:
		break;	// FIXME!! for now
	}
}
