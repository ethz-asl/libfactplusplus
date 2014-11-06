
#include "tConcept.h"
#include "TimeMetricsHelper.h"

/// init c'tor: read setup, create dirs, open files
TimeMetricsHelper :: TimeMetricsHelper ( void )
{

}

/// d'tor: flush streams, close files
TimeMetricsHelper :: ~TimeMetricsHelper ( void )
{

}

// interface calls

/// mark the beginning of the subsumption check
void
TimeMetricsHelper :: subsumptionStart ( const TConcept* p, const TConcept* q )
{

}

/// mark the finish of the subsumption check
void
TimeMetricsHelper :: subsumptionFinish ( bool result )
{

}


