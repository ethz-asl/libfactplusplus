/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2006-2013 by Dmitry Tsarkov

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
	/// data type expression
typedef ReasoningKernel::TDataTypeExpr TDataTypeExpr;
	/// data value expression
typedef ReasoningKernel::TDataValueExpr TDataValueExpr;
	/// data facet expression
typedef const TDLFacetExpression TFacetExpr;
	/// completion tree node
typedef const ReasoningKernel::TCGNode TCGNode;

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

/// throw exception with an empty c'tor and a given signature
inline
void ThrowExc ( JNIEnv * env, const char* className )
{
	jclass cls = env->FindClass(className);
	jmethodID CtorID = env->GetMethodID ( cls, "<init>", "()V" );
	jobject obj = env->NewObject ( cls, CtorID );
	env->Throw((jthrowable)obj);
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
	ThrowExc ( env, reason, "Lorg/semanticweb/owlapi/reasoner/ReasonerInternalException;" );
}

/// throw Inconsistent Ontology exception
inline
void ThrowICO ( JNIEnv* env )
{
	ThrowExc ( env, "Lorg/semanticweb/owlapi/reasoner/InconsistentOntologyException;" );
}

/// throw CR for non-simple role exception
inline
void ThrowNSR ( JNIEnv* env, const char* reason )
{
	std::string msg ("Non-simple object property '");
	msg += reason;
	msg += "' is used as a simple one";
	ThrowExc ( env, msg.c_str(), "Lorg/semanticweb/owlapi/reasoner/OWLReasonerRuntimeException;");

// not correct because does not have enough information
//	jclass ceNotProfile = env->FindClass("Lorg/semanticweb/owlapi/reasoner/ClassExpressionNotInProfileException;");
//	if ( ceNotProfile == 0 )
//	{
//		Throw ( env, "Can't get class for Pointer" );
//		return ;
//	}
//
//	jmethodID CtorID = env->GetMethodID ( ceNotProfile, "<init>", "(Lorg/semanticweb/owlapi/model/OWLClassExpression;Lorg/semanticweb/owlapi/profiles/OWLProfile;)V" );
//
//	// create an object to return
//	jobject obj = env->NewObject ( ceNotProfile, CtorID, NULL, NULL );
//	env->Throw((jthrowable)obj);
}

/// throw Role Inclusion Cycle exception
inline
void ThrowRIC ( JNIEnv* env, const char* reason )
{
	ThrowExc ( env, reason, "Lorg/semanticweb/owlapi/reasoner/AxiomNotInProfileException;" );
}

/// throw Role Inclusion Cycle exception
inline
void ThrowTO ( JNIEnv* env )
{
	ThrowExc ( env, "Lorg/semanticweb/owlapi/reasoner/TimeOutException;" );
}

/// field for Kernel's ID
extern "C" jfieldID KernelFID;

/// get Kernel local to given object
// as a side effect sets up curKernel
inline
ReasoningKernel* getK ( JNIEnv * env, jobject obj )
{
	jlong id = env->GetLongField ( obj, KernelFID );

	// this is a pointer -- should not be NULL
	if ( id == 0 )
		Throw ( env, "Uninitialized FaCT++ kernel found" );

	return (ReasoningKernel*)id;
}

/// get the expression manager corresponding local kernel
inline
TExpressionManager* getEM ( JNIEnv * env, jobject obj ) { return getK(env,obj)->getExpressionManager(); }

//------------------------------------------------------
// Keeps class names and field IDs for different Java classes in FaCT++ interface
//------------------------------------------------------

/// keep class, Node field and c'tor of an interface class
class TClassFieldMethodIDs
{
public:		// members
		/// class name
	jclass ClassID;
		/// array class type
	jclass ArrayClassID;
		/// c'tor type
	jmethodID CtorID;
		/// 'node' field
	jfieldID NodeFID;

public:		// interface
		/// init values by class name
	void init ( JNIEnv* env, const char* arrayClassName )
	{
		jclass id = env->FindClass(arrayClassName+1);
		if ( id == 0 )
		{
			Throw ( env, "Can't get class for Pointer" );
			return;
		}
		ClassID = reinterpret_cast<jclass>(env->NewGlobalRef(id));

		id = env->FindClass(arrayClassName);
		if ( id == 0 )
		{
			Throw ( env, "Can't get class for [Pointer" );
			return;
		}
		ArrayClassID = reinterpret_cast<jclass>(env->NewGlobalRef(id));

		CtorID = env->GetMethodID ( ClassID, "<init>", "()V" );
		if ( CtorID == 0 )
		{
			Throw ( env, "Can't get c'tor for Pointer" );
			return;
		}

		NodeFID = env->GetFieldID ( ClassID, "node", "J" );
		if ( NodeFID == 0 )
		{
			Throw ( env, "Can't get 'node' field" );
			return;
		}
	}
	void fini ( JNIEnv* env )
	{
		env->DeleteGlobalRef(ClassID);
		env->DeleteGlobalRef(ArrayClassID);
	}
}; // TClassFieldMethodIDs

/// IDs for different Java interface classes
extern
#ifdef __cplusplus
		"C"
#endif
TClassFieldMethodIDs
	ClassPointer,
	IndividualPointer,
	ObjectPropertyPointer,
	DataPropertyPointer,
	DataTypePointer,
	DataTypeExpressionPointer,
	DataValuePointer,
	DataTypeFacet,
	NodePointer,
	AxiomPointer;

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
ACCESSOR(FacetExpr)

// ACCESSOR(DataTypeExpr) -- doesn't work as DTE is not a const typedef
inline TDataTypeExpr* getDataTypeExpr ( JNIEnv * env, jobject obj )
	{ return const_cast<TDataTypeExpr*>(dynamic_cast<const TDataTypeExpr*>((TExpr*)getPointer(env,obj))); }

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
ACCESSOR(DataTypeExpr)
ACCESSOR(DataValueExpr)
ACCESSOR(FacetExpr)

// ACCESSOR(NodeExpr) -- doesn't work as the type is not a TExpr's descendant
inline TCGNode* getRONode ( JNIEnv * env, jobject obj )
	{ return (TCGNode*)getPointer(env,obj); }

#undef ACCESSOR

inline
TDLAxiom* getAxiom ( JNIEnv * env, jobject obj )
{
	return (TDLAxiom*)getPointer(env,obj);
}

inline
TCGNode* getNode ( JNIEnv * env, jobject obj )
{
	return (TCGNode*)getPointer(env,obj);
}

inline
jobject retObject ( JNIEnv * env, const void* t, const TClassFieldMethodIDs& ID )
{
	if ( t == NULL )
	{
		Throw ( env, "Incorrect operand by FaCT++ Kernel" );
		return (jobject)0;
	}

	// create an object to return
	jobject obj = env->NewObject ( ID.ClassID, ID.CtorID );

	if ( obj == 0 )
		Throw ( env, "Can't create Pointer object" );
	else	// set the return value
		env->SetLongField ( obj, ID.NodeFID, (jlong)t );

	return obj;
}

inline
jobject Class ( JNIEnv * env, TConceptExpr* t )
{
	return retObject ( env, t, ClassPointer );
}

inline
jobject Individual ( JNIEnv * env, TIndividualExpr* t )
{
	return retObject ( env, t, IndividualPointer );
}

inline
jobject ObjectProperty ( JNIEnv * env, TORoleExpr* t )
{
	return retObject ( env, t, ObjectPropertyPointer );
}

inline
jobject ObjectComplex ( JNIEnv * env, TORoleComplexExpr* t )
{
	return retObject ( env, t, ObjectPropertyPointer );
}

inline
jobject DataProperty ( JNIEnv * env, TDRoleExpr* t )
{
	return retObject ( env, t, DataPropertyPointer );
}

inline
jobject DataType ( JNIEnv * env, TDataExpr* t )
{
	return retObject ( env, t, DataTypePointer );
}

inline
jobject DataTypeExpression ( JNIEnv * env, TDataExpr* t )
{
	return retObject ( env, t, DataTypeExpressionPointer );
}

inline
jobject DataValue ( JNIEnv * env, TDataValueExpr* t )
{
	return retObject ( env, t, DataValuePointer );
}

inline
jobject Facet ( JNIEnv * env, TFacetExpr* t )
{
	return retObject ( env, t, DataTypeFacet );
}

inline
jobject Node ( JNIEnv * env, TCGNode* t )
{
	return retObject ( env, t, NodePointer );
}

inline
jobject Axiom ( JNIEnv * env, TDLAxiom* t )
{
	return retObject ( env, t, AxiomPointer );
}

/// create vector of Java objects of class CLASSNAME by given VEC
template<class T>
jobjectArray buildArray ( JNIEnv* env, const std::vector<T*>& vec, const TClassFieldMethodIDs& ID )
{
	jobjectArray ret = env->NewObjectArray ( vec.size(), ID.ClassID, NULL );
	for ( unsigned int i = 0; i < vec.size(); ++i )
		env->SetObjectArrayElement ( ret, i, retObject ( env, vec[i], ID ) );
	return ret;
}

#endif
