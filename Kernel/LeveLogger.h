/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2006 by Dmitry Tsarkov

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

#ifndef _LEVELOGGER_H
#define _LEVELOGGER_H

#include <fstream>

// define unused attribute for parameters
#if defined(__GNUC__) && (__GNUC__ >= 4)
#	define ATTR_UNUSED __attribute__((unused))
#else
#	define ATTR_UNUSED
#endif

class Configuration;

class LeveLogger
{
private:	// members
		/// the allowed logging level; init it once for the whole session
	unsigned int allowedLevel;

public:		// interface
		/// c'tor
	LeveLogger ( void ) : allowedLevel(0) {}
		/// d'tor: just empty
	~LeveLogger ( void ) {}

		/// sets output file and allowedLevel by given values
	bool initLogger ( unsigned int l, const char* filename );
		/** sets output file and allowedLevel by config file.
		 *  The values will be taken from Config|[LeveLogger]|{allowedLevel,file}
		 *  Default level is 0, but the file name is mandatory.
		 */
	bool initLogger ( Configuration& Config );

		/// @return true if LEVEL will be allowed
	bool isWritable ( unsigned int level ATTR_UNUSED ) const
	{
#	ifdef _USE_LOGGING
		return level <= allowedLevel;
#	else
		return false;
#	endif
	}
}; // LeveLogger

/// the only main logger/manager for all usage
extern LeveLogger LLM;
/// file stream to be used by LLM; be sure that every OP with LL is guarded by LLM
extern std::ofstream LL;

// macro for checking if LL is writable and then return
#define CHECK_LL_RETURN(val)	\
	if (!LLM.isWritable(val)) return

// macro for checking if LL is writable and then return value
#define CHECK_LL_RETURN_VALUE(val,ret)	\
	if (!LLM.isWritable(val)) return (ret)

#endif // _LEVELOGGER_H
