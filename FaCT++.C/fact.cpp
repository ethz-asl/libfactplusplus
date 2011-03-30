
#include "fact.h"
#include "Kernel.h"
// type declarations

// FaCT++ kernel
struct fact_reasoning_kernel_st
{
	ReasoningKernel* p;
	fact_reasoning_kernel_st(ReasoningKernel* q) : p(q) {}
};
// progress monitor
struct fact_progress_monitor_st
{
	TProgressMonitor* p;
};
// expression manager
struct fact_expression_manager_st
{
	TExpressionManager* p;
	fact_expression_manager_st(TExpressionManager*q) : p(q) {}
};
// axiom
struct fact_axiom_st
{
	TDLAxiom* p;
	fact_axiom_st(TDLAxiom*q) : p(q) {}
};
// expression
struct fact_expression_st
{
	ReasoningKernel::TExpr* p;
};
// concept expression
struct fact_concept_expression_st
{
	ReasoningKernel::TConceptExpr* p;
};
// role expression
struct fact_role_expression_st
{
	ReasoningKernel::TRoleExpr* p;
};
// object role expression
struct fact_o_role_expression_st
{
	ReasoningKernel::TORoleExpr* p;
};
// complex object role expression
struct fact_o_role_complex_expression_st
{
	ReasoningKernel::TORoleComplexExpr* p;
};
// data role expression
struct fact_d_role_expression_st
{
	ReasoningKernel::TDRoleExpr* p;
};
// individual expression
struct fact_individual_expression_st
{
	ReasoningKernel::TIndividualExpr* p;
};
// general data expression
struct fact_data_expression_st
{
	ReasoningKernel::TDataExpr* p;
};
// data type expression
struct fact_data_type_expression_st
{
	ReasoningKernel::TDataTypeExpr* p;
};
// data value expression
struct fact_data_value_expression_st
{
	ReasoningKernel::TDataValueExpr* p;
};
// actor
struct fact_actor_st
{
	Actor* p;
};

const char *fact_get_version ()
{
	return ReasoningKernel::getVersion();
}

fact_reasoning_kernel *fact_reasoning_kernel_new ()
{
	return new fact_reasoning_kernel_st(new ReasoningKernel());
}
void fact_reasoning_kernel_free (fact_reasoning_kernel *k)
{
	delete k->p;
	delete k;
}

/*
ifOptionSet* getOptions (  );
const ifOptionSet* getOptions (  );
 */

bool fact_is_kb_preprocessed (fact_reasoning_kernel *k)
{
	return k->p->isKBPreprocessed();
}
bool fact_is_kb_classified (fact_reasoning_kernel *k)
{
	return k->p->isKBRealised();
}
bool fact_is_kb_realised (fact_reasoning_kernel *k)
{
	return k->p->isKBRealised();
}
void fact_set_progress_monitor (fact_reasoning_kernel *k, fact_progress_monitor *m)
{
	return k->p->setProgressMonitor(m->p);
}

void fact_set_verbose_output (fact_reasoning_kernel *k, bool value)
{
	k->p->setVerboseOutput(value);
}

void fact_set_top_bottom_role_names (fact_reasoning_kernel *k,
		const char *top_o_role_name,
		const char *bot_o_role_name,
		const char *top_d_role_name,
		const char *bot_d_role_name)
{
	k->p->setTopBottomRoleNames(top_o_role_name,bot_o_role_name,top_d_role_name,bot_d_role_name);
}

void fact_set_operation_timeout (fact_reasoning_kernel *k,
		unsigned long timeout)
{
	k->p->setOperationTimeout(timeout);
}

fact_expression_manager *fact_get_expression_manager (fact_reasoning_kernel *k)
{
	return new fact_expression_manager_st(k->p->getExpressionManager());
}

bool fact_new_kb (fact_reasoning_kernel *k)
{
	return k->p->newKB();
}
bool fact_release_kb (fact_reasoning_kernel *k)
{
	return k->p->releaseKB();
}
bool fact_clear_kb (fact_reasoning_kernel *k)
{
	return k->p->clearKB();
}

fact_axiom *fact_declare (fact_reasoning_kernel *k, fact_expression *c)
{
	return new fact_axiom_st(k->p->declare(c->p));
}
fact_axiom *fact_implies_concepts (fact_reasoning_kernel *k,
		fact_concept_expression *c,
		fact_concept_expression *d)
{
	return new fact_axiom_st(k->p->impliesConcepts(c->p,d->p));
}
fact_axiom *fact_equal_concepts (fact_reasoning_kernel *k)
{
	return new fact_axiom_st(k->p->equalConcepts());
}
fact_axiom *fact_disjoint_concepts (fact_reasoning_kernel *k)
{
	return new fact_axiom_st(k->p->disjointConcepts());
}
fact_axiom *fact_disjoint_union (fact_reasoning_kernel *k,
		fact_concept_expression *C)
{
	return new fact_axiom_st(k->p->disjointUnion(C->p));
}


fact_axiom *fact_set_inverse_roles (fact_reasoning_kernel *k,
		fact_o_role_expression *r,
		fact_o_role_expression *s)
{
	return new fact_axiom_st(k->p->setInverseRoles(r->p,s->p));
}
fact_axiom *fact_implies_o_roles (fact_reasoning_kernel *k,
		fact_o_role_complex_expression *r,
		fact_o_role_expression *s)
{
	return new fact_axiom_st(k->p->impliesORoles(r->p,s->p));
}
fact_axiom *fact_implies_d_roles (fact_reasoning_kernel *k,
		fact_d_role_expression *r,
		fact_d_role_expression *s)
{
	return new fact_axiom_st(k->p->impliesDRoles(r->p,s->p));
}
fact_axiom *fact_equal_o_roles (fact_reasoning_kernel *k)
{
	return new fact_axiom_st(k->p->equalORoles());
}
fact_axiom *fact_equal_d_roles (fact_reasoning_kernel *k)
{
	return new fact_axiom_st(k->p->equalDRoles());
}
fact_axiom *fact_disjoint_o_roles (fact_reasoning_kernel *k)
{
	return new fact_axiom_st(k->p->disjointORoles());
}
fact_axiom *fact_disjoint_d_roles (fact_reasoning_kernel *k)
{
	return new fact_axiom_st(k->p->disjointDRoles());
}

fact_axiom* fact_set_o_domain (fact_reasoning_kernel *k,
		fact_o_role_expression *r,
		fact_concept_expression *c)
{
	return new fact_axiom_st(k->p->setODomain(r->p,c->p));
}
fact_axiom *fact_set_d_domain (fact_reasoning_kernel *k,
		fact_d_role_expression *r,
		fact_concept_expression *c)
{
	return new fact_axiom_st(k->p->setDDomain(r->p,c->p));
}
fact_axiom *fact_set_o_range (fact_reasoning_kernel *k,
		fact_o_role_expression *r,
		fact_concept_expression *c)
{
	return new fact_axiom_st(k->p->setORange(r->p,c->p));
}
fact_axiom *fact_set_d_range (fact_reasoning_kernel *k,
		fact_d_role_expression *r,
		fact_data_expression *e)
{
	return new fact_axiom_st(k->p->setDRange(r->p,e->p));
}

fact_axiom *fact_set_transitive (fact_reasoning_kernel *k,
		fact_o_role_expression *r)
{
	return new fact_axiom_st(k->p->setTransitive(r->p));
}
fact_axiom *fact_set_reflexive (fact_reasoning_kernel *k,
		fact_o_role_expression *r)
{
	return new fact_axiom_st(k->p->setReflexive(r->p));
}
fact_axiom *fact_set_irreflexive (fact_reasoning_kernel *k,
		fact_o_role_expression *r)
{
	return new fact_axiom_st(k->p->setIrreflexive(r->p));
}
fact_axiom *fact_set_symmetric (fact_reasoning_kernel *k,
		fact_o_role_expression *r)
{
	return new fact_axiom_st(k->p->setSymmetric(r->p));
}
fact_axiom *fact_set_asymmetric (fact_reasoning_kernel *k,
		fact_o_role_expression *r)
{
	return new fact_axiom_st(k->p->setAsymmetric(r->p));
}
fact_axiom *fact_set_o_functional (fact_reasoning_kernel *k,
		fact_o_role_expression *r)
{
	return new fact_axiom_st(k->p->setOFunctional(r->p));
}
fact_axiom *fact_set_d_functional (fact_reasoning_kernel *k,
		fact_d_role_expression *r)
{
	return new fact_axiom_st(k->p->setDFunctional(r->p));
}
fact_axiom *fact_set_inverse_functional (fact_reasoning_kernel *k,
		fact_o_role_expression *r)
{
	return new fact_axiom_st(k->p->setInverseFunctional(r->p));
}

fact_axiom *fact_instance_of (fact_reasoning_kernel *k,
		fact_individual_expression *i,
		fact_concept_expression *c)
{
	return new fact_axiom_st(k->p->instanceOf(i->p,c->p));
}
fact_axiom *fact_related_to (fact_reasoning_kernel *k,
		fact_individual_expression *i,
		fact_o_role_expression *r,
		fact_individual_expression *j)
{
	return new fact_axiom_st(k->p->relatedTo(i->p,r->p,j->p));
}
fact_axiom *fact_related_to_not (fact_reasoning_kernel *k,
		fact_individual_expression *i,
		fact_o_role_expression *r,
		fact_individual_expression *j)
{
	return new fact_axiom_st(k->p->relatedToNot(i->p,r->p,j->p));
}
fact_axiom *fact_value_of (fact_reasoning_kernel *k,
		fact_individual_expression *i,
		fact_d_role_expression *a,
		fact_data_value_expression *v)
{
	return new fact_axiom_st(k->p->valueOf(i->p,a->p,v->p));
}
fact_axiom *fact_value_of_not (fact_reasoning_kernel *k,
		fact_individual_expression *i,
		fact_d_role_expression *a,
		fact_data_value_expression *v)
{
	return new fact_axiom_st(k->p->valueOfNot(i->p,a->p,v->p));
}
fact_axiom *fact_process_same (fact_reasoning_kernel *k)
{
	return new fact_axiom_st(k->p->processSame());
}
fact_axiom *fact_process_different (fact_reasoning_kernel *k)
{
	return new fact_axiom_st(k->p->processDifferent());
}
fact_axiom *fact_set_fairness_constraint (fact_reasoning_kernel *k)
{
	return new fact_axiom_st(k->p->setFairnessConstraint());
}

void fact_retract (fact_reasoning_kernel *k, fact_axiom *axiom)
{
	k->p->retract(axiom->p);
}

bool fact_is_kb_consistent (fact_reasoning_kernel *k)
{
	return k->p->isKBConsistent();
}
void fact_preprocess_kb (fact_reasoning_kernel *k)
{
	k->p->preprocessKB();
}
void fact_classify_kb (fact_reasoning_kernel *k)
{
	k->p->classifyKB();
}
void fact_realise_kb (fact_reasoning_kernel *k)
{
	k->p->realiseKB();
}

bool fact_is_o_functional (fact_reasoning_kernel *k, fact_o_role_expression *r)
{
	return k->p->isFunctional(r->p);
}
bool fact_is_d_functional (fact_reasoning_kernel *k,
		fact_d_role_expression *r)
{
	return k->p->isFunctional(r->p);
}
bool fact_is_inverse_functional (fact_reasoning_kernel *k,
		fact_o_role_expression *r)
{
	return k->p->isInverseFunctional(r->p);
}
bool fact_is_transitive (fact_reasoning_kernel *k, fact_o_role_expression *r)
{
	return k->p->isTransitive(r->p);
}
bool fact_is_symmetric (fact_reasoning_kernel *k, fact_o_role_expression *r)
{
	return k->p->isSymmetric(r->p);
}
bool fact_is_asymmetric (fact_reasoning_kernel *k, fact_o_role_expression *r)
{
	return k->p->isAsymmetric(r->p);
}
bool fact_is_reflexive (fact_reasoning_kernel *k, fact_o_role_expression *r)
{
	return k->p->isReflexive(r->p);
}
bool fact_is_irreflexive (fact_reasoning_kernel *k, fact_o_role_expression *r)
{
	return k->p->isIrreflexive(r->p);
}
bool fact_is_sub_o_roles (fact_reasoning_kernel *k, fact_o_role_expression *r,
		fact_o_role_expression *s)
{
	return k->p->isSubRoles(r->p,s->p);
}
bool fact_is_sub_d_roles (fact_reasoning_kernel *k, fact_d_role_expression *r,
		fact_d_role_expression *s)
{
	return k->p->isSubRoles(r->p,s->p);
}
bool fact_is_disjoint_o_roles (fact_reasoning_kernel *k,
		fact_o_role_expression *r,
		fact_o_role_expression *s)
{
	return k->p->isDisjointRoles(r->p,s->p);
}
bool fact_is_disjoint_d_roles (fact_reasoning_kernel *k,
		fact_d_role_expression *r,
		fact_d_role_expression *s)
{
	return k->p->isDisjointRoles(r->p,s->p);
}
bool fact_is_disjoint_roles (fact_reasoning_kernel *k)
{
	return k->p->isDisjointRoles();
}
bool fact_is_sub_chain (fact_reasoning_kernel *k, fact_o_role_expression *r)
{
	return k->p->isSubChain(r->p);
}

bool fact_is_satisfiable (fact_reasoning_kernel *k, fact_concept_expression *c)
{
	return k->p->isSatisfiable(c->p);
}
bool fact_is_subsumed_by (fact_reasoning_kernel *k, fact_concept_expression *c,
		fact_concept_expression *d)
{
	return k->p->isSubsumedBy(c->p,d->p);
}
bool fact_is_disjoint (fact_reasoning_kernel *k, fact_concept_expression *c,
		fact_concept_expression *d)
{
	return k->p->isDisjoint(c->p,d->p);
}
bool fact_is_equivalent (fact_reasoning_kernel *k, fact_concept_expression *c,
		fact_concept_expression *d)
{
	return k->p->isEquivalent(c->p,d->p);
}

void fact_get_sup_concepts (fact_reasoning_kernel *k, fact_concept_expression *c,
		bool direct, fact_actor **actor)
{
	k->p->getSupConcepts(c->p,direct,(*actor)->p);
}
void fact_get_sub_concepts (fact_reasoning_kernel *k, fact_concept_expression *c,
		bool direct, fact_actor **actor)
{
	k->p->getSubConcepts(c->p,direct,(*actor)->p);
}
void fact_get_equivalent_concepts (fact_reasoning_kernel *k,
		fact_concept_expression *c,
		fact_actor **actor)
{
	k->p->getEquivalentConcepts(c->p,(*actor)->p);
}
void fact_get_disjoint_concepts (fact_reasoning_kernel *k,
		fact_concept_expression *c,
		fact_actor **actor)
{
	k->p->getDisjointConcepts(c->p,(*actor)->p);
}

void fact_get_sup_roles (fact_reasoning_kernel *k, fact_role_expression *r,
		bool direct,
		fact_actor **actor)
{
	k->p->getSupRoles(r->p,direct,(*actor)->p);
}
void fact_get_sub_roles (fact_reasoning_kernel *k, fact_role_expression *r,
		bool direct, fact_actor **actor)
{
	k->p->getSubRoles(r->p,direct,(*actor)->p);
}
void fact_get_equivalent_roles (fact_reasoning_kernel *k, fact_role_expression *r,
		fact_actor **actor)
{
	k->p->getEquivalentRoles(r->p,(*actor)->p);
}
void fact_get_role_domain (fact_reasoning_kernel *k, fact_role_expression *r,
		bool direct, fact_actor **actor)
{
	k->p->getRoleDomain(r->p,direct,(*actor)->p);
}
void fact_get_role_range (fact_reasoning_kernel *k, fact_o_role_expression *r,
		bool direct, fact_actor **actor)
{
	k->p->getRoleRange(r->p,direct,(*actor)->p);
}
void fact_get_direct_instances (fact_reasoning_kernel *k,
		fact_concept_expression *c, fact_actor **actor)
{
	k->p->getDirectInstances(c->p,(*actor)->p);
}
void fact_get_instances (fact_reasoning_kernel *k, fact_concept_expression *c,
		fact_actor **actor)
{
	k->p->getInstances(c->p,(*actor)->p);
}
void fact_get_types (fact_reasoning_kernel *k, fact_individual_expression *i,
		bool direct, fact_actor **actor)
{
	k->p->getTypes(i->p,direct,(*actor)->p);
}
void fact_get_same_as (fact_reasoning_kernel *k,
		fact_individual_expression *i, fact_actor **actor)
{
	k->p->getSameAs(i->p,(*actor)->p);
}

bool fact_is_same_individuals (fact_reasoning_kernel *k,
		fact_individual_expression *i,
		fact_individual_expression *j)
{
	return k->p->isSameIndividuals(i->p,j->p);
}
bool fact_is_instance (fact_reasoning_kernel *k,
		fact_individual_expression *i,
		fact_concept_expression *c)
{
	return k->p->isInstance(i->p,c->p);
}
bool fact_is_related (fact_reasoning_kernel *k,
		fact_individual_expression *i,
		fact_o_role_expression *r,
		fact_individual_expression *j)
{
	return k->p->isRelated(i->p,r->p,j->p);
}
