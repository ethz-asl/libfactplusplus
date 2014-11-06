
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

/// mark the stage of the reasoning process
/// consistency check or traversal, wrt CONSISTENCY
/// start or finish of the stage, wrt START
void
TimeMetricsHelper :: markStage ( bool consistency, bool start )
{

}

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


