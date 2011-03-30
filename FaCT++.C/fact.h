#ifndef __FACT_H__
#define __FACT_H__

#ifdef __cplusplus
extern "C" {
#endif

// type declarations

#define DECLARE_STRUCT(name) typedef struct name ## _st name
// FaCT++ kernel
DECLARE_STRUCT(fact_reasoning_kernel);
// progress monitor
DECLARE_STRUCT(fact_progress_monitor);
// expression manager
DECLARE_STRUCT(fact_expression_manager);
// axiom
DECLARE_STRUCT(fact_axiom);
// expression
DECLARE_STRUCT(fact_expression);
// concept expression
DECLARE_STRUCT(fact_concept_expression);
// data- or object-role expression
DECLARE_STRUCT(fact_role_expression);
// object role expression
DECLARE_STRUCT(fact_o_role_expression);
// complex object role expression
DECLARE_STRUCT(fact_o_role_complex_expression);
// data role expression
DECLARE_STRUCT(fact_d_role_expression);
// individual expression
DECLARE_STRUCT(fact_individual_expression);
// general data expression
DECLARE_STRUCT(fact_data_expression);
// data type expression
DECLARE_STRUCT(fact_data_type_expression);
// data value expression
DECLARE_STRUCT(fact_data_value_expression);
// actor to traverse taxonomy
DECLARE_STRUCT(fact_actor);

#undef DECLARE_STRUCT

const char *fact_get_version ();

fact_reasoning_kernel *fact_reasoning_kernel_new (void);
void fact_reasoning_kernel_free (fact_reasoning_kernel *);

/*
ifOptionSet* getOptions (  );
const ifOptionSet* getOptions (  );
 */

bool fact_is_kb_preprocessed (fact_reasoning_kernel *);
bool fact_is_kb_classified (fact_reasoning_kernel *);
bool fact_is_kb_realised (fact_reasoning_kernel *);
void fact_set_progress_monitor (fact_reasoning_kernel *, fact_progress_monitor *);

void fact_set_verbose_output (fact_reasoning_kernel *, bool value);

void fact_set_top_bottom_role_names (fact_reasoning_kernel *,
		const char *top_b_role_name,
		const char *bot_b_role_name,
		const char *top_d_role_name,
		const char *bot_d_role_name);

void fact_set_operation_timeout (fact_reasoning_kernel *,
		unsigned long timeout);

fact_expression_manager *fact_get_expression_manager (fact_reasoning_kernel *);

bool fact_new_kb (fact_reasoning_kernel *);
bool fact_release_kb (fact_reasoning_kernel *);
bool fact_clear_kb (fact_reasoning_kernel *);

fact_axiom *fact_declare (fact_reasoning_kernel *, fact_expression *c);
fact_axiom *fact_implies_concepts (fact_reasoning_kernel *,
		fact_concept_expression *c,
		fact_concept_expression *d);
fact_axiom *fact_equal_concepts (fact_reasoning_kernel *);
fact_axiom *fact_disjoint_concepts (fact_reasoning_kernel *);
fact_axiom *fact_disjoint_union (fact_reasoning_kernel *,
		fact_concept_expression *C);


fact_axiom *fact_set_inverse_roles (fact_reasoning_kernel *,
		fact_o_role_expression *r,
		fact_o_role_expression *s);
fact_axiom *fact_implies_o_roles (fact_reasoning_kernel *,
		fact_o_role_complex_expression *r,
		fact_o_role_expression *s);
fact_axiom *fact_implies_d_roles (fact_reasoning_kernel *,
		fact_d_role_expression *r,
		fact_d_role_expression *s);
fact_axiom *fact_equal_o_roles (fact_reasoning_kernel *);
fact_axiom *fact_equal_d_roles (fact_reasoning_kernel *);
fact_axiom *fact_disjoint_o_roles (fact_reasoning_kernel *);
fact_axiom *fact_disjoint_d_roles (fact_reasoning_kernel *);

fact_axiom* fact_set_o_domain (fact_reasoning_kernel *,
		fact_o_role_expression *r,
		fact_concept_expression *c);
fact_axiom *fact_set_d_domain (fact_reasoning_kernel *,
		fact_d_role_expression *r,
		fact_concept_expression *c);
fact_axiom *fact_set_o_range (fact_reasoning_kernel *,
		fact_o_role_expression *r,
		fact_concept_expression *c);
fact_axiom *fact_set_d_range (fact_reasoning_kernel *,
		fact_d_role_expression *r,
		fact_data_expression *e);

fact_axiom *fact_set_transitive (fact_reasoning_kernel *,
		fact_o_role_expression *r);
fact_axiom *fact_set_reflexive (fact_reasoning_kernel *,
		fact_o_role_expression *r);
fact_axiom *fact_set_irreflexive (fact_reasoning_kernel *,
		fact_o_role_expression *r);
fact_axiom *fact_set_symmetric (fact_reasoning_kernel *,
		fact_o_role_expression *r);
fact_axiom *fact_set_asymmetric (fact_reasoning_kernel *,
		fact_o_role_expression *r);
fact_axiom *fact_set_o_functional (fact_reasoning_kernel *,
		fact_o_role_expression *r);
fact_axiom *fact_set_d_functional (fact_reasoning_kernel *,
		fact_d_role_expression *r);
fact_axiom *fact_set_inverse_functional (fact_reasoning_kernel *,
		fact_o_role_expression *r);

fact_axiom *fact_instance_of (fact_reasoning_kernel *,
		fact_individual_expression *i,
		fact_concept_expression *c);
fact_axiom *fact_related_to (fact_reasoning_kernel *,
		fact_individual_expression *i,
		fact_o_role_expression *r,
		fact_individual_expression *j);
fact_axiom *fact_related_to_not (fact_reasoning_kernel *,
		fact_individual_expression *i,
		fact_o_role_expression *r,
		fact_individual_expression *j);
fact_axiom *fact_value_of (fact_reasoning_kernel *,
		fact_individual_expression *i,
		fact_d_role_expression *a,
		fact_data_value_expression *v);
fact_axiom *fact_value_of_not (fact_reasoning_kernel *,
		fact_individual_expression *i,
		fact_d_role_expression *a,
		fact_data_value_expression *v);
fact_axiom *fact_process_same (fact_reasoning_kernel *);
fact_axiom *fact_process_different (fact_reasoning_kernel *);
fact_axiom *fact_set_fairness_constraint (fact_reasoning_kernel *);

void fact_retract (fact_reasoning_kernel *, fact_axiom *axiom);

bool fact_is_kb_consistent (fact_reasoning_kernel *);
void fact_preprocess_kb (fact_reasoning_kernel *);
void fact_classify_kb (fact_reasoning_kernel *);
void fact_realise_kb (fact_reasoning_kernel *);

bool fact_is_o_functional (fact_reasoning_kernel *, fact_o_role_expression *r);
bool fact_is_d_functional (fact_reasoning_kernel *,
		fact_d_role_expression *r);
bool fact_is_inverse_functional (fact_reasoning_kernel *,
		fact_o_role_expression *r);
bool fact_is_transitive (fact_reasoning_kernel *, fact_o_role_expression *r);
bool fact_is_symmetric (fact_reasoning_kernel *, fact_o_role_expression *r);
bool fact_is_asymmetric (fact_reasoning_kernel *, fact_o_role_expression *r);
bool fact_is_reflexive (fact_reasoning_kernel *, fact_o_role_expression *r);
bool fact_is_irreflexive (fact_reasoning_kernel *, fact_o_role_expression *r);
bool fact_is_sub_o_roles (fact_reasoning_kernel *, fact_o_role_expression *r,
		fact_o_role_expression *s);
bool fact_is_sub_d_roles (fact_reasoning_kernel *, fact_d_role_expression *r,
		fact_d_role_expression *s);
bool fact_is_disjoint_o_roles (fact_reasoning_kernel *,
		fact_o_role_expression *r,
		fact_o_role_expression *s);
bool fact_is_disjoint_d_roles (fact_reasoning_kernel *,
		fact_d_role_expression *r,
		fact_d_role_expression *s);
bool fact_is_disjoint_roles (fact_reasoning_kernel *);
bool fact_is_sub_chain (fact_reasoning_kernel *, fact_o_role_expression *r);

bool fact_is_satisfiable (fact_reasoning_kernel *, fact_concept_expression *c);
bool fact_is_subsumed_by (fact_reasoning_kernel *, fact_concept_expression *c,
		fact_concept_expression *d);
bool fact_is_disjoint (fact_reasoning_kernel *, fact_concept_expression *c,
		fact_concept_expression *d);
bool fact_is_equivalent (fact_reasoning_kernel *, fact_concept_expression *c,
		fact_concept_expression *d);

void fact_get_sup_concepts (fact_reasoning_kernel *, fact_concept_expression *c,
		bool direct, fact_actor **actor);
void fact_get_sub_concepts (fact_reasoning_kernel *, fact_concept_expression *c,
		bool direct, fact_actor **actor);
void fact_get_equivalent_concepts (fact_reasoning_kernel *,
		fact_concept_expression *c,
		fact_actor **actor);
void fact_get_disjoint_concepts (fact_reasoning_kernel *,
		fact_concept_expression *c,
		fact_actor **actor);

void fact_get_sup_roles (fact_reasoning_kernel *, fact_role_expression *r,
		bool direct, fact_actor **actor);
void fact_get_sub_roles (fact_reasoning_kernel *, fact_role_expression *r,
		bool direct, fact_actor **actor);
void fact_get_equivalent_roles (fact_reasoning_kernel *, fact_role_expression *r,
		fact_actor **actor);
void fact_get_role_domain (fact_reasoning_kernel *, fact_role_expression *r,
		bool direct, fact_actor **actor);
void fact_get_role_range (fact_reasoning_kernel *, fact_o_role_expression *r,
		bool direct, fact_actor **actor);
void fact_get_direct_instances (fact_reasoning_kernel *,
		fact_concept_expression *c, fact_actor **actor);
void fact_get_instances (fact_reasoning_kernel *, fact_concept_expression *c,
		fact_actor **actor);
void fact_get_types (fact_reasoning_kernel *, fact_individual_expression *i,
		bool direct, fact_actor **actor);
void fact_get_same_as (fact_reasoning_kernel *,
		fact_individual_expression *i, fact_actor **actor);

bool fact_is_same_individuals (fact_reasoning_kernel *,
		fact_individual_expression *i,
		fact_individual_expression *j);
bool fact_is_instance (fact_reasoning_kernel *,
		fact_individual_expression *i,
		fact_concept_expression *c);
/*
void fact_get_related_roles (fact_reasoning_kernel *,
			     fact_individual_expression *i,
			     bool data, bool needI,
			     fact_names_vector **result);
void fact_get_role_fillers (fact_reasoning_kernel *,
			    fact_individual_expression *i,
			    fact_o_role_expression *r,
			    fact_individual_set **result);
 */
 bool fact_is_related (fact_reasoning_kernel *,
		 fact_individual_expression *i,
		 fact_o_role_expression *r,
		 fact_individual_expression *j);

#ifdef __cplusplus
}
#endif

#endif
