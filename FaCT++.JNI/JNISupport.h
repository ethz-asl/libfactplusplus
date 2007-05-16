/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2006-2007 by Dmitry Tsarkov

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

#ifndef _JNISUPPORT_H
#define _JNISUPPORT_H

#include <jni.h>

//-------------------------------------------------------------
// Support functions
//-------------------------------------------------------------

// class for easy dealing with Java strings
class JString
{
private:	// prevent copy
	JString ( const JString& );
	JString& operator = ( const JString& );
protected:
	JNIEnv* env;
	jstring str;
	const char* buf;
public:
	JString ( JNIEnv* e, jstring s ) : env(e), str(s) { buf = env->GetStringUTFChars(str,0); }
	~JString ( void ) { env->ReleaseStringUTFChars(str,buf); }
	const char* operator() ( void ) const { return buf; }
}; // JString

/// record Tree pointers and delete them when became useless (to reduce memory leaks)
class RefRecorder
{
protected:	// types
		/// main repository type
	typedef std::vector<DLTree*> RefVector;
		/// iterator
	typedef RefVector::iterator iterator;

protected:	// members
		/// repository of references
	RefVector refs;

public:		// interface
		/// empty c'tor
	RefRecorder ( void ) {}
		/// d'tor
	~RefRecorder ( void ) { clear(); }

		/// add reference to a repository
	void add ( DLTree* p ) { refs.push_back(p); }
		/// clear repository, free all memory
	void clear ( void )
	{
		for ( iterator p = refs.begin(), p_end = refs.end(); p < p_end; ++p )
			delete *p;
		refs.clear();
	}
}; // RefRecorder

extern RefRecorder RORefRecorder;

inline
void Throw ( JNIEnv * env, const char* reason )
{
	jclass cls = env->FindClass("Ljava/lang/Exception;");
	env->ThrowNew ( cls, reason );
}

/// get Kernel local to given object
inline
ReasoningKernel* getK ( JNIEnv * env, jobject obj )
{
	jclass classThis = env->GetObjectClass(obj);

	if ( classThis == 0 )
	{
		Throw ( env, "Can't get class of 'this'" );
		return NULL;
	}

	jfieldID fid = env->GetFieldID ( classThis, "KernelId", "J" );

	if ( fid == 0 )
	{
		Throw ( env, "Can't get 'KernelId' field" );
		return NULL;
	}

	jlong id = env->GetLongField ( obj, fid );

	// this is a pointer -- should not be NULL
	if ( id == 0 )
	{
		Throw ( env, "Uninitialized FaCT++ kernel found" );
		return NULL;
	}

	return (ReasoningKernel*)id;
}

// helper for getTree which extracts a JLONG from a given object
inline
jlong getPointer ( JNIEnv * env, jobject obj )
{
	jclass classThis = env->GetObjectClass(obj);

	if ( classThis == 0 )
	{
		Throw ( env, "Can't get class of 'this'" );
		return 0;
	}

	jfieldID fid = env->GetFieldID ( classThis, "node", "J" );

	if ( fid == 0 )
	{
		Throw ( env, "Can't get 'node' field" );
		return 0;
	}

	return env->GetLongField ( obj, fid );
}

inline
DLTree* getTree ( JNIEnv * env, jobject obj )
{
	// FIXME!! that's overkill but it is easiest way for now
	return clone((DLTree*)getPointer(env,obj));
}

// use this method is TREE is read-only
inline
DLTree* getROTree ( JNIEnv * env, jobject obj )
{
	return (DLTree*)getPointer(env,obj);
}

inline
TDataInterval* getFacet ( JNIEnv * env, jobject obj )
{
	return (TDataInterval*)getPointer(env,obj);
}

// check it pointer is a named concept
template<class T>
jlong getId ( T* p ATTR_UNUSED ) { return 0; }

//
template<>
jlong getId ( DLTree* p )
{
	switch ( p->Element().getToken() )
	{
	case TOP:		return 1;
	case BOTTOM:	return -1;
	case NAME:		return (jlong)p->Element().getName();
	default:		return 0;
	}
}

// add DLTree* references to a recorder
template<class T>
void registerPointer ( T* p ATTR_UNUSED ) {}

template<>
void registerPointer ( DLTree* p ) { RORefRecorder.add(p); }

template<class T>
jobject retObject ( JNIEnv * env, T* t, const char* className )
{
	if ( t == NULL )
	{
		Throw ( env, "Incorrect operand by FaCT++ Kernel" );
		return (jobject)0;
	}

	// all references that are passed to objects become read-only:
	// they either being cloned, or used as a const*
	registerPointer(t);

	jclass classPointer = env->FindClass(className);

	if ( classPointer == 0 )
	{
		Throw ( env, "Can't get class for Pointer" );
		return (jobject)0;
	}

	jmethodID methodCtor = env->GetMethodID ( classPointer, "<init>", "()V" );

	if ( methodCtor == 0 )
	{
		Throw ( env, "Can't get c'tor for Pointer" );
		return (jobject)0;
	}

	// create an object to return
	jobject obj = env->NewObject ( classPointer, methodCtor );

	if ( obj == 0 )
	{
		Throw ( env, "Can't create Pointer object" );
		return (jobject)0;
	}

	jfieldID fid = env->GetFieldID ( classPointer, "node", "J" );

	if ( fid == 0 )
	{
		Throw ( env, "Can't get 'node' field" );
		return (jobject)0;
	}

	// put the value to return
	env->SetLongField ( obj, fid, (jlong)t );

	// set unique name (if necessary)
	jlong id = getId(t);

	if ( id )	// t is a DLTree*
	{
		fid = env->GetFieldID ( classPointer, "id", "J" );

		if ( fid == 0 )
		{
			Throw ( env, "Can't get 'id' field" );
			return (jobject)0;
		}

		// put the value to return
		env->SetLongField ( obj, fid, id );
	}

	return obj;
}

inline
jobject Class ( JNIEnv * env, DLTree* t )
{
	return retObject ( env, t, "Luk/ac/manchester/cs/factplusplus/ClassPointer;" );
}

inline
jobject Individual ( JNIEnv * env, DLTree* t )
{
	return retObject ( env, t, "Luk/ac/manchester/cs/factplusplus/IndividualPointer;" );
}

inline
jobject ObjectProperty ( JNIEnv * env, DLTree* t )
{
	return retObject ( env, t, "Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;" );
}

inline
jobject DataProperty ( JNIEnv * env, DLTree* t )
{
	return retObject ( env, t, "Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;" );
}

inline
jobject DataType ( JNIEnv * env, DLTree* t )
{
	return retObject ( env, t, "Luk/ac/manchester/cs/factplusplus/DataTypePointer;" );
}

inline
jobject DataTypeExpression ( JNIEnv * env, DLTree* t )
{
	return retObject ( env, t, "Luk/ac/manchester/cs/factplusplus/DataTypeExpressionPointer;" );
}

inline
jobject DataValue ( JNIEnv * env, DLTree* t )
{
	return retObject ( env, t, "Luk/ac/manchester/cs/factplusplus/DataValuePointer;" );
}

inline
jobject Facet ( JNIEnv * env, TDataInterval* t )
{
	return retObject ( env, t, "Luk/ac/manchester/cs/factplusplus/DataTypeFacet;" );
}

#endif
