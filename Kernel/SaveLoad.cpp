/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2008 by Dmitry Tsarkov

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

//-------------------------------------------------------
//-- Saving/restoring internal state of the FaCT++
//-------------------------------------------------------

#include <istream>

#include "eFPPSaveLoad.h"
#include "Kernel.h"

const char* ReasoningKernel :: InternalStateFileHeader = "FaCT++InternalStateDump1.0";

const int bytesInInt = sizeof(int);

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
	o << "KB\n";
}

void
TBox :: Load ( istream& i, KBStatus status )
{
	Status = status;
	string KB;
	i >> KB;
}
