#include "ConjunctiveQuerySet.h"

class LUBM2Query: public CQSet
{
public:		// interface
	LUBM2Query ( TExpressionManager* pEM, VariableFactory* VarFact )
		: CQSet(pEM, VarFact, false )
	{
		QRQuery * query;
		defC(Student);
		defC(Course);
		defC(Faculty);
		defC(Department);
		defC(University);
		defC(Subj3Student);
		defC(Subj4Student);
		defC(Subj3Department);
		defC(Subj4Department);
		defC(Professor);
		defC(Publication);
		defC(Person);
		defC(Organization);

		defR(takesCourse);
		defR(teacherOf);
		defR(worksFor);
		defR(memberOf);
		defR(degreeFrom);
		defR(subOrganizationOf);
		defR(publicationAuthor);
		defR(hasAlumnus);
		defR(advisor);	// isAdvisedBy
		defR(affiliatedOrganizationOf);

		defV(v0);
		defV(v1);
		defV(v2);
		defV(v3);
		defV(v4);

		// query 0
//		query = new QRQuery();
//		R(R);
//		query->setVarFree(v0);
//		query->addAtom(new QRRoleAtom(R,v0,v1));
//		query->addAtom(new QRRoleAtom(R,v1,v0));
//		queries.push_back(query);

		// query 1
		query = new QRQuery();
		query->setVarFree(v0);
		query->setVarFree(v2);
		query->addAtom(new QRConceptAtom(Student,v0));
		query->addAtom(new QRConceptAtom(Course,v1));
		query->addAtom(new QRConceptAtom(Faculty,v2));
		query->addAtom(new QRConceptAtom(Department,v3));
		query->addAtom(new QRRoleAtom(takesCourse,v0,v1));
		query->addAtom(new QRRoleAtom(teacherOf,v2,v1));
		query->addAtom(new QRRoleAtom(worksFor,v2,v3));
		query->addAtom(new QRRoleAtom(memberOf,v0,v3));
		queries.push_back(query);

		// query 2
		query = new QRQuery();
		query->setVarFree(v0);
		query->setVarFree(v1);
		query->addAtom(new QRConceptAtom(Subj3Student,v0));
		query->addAtom(new QRConceptAtom(Subj4Student,v1));
		query->addAtom(new QRRoleAtom(takesCourse,v0,v2));
		query->addAtom(new QRRoleAtom(takesCourse,v1,v2));
		queries.push_back(query);

		// query 3
		query = new QRQuery();
		query->setVarFree(v0);
		query->addAtom(new QRConceptAtom(Faculty,v0));
		query->addAtom(new QRConceptAtom(University,v1));
		query->addAtom(new QRConceptAtom(Department,v2));
		query->addAtom(new QRRoleAtom(degreeFrom,v0,v1));
		query->addAtom(new QRRoleAtom(subOrganizationOf,v2,v1));
		query->addAtom(new QRRoleAtom(memberOf,v0,v2));
		queries.push_back(query);

		// query 4
		query = new QRQuery();
		query->setVarFree(v1);
		query->setVarFree(v4);
		query->addAtom(new QRConceptAtom(Subj3Department,v1));
		query->addAtom(new QRConceptAtom(Subj4Department,v4));
		query->addAtom(new QRConceptAtom(Professor,v0));
		query->addAtom(new QRConceptAtom(Professor,v3));
		query->addAtom(new QRRoleAtom(memberOf,v0,v1));
		query->addAtom(new QRRoleAtom(publicationAuthor,v2,v0));
		query->addAtom(new QRRoleAtom(memberOf,v3,v4));
		query->addAtom(new QRRoleAtom(publicationAuthor,v2,v3));
		queries.push_back(query);

		// query 5
		query = new QRQuery();
		query->setVarFree(v0);
		query->addAtom(new QRConceptAtom(Publication,v0));
		query->addAtom(new QRConceptAtom(Professor,v1));
		query->addAtom(new QRConceptAtom(Student,v2));
		query->addAtom(new QRRoleAtom(publicationAuthor,v0,v1));
		query->addAtom(new QRRoleAtom(publicationAuthor,v0,v2));
		queries.push_back(query);

		// query 6
		query = new QRQuery();
		query->setVarFree(v0);
		query->setVarFree(v1);
		query->addAtom(new QRConceptAtom(University,v0));
		query->addAtom(new QRConceptAtom(University,v1));
		query->addAtom(new QRConceptAtom(Student,v2));
		query->addAtom(new QRConceptAtom(Professor,v3));
		query->addAtom(new QRRoleAtom(memberOf,v2,v0));
		query->addAtom(new QRRoleAtom(memberOf,v3,v1));
		query->addAtom(new QRRoleAtom(advisor,v2,v3));
		queries.push_back(query);

		// query 7
		query = new QRQuery();
		query->setVarFree(v0);
		query->addAtom(new QRRoleAtom(worksFor,v0,v1));
		query->addAtom(new QRRoleAtom(affiliatedOrganizationOf,v1,v2));
		queries.push_back(query);

		// query 8
		query = new QRQuery();
		query->setVarFree(v0);
		query->setVarFree(v1);
		query->addAtom(new QRConceptAtom(Person,v0));
		query->addAtom(new QRConceptAtom(Course,v1));
		query->addAtom(new QRRoleAtom(teacherOf,v0,v1));
		queries.push_back(query);

		// query 9
		query = new QRQuery();
		query->setVarFree(v0);
		query->setVarFree(v1);
		query->setVarFree(v2);
		query->addAtom(new QRConceptAtom(Student,v0));
		query->addAtom(new QRConceptAtom(Faculty,v1));
		query->addAtom(new QRConceptAtom(Course,v2));
		query->addAtom(new QRRoleAtom(advisor,v0,v1));
		query->addAtom(new QRRoleAtom(takesCourse,v0,v2));
		query->addAtom(new QRRoleAtom(teacherOf,v1,v2));
		queries.push_back(query);

		// query 10
		query = new QRQuery();
		query->setVarFree(v0);
		query->setVarFree(v1);
		query->addAtom(new QRConceptAtom(Person,v0));
		query->addAtom(new QRConceptAtom(Organization,v1));
		query->addAtom(new QRRoleAtom(worksFor,v0,v1));
		queries.push_back(query);

		// query 11
		query = new QRQuery();
		query->setVarFree(v0);
		query->addAtom(new QRConceptAtom(Person,v0));
		query->addAtom(new QRConceptAtom(University,v1));
		query->addAtom(new QRRoleAtom(worksFor,v0,v1));
		query->addAtom(new QRRoleAtom(hasAlumnus,v1,v0));
		queries.push_back(query);
	}
}; // LUBM2Query
