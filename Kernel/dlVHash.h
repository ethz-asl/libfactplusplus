/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2006-2008 by Dmitry Tsarkov

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

#ifndef _DLVHASH_H
#define _DLVHASH_H

#include <map>
#include <list>
#include "dlVertex.h"
#include "tRole.h"

/// naive and simple hash table for DL Verteces
class dlVHashTable
{
protected:	// types
		/// hashed IDs of the DL verteces
	typedef std::list<BipolarPointer> HashLeaf;
		/// hash table by itself
	typedef std::map<BipolarPointer, HashLeaf> HashTable;

protected:	// members
		/// host DAG that contains actual nodes;
	const DLDag& host;
		/// HT for nodes
	HashTable Table;

protected:	// methods
		/// get a (very simple) hash of the vertex
	static BipolarPointer hash ( const DLVertex& v )
	{
		BipolarPointer sum = 0;
		if ( v.getRole() != NULL )
			sum += v.getRole()->getId();
		sum += v.getC();
		sum += v.getNumberLE();
		for ( DLVertex::const_iterator p = v.begin(), p_end = v.end(); p < p_end; ++p )
			sum += *p;
		return sum;
	}

		/// insert new POSition into a given LEAF
	void insert ( HashLeaf& leaf, BipolarPointer pos ) { leaf.push_back(pos); }
		/// locate vertex V in a given LEAF
	BipolarPointer locate ( const HashLeaf& leaf, const DLVertex& v ) const;

public:		// interface
		/// empty c'tor
	dlVHashTable ( const DLDag& dag ) : host(dag) {}
		/// empty d'tor
	~dlVHashTable ( void ) {}

		/// add an element (given by a POSition) to hash
	void addElement ( BipolarPointer pos );
		/// locate given vertice in the hash
	BipolarPointer locate ( const DLVertex& v ) const
	{
		HashTable::const_iterator p = Table.find(hash(v));
		return p == Table.end() ? bpINVALID : locate ( p->second, v );
	}
}; // dlVHashTable

#endif
