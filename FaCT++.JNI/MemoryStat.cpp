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

// set to 1 for memory logging
#define USE_MEMORY_LOG 0

#if USE_MEMORY_LOG
std::ofstream StatLogFile("MemoryLog.txt");
#endif

#ifdef __linux__
#	include <sys/sysinfo.h>
#endif

#ifdef __APPLE__
#	include <mach/mach.h>
#endif

#ifdef _WINDOWS
#	include <windows.h>
#	include <psapi.h>
#else
#	include <sys/resource.h>
#endif

static size_t getProcessMemory ( bool resident = true )
{
#ifdef __APPLE__
	struct task_basic_info t_info;
	mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

	if (KERN_SUCCESS != task_info(mach_task_self(),
                              	  TASK_BASIC_INFO, (task_info_t)&t_info,
                              	  &t_info_count))
		return 0;
	return resident ? t_info.resident_size : t_info.virtual_size;
#elif defined(_WINDOWS)
	PROCESS_MEMORY_COUNTERS pmc;
	if ( 0 == GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)) )
		return 0;
//	return resident ? pmc.WorkingSetSize : pmc.PrivateUsage;
	return resident ? pmc.WorkingSetSize : pmc.PagefileUsage;
#else	// undefined platform
	return 0;
#endif
}

MemoryStatistics :: ~MemoryStatistics ( void )
{
#if USE_MEMORY_LOG
	timer.Stop();
	StatLogFile << operation.c_str() << ": time " << timer << " sec, memory " << getProcessMemory()/1024 << " kb\n";
	timer.Reset();
#endif
}
