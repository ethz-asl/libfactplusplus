/*
 * test.c
 *
 *  Created on: Mar 30, 2011
 *      Author: tsarkov
 */

#include <stdio.h>
#include "fact.h"

int main ( void )
{
	// create kernel
	fact_reasoning_kernel* k = fact_reasoning_kernel_new();

	// create classes C,D, property R
	fact_concept_expression* c = fact_concept(k,"C");
	fact_concept_expression* d = fact_concept(k,"D");
	fact_o_role_expression* r = fact_object_role(k,"R");

	// create C [= ER.T, ER.T [= D
	fact_concept_expression* some = fact_o_exists ( k, r, fact_top(k));
	fact_implies_concepts ( k, c, some );
	fact_implies_concepts ( k, some, d );

	// classify KB is not necessary: it's done automatically depending on a query
	fact_classify_kb(k);

	// check whether C [= D
	if ( fact_is_subsumed_by(k,c,d) )
		puts("yes!\n");
	else
		puts("No...\n");

	// create a concept actor and use it to get all superclasses of D
	fact_actor* actor = fact_concept_actor_new();
	fact_get_sup_concepts(k,c,false,&actor);
	const char*** names = fact_get_elements_2d(actor);
	printf("[\n");
	int n,m;
	for ( const char** syns = names[n=0]; syns != NULL; syns=names[++n] )
	{
		printf("[");
		for ( const char* name = syns[m=0]; name != NULL; name=syns[++m] )
			printf("%s ", name);
		printf("]\n");
	}
	printf("]\n");
	fact_actor_free(actor);

	// we done so let's free memory
	fact_reasoning_kernel_free(k);
	return 0;
}
