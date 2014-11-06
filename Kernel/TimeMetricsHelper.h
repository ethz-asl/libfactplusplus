/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2014 by Dmitry Tsarkov

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef TIMEMETRICSHELPER_H
#define TIMEMETRICSHELPER_H

#include <sys/time.h>
#include <iosfwd>

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
		/// output dir as passed
	std::string dir;
		/// reasoner id as passed
	std::string reasonerId;
		/// experiment id as passed
	std::string experimentId;

		/// out the subsumptions
	std::ofstream o;

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

		/// print the subsumption test results
	void printST ( std::ostream& o, const SubTest& st ) const;

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
