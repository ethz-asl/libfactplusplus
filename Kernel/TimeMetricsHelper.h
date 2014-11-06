#ifndef TIMEMETRICSHELPER_H
#define TIMEMETRICSHELPER_H

/// class for gather and dump different timings
class TimeMetricsHelper
{
public:		// interface
		/// init c'tor: read setup, create dirs, open files
	TimeMetricsHelper ( void );
		/// d'tor: flush streams, close files
	~TimeMetricsHelper ( void );

	// interface calls

		/// mark the stage of the reasoning process
		/// consistency check or traversal, wrt CONSISTENCY
		/// start or finish of the stage, wrt START
	void markStage ( bool consistency, bool start );
		/// mark the beginning of the subsumption check
	void subsumptionStart ( const TConcept* p, const TConcept* q );
		/// mark the finish of the subsumption check
	void subsumptionFinish ( bool result );
}; // TimeMetricsHelper

#endif
