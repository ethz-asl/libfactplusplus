/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2006 by Dmitry Tsarkov

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef _TPROGRESSMONITOR_H
#define _TPROGRESSMONITOR_H

#include "globaldef.h"

/// progress monitor and canceller for the classification
class TProgressMonitor
{
public:
		/// empty c'tor
	TProgressMonitor ( void ) {}
		/// empty d'tor
	virtual ~TProgressMonitor ( void ) {}

	// interface

		/// informs about beginning of classification with number of concepts to be classified
	virtual void setClassificationStarted ( unsigned int nConcepts ATTR_UNUSED ) {}
		/// informs about beginning of classification of a given CONCEPT
	virtual void setCurrentClass ( const char* name ATTR_UNUSED ) {}
		/// informs about beginning of realisation with number of individuals to be classified
	virtual void setRealisationStarted ( unsigned int nIndividuals ATTR_UNUSED ) {}
		/// informs about beginning of classification of a given INDIVIDUAL
	virtual void setCurrentIndividual ( const char* name ATTR_UNUSED ) {}
		/// informs that the reasoning is done
	virtual void setFinished ( void ) {}
		// @return true iff reasoner have to be stopped
	virtual bool isCancelled ( void ) { return false; }
}; // TProgressMonitor

#endif
