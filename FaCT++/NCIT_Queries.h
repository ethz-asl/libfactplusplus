#include "ConjunctiveQuerySet.h"

class NCITQuery: public CQSet
{
public:		// interface
	NCITQuery ( TExpressionManager* pEM, VariableFactory* VarFact )
		: CQSet(pEM, VarFact, true )
	{
		QRQuery * query;
		defC(C16612);
		defC(C19390);
		defC(C2991);
		defC(C12745);
		defC(C13018);
		defC(C12064);
		defC(C12413);
		defC(C3262);
		defC(C36104);
		defC(C54123);
		defC(C36988);

		defR(R37);
		defR(R38);
		defR(R101);
		defR(R81);
		defR(R103);
		defR(R82);
		defR(R108);

		defV(v0);
		defV(v1);
        defV(v2);
        defV(v3);


		// query 0
        // All genes which are involved in carcerogen metabolism

		query = new QRQuery();
		query->setVarFree(v0);
		query->addAtom(new QRRoleAtom(R37,v0,v1));
		query->addAtom(new QRConceptAtom(C16612,v0));
		query->addAtom(new QRConceptAtom(C19390,v1));
		queries.push_back(query);

        // query 1
        // All genes which are associated with lymph node deseases.

		query = new QRQuery();
		query->setVarFree(v0);
		query->addAtom(new QRRoleAtom(R38,v0,v1));
		query->addAtom(new QRConceptAtom(C16612,v0));
		query->addAtom(new QRConceptAtom(C2991,v1));
		query->addAtom(new QRRoleAtom(R101,v1,v2));
		query->addAtom(new QRConceptAtom(C12745,v2));
		queries.push_back(query);

        // query 2
        // All organs which are located in abdominal cavity and which
        // can be affected by deseases which involve abnormal cells.
		query = new QRQuery();
		query->setVarFree(v0);
		query->addAtom(new QRConceptAtom(C13018,v0));
		query->addAtom(new QRRoleAtom(R81,v0,v1));
		query->addAtom(new QRConceptAtom(C12064,v1));
		query->addAtom(new QRRoleAtom(R101,v2,v0));
		query->addAtom(new QRRoleAtom(R103,v2,v3));
		queries.push_back(query);

        // query 3
        // All urinary system organs and their neoplasm deseases.

		query = new QRQuery();
		query->setVarFree(v0);
        query->setVarFree(v2);
		query->addAtom(new QRConceptAtom(C13018,v0));
		query->addAtom(new QRRoleAtom(R82,v0,v1));
		query->addAtom(new QRConceptAtom(C12413,v1));
		query->addAtom(new QRRoleAtom(R101,v2,v0));
        query->addAtom(new QRConceptAtom(C3262,v2));
		queries.push_back(query);

        // query 4
        // All deseases with aggressive clinical course an sinusoidal growth pattern
        // which involve neoplastic lymphocytes

		query = new QRQuery();
		query->setVarFree(v0);
		query->addAtom(new QRConceptAtom(C2991,v0));
		query->addAtom(new QRRoleAtom(R108,v0,v1));
		query->addAtom(new QRConceptAtom(C36104,v1));
		query->addAtom(new QRRoleAtom(R108,v0,v2));
        query->addAtom(new QRConceptAtom(C54123,v2));
		query->addAtom(new QRRoleAtom(R101,v0,v3));
        query->addAtom(new QRConceptAtom(C36988,v3));
		queries.push_back(query);
	}
}; // NCITQuery
