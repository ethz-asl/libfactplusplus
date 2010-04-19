/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2006-2010 by Dmitry Tsarkov

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

#ifndef JNISUPPORT_H
#define JNISUPPORT_H

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
// Expression typedefs
//-------------------------------------------------------------

	/// general expression
typedef ReasoningKernel::TExpr TExpr;
	/// concept expression
typedef ReasoningKernel::TConceptExpr TConceptExpr;
	/// individual expression
typedef ReasoningKernel::TIndividualExpr TIndividualExpr;
	/// role expression
typedef ReasoningKernel::TRoleExpr TRoleExpr;
	/// object role complex expression (including role chains and projections)
typedef ReasoningKernel::TORoleComplexExpr TORoleComplexExpr;
	/// object role expression
typedef ReasoningKernel::TORoleExpr TORoleExpr;
	/// data role expression
typedef ReasoningKernel::TDRoleExpr TDRoleExpr;
	/// data expression
typedef ReasoningKernel::TDataExpr TDataExpr;
	/// data value expression
typedef ReasoningKernel::TDataValueExpr TDataValueExpr;

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

	return (ReasoningKernel*)id;
}

/// get the expression manager corresponding local kernel
inline
TExpressionManager* getEM ( JNIEnv * env, jobject obj ) { return getK(env,obj)->getExpressionManager(); }

// get trees for the names in the unified way

/// get tree for the class name by the EM and the name
inline
TConceptExpr* getCName ( TExpressionManager* EM, const std::string& name ) { return EM->Concept(name); }
/// get tree for the class name by the env:obj and the name
inline
TConceptExpr* getCName ( JNIEnv * env, jobject obj, const std::string& name ) { return getCName ( getEM(env,obj), name ); }
/// get tree for the individual name by the EM and the name
inline
TIndividualExpr* getIName ( TExpressionManager* EM, const std::string& name ) { return EM->Individual(name); }
/// get tree for the individual name by the env:obj and the name
inline
TIndividualExpr* getIName ( JNIEnv * env, jobject obj, const std::string& name ) { return getIName ( getEM(env,obj), name ); }
/// get tree for the object property name by the EM and the name
inline
TORoleExpr* getOName ( TExpressionManager* EM, const std::string& name ) { return EM->ObjectRole(name); }
/// get tree for the object property name by the env:obj and the name
inline
TORoleExpr* getOName ( JNIEnv * env, jobject obj, const std::string& name ) { return getOName ( getEM(env,obj), name ); }
/// get tree for the data property name by the EM and the name
inline
TDRoleExpr* getDName ( TExpressionManager* EM, const std::string& name ) { return EM->DataRole(name); }
/// get tree for the data property name by the env:obj and the name
inline
TDRoleExpr* getDName ( JNIEnv * env, jobject obj, const std::string& name ) { return getDName ( getEM(env,obj), name ); }

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

// macro to expand into the accessor function that transforms pointer into appropriate type
#define ACCESSOR(Name)	\
inline T ## Name* get ## Name ( JNIEnv * env, jobject obj ) {	\
	return dynamic_cast<T ## Name*>((TExpr*)getPointer(env,obj)); }

// accessors for different expression types
ACCESSOR(Expr)
ACCESSOR(ConceptExpr)
ACCESSOR(IndividualExpr)
ACCESSOR(RoleExpr)
ACCESSOR(ORoleComplexExpr)
ACCESSOR(ORoleExpr)
ACCESSOR(DRoleExpr)
ACCESSOR(DataExpr)
ACCESSOR(DataValueExpr)

#undef ACCESSOR

// macro to expand into the RO accessor function that transforms pointer into appropriate type
#define ACCESSOR(Name)	\
inline const T ## Name* getRO ## Name ( JNIEnv * env, jobject obj ) {	\
	return dynamic_cast<const T ## Name*>((const TExpr*)getPointer(env,obj)); }

// accessors for different expression types
ACCESSOR(Expr)
ACCESSOR(ConceptExpr)
ACCESSOR(IndividualExpr)
ACCESSOR(RoleExpr)
ACCESSOR(ORoleComplexExpr)
ACCESSOR(ORoleExpr)
ACCESSOR(DRoleExpr)
ACCESSOR(DataExpr)
ACCESSOR(DataValueExpr)

#undef ACCESSOR

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

template<class T>
jobject retObject ( JNIEnv * env, T* t, const char* className )
{
	if ( t == NULL )
	{
		Throw ( env, "Incorrect operand by FaCT++ Kernel" );
		return (jobject)0;
	}

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
jobject Class ( JNIEnv * env, TConceptExpr* t )
{
	return retObject ( env, t, cnClassPointer() );
}

inline
jobject Individual ( JNIEnv * env, TIndividualExpr* t )
{
	return retObject ( env, t, cnIndividualPointer() );
}

inline
jobject ObjectProperty ( JNIEnv * env, TORoleExpr* t )
{
	return retObject ( env, t, cnObjectPropertyPointer() );
}

inline
jobject ObjectComplex ( JNIEnv * env, TORoleComplexExpr* t )
{
	return retObject ( env, t, cnObjectPropertyPointer() );
}

inline
jobject DataProperty ( JNIEnv * env, TDRoleExpr* t )
{
	return retObject ( env, t, cnDataPropertyPointer() );
}

inline
jobject DataType ( JNIEnv * env, TDataExpr* t )
{
	return retObject ( env, t, cnDataTypePointer() );
}

inline
jobject DataTypeExpression ( JNIEnv * env, TDataExpr* t )
{
	return retObject ( env, t, cnDataTypeExpressionPointer() );
}

inline
jobject DataValue ( JNIEnv * env, TDataValueExpr* t )
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
jobjectArray buildArray ( JNIEnv* env, const std::vector<TExpr*>& vec, const char* className )
{
	jclass objClass = env->FindClass(className);
	jobjectArray ret = env->NewObjectArray ( vec.size(), objClass, NULL );
	for ( unsigned int i = 0; i < vec.size(); ++i )
		env->SetObjectArrayElement ( ret, i, retObject ( env, vec[i], className ) );
	return ret;
}

#endif
