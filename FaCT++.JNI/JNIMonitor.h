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

#ifndef _JNIMONITOR_H
#define _JNIMONITOR_H

#include "tProgressMonitor.h"
#include "JNISupport.h"

class JNIProgressMonitor: public TProgressMonitor
{
protected:
	JNIEnv* env;
	jobject javaMonitor;
	jmethodID sCS, sCC, sF, iC;
public:
	JNIProgressMonitor ( JNIEnv* Env, jobject obj )
		: TProgressMonitor()
		, env(Env)
		, javaMonitor(obj)
	{
		jclass cls = env->GetObjectClass(obj);
		if ( cls == 0 )
			Throw ( env, "Can't get class of ProgressMonitor object" );
		sCS = env->GetMethodID ( cls, "setClassificationStarted", "(I)V" );
		if ( sCS == 0 )
			Throw ( env, "Can't get method setClassificationStarted" );
		sCC = env->GetMethodID ( cls, "setCurrentClass", "(Ljava/lang/String;)V" );
		if ( sCC == 0 )
			Throw ( env, "Can't get method setCurrentClass" );
		sF = env->GetMethodID ( cls, "setFinished", "()V" );
		if ( sF == 0 )
			Throw ( env, "Can't get method setFinished" );
		iC = env->GetMethodID ( cls, "isCancelled", "()Z" );
		if ( iC == 0 )
			Throw ( env, "Can't get method isCancelled" );
	}
	~JNIProgressMonitor ( void ) {}

		/// informs about beginning of classification with number of concepts to be classified
	virtual void setClassificationStarted ( unsigned int nConcepts )
		{ env->CallVoidMethod ( javaMonitor, sCS, nConcepts ); }
		/// informs about beginning of classification of a given CONCEPT
	virtual void setCurrentClass ( const char* name )
		{ env->CallVoidMethod ( javaMonitor, sCC, env->NewStringUTF(name) ); }
		/// informs that the reasoning is done
	virtual void setFinished ( void ) { env->CallVoidMethod ( javaMonitor, sF ); }
		// @return true iff reasoner have to be stopped
	virtual bool isCancelled ( void ) { return env->CallBooleanMethod ( javaMonitor, iC ); }
}; // JNIProgressMonitor

#endif
