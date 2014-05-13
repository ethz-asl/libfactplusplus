/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2014 by Dmitry Tsarkov

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

#ifndef TODOLIST_H
#define TODOLIST_H

#include "globaldef.h"
#include "fpp_assert.h"
#include "PriorityMatrix.h"
#include "tRareSaveStack.h"

/// the entry of TODO table
struct ToDoEntry
{
		/// node to include concept
	DlCompletionTree* Node;
		/// offset of included concept in Node's label
		// (it's not possible to use pointers because
		// std::vector invalidates them)
	int offset;

		/// empty C'tor
	ToDoEntry ( void ) : Node(NULL), offset(0) {}	// for initialisation
		/// C'tor (init values)
	ToDoEntry ( DlCompletionTree* n, int off ) : Node(n), offset(off) {}
}; // ToDoEntry

/// All-in-one version of arrayToDoTable
class ToDoList
{
protected:	// classes
		/// class for saving/restoring ToDoQueue
	class QueueSaveState
	{
	public:		// members
			/// save start point of queue of entries
		unsigned int sp;
			/// save end point of queue of entries
		unsigned int ep;

	public:		// methods
			/// empty c'tor
		QueueSaveState ( void ) {}
			/// empty d'tor
		~QueueSaveState ( void ) {}
	}; // QueueSaveState
	//--------------------------------------------------------------------------

		/// class to represent single queue
	class arrayQueue
	{
	protected:	// members
			/// waiting ops queue
		growingArray<ToDoEntry> Wait;
			/// start pointer; points to the 1st element in the queue
		unsigned int sPointer;

	public:		// interface
			/// c'tor: init queue with proper size and reset it
		arrayQueue ( void ) : sPointer(0)
		{
			Wait.reserve(50);	// initial size
			Wait.clear();
		}
			/// empty d'tor
		~arrayQueue ( void ) {}

			/// add entry to a queue
		void add ( DlCompletionTree* node, int offset ) { Wait.add(ToDoEntry(node,offset)); }
			/// clear queue
		void clear ( void ) { sPointer = 0; Wait.clear(); }
			/// check if queue empty
		bool empty ( void ) const { return sPointer == Wait.size(); }
			/// get next entry from the queue; works for non-empty queues
		const ToDoEntry* get ( void ) { return &(Wait[sPointer++]); }

			/// save queue content to the given entry
		void save ( QueueSaveState& tss ) const
		{
			tss.sp = sPointer;
			tss.ep = Wait.size();
		}
			/// restore queue content from the given entry
		void restore ( const QueueSaveState& tss )
		{
			sPointer = tss.sp;
			Wait.resize(tss.ep);
		}
	}; // arrayQueue
	//--------------------------------------------------------------------------

		/// class to represent single priority queue
	class queueQueue
	{
	protected:	// types
			/// type for restore the whole queue
		class QueueRestorer: public TRestorer
		{
		protected:	// members
				/// copy of a queue
			growingArray<ToDoEntry> Wait;
				/// pointer to a queue to restore
			queueQueue* queue;
				/// start pointer
			unsigned int sp;

		public:		// interface
				/// init c'tor
			QueueRestorer ( queueQueue* q ) : Wait(q->Wait), queue(q), sp(q->sPointer) {}
				/// empty d'tor
			virtual ~QueueRestorer ( void ) {}
				/// restore: copy the queue back, adjust pointers
			virtual void restore ( void ) { queue->Wait = Wait; queue->sPointer = sp; }
		};

	protected:	// members
			/// waiting ops queue
		growingArray<ToDoEntry> Wait;
			/// stack to save states for the overwritten queue
		TRareSaveStack* stack;
			/// start pointer; points to the 1st element in the queue
		unsigned int sPointer;

	public:		// interface
			/// c'tor: make an empty queue
		queueQueue ( TRareSaveStack* s ) : stack(s), sPointer(0) {}
			/// empty d'tor
		~queueQueue ( void ) {}

			/// add entry to a queue
		void add ( DlCompletionTree* Node, int offset )
		{
			ToDoEntry e(Node,offset);
			if ( empty() ||	// no problems with empty queue and if no priority clashes
				 Wait[Wait.size()-1].Node->getNominalLevel() <= Node->getNominalLevel() )
			{
				Wait.add(e);
				return;
			}

			// here we need to put e on the proper place
			stack->push(new QueueRestorer(this));
			unsigned int n = Wait.size();
			Wait.add(e);	// will be rewritten
			while ( n > sPointer && Wait[n-1].Node->getNominalLevel() > Node->getNominalLevel() )
			{
				Wait[n] = Wait[n-1];
				--n;
			}
			Wait[n] = e;
		}
			/// clear queue
		void clear ( void ) { sPointer = 0; Wait.clear(); }
			/// check if queue empty
		bool empty ( void ) const { return sPointer == Wait.size(); }
			/// get next entry from the queue; works for non-empty queues
		const ToDoEntry* get ( void ) { return &(Wait[sPointer++]); }

			/// save queue content to the given entry
		void save ( QueueSaveState& tss )
		{
			tss.sp = sPointer;
			tss.ep = Wait.size();
		}
			/// restore queue content from the given entry
		void restore ( const QueueSaveState& tss )
		{
			sPointer = tss.sp;
			Wait.resize(tss.ep);
		}
	}; // queueQueue
	//--------------------------------------------------------------------------

protected:	// internal typedefs
		/// typedef for NN-queue (which should support complete S/R)
	typedef queueQueue NNQueue;

protected:	// classes
		/// class for saving/restoring array TODO table
	class SaveState
	{
	public:		// members
			/// save state for queueID
		QueueSaveState backupID;
			/// save state for queueNN
		QueueSaveState backupNN;
			/// save state of all regular queues
		QueueSaveState backup[nRegularOps];
			/// save number-of-entries to do
		unsigned int noe;

	public:		// methods
			/// empty c'tor
		SaveState ( void ) {}
			/// empty d'tor
		~SaveState ( void ) {}
	}; // SaveState
	//--------------------------------------------------------------------------

private:	// safety
		/// no copy c'tor
	ToDoList ( ToDoList& );
		/// no assignment
	ToDoList& operator = ( ToDoList& );

protected:	// members
		/// waiting ops queue for IDs
	arrayQueue queueID;
		/// waiting ops queue for <= ops in nominal nodes
	NNQueue queueNN;
		/// waiting ops queues
	arrayQueue Wait[nRegularOps];
		/// stack of saved states
	TSaveStack<SaveState> SaveStack;
		/// priority matrix
	const ToDoPriorMatrix& Matrix;
		/// number of un-processed entries
	unsigned int noe;

protected:	// methods
		/// save current TODO table content to given saveState entry
	void saveState ( SaveState* tss )
	{
		queueID.save(tss->backupID);
		queueNN.save(tss->backupNN);
		for ( register int i = nRegularOps-1; i >= 0; --i )
			Wait[i].save(tss->backup[i]);

		tss->noe = noe;
	}
		/// restore TODO table content from given saveState entry
	void restoreState ( const SaveState* tss )
	{
		queueID.restore(tss->backupID);
		queueNN.restore(tss->backupNN);
		for ( register int i = nRegularOps-1; i >= 0; --i )
			Wait[i].restore(tss->backup[i]);

		noe = tss->noe;
	}

public:
		/// init c'tor
	ToDoList ( const ToDoPriorMatrix& matrix, TRareSaveStack* stack ) : queueNN(stack), Matrix(matrix), noe(0) {}
		/// d'tor: delete all entries
	~ToDoList ( void ) { clear(); }

	// global methods

		/// clear TODO table
	void clear ( void )
	{
		queueID.clear();
		queueNN.clear();
		for ( register int i = nRegularOps-1; i >= 0; --i )
			Wait[i].clear();

		SaveStack.clear();
		noe = 0;
	}
		/// check if TODO table is empty
	bool empty ( void ) const { return !noe; }

	// work with entries

		/// add entry with given NODE and CONCEPT with given OFFSET to the TODO table
	void addEntry ( DlCompletionTree* node, DagTag type, const ConceptWDep& C, int offset )
	{
		unsigned int index = Matrix.getIndex ( type, isPositive(C.bp()), node->isNominalNode() );
		switch ( index )
		{
		case nRegularOps:	// unused entry
			return;
		case iId:			// ID
			queueID.add(node,offset); break;
		case iNN:			// NN
			queueNN.add(node,offset); break;
		default:			// regular queue
			Wait[index].add(node,offset); break;
		}
		++noe;
	}
		/// add entry with given NODE and CONCEPT of a TYPE to the ToDo table
	void addEntry ( DlCompletionTree* node, DagTag type, const ConceptWDep& C )
		{ addEntry ( node, type, C, node->label().getLast(type) ); }
		/// get the next TODO entry. @return NULL if the table is empty
	const ToDoEntry* getNextEntry ( void );

	// save/restore methods

		/// save current state using internal stack
	void save ( void ) { saveState(SaveStack.push()); }
		/// restore state using internal stack
	void restore ( void ) { fpp_assert ( !SaveStack.empty() ); restoreState(SaveStack.pop()); }
		/// restore state to the given level using internal stack
	void restore ( unsigned int level ) { restoreState(SaveStack.pop(level)); }
}; // ToDoList

inline const ToDoEntry* ToDoList :: getNextEntry ( void )
{
#ifdef ENABLE_CHECKING
	fpp_assert ( !empty () );	// safety check
#endif

	// decrease amount of elements-to-process
	--noe;

	// check ID queue
	if ( !queueID.empty() )
		return queueID.get();

	// check NN queue
	if ( !queueNN.empty() )
		return queueNN.get();

	// check regular queues
	for ( register unsigned int i = 0; i < nRegularOps; ++i )
		if ( !Wait[i].empty() )
			return Wait[i].get();

	// that's impossible, but still...
	return NULL;
}

#endif
