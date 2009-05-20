/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2006-2009 by Dmitry Tsarkov

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

#ifndef _JNISUPPORT_H
#define _JNISUPPORT_H

#include <jni.h>

//-------------------------------------------------------------
// class names for different Java classes corresponding to FaCT++ structures
//-------------------------------------------------------------

inline const char* cnClassPointer ( void ) { return "Luk/ac/manchester/cs/factplusplus/ClassPointer;"; }
inline const char* cnIndividualPointer ( void ) { return "Luk/ac/manchester/cs/factplusplus/IndividualPointer;"; }
inline const char* cnObjectPropertyPointer ( void ) { return "Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;"; }
inline const char* cnDataPropertyPointer ( void ) { return "Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;"; }
inline const char* cnDataTypePointer ( void ) { return "Luk/ac/manchester/cs/factplusplus/DataTypePointer;"; }
inline const char* cnDataTypeExpressionPointer ( void ) { return "Luk/ac/manchester/cs/factplusplus/DataTypeExpressionPointer;"; }
inline const char* cnDataValuePointer ( void ) { return "Luk/ac/manchester/cs/factplusplus/DataValuePointer;"; }


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
	void add ( DLTree* p )
	{
#	ifdef JNI_TRACING
		std::cerr << "Registering (" << (void*)p << ")" << p << "\n";
#	endif
		refs.push_back(p);
	}
		/// check whether P is in the repository
	bool in ( DLTree* p ) const { return std::find ( refs.begin(), refs.end(), p ) != refs.end(); }
		/// clear repository, free all memory
	void clear ( void )
	{
		for ( iterator p = refs.begin(), p_end = refs.end(); p < p_end; ++p )
		{
#		ifdef JNI_TRACING
			std::cerr << "Deleting (" << (void*)(*p) << ")" << *p << "\n";
#		endif
			deleteTree(*p);
		}
		refs.clear();
	}
}; // RefRecorder

/// kernel with a memory management component
class MMKernel
{
public:		// members
	ReasoningKernel* pKernel;
	RefRecorder* pRefRecorder;
public:		// interface
		/// c'tor
	MMKernel ( void )
	{
		pKernel = new ReasoningKernel();
		pKernel->newKB();
		pRefRecorder = new RefRecorder();
	}
		/// d'tor
	~MMKernel ( void )
	{
		delete pRefRecorder;
		delete pKernel;
	}
}; // MMKernel

// current kernel + reference recorder
extern MMKernel* curKernel;

/// throw exception with a given signature
inline
void ThrowExc ( JNIEnv * env, const char* reason, const char* className )
{
	jclass cls = env->FindClass(className);
	env->ThrowNew ( cls, reason );
}

/// throw general Java exception
inline
void ThrowGen ( JNIEnv* env, const char* reason )
{
	ThrowExc ( env, reason, "Ljava/lang/Exception;" );
}

/// throw general FaCT++ exception
inline
void Throw ( JNIEnv* env, const char* reason )
{
	ThrowExc ( env, reason, "Luk/ac/manchester/cs/factplusplus/FaCTPlusPlusException;" );
}

/// throw Inconsistent Ontology exception
inline
void ThrowICO ( JNIEnv* env )
{
	ThrowExc ( env, "FaCT++.Kernel: inconsistent ontology", "Luk/ac/manchester/cs/factplusplus/InconsistentOntologyException;" );
}

/// throw CR for non-simple role exception
inline
void ThrowNSR ( JNIEnv* env, const char* reason )
{
	ThrowExc ( env, reason, "Luk/ac/manchester/cs/factplusplus/NonSimpleRoleInNumberRestrictionException;" );
}

/// throw Role Inclusion Cycle exception
inline
void ThrowRIC ( JNIEnv* env, const char* reason )
{
	ThrowExc ( env, reason, "Luk/ac/manchester/cs/factplusplus/RoleInclusionCycleException;" );
}

/// get Kernel local to given object
// as a side effect sets up curKernel
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

	curKernel = (MMKernel*)id;
	return curKernel->pKernel;
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

inline
TDLAxiom* getAxiom ( JNIEnv * env, jobject obj )
{
	return (TDLAxiom*)getPointer(env,obj);
}

// check it pointer is a named concept
template<class T>
inline
jlong getId ( T* p ATTR_UNUSED ) { return 0; }

// specialisation for the DLTree
template<>
inline
jlong getId ( DLTree* p )
{
	switch ( p->Element().getToken() )
	{
	case TOP:		return 1;
	case BOTTOM:	return -1;
	case NAME:
#	ifdef JNI_TRACING
		std::cerr << "ID for " << TokenName(p->Element().getToken()) << p << ": " << (jlong)p->Element().getNE() << "\n";
#	endif
		return (jlong)p->Element().getNE();
	default:		return 0;
	}
}

// add DLTree* references to a recorder
template<class T>
inline
void registerPointer ( T* p ATTR_UNUSED ) {}

template<>
inline
void registerPointer ( DLTree* p )
{
	assert(curKernel != NULL );
	curKernel->pRefRecorder->add(p);
}

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
	return retObject ( env, t, cnClassPointer() );
}

inline
jobject Individual ( JNIEnv * env, DLTree* t )
{
	return retObject ( env, t, cnIndividualPointer() );
}

inline
jobject ObjectProperty ( JNIEnv * env, DLTree* t )
{
	return retObject ( env, t, cnObjectPropertyPointer() );
}

inline
jobject DataProperty ( JNIEnv * env, DLTree* t )
{
	return retObject ( env, t, cnDataPropertyPointer() );
}

inline
jobject DataType ( JNIEnv * env, DLTree* t )
{
	return retObject ( env, t, cnDataTypePointer() );
}

inline
jobject DataTypeExpression ( JNIEnv * env, DLTree* t )
{
	return retObject ( env, t, cnDataTypeExpressionPointer() );
}

inline
jobject DataValue ( JNIEnv * env, DLTree* t )
{
	return retObject ( env, t, cnDataValuePointer() );
}

inline
jobject Facet ( JNIEnv * env, TDataInterval* t )
{
	return retObject ( env, t, "Luk/ac/manchester/cs/factplusplus/DataTypeFacet;" );
}

inline
jobject Axiom ( JNIEnv * env, TDLAxiom* t )
{
	return retObject ( env, t, "Luk/ac/manchester/cs/factplusplus/AxiomPointer;" );
}

/// create vector of Java objects of class CLASSNAME by given VEC
inline
jobjectArray buildArray ( JNIEnv* env, const std::vector<DLTree*>& vec, const char* className )
{
	jclass objClass = env->FindClass(className);
	jobjectArray ret = env->NewObjectArray ( vec.size(), objClass, NULL );
	for ( unsigned int i = 0; i < vec.size(); ++i )
		env->SetObjectArrayElement ( ret, i, retObject ( env, vec[i], className ) );
	return ret;
}

#endif
