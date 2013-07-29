/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2013 by Dmitry Tsarkov

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

#include <fstream>

#include "MemoryStat.h"

#if 0
std::ofstream StatLogFile("MemoryLog.txt");

#include<mach/mach.h>

static long getProcessMemory ( void )
{
	struct task_basic_info t_info;
	mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

	if (KERN_SUCCESS != task_info(mach_task_self(),
                              	  TASK_BASIC_INFO, (task_info_t)&t_info,
                              	  &t_info_count))
		return 0;
	return t_info.resident_size;
}

#include "windows.h"
#include "psapi.h"

static long getProcessMemory ( void )
{
	PROCESS_MEMORY_COUNTERS_EX pmc;
	if ( 0 == GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)) )
		return 0;
	SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;
	SIZE_T physMemUsedByMe = pmc.WorkingSetSize;
	return physMemUsedByMe;
}

MemoryStatistics :: ~MemoryStatistics ( void )
{
	timer.Stop();
	StatLogFile << operation.c_str() << ": time " << timer << " sec, memory " << getProcessMemory()/1024 << " kb\n";
	timer.Reset();
}
#else
MemoryStatistics :: ~MemoryStatistics ( void ) {}
#endif
