/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2007 by Dmitry Tsarkov

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

#ifndef _PRECOMPLETE_H
#define _PRECOMPLETE_H

#include "dlTBox.h"

/// general precompletion exception
class PCException : public std::exception
{
public:
	PCException ( void ) throw() {}
	~PCException ( void ) throw() {}
	const char* what ( void ) const throw() { return "FaCT++ Kernel: Precompletion exception"; }
}; // PCException

/// precompletion exception: branching op (not supported yet)
class PCExceptionBranch : public PCException
{
public:
	PCExceptionBranch ( void ) throw() {}
	~PCExceptionBranch ( void ) throw() {}
	const char* what ( void ) const throw() { return "FaCT++ Kernel: Precompletion: branching is unsupported"; }
}; // PCExceptionBranch

/// precompletion exception: merging ops (not supported yet)
class PCExceptionMerge : public PCException
{
public:
	PCExceptionMerge ( void ) throw() {}
	~PCExceptionMerge ( void ) throw() {}
	const char* what ( void ) const throw()
		{ return "FaCT++ Kernel: Precompletion: merge operations are unsupported"; }
}; // PCExceptionMerge

//-------------------------------------------------------------------------
//	Precompletion ToDo List implementation
//-------------------------------------------------------------------------

/// precompletion ToDo List Entry
struct PCToDoEntry
{
		/// individual to expand
	TIndividual* Ind;
		/// concept expression to expand
	const DLTree* Expr;

		/// c'tor
	PCToDoEntry ( TIndividual* ind, const DLTree* expr ) : Ind(ind), Expr(expr) {}
}; // PCToDoEntry

/// ToDo List for Precompletion
class PCToDoList
{
protected:
		/// remember all the entries here
	std::vector<PCToDoEntry> Base;

public:		// interface
		/// empty c'tor
	PCToDoList ( void ) {}
		/// empty d'tor
	~PCToDoList ( void ) {}

		/// add new entry
	void add ( TIndividual* ind, const DLTree* expr ) { Base.push_back(PCToDoEntry(ind,expr)); }
		/// get next entry
	const PCToDoEntry& get ( void )
	{
		const PCToDoEntry& ret = Base.back();
		Base.pop_back();
		return ret;
	}
		/// check whether ToDo list is empty
	bool empty ( void ) const { return Base.empty(); }
}; // PCToDoList

/// class to perform the precompletion
class Precompletor
{
protected:	// typedefs
		/// access to TRelated
	typedef TBox::RelatedCollection::iterator r_iterator;
		/// access to individuals
	typedef TBox::i_iterator i_iterator;

protected:	// members
		/// host KB
	TBox& KB;
		/// ToDo List for precompletion
	PCToDoList ToDoList;

protected:	// methods
		/// init precompletion process
	void initPrecompletion ( void ) {}
		/// propagate RIA and rules throughout individual cloud
	void propagateRIA ( void ) {}
		/// init labels of related elements with R&D
	void addRnD ( void ) {}
		/// init ToDo List with elements in every individuals' description
	void initToDoList ( void )
	{
		for ( i_iterator p = KB.i_begin(), p_end = KB.i_end(); p < p_end; ++p )
			processTree ( (*p), (*p)->Description );
	}
		/// run precompletion algorithm; @return true if KB is inconsistent
	bool runPrecompletion ( void ) throw(PCException);
		/// update individuals' info from precompletion
	void updateIndividualsFromPrecompletion ( void )
	{
		for ( i_iterator p = KB.i_begin(), p_end = KB.i_end(); p < p_end; ++p )
		{
			(*p)->usePCInfo();
			// we change description of a concept, so we need to rebuild the TS info
			// note that precompletion succeed; so there is no need to take into account
			// RELATED information
			(*p)->initToldSubsumersC(KB.pTop);
		}
	}
		/// remove all precompletion-related references from KB
	void clearPrecompletion ( void )
	{
		for ( i_iterator p = KB.i_begin(), p_end = KB.i_end(); p < p_end; ++p )
			(*p)->clearPCInfo();
	}

		/// add (Ind:Expr) to the ToDo List
	void addToDoEntry ( TIndividual* ind, const DLTree* expr )
	{
		if ( ind->addPCExpr(expr) )
			ToDoList.add(ind,expr);
	}
		/// process the whole tree
	void processTree ( TIndividual* ind, const DLTree* expr )
	{
		if ( expr == NULL )
			return;
		switch ( expr->Element().getToken() )
		{
		case AND:	// go recursively
			processTree ( ind, expr->Left() );
			processTree ( ind, expr->Right() );
			break;
		default:	// add non-AND expression to the ToDo list
			addToDoEntry(ind,expr);
			break;
		}
	}
		/// process forall restriction
	void processForall ( TIndividual* ind, const TRole* R, const DLTree* expr )
	{
		for ( r_iterator i = ind->RelatedIndex.begin(), i_end = ind->RelatedIndex.end(); i < i_end; ++i )
			if ( *(*i)->getRole() <= *R )
				processTree ( (*i)->b, expr );
	}

public:		// interface
		/// empty c'tor
	Precompletor ( TBox& box ) : KB(box) {}
		/// empty d'tor
	~Precompletor ( void ) { clearPrecompletion(); }

		/// perform precompletion; @return true if precompletion failed
	bool performPrecompletion ( void );
}; // Precompletor

#endif
