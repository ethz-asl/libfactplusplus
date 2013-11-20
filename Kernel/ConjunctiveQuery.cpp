/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2013 by Dmitry Tsarkov

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "Kernel.h"
#include "ReasonerNom.h"
#include "Actor.h"

typedef std::multimap<std::string, ReasoningKernel::TConceptExpr*> V2CMap;

std::map<std::string, int> Var2I;
std::vector<std::string> I2Var;

// in ConjunctiveQueryFolding.cpp
typedef std::map<std::string, ReasoningKernel::TConceptExpr*> VarMap;
extern VarMap VarRestrictions;

/// fills in variable index
void fillVarIndex ( const V2CMap& query )
{
	size_t n = 0;
	Var2I.clear();
	I2Var.clear();
	for ( V2CMap::const_iterator p = query.begin(), p_end = query.end(); p != p_end; ++p )
		if ( Var2I.count(p->first) == 0 )	// new name
		{
			Var2I[p->first] = n++;
			I2Var.push_back(p->first);
		}
	fpp_assert ( I2Var.size() == n );
}

static void fillIVec ( ReasoningKernel* kernel, bool artificialABox );

void
ReasoningKernel :: evaluateQuery ( const V2CMap& query, bool artificialABox )
{
	// make index of all vars
	fillVarIndex(query);

	if ( I2Var.empty() )
	{
		std::cout << "No query variables\n";
		return;
	}

	// for every var: create an expression of vars
	std::vector<DLTree*> Concepts;
	std::cout << "Tuple <";
	TExpressionManager* pEM = getExpressionManager();
	for ( size_t i = 0; i < I2Var.size(); ++i )
	{
		const std::string& var = I2Var[i];
		if ( i != 0 )
			std::cout << ", ";
		std::cout << var.c_str();
		pEM->newArgList();
		for ( V2CMap::const_iterator p = query.lower_bound(var), p_end = query.upper_bound(var); p != p_end; ++p )
			pEM->addArg(p->second);
		Concepts.push_back(e(pEM->And()));
	}
	std::cout << ">\n";

	// fills iterable vector
	fillIVec ( this, artificialABox );

//	if ( Concepts.size() == 1 )
		getTBox()->answerQuery(Concepts);
}

template<typename Elem>
class Iterable
{
public:		// interface
	typedef std::vector<Elem> ElemVec;
	const ElemVec Elems;
	typename ElemVec::const_iterator pBeg, pEnd, pCur;
		/// init c'tor
	Iterable ( const ElemVec& Init )
		: Elems(Init)
		, pBeg(Elems.begin())
		, pEnd(Elems.end())
		, pCur(pBeg)
	{
		if ( Elems.empty() )	// no empty vecs allowed here
			fpp_unreachable();
		std::cout << " " << Init.size();
	}

	const Elem getCur ( void ) const { return *pCur; }
	bool next ( void )
	{
		if ( ++pCur == pEnd )
		{
			pCur = pBeg;
			return true;
		}
		return false;
	}
}; // Iterable

template<typename Elem>
class IterableVec
{
protected:	// members
		/// cached size of a vec
	int last;

protected:	// methods
		/// move I'th iterable forward; deal with end-case
	bool next ( int i )
	{
		if ( Base[i]->next() )	// finish with them
			return i == 0 ? true : next(i-1);
		return false;
	}
public:
	typedef std::vector<Iterable<Elem>* > ItVec;
	ItVec Base;
		/// empty c'tor
	IterableVec ( void ) : last(-1) {}

	void clear ( void )
	{
		for ( typename ItVec::iterator p = Base.begin(), p_end = Base.end(); p != p_end; ++p )
			delete *p;
		Base.clear();
		last = -1;
	}
		/// d'tor: delete individual iterables
	~IterableVec ( void ) { clear(); }

		/// add a new iteralbe to a vec
	void add ( Iterable<Elem>* It ) { Base.push_back(It); last++; fpp_assert ( last == int(Base.size()-1) ); }
		/// get next position
	bool next ( void ) { return next(last); }

	size_t size ( void ) const { return Base.size(); }
	Elem get(size_t i ) const { return Base[i]->getCur(); }
}; // IterableVec

IterableVec<TIndividual*> IV;
typedef std::vector<BipolarPointer> BPvec;
BPvec concepts;

static void
getABoxInstances ( ReasoningKernel* kernel, const TDLConceptExpression* C, bool artificialABox )
{
	// get all instances of C
	Actor a;
	std::vector<TIndividual*> individuals;
	Actor::Array1D result;
	if ( artificialABox )	// HACK: work only for our individualisation of NCIt/etc
	{
		a.needConcepts();
		kernel->getSubConcepts(C,false,a);
		a.getFoundData(result);
		TExpressionManager* pEM = kernel->getExpressionManager();
		for ( Actor::Array1D::iterator p = result.begin(), p_end = result.end(); p != p_end; ++p )
		{
			const TDLIndividualName* ind = pEM->Individual((*p)->getName());
			individuals.push_back(static_cast<TIndividual*>(ind->getEntry()));
		}
	}
	else
	{
		a.needIndividuals();
		kernel->getInstances(C,a);
		a.getFoundData(result);
		for ( Actor::Array1D::iterator p = result.begin(), p_end = result.end(); p != p_end; ++p )
			individuals.push_back(static_cast<TIndividual*>(const_cast<ClassifiableEntry*>(*p)));
	}
	IV.add(new Iterable<TIndividual*>(individuals));

}
static void
fillIVec ( ReasoningKernel* kernel, bool artificialABox )
{
	std::cout << "Creating iterables...";
	IV.clear();
	for ( size_t i = 0; i < I2Var.size(); i++ )
		// The i'th var is I2Var[i]; get its concept
		getABoxInstances ( kernel, VarRestrictions[I2Var[i]], artificialABox );

	std::cout << " done" << std::endl;
}

void
TBox :: answerQuery ( const std::vector<DLTree*>& Cs )
{
	DLHeap.removeQuery();
	std::cout << "Transforming concepts...";
	// create BPs for all the concepts
	concepts.clear();
	for ( std::vector<DLTree*>::const_iterator q = Cs.begin(), q_end = Cs.end(); q != q_end; ++q )
		concepts.push_back(tree2dag(*q));
	std::cout << " done" << std::endl << "Filling all individuals...";

	// all individuals to go thru
	std::vector<TIndividual*> AllInd;
	for ( i_iterator i = i_begin(), i_e = i_end(); i != i_e; i++ )
		AllInd.push_back(*i);

	std::cout << " done with " << AllInd.size() << " individuals" << std::endl;
	size_t size = Cs.size();

	std::cout << "Creating iterables...";
	IV.clear();
	for ( size_t j = 0; j < size; j++ )
		IV.add(new Iterable<TIndividual*>(AllInd));
	std::cout << " done\n";

	std::cout << "Run consistency checks...";

	size_t n = 0, nAns = 0;
	TsProcTimer timer;
	timer.Start();
	do
	{
		if ( n++ % 100 == 0 )
		{
			float time = timer;
			std::cout << n << " tries, " << nAns << " answers, " << time << " total time, " << time/n << " avg time" << std::endl;
		}
		if ( static_cast<NominalReasoner*>(nomReasoner)->checkExtraCond() )
		{
			for ( size_t k = 0; k < size; k++ )
				std::cout << IV.get(k)->getName() << " ";
			std::cout << "\n";
		}
	} while ( !IV.next() );
	timer.Stop();
	std::cout << "Total " << n << " tries, " << nAns << " answers, " << timer << " total time, " << timer/n << " avg time" << std::endl;
}

bool
NominalReasoner :: checkExtraCond ( void )
{
	prepareReasoner();
	DepSet dummy;
	for ( size_t i = 0; i < IV.size(); i++ )
		if ( addToDoEntry ( IV.get(i)->node, concepts[i], dummy, "QA" ) )
			return true;
	return !checkSatisfiability();
}
