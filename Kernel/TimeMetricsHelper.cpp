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

#include <iomanip>
#include <fstream>

#include "tConcept.h"
#include "TimeMetricsHelper.h"
#include "dir_util.h"


/// init c'tor: read setup, create dirs, open files
TimeMetricsHelper :: TimeMetricsHelper ( void )
{
	// init input values

	std::ifstream i("fpp_args.txt");
	// that is fine since none of the strings have ws
	i >> dir >> reasonerId >> experimentId;
	i.close();

	// create requested directory if necessary
	dirCreate(dir.c_str());

	// create $dir/tmp
	std::string tmp_dir(dir+"/tmp");
	dirCreate(tmp_dir.c_str());

	// create file for sub test stats
	std::string file_sub(dir+"/"+experimentId+"subtestfile.txt");
	o.open(file_sub.c_str());

	static SubTest st;
	pCur = &st;
}

// output

	/// print the nano-time out of a timeval
inline std::ostream&
operator << ( std::ostream& o, const timeval& tv )
{
	o << "," << tv.tv_sec << std::setw(6) << tv.tv_usec << "000";
	return o;
}
	/// print the concept name
inline std::ostream&
operator << ( std::ostream& o, const TConcept* pC )
{
	if ( unlikely(pC->isTop()) )
		o << "owl:Thing,";
	else if ( unlikely(pC->isBottom()) )
		o << "owl:Nothing,";
	else
		o << pC->getName() << ",";
	return o;
}

/// print the subsumption test results
void
TimeMetricsHelper :: printST ( std::ostream& o, const SubTest& st ) const
{
	o << reasonerId.c_str() << "," << st.subConcept << st.supConcept << "fullsat,"
	  << (st.result?"true":"false") << st.start << st.finish << "\n";
}

// interface calls

/// move pCur to the next available slot
void
TimeMetricsHelper :: progressCur ( void )
{

}

/// d'tor: flush streams, close files
TimeMetricsHelper :: ~TimeMetricsHelper ( void )
{
	// create time output file if necessary
	std::string file_stage(dir+"/classificationmetadata_fact.csv");
	std::ofstream ofs ( file_stage.c_str(), std::ofstream::out | std::ofstream::app );
	// reasoner_id, experiment_id
	ofs << reasonerId.c_str() << "," << experimentId.c_str()
	// consistency_start, consistency_finish
		<< *getStageTimeVal(true,true) << *getStageTimeVal(true,false)
	// classification_start, classification_finish
		<< *getStageTimeVal(false,true) << *getStageTimeVal(false,false)
	// end of csv line
		<< "\n";
}


