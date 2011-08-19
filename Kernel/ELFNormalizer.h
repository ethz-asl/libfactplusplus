/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2011 by Dmitry Tsarkov

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

#ifndef ELFNORMALIZER_H
#define ELFNORMALIZER_H

#include "ELFAxiomChecker.h"
#include "tOntologyPrinterLISP.h"

class ELFNormalizer: public DLAxiomVisitorEmpty
{
protected:
		/// expression manager to build aux expressions
	TExpressionManager* pEM;
	TLISPOntologyPrinter LP;
		/// set of new/procesed axioms
	std::vector<TDLAxiom*> Axioms;
		/// index of a freah variable
	unsigned int index;
		/// true iff the axiom was changed after visiting
	bool changed;
		/// true iff RHS is in a form \ER.C
	bool eRHS;

protected:	// methods
		/// process the axiom and mark it unused if necessary
	void v ( TDLAxiom* ax )
	{
//		std::cout << "Processing ";
//		ax->accept(LP);
		ax->accept(*this);
		if ( changed )
			ax->setUsed(false);
	}
		/// add axiom to a list
	void addAxiom ( TDLAxiom* ax )
	{
//		std::cout << "Adding ";
//		ax->accept(LP);
		Axioms.push_back(ax);
	}
		/// create a new name
	const TDLConceptExpression* buildFreshName ( void )
	{
		std::stringstream s;
		s << " ELF_aux_" << ++index;
		return pEM->Concept(s.str());
	}
		/// split C [= D1 \and \and Dn into C [= D1, C [= Dn
	bool splitAndRHS ( const TDLConceptExpression* C, const TDLConceptAnd* D )
	{
		if ( D == NULL )	// not an and
			return false;
		// make new axioms
		for ( TDLConceptAnd::iterator p = D->begin(), p_end = D->end(); p != p_end; ++p )
			addAxiom(new TDLAxiomConceptInclusion(C,*p));

		return true;
	}
		/// transform RHS into normalized one. @return a normalized RHS. Set the eRHS flag if it is an existential
	const TDLConceptExpression* transformExists ( const TDLConceptExpression* D )
	{
		eRHS = false;
		// RHS now contains only Bot, A, \E R.C
		const TDLConceptObjectExists* exists = dynamic_cast<const TDLConceptObjectExists*>(D);
		if ( exists == NULL )
		{
			fpp_assert ( dynamic_cast<const TDLConceptName*>(D) != NULL ||
						 dynamic_cast<const TDLConceptBottom*>(D) != NULL ||
						 dynamic_cast<const TDLConceptTop*>(D) != NULL);	// for LHS
			return D;
		}
		// check the filler
		const TDLConceptExpression* C = exists->getC();
		// if the filler is Bot, then the whole expression is bottom
		if ( dynamic_cast<const TDLConceptBottom*>(C) != NULL )
			return C;
		// if the filler is Top or CN then keep the expression
		eRHS = true;
		if ( dynamic_cast<const TDLConceptName*>(C) != NULL ||
			 dynamic_cast<const TDLConceptTop*>(C) != NULL )
			return D;
		// complex filler: replace C with new B and the axiom B = C
		const TDLConceptExpression* B = buildFreshName();
		TDLAxiomEquivalentConcepts::ExpressionArray args;
		args.push_back(B);
		args.push_back(C);
		addAxiom(new TDLAxiomEquivalentConcepts(args));
		return pEM->Exists (exists->getOR(), B);
	}
		/// transform conjunction into the binary one with named concepts in it; simplify
	const TDLConceptExpression* normalizeLHSAnd ( const TDLConceptAnd* C )
	{
		if ( C == NULL )
			return NULL;
		std::vector<const TDLConceptExpression*> args;
		TDLConceptAnd::iterator p, p_end = C->end();
		// check for bottom argument
		for ( p = C->begin(); p != p_end; ++p )
			if ( dynamic_cast<const TDLConceptBottom*>(*p) != NULL )
				return *p;	// all And is equivalent to bottom
		// preprocess conjunctions
		bool change = false;
		for ( p = C->begin(); p != p_end; ++p )
		{
			if ( unlikely ( dynamic_cast<const TDLConceptTop*>(*p) != NULL ) )
			{
				change = true;
				continue;	// skip Tops there
			}
			if ( dynamic_cast<const TDLConceptName*>(*p) != NULL )
				args.push_back(*p);	// keep concept name
			else
			{
				// complex expression -- replace with new name
				const TDLConceptExpression* A = buildFreshName();
				// it's enough to use implication here
				addAxiom(new TDLAxiomConceptInclusion(*p,A));
				args.push_back(A);
				change = true;
			}
		}
		// check already-binary thing
		if ( !change && args.size() == 2 && !eRHS )
			return C;
		// make conjunction binary
//		std::cout << "Args.size()==" << args.size() << "\n";
		std::vector<const TDLConceptExpression*>::iterator q = args.begin(), q_last = args.end()-(eRHS ? 0 : 1);
		const TDLConceptExpression* B = *q;
		// check the corner case: singleton conjunction
		if ( unlikely(args.size() == 1) )
			return B;
		// now we have B1 and ... and Bn [= X
		while ( ++q != q_last )
		{
			// transform into B1 and B2 [= A, A and... Bn [= X
			const TDLConceptExpression* A = buildFreshName();
			addAxiom(new TDLAxiomConceptInclusion(pEM->And(B,*q),A));
			B = A;
		}
		// now B and q=q_last are the only conjuncts
		return eRHS ? B : pEM->And(B,*q);
	}
		/// transform LHS into normalized one. @return a normalized LHS
	const TDLConceptExpression* transformLHS ( const TDLConceptExpression* C )
	{
		// here C is Top, A, AND and Exists

		// first normalize LHS And to contain only 2 names (or less)
		const TDLConceptExpression* And = normalizeLHSAnd(dynamic_cast<const TDLConceptAnd*>(C));
		if ( And != NULL )
			C = And;
		if ( dynamic_cast<const TDLConceptAnd*>(C) != NULL )
			return C;
		// LHS is Top,Bot,A and exists
		bool flag = eRHS;
		C = transformExists(C);	// normalize exists
		const TDLConceptObjectExists* exists = dynamic_cast<const TDLConceptObjectExists*>(C);
		if ( likely(exists == NULL) )
			return C;
		if ( flag )	// need intermediate var: can't have ER.X [= ES.D
		{
			// make C=ER.X [= B for fresh B
			const TDLConceptExpression* B = buildFreshName();
			addAxiom(new TDLAxiomConceptInclusion(C,B));
			return B;
		}
		// here ER.X [= A
		return C;
	}

public:		// visitor interface
	// need only do something with a very few axioms as others doesn't present here
		/// replace it with C0 [= Ci, Ci [= C0
	virtual void visit ( const TDLAxiomEquivalentConcepts& axiom )
	{
		TDLAxiomEquivalentConcepts::iterator p = axiom.begin(), p_end = axiom.end();
		const TDLConceptExpression* C = *p;
		while ( ++p != p_end )
		{
			addAxiom(new TDLAxiomConceptInclusion(C,*p));
			addAxiom(new TDLAxiomConceptInclusion(*p,C));
		}
		changed = true;
	}
		/// replace with Ci \and Cj [= \bot for 0 <= i < j < n
	virtual void visit ( const TDLAxiomDisjointConcepts& axiom )
	{
		const TDLConceptExpression* bot = pEM->Bottom();
		for ( TDLAxiomDisjointConcepts::iterator p = axiom.begin(), p_end = axiom.end(); p != p_end; ++p )
		{
			// FIXME!! replace with new var strait away if necessary
			const TDLConceptExpression* C = *p;
			for ( TDLAxiomDisjointConcepts::iterator q = p+1; q != p_end; ++q )
				addAxiom(new TDLAxiomConceptInclusion(pEM->And(C,*q),bot));
		}
		changed = true;
	}
		/// the only legal one contains a single element, so is C = D
	virtual void visit ( const TDLAxiomDisjointUnion& axiom )
	{
		changed = true;	// replace it anyway
		switch ( axiom.size() )
		{
		case 0:	// strange, but...
			return;
		case 1:	// single element, use C=D processing
			addAxiom(new TDLAxiomConceptInclusion(axiom.getC(),*axiom.begin()));
			addAxiom(new TDLAxiomConceptInclusion(*axiom.begin(),axiom.getC()));
			break;
		default:	// impossible here
			fpp_unreachable();
		}
	}
		/// normalize equivalence as a number of subsumptions R0 [= Ri, Ri [= R0
	virtual void visit ( const TDLAxiomEquivalentORoles& axiom )
	{
		TDLAxiomEquivalentORoles::iterator p = axiom.begin(), p_end = axiom.end();
		const TDLObjectRoleExpression* R = *p;
		while ( ++p != p_end )
		{
			addAxiom(new TDLAxiomORoleSubsumption(R,*p));
			addAxiom(new TDLAxiomORoleSubsumption(*p,R));
		}
		changed = true;
	}
		/// already canonical
	virtual void visit ( const TDLAxiomORoleSubsumption& axiom ATTR_UNUSED ) { changed = false; }
		/// normalize transitivity as role inclusion
	virtual void visit ( const TDLAxiomRoleTransitive& axiom )
	{
		const TDLObjectRoleExpression* R = axiom.getRole();
		pEM->newArgList();
		pEM->addArg(R);
		pEM->addArg(R);
		addAxiom(new TDLAxiomORoleSubsumption(pEM->Compose(),R));
		changed = true;
	}

		/// all the job is done here
	virtual void visit ( const TDLAxiomConceptInclusion& axiom )
	{
		const TDLConceptExpression* C = axiom.getSubC(), *D = axiom.getSupC();
		// skip tautologies
		changed = true;
		// \bot [= D is skipped
		if ( unlikely(dynamic_cast<const TDLConceptBottom*>(C) != NULL) )
			return;
		// C [= \top is skipped
		if ( unlikely(dynamic_cast<const TDLConceptTop*>(D) != NULL) )
			return;
		// split the RHS if necessary
		if ( splitAndRHS (C, dynamic_cast<const TDLConceptAnd*>(D)) )
			return;
		// do the transformation itself
		// RHS now contains only Bot, A, \E R.C
		const TDLConceptExpression* newD = transformExists(D);
		const TDLConceptExpression* newC = transformLHS(C);
		if ( newC == C && newD == D )
			changed = false;	// nothing changed
		else	// not add axiom
			if ( likely(dynamic_cast<const TDLConceptBottom*>(newC) == NULL) )
				addAxiom(new TDLAxiomConceptInclusion(newC,newD));
	}

	virtual void visitOntology ( TOntology& ontology )
	{
		for ( TOntology::iterator p = ontology.begin(), p_end = ontology.end(); p != p_end; ++p )
			if ( (*p)->isUsed() )
				v(*p);
		// will add to Axioms during the process, so can't use iterators
		for ( size_t i = 0; i < Axioms.size(); ++i )
		{
			if ( i%100000 == 0 )
				std::cerr << "\nprocessing axiom " << i << " out of " << Axioms.size();
			v(Axioms[i]);
		}
		for ( std::vector<TDLAxiom*>::iterator p = Axioms.begin(), p_end = Axioms.end(); p != p_end; ++p )
			if ( (*p)->isUsed() )
				ontology.add(*p);
			else
				delete *p;
	}
	ELFNormalizer ( TExpressionManager* p ) : pEM(p), LP(std::cout), index(0) {}
	virtual ~ELFNormalizer ( void ) {}
}; // ELFNormalizer

#endif
