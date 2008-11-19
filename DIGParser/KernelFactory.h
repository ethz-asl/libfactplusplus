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

#ifndef _KERNELFACTORY_H
#define _KERNELFACTORY_H

#include <string>
#include <map>

class ReasoningKernel;

/// class for mapping names to Reasoning Kernels
class KernelFactory
{
public:		/// type definitions
	typedef std::map<std::string,ReasoningKernel*> KernelCollection;
private:	// C'tors
	/// no copy c'tor
	KernelFactory ( KernelFactory& );
	/// no assignment
	KernelFactory& operator = ( KernelFactory& );

protected:	// members
		/// map name->Kernel
	KernelCollection Factory;

protected:	// methods
	/// create new unique name (using count and date+time)
	void createName ( std::string& id );
	/// create new KB with given name. @return true if there exists one
	bool createKB ( const std::string& id );

public:		// interface
	/// C'tor: init the default kernel
	KernelFactory ( void ) { createKB("default"); }

	/// D'tor: delete all kernels
	~KernelFactory ( void );

	/// create a new KB, put it's name to an 'id'. @return true if unable to proceed
	bool newKB ( std::string& id )
	{
		createName(id);
		return createKB (id);
	}
	/// release existing KB with given name. @return true if unable to proceed
	bool releaseKB ( const std::string& id );

	/** get access to KB with given id.
	 *  @return 'default' KB for empty id.
	 *  @return NULL for incorrect (not registered) ID
	 */
	ReasoningKernel* getKernel ( const std::string& Id ) const;
	/// special access to a 'default' kernel
	ReasoningKernel* getKernel ( void ) const
		{ return getKernel ( "default" ); }
}; // KernelFactory

#endif
