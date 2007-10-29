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

#include "uk_ac_manchester_cs_factplusplus_FaCTPlusPlus.h"
#include "Kernel.h"
#include "JNISupport.h"
#include "JNIActor.h"
#include "JNIMonitor.h"

/// remember (and clear) references for the RO trees
MMKernel* curKernel = NULL;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ENABLE_CHECKING
#	define TRACE_JNI(func) std::cerr << "call " << func << " from JNI\n"
#else
#	define TRACE_JNI(func) (void)NULL
#endif

//-------------------------------------------------------------
// Kernel management (like newKB/curKB/releaseKB)
//-------------------------------------------------------------

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    initKernel
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_initKernel
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("initKernel");
	jclass classThis = env->GetObjectClass(obj);

	if ( classThis == 0 )
		ThrowGen ( env, "Can't get class of 'this'" );

	jfieldID fid = env->GetFieldID ( classThis, "KernelId", "J" );

	if ( fid == 0 )
		ThrowGen ( env, "Can't get 'KernelId' field" );

	// create new kernel and save it in an FaCTPlusPlus object
	env->SetLongField ( obj, fid, (jlong)(curKernel=new MMKernel()) );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    deleteKernel
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_deleteKernel
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("deleteKernel");
	getK(env,obj);
	delete curKernel;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    clearKernel
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_clearKernel
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("clearKernel");
	getK(env,obj)->clearKB();
	curKernel->pRefRecorder->clear();
}


//-------------------------------------------------------------
// Concept/role/datatype language
//-------------------------------------------------------------


/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getThing
 * Signature: ()Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getThing
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("getThing");
	return Class ( env, getK(env,obj)->Top() );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getNothing
 * Signature: ()Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getNothing
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("getNothing");
	return Class ( env, getK(env,obj)->Bottom() );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getNamedClass
 * Signature: (Ljava/lang/String;)Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getNamedClass
  (JNIEnv * env, jobject obj, jstring str)
{
	TRACE_JNI("getNamedClass");
	JString name(env,str);
	jobject ret = (jobject)0;
	try
	{
		ret = Class ( env, getK(env,obj)->ensureConceptName(name()) );
	}
	catch (CantRegName)
	{
		Throw ( env, "FaCT++ Kernel: Can not register new class name" );
	}
	return ret;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getObjectProperty
 * Signature: (Ljava/lang/String;)Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getObjectProperty
  (JNIEnv * env, jobject obj, jstring str)
{
	TRACE_JNI("getObjectProperty");
	JString name(env,str);
	jobject ret = (jobject)0;
	try
	{
		ret = ObjectProperty ( env, getK(env,obj)->ensureRoleName(name()) );
	}
	catch (CantRegName)
	{
		Throw ( env, "FaCT++ Kernel: Can not register new object property name" );
	}
	return ret;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getDataProperty
 * Signature: (Ljava/lang/String;)Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getDataProperty
  (JNIEnv * env, jobject obj, jstring str)
{
	TRACE_JNI("getDataProperty");
	JString name(env,str);
	jobject ret = (jobject)0;
	try
	{
		ret = DataProperty ( env, getK(env,obj)->ensureDataRoleName(name()) );
	}
	catch (CantRegName)
	{
		Throw ( env, "FaCT++ Kernel: Can not register new data property name" );
	}
	return ret;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getIndividual
 * Signature: (Ljava/lang/String;)Luk/ac/manchester/cs/factplusplus/IndividualPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getIndividual
  (JNIEnv * env, jobject obj, jstring str)
{
	TRACE_JNI("getIndividual");
	JString name(env,str);
	jobject ret = (jobject)0;
	try
	{
		ret = Individual ( env, getK(env,obj)->ensureSingletonName(name()) );
	}
	catch (CantRegName)
	{
		Throw ( env, "FaCT++ Kernel: Can not register new individual name" );
	}
	return ret;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getBuiltInDataType
 * Signature: (Ljava/lang/String;)Luk/ac/manchester/cs/factplusplus/DataTypePointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getBuiltInDataType
  (JNIEnv * env, jobject obj, jstring str)
{
	TRACE_JNI("getBuiltInDataType");
	JString name(env,str);
	std::string DTName(name());

	// FIXME!! Data Top
	if ( DTName == "http://www.w3.org/2000/01/rdf-schema#Literal" )
		return DataType ( env, getK(env,obj)->getDataTypeCenter().getStringType() );

	if ( DTName == "http://www.w3.org/2001/XMLSchema#string" )
		return DataType ( env, getK(env,obj)->getDataTypeCenter().getStringType() );

	if ( DTName == "http://www.w3.org/2001/XMLSchema#integer" )
		return DataType ( env, getK(env,obj)->getDataTypeCenter().getNumberType());
	if ( DTName == "http://www.w3.org/2001/XMLSchema#int" )
		return DataType ( env, getK(env,obj)->getDataTypeCenter().getNumberType());
	if ( DTName == "http://www.w3.org/2001/XMLSchema#nonNegativeInteger" )
		return DataType ( env, getK(env,obj)->getDataTypeCenter().getNumberType());

	if ( DTName == "http://www.w3.org/2001/XMLSchema#float" )
		return DataType ( env, getK(env,obj)->getDataTypeCenter().getRealType());
	if ( DTName == "http://www.w3.org/2001/XMLSchema#double" )
		return DataType ( env, getK(env,obj)->getDataTypeCenter().getRealType());

	Throw ( env, "Unsupported datatype in getBuiltInDataType" );
	return (jobject)0;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getDataSubType
 * Signature: (Ljava/lang/String;Luk/ac/manchester/cs/factplusplus/DataTypeExpressionPointer;)Luk/ac/manchester/cs/factplusplus/DataTypeExpressionPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getDataSubType
  (JNIEnv * env, jobject obj, jstring str, jobject type)
{
	TRACE_JNI("getDataSubType");
	JString name(env,str);
	jobject ret = (jobject)0;
	try
	{
		ret = DataTypeExpression ( env, getK(env,obj)->getDataTypeCenter().
								   getDataType ( name(), getTree(env,type) ) );
	}
	catch (CantRegName)
	{
		Throw ( env, "FaCT++ Kernel: Can not register new data type" );
	}
	return ret;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getDataTop
 * Signature: ()Luk/ac/manchester/cs/factplusplus/DataTypePointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getDataTop
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("getDataTop");
	return DataType ( env, getK(env,obj)->Top() );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getDataEnumeration
 * Signature: ()Luk/ac/manchester/cs/factplusplus/DataTypeExpressionPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getDataEnumeration
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("getDataEnumeration");
	Throw ( env, "FaCT++ Kernel: unsupported operation" );
	return NULL;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getRestrictedDataType
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataTypeExpressionPointer;Luk/ac/manchester/cs/factplusplus/DataTypeFacet;)Luk/ac/manchester/cs/factplusplus/DataTypeExpressionPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getRestrictedDataType
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	TRACE_JNI("getRestrictedDataType");
	DataTypeCenter& center = getK(env,obj)->getDataTypeCenter();

	// create new DTExpression of the type ARG1 and add both facets to it
	DLTree* type = getTree(env,arg1);
	DLTree* ret = center.getDataExpr(type);
	ret = center.applyFacet(ret,type);
	ret = center.applyFacet(ret,getFacet(env,arg2));

	return DataTypeExpression(env,ret);
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getLength
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataValuePointer;)Luk/ac/manchester/cs/factplusplus/DataTypeFacet;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getLength
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("getLength");
	Throw ( env, "FaCT++ Kernel: unsupported operation" );
	return NULL;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getMinLength
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataValuePointer;)Luk/ac/manchester/cs/factplusplus/DataTypeFacet;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getMinLength
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("getMinLength");
	Throw ( env, "FaCT++ Kernel: unsupported operation" );
	return NULL;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getMaxLength
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataValuePointer;)Luk/ac/manchester/cs/factplusplus/DataTypeFacet;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getMaxLength
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("getMaxLength");
	Throw ( env, "FaCT++ Kernel: unsupported operation" );
	return NULL;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getPattern
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataValuePointer;)Luk/ac/manchester/cs/factplusplus/DataTypeFacet;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getPattern
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("getPattern");
	Throw ( env, "FaCT++ Kernel: unsupported operation" );
	return NULL;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getEnumeration
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataValuePointer;)Luk/ac/manchester/cs/factplusplus/DataTypeFacet;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getEnumeration
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("getEnumeration");
	Throw ( env, "FaCT++ Kernel: unsupported operation" );
	return NULL;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getMinExclusiveFacet
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataValuePointer;)Luk/ac/manchester/cs/factplusplus/DataTypeFacet;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getMinExclusiveFacet
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("getMinExclusiveFacet");
	return Facet ( env, getK(env,obj)->getDataTypeCenter().getMinExclusiveFacet(getTree(env,arg)) );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getMaxExclusiveFacet
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataValuePointer;)Luk/ac/manchester/cs/factplusplus/DataTypeFacet;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getMaxExclusiveFacet
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("getMaxExclusiveFacet");
	return Facet ( env, getK(env,obj)->getDataTypeCenter().getMaxExclusiveFacet(getTree(env,arg)) );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getMinInclusiveFacet
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataValuePointer;)Luk/ac/manchester/cs/factplusplus/DataTypeFacet;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getMinInclusiveFacet
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("getMinInclusiveFacet");
	return Facet ( env, getK(env,obj)->getDataTypeCenter().getMinInclusiveFacet(getTree(env,arg)) );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getMaxInclusiveFacet
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataValuePointer;)Luk/ac/manchester/cs/factplusplus/DataTypeFacet;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getMaxInclusiveFacet
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("getMaxInclusiveFacet");
	return Facet ( env, getK(env,obj)->getDataTypeCenter().getMaxInclusiveFacet(getTree(env,arg)) );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getTotalDigitsFacet
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataValuePointer;)Luk/ac/manchester/cs/factplusplus/DataTypeFacet;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getTotalDigitsFacet
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("getTotalDigitsFacet");
	Throw ( env, "FaCT++ Kernel: unsupported operation" );
	return NULL;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getFractionDigitsFacet
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataValuePointer;)Luk/ac/manchester/cs/factplusplus/DataTypeFacet;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getFractionDigitsFacet
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("getFractionDigitsFacet");
	Throw ( env, "FaCT++ Kernel: unsupported operation" );
	return NULL;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getNot
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataTypeExpressionPointer;)Luk/ac/manchester/cs/factplusplus/DataTypeExpressionPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getNot
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("getNot");
	Throw ( env, "FaCT++ Kernel: unsupported operation" );
	return NULL;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getDataValue
 * Signature: (Ljava/lang/String;Luk/ac/manchester/cs/factplusplus/DataTypePointer;)Luk/ac/manchester/cs/factplusplus/DataValuePointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getDataValue__Ljava_lang_String_2Luk_ac_manchester_cs_factplusplus_DataTypePointer_2
  (JNIEnv * env, jobject obj, jstring str, jobject type)
{
	TRACE_JNI("getDataValue");
	JString name(env,str);
	jobject ret = (jobject)0;
	try
	{
		ret = DataValue ( env, getK(env,obj)->getDataTypeCenter().
							   getDataValue ( name(), getTree(env,type) ) );
	}
	catch (CantRegName)
	{
		Throw ( env, "FaCT++ Kernel: Can not register new data value" );
	}
	return ret;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getConceptAnd
 * Signature: ()Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getConceptAnd
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("getConceptAnd");
	return Class ( env, getK(env,obj)->And() );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getConceptOr
 * Signature: ()Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getConceptOr
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("getConceptOr");
	return Class ( env, getK(env,obj)->Or() );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getConceptNot
 * Signature: (Luk/ac/manchester/cs/factplusplus/ClassPointer;)Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getConceptNot
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("getConceptNot");
	return Class ( env, getK(env,obj)->Not(getTree(env,arg)) );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getObjectSome
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;Luk/ac/manchester/cs/factplusplus/ClassPointer;)Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getObjectSome
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	TRACE_JNI("getObjectSome");
	return Class ( env, getK(env,obj)->Exists ( getTree(env,arg1), getTree(env,arg2) ) );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getObjectAll
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;Luk/ac/manchester/cs/factplusplus/ClassPointer;)Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getObjectAll
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	TRACE_JNI("getObjectAll");
	return Class ( env, getK(env,obj)->Forall ( getTree(env,arg1), getTree(env,arg2) ) );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getObjectValue
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;Luk/ac/manchester/cs/factplusplus/IndividualPointer;)Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getObjectValue
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	TRACE_JNI("getObjectValue");
	return Class ( env, getK(env,obj)->Value ( getTree(env,arg1), getTree(env,arg2) ) );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getDataSome
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;Luk/ac/manchester/cs/factplusplus/DataTypeExpressionPointer;)Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getDataSome
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	TRACE_JNI("getDataSome");
	return Class ( env, getK(env,obj)->Exists ( getTree(env,arg1), getTree(env,arg2) ) );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getDataAll
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;Luk/ac/manchester/cs/factplusplus/DataTypeExpressionPointer;)Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getDataAll
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	TRACE_JNI("getDataAll");
	return Class ( env, getK(env,obj)->Forall ( getTree(env,arg1), getTree(env,arg2) ) );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getDataValue
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;Luk/ac/manchester/cs/factplusplus/DataValuePointer;)Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getDataValue__Luk_ac_manchester_cs_factplusplus_DataPropertyPointer_2Luk_ac_manchester_cs_factplusplus_DataValuePointer_2
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	TRACE_JNI("getDataValue");
	return Class ( env, getK(env,obj)->Exists ( getTree(env,arg1), getTree(env,arg2) ) );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getObjectAtLeast
 * Signature: (ILuk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;Luk/ac/manchester/cs/factplusplus/ClassPointer;)Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getObjectAtLeast
  (JNIEnv * env, jobject obj, jint n, jobject arg1, jobject arg2)
{
	TRACE_JNI("getObjectAtLeast");
	return Class ( env, getK(env,obj)->MinCardinality ( n, getTree(env,arg1), getTree(env,arg2) ) );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getObjectExact
 * Signature: (ILuk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;Luk/ac/manchester/cs/factplusplus/ClassPointer;)Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getObjectExact
  (JNIEnv * env, jobject obj, jint n, jobject arg1, jobject arg2)
{
	TRACE_JNI("getObjectExact");
	return Class ( env, getK(env,obj)->Cardinality ( n, getTree(env,arg1), getTree(env,arg2) ) );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getObjectAtMost
 * Signature: (ILuk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;Luk/ac/manchester/cs/factplusplus/ClassPointer;)Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getObjectAtMost
  (JNIEnv * env, jobject obj, jint n, jobject arg1, jobject arg2)
{
	TRACE_JNI("getObjectAtMost");
	return Class ( env, getK(env,obj)->MaxCardinality ( n, getTree(env,arg1), getTree(env,arg2) ) );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getDataAtLeast
 * Signature: (ILuk/ac/manchester/cs/factplusplus/DataPropertyPointer;Luk/ac/manchester/cs/factplusplus/DataTypePointer;)Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getDataAtLeast
  (JNIEnv * env, jobject obj, jint n, jobject arg1, jobject arg2)
{
	TRACE_JNI("getDataAtLeast");
	return Class ( env, getK(env,obj)->MinCardinality ( n, getTree(env,arg1), getTree(env,arg2) ) );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getDataExact
 * Signature: (ILuk/ac/manchester/cs/factplusplus/DataPropertyPointer;Luk/ac/manchester/cs/factplusplus/DataTypePointer;)Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getDataExact
  (JNIEnv * env, jobject obj, jint n, jobject arg1, jobject arg2)
{
	TRACE_JNI("getDataExact");
	return Class ( env, getK(env,obj)->Cardinality ( n, getTree(env,arg1), getTree(env,arg2) ) );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getDataAtMost
 * Signature: (ILuk/ac/manchester/cs/factplusplus/DataPropertyPointer;Luk/ac/manchester/cs/factplusplus/DataTypePointer;)Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getDataAtMost
  (JNIEnv * env, jobject obj, jint n, jobject arg1, jobject arg2)
{
	TRACE_JNI("getDataAtMost");
	return Class ( env, getK(env,obj)->MaxCardinality ( n, getTree(env,arg1), getTree(env,arg2) ) );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getInverseProperty
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getInverseProperty
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("getInverseProperty");
	return ObjectProperty ( env, getK(env,obj)->Inverse(getTree(env,arg)) );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getPropertyComposition
 * Signature: ()Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getPropertyComposition
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("getPropertyComposition");
	return ObjectProperty ( env, getK(env,obj)->Compose() );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getOneOf
 * Signature: ()Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getOneOf
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("getOneOf");
	return Class ( env, getK(env,obj)->processOneOf() );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getSelf
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getSelf
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("getSelf");
	return Class ( env, getK(env,obj)->SelfReference(getTree(env,arg)) );
}


//-------------------------------------------------------------
// Concept/role/individual axioms (TELL language)
//-------------------------------------------------------------

#define PROCESS_QUERY(Action,Reason)			\
	bool fail = false;							\
	try { fail = Action; }						\
	catch ( std::exception ) { fail = true; }	\
	if ( fail ) Throw ( env, Reason )

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellSubClassOf
 * Signature: (Luk/ac/manchester/cs/factplusplus/ClassPointer;Luk/ac/manchester/cs/factplusplus/ClassPointer;)V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellSubClassOf
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	TRACE_JNI("tellSubClassOf");
	PROCESS_QUERY (
		getK(env,obj)->impliesConcepts ( getTree(env,arg1), getTree(env,arg2) ),
		"FaCT++ Kernel: error during tellSubClassOf processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellEquivalentClass
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellEquivalentClass
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("tellEquivalentClass");
	PROCESS_QUERY (
		getK(env,obj)->equalConcepts(),
		"FaCT++ Kernel: error during tellEquivalentClasses processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellDisjointClasses
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellDisjointClasses
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("tellDisjointClasses");
	PROCESS_QUERY (
		getK(env,obj)->processDisjoint(),
		"FaCT++ Kernel: error during tellDisjointClasses processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellSubObjectProperties
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellSubObjectProperties
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	TRACE_JNI("tellSubObjectProperties");
	PROCESS_QUERY (
		getK(env,obj)->impliesRoles ( getROTree(env,arg1), getROTree(env,arg2) ),
		"FaCT++ Kernel: error during tellSubObjectProperties processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellEquivalentObjectProperties
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellEquivalentObjectProperties
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("tellEquivalentObjectProperties");
	PROCESS_QUERY (
		getK(env,obj)->equalRoles(),
		"FaCT++ Kernel: error during tellEquivalentObjectProperties processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellInverseProperties
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellInverseProperties
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	TRACE_JNI("tellInverseProperties");
	PROCESS_QUERY (
		getK(env,obj)->equalRoles ( getROTree(env,arg1), getK(env,obj)->Inverse(getTree(env,arg2)) ),
		"FaCT++ Kernel: error during tellInverseProperties processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellObjectPropertyRange
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;Luk/ac/manchester/cs/factplusplus/ClassPointer;)V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellObjectPropertyRange
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	TRACE_JNI("tellObjectPropertyRange");
	PROCESS_QUERY (
		getK(env,obj)->setRange ( getROTree(env,arg1), getTree(env,arg2) ),
		"FaCT++ Kernel: error during tellObjectPropertyRange processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellDataPropertyRange
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;Luk/ac/manchester/cs/factplusplus/DataTypePointer;)V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellDataPropertyRange
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	TRACE_JNI("tellDataPropertyRange");
	PROCESS_QUERY (
		getK(env,obj)->setRange ( getROTree(env,arg1), getTree(env,arg2) ),
		"FaCT++ Kernel: error during tellDataPropertyRange processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellObjectPropertyDomain
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;Luk/ac/manchester/cs/factplusplus/ClassPointer;)V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellObjectPropertyDomain
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	TRACE_JNI("tellObjectPropertyDomain");
	PROCESS_QUERY (
		getK(env,obj)->setDomain ( getROTree(env,arg1), getTree(env,arg2) ),
		"FaCT++ Kernel: error during tellObjectPropertyDomain processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellDataPropertyDomain
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;Luk/ac/manchester/cs/factplusplus/ClassPointer;)V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellDataPropertyDomain
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	TRACE_JNI("tellDataPropertyDomain");
	PROCESS_QUERY (
		getK(env,obj)->setDomain ( getROTree(env,arg1), getTree(env,arg2) ),
		"FaCT++ Kernel: error during tellDataPropertyDomain processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellDisjointObjectProperties
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellDisjointObjectProperties
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("tellDisjointObjectProperties");
	PROCESS_QUERY (
		getK(env,obj)->disjointRoles(),
		"FaCT++ Kernel: error during tellDisjointObjectProperties processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellFunctionalObjectProperty
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellFunctionalObjectProperty
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("tellFunctionalObjectProperty");
	PROCESS_QUERY (
		getK(env,obj)->setFunctional(getROTree(env,arg)),
		"FaCT++ Kernel: error during tellFunctionalObjectProperty processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellInverseFunctionalObjectProperty
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellInverseFunctionalObjectProperty
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("tellInverseFunctionalObjectProperty");
	PROCESS_QUERY (
		getK(env,obj)->setFunctional(getK(env,obj)->Inverse(getTree(env,arg))),
		"FaCT++ Kernel: error during tellInverseFunctionalObjectProperty processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellSymmetricObjectProperty
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellSymmetricObjectProperty
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("tellSymmetricObjectProperty");
	PROCESS_QUERY (
		getK(env,obj)->setSymmetric(getROTree(env,arg)),
		"FaCT++ Kernel: error during tellSymmetricObjectProperty processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellAntiSymmetricObjectProperty
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellAntiSymmetricObjectProperty
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("tellAntiSymmetricObjectProperty");
	PROCESS_QUERY (
		getK(env,obj)->setAntiSymmetric(getROTree(env,arg)),
		"FaCT++ Kernel: error during tellAntiSymmetricObjectProperty processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellReflexiveObjectProperty
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellReflexiveObjectProperty
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("tellReflexiveObjectProperty");
	PROCESS_QUERY (
		getK(env,obj)->setReflexive(getROTree(env,arg)),
		"FaCT++ Kernel: error during tellReflexiveObjectProperty processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellIrreflexiveObjectProperty
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellIrreflexiveObjectProperty
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("tellIrreflexiveObjectProperty");
	PROCESS_QUERY (
		getK(env,obj)->setIrreflexive(getROTree(env,arg)),
		"FaCT++ Kernel: error during tellIrreflexiveObjectProperty processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellTransitiveObjectProperty
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellTransitiveObjectProperty
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("tellTransitiveObjectProperty");
	PROCESS_QUERY (
		getK(env,obj)->setTransitive(getROTree(env,arg)),
		"FaCT++ Kernel: error during tellTransitiveObjectProperty processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellSubDataProperties
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;)V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellSubDataProperties
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	TRACE_JNI("tellSubDataProperties");
	PROCESS_QUERY (
		getK(env,obj)->impliesRoles ( getROTree(env,arg1), getROTree(env,arg2) ),
		"FaCT++ Kernel: error during tellSubDataProperties processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellEquivalentDataProperties
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;)V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellEquivalentDataProperties
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	TRACE_JNI("tellEquivalentDataProperties");
	PROCESS_QUERY (
		getK(env,obj)->equalRoles ( getROTree(env,arg1), getROTree(env,arg2) ),
		"FaCT++ Kernel: error during tellEquivalentDataProperties processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellDisjointDataProperties
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;)V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellDisjointDataProperties
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	TRACE_JNI("tellDisjointDataProperties");
	PROCESS_QUERY (
		getK(env,obj)->disjointRoles ( getROTree(env,arg1), getROTree(env,arg2) ),
		"FaCT++ Kernel: error during tellDisjointDataProperties processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellFunctionalDataProperty
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;)V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellFunctionalDataProperty
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("tellFunctionalDataProperty");
	PROCESS_QUERY (
		getK(env,obj)->setFunctional(getROTree(env,arg)),
		"FaCT++ Kernel: error during tellFunctionalDataProperty processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellIndividualType
 * Signature: (Luk/ac/manchester/cs/factplusplus/IndividualPointer;Luk/ac/manchester/cs/factplusplus/ClassPointer;)V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellIndividualType
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	TRACE_JNI("tellIndividualType");
	PROCESS_QUERY (
		getK(env,obj)->instanceOf ( getROTree(env,arg1), getTree(env,arg2) ),
		"FaCT++ Kernel: error during tellIndividualType processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellRelatedIndividuals
 * Signature: (Luk/ac/manchester/cs/factplusplus/IndividualPointer;Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;Luk/ac/manchester/cs/factplusplus/IndividualPointer;)V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellRelatedIndividuals
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2, jobject arg3)
{
	TRACE_JNI("tellRelatedIndividuals");
	PROCESS_QUERY (
		getK(env,obj)->relatedTo ( getROTree(env,arg1), getROTree(env,arg2), getROTree(env,arg3) ),
		"FaCT++ Kernel: error during tellRelatedIndividuals processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellNotRelatedIndividuals
 * Signature: (Luk/ac/manchester/cs/factplusplus/IndividualPointer;Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;Luk/ac/manchester/cs/factplusplus/IndividualPointer;)V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellNotRelatedIndividuals
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2, jobject arg3)
{
	TRACE_JNI("tellNotRelatedIndividuals");
	PROCESS_QUERY (
		getK(env,obj)->relatedToNot ( getROTree(env,arg1), getTree(env,arg2), getTree(env,arg3) ),
		"FaCT++ Kernel: error during tellNotRelatedIndividuals processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellRelatedIndividualValue
 * Signature: (Luk/ac/manchester/cs/factplusplus/IndividualPointer;Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;Luk/ac/manchester/cs/factplusplus/DataValuePointer;)V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellRelatedIndividualValue
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2, jobject arg3)
{
	TRACE_JNI("tellRelatedIndividualValue");
	PROCESS_QUERY (
		getK(env,obj)->valueOf ( getROTree(env,arg1), getTree(env,arg2), getTree(env,arg3) ),
		"FaCT++ Kernel: error during tellRelatedIndividualValue processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellNotRelatedIndividualValue
 * Signature: (Luk/ac/manchester/cs/factplusplus/IndividualPointer;Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;Luk/ac/manchester/cs/factplusplus/DataValuePointer;)V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellNotRelatedIndividualValue
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2, jobject arg3)
{
	TRACE_JNI("tellNotRelatedIndividualValue");
	PROCESS_QUERY (
		getK(env,obj)->valueOfNot ( getROTree(env,arg1), getTree(env,arg2), getTree(env,arg3) ),
		"FaCT++ Kernel: error during tellNotRelatedIndividualValue processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellSameIndividuals
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellSameIndividuals
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("tellSameIndividuals");
	PROCESS_QUERY (
		getK(env,obj)->processSame(),
		"FaCT++ Kernel: error during tellSameIndividuals processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellDifferentIndividuals
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellDifferentIndividuals
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("tellDifferentIndividuals");
	PROCESS_QUERY (
		getK(env,obj)->processDifferent(),
		"FaCT++ Kernel: error during tellDifferentIndividuals processing" );
}


//-------------------------------------------------------------
// minimal query language (ASK languages)
//-------------------------------------------------------------

#define PROCESS_ASK_QUERY(Action,Reason)		\
	bool fail = false;							\
	try { fail = Action; }						\
	catch ( InconsistentKB )					\
	{ ThrowICO(env); }							\
	catch ( std::exception ) { fail = true; }	\
	if ( fail ) Throw ( env, Reason )

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    isKBConsistent
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_isKBConsistent
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("isKBConsistent");
	bool ret = true;
	try
	{
		ret = getK(env,obj)->isKBConsistent();
	}
	catch ( std::exception )
	{
		Throw ( env, "FaCT++ Kernel: out of memory" );
	}

	return ret;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    classify
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_classify
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("classify");
	try
	{
		getK(env,obj)->classifyKB();
	}
	catch ( InconsistentKB )
	{
		ThrowICO(env);
	}
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    realise
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_realise
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("realise");
	try
	{
		getK(env,obj)->realiseKB();
	}
	catch ( InconsistentKB )
	{
		ThrowICO(env);
	}
}


/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    isClassSatisfiable
 * Signature: (Luk/ac/manchester/cs/factplusplus/ClassPointer;)Z
 */
JNIEXPORT jboolean JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_isClassSatisfiable
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("isClassSatisfiable");
	bool ret = false;
	PROCESS_ASK_QUERY (
		getK(env,obj)->isSatisfiable ( getROTree(env,arg), ret ),
		"FaCT++ Kernel: error during isClassSatisfiable processing" );
	return ret;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    isClassSubsumedBy
 * Signature: (Luk/ac/manchester/cs/factplusplus/ClassPointer;Luk/ac/manchester/cs/factplusplus/ClassPointer;)Z
 */
JNIEXPORT jboolean JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_isClassSubsumedBy
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	TRACE_JNI("isClassSubsumedBy");
	bool ret = false;
	PROCESS_ASK_QUERY (
		getK(env,obj)->isSubsumedBy ( getROTree(env,arg1), getROTree(env,arg2), ret ),
		"FaCT++ Kernel: error during isClassSubsumedBy processing" );
	return ret;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    isClassEquivalentTo
 * Signature: (Luk/ac/manchester/cs/factplusplus/ClassPointer;Luk/ac/manchester/cs/factplusplus/ClassPointer;)Z
 */
JNIEXPORT jboolean JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_isClassEquivalentTo
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	TRACE_JNI("isClassEquivalentTo");
	bool ret = false;
	PROCESS_ASK_QUERY (
		getK(env,obj)->isEquivalent ( getROTree(env,arg1), getROTree(env,arg2), ret ),
		"FaCT++ Kernel: error during isClassEquivalentTo processing" );
	return ret;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    isClassDisjointWith
 * Signature: (Luk/ac/manchester/cs/factplusplus/ClassPointer;Luk/ac/manchester/cs/factplusplus/ClassPointer;)Z
 */
JNIEXPORT jboolean JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_isClassDisjointWith
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	TRACE_JNI("isClassDisjointWith");
	bool ret = false;
	PROCESS_ASK_QUERY (
		getK(env,obj)->isDisjoint ( getROTree(env,arg1), getROTree(env,arg2), ret ),
		"FaCT++ Kernel: error during isClassDisjointWith processing" );
	return ret;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    askSubClasses
 * Signature: (Luk/ac/manchester/cs/factplusplus/ClassPointer;Z)[[Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobjectArray JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_askSubClasses
  (JNIEnv * env, jobject obj, jobject arg, jboolean direct)
{
	TRACE_JNI("askSubClasses");
	JTaxonomyActor<ClassPolicy> actor(env);
	DLTree* p = getROTree(env,arg);
	PROCESS_ASK_QUERY (
		direct ? getK(env,obj)->getChildren(p,actor) : getK(env,obj)->getDescendants(p,actor),
		"FaCT++ Kernel: error during askSubClasses processing" );
	return actor.getElements();
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    askSuperClasses
 * Signature: (Luk/ac/manchester/cs/factplusplus/ClassPointer;Z)[[Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobjectArray JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_askSuperClasses
  (JNIEnv * env, jobject obj, jobject arg, jboolean direct)
{
	TRACE_JNI("askSuperClasses");
	JTaxonomyActor<ClassPolicy> actor(env);
	DLTree* p = getROTree(env,arg);
	PROCESS_ASK_QUERY (
		direct ? getK(env,obj)->getParents(p,actor) : getK(env,obj)->getAncestors(p,actor),
		"FaCT++ Kernel: error during askSuperClasses processing" );
	return actor.getElements();
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    askEquivalentClasses
 * Signature: (Luk/ac/manchester/cs/factplusplus/ClassPointer;)[Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobjectArray JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_askEquivalentClasses
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("askEquivalentClasses");
	JTaxonomyActor<ClassPolicy> actor(env);
	PROCESS_ASK_QUERY (
		getK(env,obj)->getEquivalents ( getROTree(env,arg), actor ),
		"FaCT++ Kernel: error during askEquivalentClasses processing" );
	return actor.getSynonyms();
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    askSuperObjectProperties
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;Z)[[Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;
 */
JNIEXPORT jobjectArray JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_askSuperObjectProperties
  (JNIEnv * env, jobject obj, jobject arg, jboolean direct)
{
	TRACE_JNI("askSuperObjectProperties");
	JTaxonomyActor<ObjectPropertyPolicy> actor(env);
	DLTree* p = getROTree(env,arg);
	PROCESS_ASK_QUERY (
		direct ? getK(env,obj)->getRParents(p,actor) : getK(env,obj)->getRAncestors(p,actor),
		"FaCT++ Kernel: error during askSuperObjectProperties processing" );
	return actor.getElements();
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    askSubObjectProperties
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;Z)[[Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;
 */
JNIEXPORT jobjectArray JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_askSubObjectProperties
  (JNIEnv * env, jobject obj, jobject arg, jboolean direct)
{
	TRACE_JNI("askSubObjectProperties");
	JTaxonomyActor<ObjectPropertyPolicy> actor(env);
	DLTree* p = getROTree(env,arg);
	PROCESS_ASK_QUERY (
		direct ? getK(env,obj)->getRChildren(p,actor) : getK(env,obj)->getRDescendants(p,actor),
		"FaCT++ Kernel: error during askSubObjectProperties processing" );
	return actor.getElements();
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    askEquivalentObjectProperties
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)[Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;
 */
JNIEXPORT jobjectArray JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_askEquivalentObjectProperties
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("askEquivalentObjectProperties");
	JTaxonomyActor<ObjectPropertyPolicy> actor(env);
	PROCESS_ASK_QUERY (
		getK(env,obj)->getREquivalents ( getROTree(env,arg), actor ),
		"FaCT++ Kernel: error during askEquivalentObjectProperties processing" );
	return actor.getSynonyms();
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    askObjectPropertDomain
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_askObjectPropertDomain
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("askObjectPropertDomain");
	Throw ( env, "FaCT++ Kernel: unsupported operation" );
	return NULL;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    askObjectPropertyRange
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_askObjectPropertyRange
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("askObjectPropertyRange");
	Throw ( env, "FaCT++ Kernel: unsupported operation" );
	return NULL;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    isObjectPropertyFunctional
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)Z
 */
JNIEXPORT jboolean JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_isObjectPropertyFunctional
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("isObjectPropertyFunctional");
	bool ret = false;
	PROCESS_ASK_QUERY (
		getK(env,obj)->isFunctional ( getROTree(env,arg), ret ),
		"FaCT++ Kernel: error during isObjectPropertyFunctional processing" );
	return ret;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    isObjectPropertyInverseFunctional
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)Z
 */
JNIEXPORT jboolean JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_isObjectPropertyInverseFunctional
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("isObjectPropertyInverseFunctional");
	bool ret = false;
	PROCESS_ASK_QUERY (
		getK(env,obj)->isInverseFunctional ( getROTree(env,arg), ret ),
		"FaCT++ Kernel: error during isObjectPropertyInverseFunctional processing" );
	return ret;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    isObjectPropertySymmetric
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)Z
 */
JNIEXPORT jboolean JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_isObjectPropertySymmetric
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("isObjectPropertySymmetric");
	Throw ( env, "FaCT++ Kernel: unsupported operation" );
	return false;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    isObjectPropertyAntiSymmetric
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)Z
 */
JNIEXPORT jboolean JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_isObjectPropertyAntiSymmetric
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("isObjectPropertyAntiSymmetric");
	Throw ( env, "FaCT++ Kernel: unsupported operation" );
	return false;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    isObjectPropertyTransitive
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)Z
 */
JNIEXPORT jboolean JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_isObjectPropertyTransitive
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("isObjectPropertyTransitive");
	Throw ( env, "FaCT++ Kernel: unsupported operation" );
	return false;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    isObjectPropertyReflexive
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)Z
 */
JNIEXPORT jboolean JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_isObjectPropertyReflexive
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("isObjectPropertyReflexive");
	Throw ( env, "FaCT++ Kernel: unsupported operation" );
	return false;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    isObjectPropertyIrreflexive
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)Z
 */
JNIEXPORT jboolean JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_isObjectPropertyIrreflexive
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("isObjectPropertyIrreflexive");
	Throw ( env, "FaCT++ Kernel: unsupported operation" );
	return false;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    askSuperDataProperties
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;Z)[[Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;
 */
JNIEXPORT jobjectArray JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_askSuperDataProperties
  (JNIEnv * env, jobject obj, jobject arg, jboolean direct)
{
	TRACE_JNI("askSuperDataProperties");
	JTaxonomyActor<DataPropertyPolicy> actor(env);
	DLTree* p = getROTree(env,arg);
	PROCESS_ASK_QUERY (
		direct ? getK(env,obj)->getRParents(p,actor) : getK(env,obj)->getRAncestors(p,actor),
		"FaCT++ Kernel: error during askSuperDataProperties processing" );
	return actor.getElements();
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    askSubDataProperties
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;Z)[[Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;
 */
JNIEXPORT jobjectArray JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_askSubDataProperties
  (JNIEnv * env, jobject obj, jobject arg, jboolean direct)
{
	TRACE_JNI("askSubDataProperties");
	JTaxonomyActor<DataPropertyPolicy> actor(env);
	DLTree* p = getROTree(env,arg);
	PROCESS_ASK_QUERY (
		direct ? getK(env,obj)->getRChildren(p,actor) : getK(env,obj)->getRDescendants(p,actor),
		"FaCT++ Kernel: error during askSubDataProperties processing" );
	return actor.getElements();
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    askEquivalentDataProperties
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;)[Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;
 */
JNIEXPORT jobjectArray JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_askEquivalentDataProperties
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("askEquivalentDataProperties");
	JTaxonomyActor<DataPropertyPolicy> actor(env);
	PROCESS_ASK_QUERY (
		getK(env,obj)->getREquivalents ( getROTree(env,arg), actor ),
		"FaCT++ Kernel: error during askEquivalentDataProperties processing" );
	return actor.getSynonyms();
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    askDataPropertyDomain
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;)Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_askDataPropertyDomain
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("askDataPropertyDomain");
	Throw ( env, "FaCT++ Kernel: unsupported operation" );
	return NULL;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    askDataPropertyRange
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;)Luk/ac/manchester/cs/factplusplus/DataTypeExpressionPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_askDataPropertyRange
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("askDataPropertyRange");
	Throw ( env, "FaCT++ Kernel: unsupported operation" );
	return NULL;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    isDataPropertyFunctional
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;)Z
 */
JNIEXPORT jboolean JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_isDataPropertyFunctional
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("isDataPropertyFunctional");
	bool ret = false;
	PROCESS_ASK_QUERY (
		getK(env,obj)->isFunctional ( getROTree(env,arg), ret ),
		"FaCT++ Kernel: error during isDataPropertyFunctional processing" );
	return ret;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    askIndividualTypes
 * Signature: (Luk/ac/manchester/cs/factplusplus/IndividualPointer;Z)[[Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobjectArray JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_askIndividualTypes
  (JNIEnv * env, jobject obj, jobject arg, jboolean direct)
{
	TRACE_JNI("askIndividualTypes");
	JTaxonomyActor<ClassPolicy> actor(env);
	DLTree* p = getROTree(env,arg);
	PROCESS_ASK_QUERY (
		direct ? getK(env,obj)->getDirectTypes(p,actor) : getK(env,obj)->getTypes(p,actor),
		"FaCT++ Kernel: error during askIndividualTypes processing" );
	return actor.getElements();
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    isInstanceOf
 * Signature: (Luk/ac/manchester/cs/factplusplus/IndividualPointer;Luk/ac/manchester/cs/factplusplus/ClassPointer;)Z
 */
JNIEXPORT jboolean JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_isInstanceOf
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	TRACE_JNI("isInstanceOf");
	bool ret = false;
	PROCESS_ASK_QUERY (
		getK(env,obj)->isInstance ( getROTree(env,arg1), getROTree(env,arg2), ret ),
		"FaCT++ Kernel: error during isInstanceOf processing" );
	return ret;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    askInstances
 * Signature: (Luk/ac/manchester/cs/factplusplus/ClassPointer;)[Luk/ac/manchester/cs/factplusplus/IndividualPointer;
 */
JNIEXPORT jobjectArray JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_askInstances
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("askInstances");
	JTaxonomyActor<IndividualPolicy> actor(env);
	PROCESS_ASK_QUERY (
		getK(env,obj)->getInstances(getROTree(env,arg),actor),
		"FaCT++ Kernel: error during askInstances processing" );
	return actor.getElements();
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    askSameAs
 * Signature: (Luk/ac/manchester/cs/factplusplus/IndividualPointer;)[Luk/ac/manchester/cs/factplusplus/IndividualPointer;
 */
JNIEXPORT jobjectArray JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_askSameAs
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("askSameAs");
	JTaxonomyActor<IndividualPolicy> actor(env);
	PROCESS_ASK_QUERY (
		getK(env,obj)->getSameAs ( getROTree(env,arg), actor ),
		"FaCT++ Kernel: error during askSameAs processing" );
	return actor.getSynonyms();
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    isSameAs
 * Signature: (Luk/ac/manchester/cs/factplusplus/IndividualPointer;Luk/ac/manchester/cs/factplusplus/IndividualPointer;)Z
 */
JNIEXPORT jboolean JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_isSameAs
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	TRACE_JNI("isSameAs");
	Throw ( env, "FaCT++ Kernel: unsupported operation" );
	return false;
}

//-------------------------------------------------------------
// Aux methods for lists of concepts/roles/individuals
//-------------------------------------------------------------


/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    initArgList
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_initArgList
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("initArgList");
	PROCESS_QUERY (
		getK(env,obj)->openConceptList(),
		"FaCT++ Kernel: error during initArgList processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    addArg
 * Signature: (Luk/ac/manchester/cs/factplusplus/Pointer;)V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_addArg
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("addArg");
	PROCESS_QUERY (
		getK(env,obj)->contConceptList(getTree(env,arg)),
		"FaCT++ Kernel: error during addArg processing" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    closeArgList
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_closeArgList
  (JNIEnv * env ATTR_UNUSED, jobject obj ATTR_UNUSED)
{
	TRACE_JNI("closeArgList");
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    setProgressMonitor
 * Signature: (Luk/ac/manchester/cs/factplusplus/FaCTPlusPlusProgressMonitor;)V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_setProgressMonitor
  (JNIEnv * env, jobject obj, jobject monitor)
{
	TRACE_JNI("setProgressMonitor");
	getK(env,obj)->setProgressMonitor ( new JNIProgressMonitor ( env, monitor ) );
}

#undef PROCESS_QUERY

#ifdef __cplusplus
}
#endif
