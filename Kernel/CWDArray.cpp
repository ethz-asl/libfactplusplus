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

#include "CWDArray.h"
#include "tRestorer.h"

DepSet CWDArray :: clashSet;
unsigned int CWDArray :: nLookups;

/// restore dep-set of the duplicated label element of the merged node
class UnMerge: public TRestorer
{
protected:
	CWDArray& label;
	int offset;
	DepSet dep;
public:
	UnMerge ( CWDArray& lab, CWDArray::iterator p ) : label(lab), offset(p-lab.begin()), dep(p->getDep()) {}
	virtual ~UnMerge ( void ) {}
	void restore ( void )
	{
		CWDArray::iterator p = label.Base.begin() + offset;
		*p = ConceptWDep(p->bp(),dep);
	}
}; // UnMerge

TRestorer*
CWDArray :: updateDepSet ( BipolarPointer bp, const DepSet& dep )
{
	if ( dep.empty() )
		return NULL;

	for ( iterator i = Base.begin(), i_end = Base.end(); i < i_end; ++i )
		if ( i->bp() == bp )
		{
			TRestorer* ret = new UnMerge ( *this, i );
			DepSet odep(i->getDep());
			i->addDep(dep);
/*			if ( odep == i->getDep() )
			{
				delete ret;
				ret = NULL;
			}*/
			return ret;
		}
	return NULL;
}

/// check if it is possible to add a concept to a label; ~P can't appear in the label
addConceptResult
CWDArray :: checkAddedConceptP ( const BipolarPointer p ) const
{
	// increase lookup statistics
	++nLookups;

	for ( const_iterator i = begin(), i_end = end(); i < i_end; ++i )
		if ( i->bp() == p )
			return acrExist;

	// we are able to insert a concept
	return acrDone;
}

/// check if it is possible to add a concept to a label; P can't appear in the label
addConceptResult
CWDArray :: checkAddedConceptN ( const BipolarPointer p, const DepSet& dep ) const
{
	const BipolarPointer inv_p = inverse(p);

	// increase lookup statistics
	++nLookups;

	for ( const_iterator i = begin(), i_end = end(); i < i_end; ++i )
		if ( i->bp() == inv_p )
		{
			// create clashSet
			clashSet = dep + i->getDep();
			return acrClash;
		}

	// we are able to insert a concept
	return acrDone;
}

/// check if it is possible to add a concept to a set
addConceptResult
CWDArray :: checkAddedConcept ( const BipolarPointer p, const DepSet& dep ) const
{
	const BipolarPointer inv_p = inverse(p);

	// increase lookup statistics
	++nLookups;
	++nLookups;

	for ( const_iterator i = begin(), i_end = end(); i < i_end; ++i )
	{
		if ( i->bp() == p )
			return acrExist;
		else if ( i->bp() == inv_p )
		{
			// create clashSet
			clashSet = dep + i->getDep();
			return acrClash;
		}
	}

	// we are able to insert a concept
	return acrDone;
}

/// restore label to given LEVEL using given SS
void
CWDArray :: restore ( const SaveState& ss, unsigned int level ATTR_UNUSED )
{
#ifndef __DYNAMIC_NODE_RESTORE
	Base.reset(ss.ep);
#else
	unsigned int j = ss.ep;
	unsigned int k = j;
	while ( k < Base.size() )
	{
		if ( Base[k].getDep().level() >= level )
			++k;	// remove entry
		else
		{
			Base[j] = Base[k];
			++j; ++k;
		}
	}

	Base.reset(j);
#endif
}

/// print label part between given iterators
void
CWDArray :: print ( std::ostream& o ) const
{
	o << " [";
	const_iterator p = begin(), p_end = end();

	if ( p != p_end )
	{
		o << *p;

		while ( ++p < p_end )
			o << ", " << *p;
	}
	o << "]";
}
