#ifndef TIMEMETRICSHELPER_H
#define TIMEMETRICSHELPER_H

#include <sys/time.h>

/// class for gather and dump different timings
class TimeMetricsHelper
{
protected:	// classes
		/// structure to keep a single subsumption test data
	struct SubTest
	{
		const TConcept* subConcept;
		const TConcept* supConcept;
		timeval start;
		timeval finish;
		bool result;
	};

protected:	// members
		/// reasoner id as passed
	std::string reasonerId;
		/// experiment id as passed
	std::string experimentId;

		/// time for stages
	timeval stageTime[4];

		/// pointer to the current subsumption test
	SubTest* pCur;

protected:	// methods
		/// get the timeval pointer to the stage time
	timeval* getStageTimeVal ( bool consistency, bool start )
		{ return &stageTime[!consistency*2+!start]; }

		/// move pCur to the next available slot
	void progressCur ( void );

public:		// interface
		/// init c'tor: read setup, create dirs, open files
	TimeMetricsHelper ( void );
		/// d'tor: flush streams, close files
	~TimeMetricsHelper ( void );

	// interface calls

		/// mark the stage of the reasoning process
		/// consistency check or traversal, wrt CONSISTENCY
		/// start or finish of the stage, wrt START
	void markStage ( bool consistency, bool start )
		{ gettimeofday ( getStageTimeVal ( consistency, start ), NULL ); }
		/// mark the beginning of the subsumption check
	void subsumptionStart ( const TConcept* p, const TConcept* q )
	{
		pCur->subConcept = p;
		pCur->supConcept = q;
		gettimeofday ( &pCur->start, NULL );
	}
		/// mark the finish of the subsumption check
	void subsumptionFinish ( bool result )
	{
		pCur->result = result;
		gettimeofday ( &pCur->finish, NULL );
		progressCur();
	}
}; // TimeMetricsHelper

#endif
