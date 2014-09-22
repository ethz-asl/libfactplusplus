#include "ConjunctiveQuerySet.h"

class BSPOQuery: public CQSet
{
public:		// interface
	BSPOQuery ( TExpressionManager* pEM, VariableFactory* VarFact )
		: CQSet(pEM, VarFact, true )
	{
		QRQuery * query;
		defC(BSPO_0000400);
		defC(BSPO_0000070);
		defC(BSPO_0000017);
		defC(BSPO_0000063);
		defC(BSPO_0000083);
		defC(BSPO_0000055);
		defC(BSPO_0000066);
		defC(BSPO_0000054);
		defC(BSPO_0000062);

		defR(RO_0002131);
		defR(BSPO_0015005);
		defR(BSPO_0000113);

		defV(v0);
		defV(v1);
		defV(v2);

		// query 0
		query = new QRQuery();
 		query->setVarFree(v0);
 		query->addAtom(new QRRoleAtom(BSPO_0015005,v0,v1));
 		query->addAtom(new QRConceptAtom(BSPO_0000400,v0));
 		query->addAtom(new QRConceptAtom(BSPO_0000017,v1));
 		queries.push_back(query);

		// query 1
		query = new QRQuery();
		query->setVarFree(v0);
		query->addAtom(new QRRoleAtom(RO_0002131,v0,v1));
		query->addAtom(new QRConceptAtom(BSPO_0000070,v0));
		query->addAtom(new QRConceptAtom(BSPO_0000063,v1));
 		queries.push_back(query);

		// query 2
		query = new QRQuery();
		query->setVarFree(v0);
		query->addAtom(new QRConceptAtom(BSPO_0000083,v0));
		queries.push_back(query);

		// query 3
		query = new QRQuery();
		query->setVarFree(v0);
		query->addAtom(new QRRoleAtom(RO_0002131,v0,v1));
		query->addAtom(new QRConceptAtom(BSPO_0000070,v0));
		query->addAtom(new QRConceptAtom(BSPO_0000055,v1));
		query->addAtom(new QRRoleAtom(RO_0002131,v0,v2));
		query->addAtom(new QRConceptAtom(BSPO_0000066,v2));
 		queries.push_back(query);

		// query 4
		query = new QRQuery();
		query->setVarFree(v0);
		query->addAtom(new QRRoleAtom(RO_0002131,v0,v1));
		query->addAtom(new QRConceptAtom(BSPO_0000070,v0));
		query->addAtom(new QRConceptAtom(BSPO_0000054,v1));
		query->addAtom(new QRRoleAtom(BSPO_0000113,v1,v2));
		query->addAtom(new QRConceptAtom(BSPO_0000062,v2));
 		queries.push_back(query);
	}
}; // BSPOQuery
