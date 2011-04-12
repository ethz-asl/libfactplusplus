/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2006-2011 by Dmitry Tsarkov

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

#include <sstream>

// switch tracing on
//#define JNI_TRACING

#ifdef ENABLE_CHECKING
#	define JNI_TRACING
#endif

// to set up the reference recorder debug info
#ifdef JNI_TRACING
#	define REF_RECORDER_TRACING
#endif

#include "uk_ac_manchester_cs_factplusplus_FaCTPlusPlus.h"
#include "Kernel.h"
#include "JNISupport.h"
#include "JNIActor.h"
#include "JNIMonitor.h"
#include "eFPPTimeout.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef JNI_TRACING
#	define TRACE_JNI(func) std::cerr << "JNI Kernel " << getK(env,obj) << " Call " << func << "\n"
#	define TRACE_ARG(env,obj,arg) do {	\
		getK(env,obj);					\
		} while(0)
#else
#	define TRACE_JNI(func) (void)NULL
#	define TRACE_ARG(env,obj,arg) (void)NULL
#endif

//-------------------------------------------------------------
// Different fields/method IDs and their setup
//-------------------------------------------------------------

/// field for Kernel's ID
jfieldID KernelFID;

/// IDs for different Java interface classes
TClassFieldMethodIDs
	ClassPointer,
	IndividualPointer,
	ObjectPropertyPointer,
	DataPropertyPointer,
	DataTypePointer,
	DataTypeExpressionPointer,
	DataValuePointer,
	DataTypeFacet,
	AxiomPointer;

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    initMethodsFieldsIDs
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_initMethodsFieldsIDs
  (JNIEnv * env, jclass cls)
{
	KernelFID = env->GetFieldID ( cls, "KernelId", "J" );

	if ( KernelFID == 0 )
	{
		Throw ( env, "Can't get 'KernelId' field" );
		return;
	}

	// init IDs for different classes
	ClassPointer.init(env,"[Luk/ac/manchester/cs/factplusplus/ClassPointer;");
	IndividualPointer.init(env,"[Luk/ac/manchester/cs/factplusplus/IndividualPointer;");
	ObjectPropertyPointer.init(env,"[Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;");
	DataPropertyPointer.init(env,"[Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;");
	DataTypePointer.init(env,"[Luk/ac/manchester/cs/factplusplus/DataTypePointer;");
	DataTypeExpressionPointer.init(env,"[Luk/ac/manchester/cs/factplusplus/DataTypeExpressionPointer;");
	DataValuePointer.init(env,"[Luk/ac/manchester/cs/factplusplus/DataValuePointer;");
	DataTypeFacet.init(env,"[Luk/ac/manchester/cs/factplusplus/DataTypeFacet;");
	AxiomPointer.init(env,"[Luk/ac/manchester/cs/factplusplus/AxiomPointer;");
}

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
	// create new kernel and save it in an FaCTPlusPlus object
	ReasoningKernel* Kernel = new ReasoningKernel();
	env->SetLongField ( obj, KernelFID, (jlong)Kernel );
	TRACE_JNI("initKernel");

#ifdef _USE_LOGGING
	// initialize LeveLogger
//	LLM.initLogger ( 20, "reasoning.log" );
#endif
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
	delete getK(env,obj);
	// set to NULL
	env->SetLongField ( obj, KernelFID, 0 );
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
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    setTopBottomPropertyNames
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_setTopBottomPropertyNames
  (JNIEnv * env, jobject obj, jstring ton, jstring bon, jstring tdn, jstring bdn)
{
	TRACE_JNI("setTopBottomPropertyNames");
	JString topObjectName(env,ton);
	JString botObjectName(env,bon);
	JString topDataName(env,tdn);
	JString botDataName(env,bdn);
	getK(env,obj)->setTopBottomRoleNames ( topObjectName(), botObjectName(), topDataName(), botDataName() );
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
	return Class ( env, getEM(env,obj)->Top() );
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
	return Class ( env, getEM(env,obj)->Bottom() );
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
		ret = Class ( env, getCName(env,obj,name()) );
	}
	catch (const EFPPCantRegName&)
	{
		Throw ( env, "FaCT++ Kernel: Can not register new class name" );
	}
	return ret;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getTopObjectProperty
 * Signature: ()Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getTopObjectProperty
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("getTopObjectProperty");
	return ObjectProperty ( env, getOName(env,obj,"http://www.w3.org/2002/07/owl#topObjectProperty") );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getBottomObjectProperty
 * Signature: ()Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getBottomObjectProperty
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("getBottomObjectProperty");
	return ObjectProperty ( env, getOName(env,obj,"http://www.w3.org/2002/07/owl#bottomObjectProperty") );
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
		ret = ObjectProperty ( env, getOName(env,obj,name()) );
	}
	catch (const EFPPCantRegName&)
	{
		Throw ( env, "FaCT++ Kernel: Can not register new object property name" );
	}
	return ret;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getTopDataProperty
 * Signature: ()Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getTopDataProperty
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("getTopDataProperty");
	return DataProperty ( env, getDName(env,obj,"http://www.w3.org/2002/07/owl#topDataProperty") );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getBottomDataProperty
 * Signature: ()Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getBottomDataProperty
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("getBottomDataProperty");
	return DataProperty ( env, getDName(env,obj,"http://www.w3.org/2002/07/owl#bottomDataProperty") );
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
		ret = DataProperty ( env, getDName(env,obj,name()) );
	}
	catch (const EFPPCantRegName&)
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
		ret = Individual ( env, getIName(env,obj,name()) );
	}
	catch (const EFPPCantRegName&)
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
	TExpressionManager* EM = getEM(env,obj);
	if ( DTName == "http://www.w3.org/2000/01/rdf-schema#Literal" ||
		 DTName == "http://www.w3.org/2000/01/rdf-schema#anySimpleType" )
		return DataType ( env, EM->DataTop() );

	if ( DTName == "http://www.w3.org/1999/02/22-rdf-syntax-ns#PlainLiteral" )
		return DataType ( env, EM->getStrDataType() );

	if ( DTName == "http://www.w3.org/2001/XMLSchema#string" )
		return DataType ( env, EM->getStrDataType() );
	if ( DTName == "http://www.w3.org/2001/XMLSchema#anyURI" )
		return DataType ( env, EM->getStrDataType() );

	if ( DTName == "http://www.w3.org/2001/XMLSchema#integer" )
		return DataType ( env, EM->getIntDataType() );
	if ( DTName == "http://www.w3.org/2001/XMLSchema#int" )
		return DataType ( env, EM->getIntDataType() );
	if ( DTName == "http://www.w3.org/2001/XMLSchema#nonNegativeInteger" )
		return DataType ( env, EM->getIntDataType() );

	if ( DTName == "http://www.w3.org/2001/XMLSchema#float" )
		return DataType ( env, EM->getRealDataType() );
	if ( DTName == "http://www.w3.org/2001/XMLSchema#double" )
		return DataType ( env, EM->getRealDataType() );

	if ( DTName == "http://www.w3.org/2001/XMLSchema#boolean" )
		return DataType ( env, EM->getBoolDataType() );

	if ( DTName == "http://www.w3.org/2001/XMLSchema#dateTimeAsLong" )
		return DataType ( env, EM->getTimeDataType() );

	std::stringstream err;
	err << "Unsupported datatype '" << DTName.c_str() << "'";
	Throw ( env, err.str().c_str() );
	return (jobject)0;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getDataSubType
 * Signature: (Ljava/lang/String;Luk/ac/manchester/cs/factplusplus/DataTypeExpressionPointer;)Luk/ac/manchester/cs/factplusplus/DataTypeExpressionPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getDataSubType
  (JNIEnv * env, jobject obj ATTR_UNUSED, jstring str ATTR_UNUSED, jobject type ATTR_UNUSED)
{
	TRACE_JNI("getDataSubType");
	JString name(env,str);
	Throw ( env, "FaCT++ Kernel: unsupported operation 'getDataSubType'" );
	jobject ret = (jobject)0;
#if 0
	try
	{
		ret = DataTypeExpression ( env, getK(env,obj)->getDataTypeCenter().
								   getDataType ( name(), getDataExpr(env,type) ) );
	}
	catch (const EFPPCantRegName&)
	{
		Throw ( env, "FaCT++ Kernel: Can not register new data type" );
	}
#endif
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
	return DataType ( env, getEM(env,obj)->DataTop() );
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
	return DataTypeExpression ( env, getEM(env,obj)->DataOneOf() );
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
	return DataTypeExpression ( env, getEM(env,obj)->RestrictedType ( getDataTypeExpr(env,arg1), getFacetExpr(env,arg2) ) );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getLength
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataValuePointer;)Luk/ac/manchester/cs/factplusplus/DataTypeFacet;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getLength
  (JNIEnv * env, jobject obj ATTR_UNUSED, jobject arg ATTR_UNUSED)
{
	TRACE_JNI("getLength");
	Throw ( env, "FaCT++ Kernel: unsupported facet 'getLength'" );
	return NULL;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getMinLength
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataValuePointer;)Luk/ac/manchester/cs/factplusplus/DataTypeFacet;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getMinLength
  (JNIEnv * env, jobject obj ATTR_UNUSED, jobject arg ATTR_UNUSED)
{
	TRACE_JNI("getMinLength");
	Throw ( env, "FaCT++ Kernel: unsupported facet 'getMinLength'" );
	return NULL;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getMaxLength
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataValuePointer;)Luk/ac/manchester/cs/factplusplus/DataTypeFacet;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getMaxLength
  (JNIEnv * env, jobject obj ATTR_UNUSED, jobject arg ATTR_UNUSED)
{
	TRACE_JNI("getMaxLength");
	Throw ( env, "FaCT++ Kernel: unsupported facet 'getMaxLength'" );
	return NULL;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getPattern
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataValuePointer;)Luk/ac/manchester/cs/factplusplus/DataTypeFacet;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getPattern
  (JNIEnv * env, jobject obj ATTR_UNUSED, jobject arg ATTR_UNUSED)
{
	TRACE_JNI("getPattern");
	Throw ( env, "FaCT++ Kernel: unsupported facet 'getPattern'" );
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
	return Facet ( env, getEM(env,obj)->FacetMinExclusive(getDataValueExpr(env,arg)) );
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
	return Facet ( env, getEM(env,obj)->FacetMaxExclusive(getDataValueExpr(env,arg)) );
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
	return Facet ( env, getEM(env,obj)->FacetMinInclusive(getDataValueExpr(env,arg)) );
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
	return Facet ( env, getEM(env,obj)->FacetMaxInclusive(getDataValueExpr(env,arg)) );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getTotalDigitsFacet
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataValuePointer;)Luk/ac/manchester/cs/factplusplus/DataTypeFacet;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getTotalDigitsFacet
  (JNIEnv * env, jobject obj ATTR_UNUSED, jobject arg ATTR_UNUSED)
{
	TRACE_JNI("getTotalDigitsFacet");
	Throw ( env, "FaCT++ Kernel: unsupported facet 'getTotalDigitsFacet'" );
	return NULL;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getFractionDigitsFacet
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataValuePointer;)Luk/ac/manchester/cs/factplusplus/DataTypeFacet;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getFractionDigitsFacet
  (JNIEnv * env, jobject obj ATTR_UNUSED, jobject arg ATTR_UNUSED)
{
	TRACE_JNI("getFractionDigitsFacet");
	Throw ( env, "FaCT++ Kernel: unsupported facet 'getFractionDigitsFacet'" );
	return NULL;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getDataNot
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataTypeExpressionPointer;)Luk/ac/manchester/cs/factplusplus/DataTypeExpressionPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getDataNot
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("getDataNot");
	return DataTypeExpression ( env, getEM(env,obj)->DataNot(getDataExpr(env,arg)) );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getDataIntersectionOf
 * Signature: ()Luk/ac/manchester/cs/factplusplus/DataTypeExpressionPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getDataIntersectionOf
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("getDataIntersectionOf");
	return DataTypeExpression ( env, getEM(env,obj)->DataAnd() );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getDataUnionOf
 * Signature: ()Luk/ac/manchester/cs/factplusplus/DataTypeExpressionPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getDataUnionOf
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("getDataUnionOf");
	return DataTypeExpression ( env, getEM(env,obj)->DataOr() );
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
		ret = DataValue ( env, getEM(env,obj)->DataValue ( name(), getDataTypeExpr(env,type) ) );
	}
	catch (const EFPPCantRegName&)
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
	return Class ( env, getEM(env,obj)->And() );
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
	return Class ( env, getEM(env,obj)->Or() );
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
	return Class ( env, getEM(env,obj)->Not(getConceptExpr(env,arg)) );
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
	return Class ( env, getEM(env,obj)->Exists ( getORoleExpr(env,arg1), getConceptExpr(env,arg2) ) );
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
	return Class ( env, getEM(env,obj)->Forall ( getORoleExpr(env,arg1), getConceptExpr(env,arg2) ) );
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
	return Class ( env, getEM(env,obj)->Value ( getORoleExpr(env,arg1), getIndividualExpr(env,arg2) ) );
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
	return Class ( env, getEM(env,obj)->Exists ( getDRoleExpr(env,arg1), getDataExpr(env,arg2) ) );
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
	return Class ( env, getEM(env,obj)->Forall ( getDRoleExpr(env,arg1), getDataExpr(env,arg2) ) );
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
	return Class ( env, getEM(env,obj)->Value ( getDRoleExpr(env,arg1), getDataValueExpr(env,arg2) ) );
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
	return Class ( env, getEM(env,obj)->MinCardinality ( n, getORoleExpr(env,arg1), getConceptExpr(env,arg2) ) );
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
	return Class ( env, getEM(env,obj)->Cardinality ( n, getORoleExpr(env,arg1), getConceptExpr(env,arg2) ) );
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
	return Class ( env, getEM(env,obj)->MaxCardinality ( n, getORoleExpr(env,arg1), getConceptExpr(env,arg2) ) );
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
	return Class ( env, getEM(env,obj)->MinCardinality ( n, getDRoleExpr(env,arg1), getDataExpr(env,arg2) ) );
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
	return Class ( env, getEM(env,obj)->Cardinality ( n, getDRoleExpr(env,arg1), getDataExpr(env,arg2) ) );
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
	return Class ( env, getEM(env,obj)->MaxCardinality ( n, getDRoleExpr(env,arg1), getDataExpr(env,arg2) ) );
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
	return ObjectProperty ( env, getEM(env,obj)->Inverse(getORoleExpr(env,arg)) );
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
	return ObjectComplex ( env, getEM(env,obj)->Compose() );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getDataPropertyKey
 * Signature: ()Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getDataPropertyKey
  (JNIEnv * env, jobject obj ATTR_UNUSED)
{
	TRACE_JNI("getDataPropertyKey");
	Throw ( env, "FaCT++ Kernel: unsupported operation 'getDataPropertyKey'" );
	return NULL;
}


/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getObjectPropertyKey
 * Signature: ()Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getObjectPropertyKey
  (JNIEnv * env, jobject obj ATTR_UNUSED)
{
	TRACE_JNI("getObjectPropertyKey");
	Throw ( env, "FaCT++ Kernel: unsupported operation 'getObjectPropertyKey'" );
	return NULL;
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
	return Class ( env, getEM(env,obj)->OneOf() );
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
	return Class ( env, getEM(env,obj)->SelfReference(getORoleExpr(env,arg)) );
}


//-------------------------------------------------------------
// Concept/role/individual axioms (TELL language)
//-------------------------------------------------------------

#define PROCESS_QUERY(Action,Name)				\
	do { TRACE_JNI(Name);						\
	try { return Axiom(env,Action); }			\
	catch ( const EFPPInconsistentKB& )			\
	{ ThrowICO(env); }							\
	catch ( const EFPPNonSimpleRole& nsr )		\
	{ ThrowNSR ( env, nsr.getRoleName() ); }	\
	catch ( const EFPPCycleInRIA& cir )			\
	{ ThrowRIC ( env, cir.getRoleName() ); }	\
	catch ( const EFaCTPlusPlus& fpp )			\
	{ Throw ( env, fpp.what() ); }				\
	catch ( const std::exception& ex )			\
	{ Throw ( env, ex.what() ); }				\
		return NULL;  } while(0)
//	Throw ( env, "FaCT++ Kernel: error during " Name " processing" )

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellClassDeclaration
 * Signature: (Luk/ac/manchester/cs/factplusplus/ClassPointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellClassDeclaration
  (JNIEnv * env, jobject obj, jobject arg)
{
	PROCESS_QUERY ( getK(env,obj)->declare(getConceptExpr(env,arg)), "tellClassDeclaration" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellObjectPropertyDeclaration
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellObjectPropertyDeclaration
  (JNIEnv * env, jobject obj, jobject arg)
{
	PROCESS_QUERY ( getK(env,obj)->declare(getORoleExpr(env,arg)), "tellObjectPropertyDeclaration" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellDataPropertyDeclaration
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellDataPropertyDeclaration
  (JNIEnv * env, jobject obj, jobject arg)
{
	PROCESS_QUERY ( getK(env,obj)->declare(getDRoleExpr(env,arg)), "tellDataPropertyDeclaration" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellIndividualDeclaration
 * Signature: (Luk/ac/manchester/cs/factplusplus/IndividualPointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellIndividualDeclaration
  (JNIEnv * env, jobject obj, jobject arg)
{
	PROCESS_QUERY ( getK(env,obj)->declare(getIndividualExpr(env,arg)), "tellIndividualDeclaration" );
}


/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellDatatypeDeclaration
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataTypePointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellDatatypeDeclaration
  (JNIEnv * env, jobject obj ATTR_UNUSED, jobject arg ATTR_UNUSED)
{
	TRACE_JNI("tellDatatypeDeclaration");
	Throw ( env, "FaCT++ Kernel: unsupported operation 'tellDatatypeDeclaration'" );
	return NULL;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellSubClassOf
 * Signature: (Luk/ac/manchester/cs/factplusplus/ClassPointer;Luk/ac/manchester/cs/factplusplus/ClassPointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellSubClassOf
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	PROCESS_QUERY ( getK(env,obj)->impliesConcepts ( getConceptExpr(env,arg1), getConceptExpr(env,arg2) ), "tellSubClassOf" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellEquivalentClass
 * Signature: ()Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellEquivalentClass
  (JNIEnv * env, jobject obj)
{
	PROCESS_QUERY ( getK(env,obj)->equalConcepts(), "tellEquivalentClasses" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellDisjointClasses
 * Signature: ()Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellDisjointClasses
  (JNIEnv * env, jobject obj)
{
	PROCESS_QUERY ( getK(env,obj)->disjointConcepts(), "tellDisjointClasses" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellDisjointUnion
 * Signature: (Luk/ac/manchester/cs/factplusplus/ClassPointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellDisjointUnion
  (JNIEnv * env, jobject obj, jobject arg)
{
	PROCESS_QUERY ( getK(env,obj)->disjointUnion(getConceptExpr(env,arg)), "tellDisjointUnion" );
}


/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellHasKey
 * Signature: (Luk/ac/manchester/cs/factplusplus/ClassPointer;Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellHasKey
  (JNIEnv * env, jobject obj ATTR_UNUSED, jobject cls ATTR_UNUSED, jobject dataprops ATTR_UNUSED, jobject objectprops ATTR_UNUSED)
{
	TRACE_JNI("tellHasKey");
	Throw ( env, "FaCT++ Kernel: unsupported operation 'tellHasKey'" );
	return NULL;
}


/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellSubObjectProperties
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellSubObjectProperties
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	PROCESS_QUERY ( getK(env,obj)->impliesORoles ( getORoleComplexExpr(env,arg1), getORoleExpr(env,arg2) ), "tellSubObjectProperties" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellEquivalentObjectProperties
 * Signature: ()Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellEquivalentObjectProperties
  (JNIEnv * env, jobject obj)
{
	PROCESS_QUERY ( getK(env,obj)->equalORoles(), "tellEquivalentObjectProperties" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellInverseProperties
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellInverseProperties
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	PROCESS_QUERY ( getK(env,obj)->setInverseRoles ( getORoleExpr(env,arg1), getORoleExpr(env,arg2) ), "tellInverseProperties" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellObjectPropertyRange
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;Luk/ac/manchester/cs/factplusplus/ClassPointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellObjectPropertyRange
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	PROCESS_QUERY ( getK(env,obj)->setORange ( getORoleExpr(env,arg1), getConceptExpr(env,arg2) ), "tellObjectPropertyRange" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellDataPropertyRange
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;Luk/ac/manchester/cs/factplusplus/DataTypePointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellDataPropertyRange
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	PROCESS_QUERY ( getK(env,obj)->setDRange ( getDRoleExpr(env,arg1), getDataExpr(env,arg2) ), "tellDataPropertyRange" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellObjectPropertyDomain
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;Luk/ac/manchester/cs/factplusplus/ClassPointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellObjectPropertyDomain
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	PROCESS_QUERY ( getK(env,obj)->setODomain ( getORoleExpr(env,arg1), getConceptExpr(env,arg2) ), "tellObjectPropertyDomain" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellDataPropertyDomain
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;Luk/ac/manchester/cs/factplusplus/ClassPointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellDataPropertyDomain
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	PROCESS_QUERY ( getK(env,obj)->setDDomain ( getDRoleExpr(env,arg1), getConceptExpr(env,arg2) ), "tellDataPropertyDomain" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellDisjointObjectProperties
 * Signature: ()Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellDisjointObjectProperties
  (JNIEnv * env, jobject obj)
{
	PROCESS_QUERY ( getK(env,obj)->disjointORoles(), "tellDisjointObjectProperties" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellFunctionalObjectProperty
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellFunctionalObjectProperty
  (JNIEnv * env, jobject obj, jobject arg)
{
	PROCESS_QUERY ( getK(env,obj)->setOFunctional(getORoleExpr(env,arg)), "tellFunctionalObjectProperty" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellInverseFunctionalObjectProperty
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellInverseFunctionalObjectProperty
  (JNIEnv * env, jobject obj, jobject arg)
{
	PROCESS_QUERY ( getK(env,obj)->setInverseFunctional(getORoleExpr(env,arg)), "tellInverseFunctionalObjectProperty" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellSymmetricObjectProperty
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellSymmetricObjectProperty
  (JNIEnv * env, jobject obj, jobject arg)
{
	PROCESS_QUERY ( getK(env,obj)->setSymmetric(getORoleExpr(env,arg)), "tellSymmetricObjectProperty" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellAsymmetricObjectProperty
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellAsymmetricObjectProperty
  (JNIEnv * env, jobject obj, jobject arg)
{
	PROCESS_QUERY ( getK(env,obj)->setAsymmetric(getORoleExpr(env,arg)), "tellAsymmetricObjectProperty" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellReflexiveObjectProperty
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellReflexiveObjectProperty
  (JNIEnv * env, jobject obj, jobject arg)
{
	PROCESS_QUERY ( getK(env,obj)->setReflexive(getORoleExpr(env,arg)), "tellReflexiveObjectProperty" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellIrreflexiveObjectProperty
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellIrreflexiveObjectProperty
  (JNIEnv * env, jobject obj, jobject arg)
{
	PROCESS_QUERY ( getK(env,obj)->setIrreflexive(getORoleExpr(env,arg)), "tellIrreflexiveObjectProperty" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellTransitiveObjectProperty
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellTransitiveObjectProperty
  (JNIEnv * env, jobject obj, jobject arg)
{
	PROCESS_QUERY ( getK(env,obj)->setTransitive(getORoleExpr(env,arg)), "tellTransitiveObjectProperty" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellSubDataProperties
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellSubDataProperties
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	PROCESS_QUERY ( getK(env,obj)->impliesDRoles ( getDRoleExpr(env,arg1), getDRoleExpr(env,arg2) ), "tellSubDataProperties" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellEquivalentDataProperties
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellEquivalentDataProperties
  (JNIEnv * env, jobject obj)
{
	PROCESS_QUERY ( getK(env,obj)->equalDRoles(), "tellEquivalentDataProperties" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellDisjointDataProperties
 * Signature: ()Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellDisjointDataProperties
  (JNIEnv * env, jobject obj)
{
	PROCESS_QUERY ( getK(env,obj)->disjointDRoles(), "tellDisjointDataProperties" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellFunctionalDataProperty
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellFunctionalDataProperty
  (JNIEnv * env, jobject obj, jobject arg)
{
	PROCESS_QUERY ( getK(env,obj)->setDFunctional(getDRoleExpr(env,arg)), "tellFunctionalDataProperty" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellIndividualType
 * Signature: (Luk/ac/manchester/cs/factplusplus/IndividualPointer;Luk/ac/manchester/cs/factplusplus/ClassPointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellIndividualType
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	PROCESS_QUERY ( getK(env,obj)->instanceOf ( getIndividualExpr(env,arg1), getConceptExpr(env,arg2) ), "tellIndividualType" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellRelatedIndividuals
 * Signature: (Luk/ac/manchester/cs/factplusplus/IndividualPointer;Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;Luk/ac/manchester/cs/factplusplus/IndividualPointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellRelatedIndividuals
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2, jobject arg3)
{
	PROCESS_QUERY ( getK(env,obj)->relatedTo ( getIndividualExpr(env,arg1), getORoleExpr(env,arg2), getIndividualExpr(env,arg3) ), "tellRelatedIndividuals" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellNotRelatedIndividuals
 * Signature: (Luk/ac/manchester/cs/factplusplus/IndividualPointer;Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;Luk/ac/manchester/cs/factplusplus/IndividualPointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellNotRelatedIndividuals
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2, jobject arg3)
{
	PROCESS_QUERY ( getK(env,obj)->relatedToNot ( getIndividualExpr(env,arg1), getORoleExpr(env,arg2), getIndividualExpr(env,arg3) ), "tellNotRelatedIndividuals" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellRelatedIndividualValue
 * Signature: (Luk/ac/manchester/cs/factplusplus/IndividualPointer;Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;Luk/ac/manchester/cs/factplusplus/DataValuePointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellRelatedIndividualValue
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2, jobject arg3)
{
	PROCESS_QUERY ( getK(env,obj)->valueOf ( getIndividualExpr(env,arg1), getDRoleExpr(env,arg2), getDataValueExpr(env,arg3) ), "tellRelatedIndividualValue" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellNotRelatedIndividualValue
 * Signature: (Luk/ac/manchester/cs/factplusplus/IndividualPointer;Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;Luk/ac/manchester/cs/factplusplus/DataValuePointer;)Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellNotRelatedIndividualValue
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2, jobject arg3)
{
	PROCESS_QUERY ( getK(env,obj)->valueOfNot ( getIndividualExpr(env,arg1), getDRoleExpr(env,arg2), getDataValueExpr(env,arg3) ), "tellNotRelatedIndividualValue" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellSameIndividuals
 * Signature: ()Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellSameIndividuals
  (JNIEnv * env, jobject obj)
{
	PROCESS_QUERY ( getK(env,obj)->processSame(), "tellSameIndividuals" );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    tellDifferentIndividuals
 * Signature: ()Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobject JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_tellDifferentIndividuals
  (JNIEnv * env, jobject obj)
{
	PROCESS_QUERY ( getK(env,obj)->processDifferent(), "tellDifferentIndividuals" );
}

#undef PROCESS_QUERY

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    retract
 * Signature: (Luk/ac/manchester/cs/factplusplus/AxiomPointer;)V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_retract
  (JNIEnv * env, jobject obj, jobject axiom)
{
	TRACE_JNI("retract");
	getK(env,obj)->retract(getAxiom(env,axiom));
}

//-------------------------------------------------------------
// minimal query language (ASK languages)
//-------------------------------------------------------------

#define PROCESS_ASK_QUERY(Action,Name)			\
	do { try { Action; }						\
	catch ( const EFPPInconsistentKB& )			\
	{ ThrowICO(env); }							\
	catch ( const EFPPNonSimpleRole& nsr )		\
	{ ThrowNSR ( env, nsr.getRoleName() ); }	\
	catch ( const EFPPCycleInRIA& cir )			\
	{ ThrowRIC ( env, cir.getRoleName() ); }	\
	catch ( const EFPPTimeout& )				\
	{ ThrowTO(env); }							\
	catch ( const EFaCTPlusPlus& fpp )			\
	{ Throw ( env, fpp.what() ); }				\
	catch ( const std::exception& ex )			\
	{ Throw ( env, ex.what() ); }  } while(0)

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    isKBConsistent
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_isKBConsistent
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("isKBConsistent");
	bool ret = false;
	PROCESS_ASK_QUERY ( ret=getK(env,obj)->isKBConsistent(),"isKBConsistent");
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
	PROCESS_ASK_QUERY ( getK(env,obj)->classifyKB(),"classify");
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
	PROCESS_ASK_QUERY ( getK(env,obj)->realiseKB(),"realise");
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    isRealised
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_isRealised
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("isRealised");
	return getK(env,obj)->isKBRealised();
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
	TRACE_ARG(env,obj,arg);
	bool ret = false;
	PROCESS_ASK_QUERY ( ret=getK(env,obj)->isSatisfiable ( getROConceptExpr(env,arg) ),"isClassSatisfiable");
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
	TRACE_ARG(env,obj,arg1);
	TRACE_ARG(env,obj,arg2);
	bool ret = false;
	PROCESS_ASK_QUERY ( ret=getK(env,obj)->isSubsumedBy ( getROConceptExpr(env,arg1), getROConceptExpr(env,arg2) ),"isClassSubsumedBy");
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
	TRACE_ARG(env,obj,arg1);
	TRACE_ARG(env,obj,arg2);
	bool ret = false;
	PROCESS_ASK_QUERY ( ret=getK(env,obj)->isEquivalent ( getROConceptExpr(env,arg1), getROConceptExpr(env,arg2) ),"isClassEquivalentTo");
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
	TRACE_ARG(env,obj,arg1);
	TRACE_ARG(env,obj,arg2);
	bool ret = false;
	PROCESS_ASK_QUERY ( ret=getK(env,obj)->isDisjoint ( getROConceptExpr(env,arg1), getROConceptExpr(env,arg2) ),"isClassDisjointWith");
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
	TRACE_ARG(env,obj,arg);
	JTaxonomyActor<ClassPolicy> actor(env,obj);
	const TConceptExpr* p = getROConceptExpr(env,arg);
	PROCESS_ASK_QUERY ( getK(env,obj)->getSubConcepts(p,direct,actor),"askSubClasses");
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
	TRACE_ARG(env,obj,arg);
	JTaxonomyActor<ClassPolicy> actor(env,obj);
	const TConceptExpr* p = getROConceptExpr(env,arg);
	PROCESS_ASK_QUERY ( getK(env,obj)->getSupConcepts(p,direct,actor),"askSuperClasses");
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
	TRACE_ARG(env,obj,arg);
	JTaxonomyActor<ClassPolicy> actor(env,obj);
	PROCESS_ASK_QUERY (
		getK(env,obj)->getEquivalentConcepts ( getROConceptExpr(env,arg), actor ),"askEquivalentClasses");
	return actor.getSynonyms();
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    askDisjointClasses
 * Signature: (Luk/ac/manchester/cs/factplusplus/ClassPointer;)[[Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobjectArray JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_askDisjointClasses
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("askDisjointClasses");
	TRACE_ARG(env,obj,arg);
	JTaxonomyActor<ClassPolicy> actor(env,obj);
	const TConceptExpr* p = getROConceptExpr(env,arg);
	PROCESS_ASK_QUERY ( getK(env,obj)->getDisjointConcepts(p,actor),"askDisjointClasses");
	return actor.getElements();
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
	TRACE_ARG(env,obj,arg);
	JTaxonomyActor<ObjectPropertyPolicy> actor(env,obj);
	const TORoleExpr* p = getROORoleExpr(env,arg);
	PROCESS_ASK_QUERY ( getK(env,obj)->getSupRoles(p,direct,actor),"askSuperObjectProperties");
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
	TRACE_ARG(env,obj,arg);
	JTaxonomyActor<ObjectPropertyPolicy> actor(env,obj);
	const TORoleExpr* p = getROORoleExpr(env,arg);
	PROCESS_ASK_QUERY ( getK(env,obj)->getSubRoles(p,direct,actor),"askSubObjectProperties");
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
	TRACE_ARG(env,obj,arg);
	JTaxonomyActor<ObjectPropertyPolicy> actor(env,obj);
	PROCESS_ASK_QUERY ( getK(env,obj)->getEquivalentRoles ( getROORoleExpr(env,arg), actor ),"askEquivalentObjectProperties");
	return actor.getSynonyms();
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    askObjectPropertyDomain
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)[[Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobjectArray JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_askObjectPropertyDomain
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("askObjectPropertyDomain");
	TRACE_ARG(env,obj,arg);
	JTaxonomyActor<ClassPolicy> actor(env,obj);
	PROCESS_ASK_QUERY ( getK(env,obj)->getRoleDomain ( getROORoleExpr(env,arg), true, actor ),"askObjectPropertyDomain");
	return actor.getElements();
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    askObjectPropertyRange
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)[[Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobjectArray JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_askObjectPropertyRange
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("askObjectPropertyRange");
	TRACE_ARG(env,obj,arg);
	JTaxonomyActor<ClassPolicy> actor(env,obj);
	PROCESS_ASK_QUERY ( getK(env,obj)->getRoleRange ( getROORoleExpr(env,arg), true, actor ),"askObjectPropertyRange");
	return actor.getElements();
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
	TRACE_ARG(env,obj,arg);
	bool ret = false;
	PROCESS_ASK_QUERY ( ret=getK(env,obj)->isFunctional ( getROORoleExpr(env,arg) ),"isObjectPropertyFunctional");
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
	TRACE_ARG(env,obj,arg);
	bool ret = false;
	PROCESS_ASK_QUERY ( ret=getK(env,obj)->isInverseFunctional ( getROORoleExpr(env,arg) ),"isObjectPropertyInverseFunctional");
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
	TRACE_ARG(env,obj,arg);
	bool ret = false;
	PROCESS_ASK_QUERY ( ret=getK(env,obj)->isSymmetric ( getROORoleExpr(env,arg) ),"isObjectPropertySymmetric");
	return ret;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    isObjectPropertyAsymmetric
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)Z
 */
JNIEXPORT jboolean JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_isObjectPropertyAsymmetric
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("isObjectPropertyAsymmetric");
	TRACE_ARG(env,obj,arg);
	bool ret = false;
	PROCESS_ASK_QUERY ( ret=getK(env,obj)->isAsymmetric ( getROORoleExpr(env,arg) ),"isObjectPropertyAsymmetric");
	return ret;
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
	TRACE_ARG(env,obj,arg);
	bool ret = false;
	PROCESS_ASK_QUERY ( ret=getK(env,obj)->isTransitive ( getROORoleExpr(env,arg) ),"isObjectPropertyTransitive");
	return ret;
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
	TRACE_ARG(env,obj,arg);
	bool ret = false;
	PROCESS_ASK_QUERY ( ret=getK(env,obj)->isReflexive ( getROORoleExpr(env,arg) ),"isObjectPropertyReflexive");
	return ret;
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
	TRACE_ARG(env,obj,arg);
	bool ret = false;
	PROCESS_ASK_QUERY ( ret=getK(env,obj)->isIrreflexive ( getROORoleExpr(env,arg) ),"isObjectPropertyIrreflexive");
	return ret;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    isObjectSubPropertyOf
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)Z
 */
JNIEXPORT jboolean JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_isObjectSubPropertyOf
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	TRACE_JNI("isObjectSubPropertyOf");
	TRACE_ARG(env,obj,arg1);
	TRACE_ARG(env,obj,arg2);
	bool ret = false;
	PROCESS_ASK_QUERY ( ret=getK(env,obj)->isSubRoles ( getROORoleExpr(env,arg1), getROORoleExpr(env,arg2) ),"isObjectSubPropertyOf");
	return ret;
}
/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    isObjectPropertyDisjointWith
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)Z
 */
JNIEXPORT jboolean JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_isObjectPropertyDisjointWith
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	TRACE_JNI("isObjectPropertyDisjointWith");
	TRACE_ARG(env,obj,arg1);
	TRACE_ARG(env,obj,arg2);
	bool ret = false;
	PROCESS_ASK_QUERY ( ret=getK(env,obj)->isDisjointRoles ( getROORoleExpr(env,arg1), getROORoleExpr(env,arg2) ),"isObjectPropertyDisjointWith");
	return ret;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    isSubPropertyChainOf
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)Z
 */
JNIEXPORT jboolean JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_isSubPropertyChainOf
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("isSubPropertyChainOf");
	TRACE_ARG(env,obj,arg);
	bool ret = false;
	PROCESS_ASK_QUERY ( ret=getK(env,obj)->isSubChain(getROORoleExpr(env,arg)),"isSubPropertyChainOf");
	return ret;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    arePropertiesDisjoint
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_arePropertiesDisjoint
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("arePropertiesDisjoint");
	bool ret = false;
	PROCESS_ASK_QUERY ( ret=getK(env,obj)->isDisjointRoles(),"arePropertiesDisjoint");
	return ret;
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
	TRACE_ARG(env,obj,arg);
	JTaxonomyActor<DataPropertyPolicy> actor(env,obj);
	const TDRoleExpr* p = getRODRoleExpr(env,arg);
	PROCESS_ASK_QUERY ( getK(env,obj)->getSupRoles(p,direct,actor),"askSuperDataProperties");
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
	TRACE_ARG(env,obj,arg);
	JTaxonomyActor<DataPropertyPolicy> actor(env,obj);
	const TDRoleExpr* p = getRODRoleExpr(env,arg);
	PROCESS_ASK_QUERY ( getK(env,obj)->getSubRoles(p,direct,actor),"askSubDataProperties");
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
	TRACE_ARG(env,obj,arg);
	JTaxonomyActor<DataPropertyPolicy> actor(env,obj);
	PROCESS_ASK_QUERY ( getK(env,obj)->getEquivalentRoles ( getRODRoleExpr(env,arg), actor ),"askEquivalentDataProperties");
	return actor.getSynonyms();
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    askDataPropertyDomain
 * Signature: (Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;)[[Luk/ac/manchester/cs/factplusplus/ClassPointer;
 */
JNIEXPORT jobjectArray JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_askDataPropertyDomain
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("askDataPropertyDomain");
	TRACE_ARG(env,obj,arg);
	JTaxonomyActor<ClassPolicy> actor(env,obj);
	PROCESS_ASK_QUERY ( getK(env,obj)->getRoleDomain ( getRODRoleExpr(env,arg), true, actor ),"askDataPropertyDomain");
	return actor.getElements();
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
	TRACE_ARG(env,obj,arg);
	bool ret = false;
	PROCESS_ASK_QUERY ( ret=getK(env,obj)->isFunctional ( getRODRoleExpr(env,arg) ),"isDataPropertyFunctional");
	return ret;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    isDataSubPropertyOf
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)Z
 */
JNIEXPORT jboolean JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_isDataSubPropertyOf
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	TRACE_JNI("isDataSubPropertyOf");
	TRACE_ARG(env,obj,arg1);
	TRACE_ARG(env,obj,arg2);
	bool ret = false;
	PROCESS_ASK_QUERY ( ret=getK(env,obj)->isSubRoles ( getRODRoleExpr(env,arg1), getRODRoleExpr(env,arg2) ),"isDataSubPropertyOf");
	return ret;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    isObjectPropertyDisjointWith
 * Signature: (Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)Z
 */
JNIEXPORT jboolean JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_isDataPropertyDisjointWith
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	TRACE_JNI("isDataPropertyDisjointWith");
	TRACE_ARG(env,obj,arg1);
	TRACE_ARG(env,obj,arg2);
	bool ret = false;
	PROCESS_ASK_QUERY ( ret=getK(env,obj)->isDisjointRoles ( getRODRoleExpr(env,arg1), getRODRoleExpr(env,arg2) ),"isDataPropertyDisjointWith");
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
	TRACE_ARG(env,obj,arg);
	JTaxonomyActor<ClassPolicy> actor(env,obj);
	const TIndividualExpr* p = getROIndividualExpr(env,arg);
	PROCESS_ASK_QUERY ( getK(env,obj)->getTypes(p,direct,actor),"askIndividualTypes");
	return actor.getElements();
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    askObjectProperties
 * Signature: (Luk/ac/manchester/cs/factplusplus/IndividualPointer;)[Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;
 */
JNIEXPORT jobjectArray JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_askObjectProperties
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("askObjectProperties");
	TRACE_ARG(env,obj,arg);
	ReasoningKernel::NamesVector Rs;
	PROCESS_ASK_QUERY ( getK(env,obj)->getRelatedRoles ( getROIndividualExpr(env,arg), Rs, /*data=*/false, /*needI=*/false ),"askObjectProperties");
	std::vector<TExpr*> acc;
	for ( ReasoningKernel::NamesVector::const_iterator p = Rs.begin(), p_end = Rs.end(); p < p_end; ++p )
		acc.push_back(getOName(env,obj,(*p)->getName()));
	return buildArray ( env, acc, ObjectPropertyPointer );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    askRelatedIndividuals
 * Signature: (Luk/ac/manchester/cs/factplusplus/IndividualPointer;Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;)[Luk/ac/manchester/cs/factplusplus/IndividualPointer;
 */
JNIEXPORT jobjectArray JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_askRelatedIndividuals
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2)
{
	TRACE_JNI("askRelatedIndividuals");
	TRACE_ARG(env,obj,arg1);
	TRACE_ARG(env,obj,arg2);
	ReasoningKernel::NamesVector Js;
	PROCESS_ASK_QUERY ( getK(env,obj)->getRoleFillers ( getROIndividualExpr(env,arg1), getROORoleExpr(env,arg2), Js ),"askRelatedIndividuals");
	std::vector<TExpr*> acc;
	for ( ReasoningKernel::NamesVector::const_iterator p = Js.begin(), p_end = Js.end(); p < p_end; ++p )
		acc.push_back(getIName(env,obj,(*p)->getName()));
	return buildArray ( env, acc, IndividualPointer );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    askDataProperties
 * Signature: (Luk/ac/manchester/cs/factplusplus/IndividualPointer;)[Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;
 */
JNIEXPORT jobjectArray JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_askDataProperties
  (JNIEnv * env, jobject obj, jobject arg)
{
	TRACE_JNI("askDataProperties");
	TRACE_ARG(env,obj,arg);
	ReasoningKernel::NamesVector Rs;
	PROCESS_ASK_QUERY ( getK(env,obj)->getRelatedRoles ( getROIndividualExpr(env,arg), Rs, /*data=*/true, /*needI=*/false ),"askDataProperties");
	std::vector<TExpr*> acc;
	for ( ReasoningKernel::NamesVector::const_iterator p = Rs.begin(), p_end = Rs.end(); p < p_end; ++p )
		acc.push_back(getDName(env,obj,(*p)->getName()));
	return buildArray ( env, acc, DataPropertyPointer );
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    askRelatedValues
 * Signature: (Luk/ac/manchester/cs/factplusplus/IndividualPointer;Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;)[Luk/ac/manchester/cs/factplusplus/DataValuePointer;
 */
JNIEXPORT jobjectArray JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_askRelatedValues
  (JNIEnv * env, jobject obj ATTR_UNUSED, jobject arg1 ATTR_UNUSED, jobject arg2 ATTR_UNUSED)
{
	TRACE_JNI("askRelatedValues");
	TRACE_ARG(env,obj,arg1);
	TRACE_ARG(env,obj,arg2);
	Throw ( env, "FaCT++ Kernel: unsupported operation 'askRelatedValues'" );
	return NULL;
#if 0
	ReasoningKernel::NamesVector Js;
	PROCESS_ASK_QUERY ( getK(env,obj)->getRoleFillers ( getROIndividualExpr(env,arg1), getRODRoleExpr(env,arg2), Js ),"askRelatedValues");
	std::vector<TExpr*> acc;
	for ( ReasoningKernel::NamesVector::const_iterator p = Js.begin(), p_end = Js.end(); p < p_end; ++p )
		acc.push_back(new TExpr(TLexeme(DATAEXPR,const_cast<TNamedEntry*>(*p))));
	return buildArray ( env, acc, DataValuePointer );
#endif
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    hasDataPropertyRelationship
 * Signature: (Luk/ac/manchester/cs/factplusplus/IndividualPointer;Luk/ac/manchester/cs/factplusplus/DataPropertyPointer;Luk/ac/manchester/cs/factplusplus/DataValuePointer;)Z
 */
JNIEXPORT jboolean JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_hasDataPropertyRelationship
  (JNIEnv * env, jobject obj ATTR_UNUSED, jobject arg1 ATTR_UNUSED, jobject arg2 ATTR_UNUSED, jobject arg3 ATTR_UNUSED)
{
	TRACE_JNI("hasDataPropertyRelationship");
	TRACE_ARG(env,obj,arg1);
	TRACE_ARG(env,obj,arg2);
	TRACE_ARG(env,obj,arg3);
	Throw ( env, "FaCT++ Kernel: unsupported operation 'hasDataPropertyRelationship'" );
	return NULL;
#if 0
	bool ret = false;
	PROCESS_ASK_QUERY ( ret=getK(env,obj)->isRelated ( getROIndividualExpr(env,arg1), getRODRoleExpr(env,arg2), getROIndividualExpr(env,arg3) ),"hasDataPropertyRelationship");
	return ret;
#endif
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    hasObjectPropertyRelationship
 * Signature: (Luk/ac/manchester/cs/factplusplus/IndividualPointer;Luk/ac/manchester/cs/factplusplus/ObjectPropertyPointer;Luk/ac/manchester/cs/factplusplus/IndividualPointer;)Z
 */
JNIEXPORT jboolean JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_hasObjectPropertyRelationship
  (JNIEnv * env, jobject obj, jobject arg1, jobject arg2, jobject arg3)
{
	TRACE_JNI("hasObjectPropertyRelationship");
	TRACE_ARG(env,obj,arg1);
	TRACE_ARG(env,obj,arg2);
	TRACE_ARG(env,obj,arg3);
	bool ret = false;
	PROCESS_ASK_QUERY ( ret=getK(env,obj)->isRelated ( getROIndividualExpr(env,arg1), getROORoleExpr(env,arg2), getROIndividualExpr(env,arg3) ),"hasObjectPropertyRelationship");
	return ret;
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
	PROCESS_ASK_QUERY ( ret=getK(env,obj)->isInstance ( getROIndividualExpr(env,arg1), getROConceptExpr(env,arg2) ),"isInstanceOf");
	return ret;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    askInstances
 * Signature: (Luk/ac/manchester/cs/factplusplus/ClassPointer;Z)[Luk/ac/manchester/cs/factplusplus/IndividualPointer;
 */
JNIEXPORT jobjectArray JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_askInstances
  (JNIEnv * env, jobject obj, jobject arg, jboolean direct)
{
	TRACE_JNI("askInstances");
	TRACE_ARG(env,obj,arg);
	JTaxonomyActor<IndividualPolicy</*plain=*/true> > actor(env,obj);
	const TConceptExpr* p = getROConceptExpr(env,arg);
	PROCESS_ASK_QUERY ( direct ? getK(env,obj)->getDirectInstances(p,actor) : getK(env,obj)->getInstances(p,actor),"askInstances");
	return actor.getElements();
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    askInstancesGrouped
 * Signature: (Luk/ac/manchester/cs/factplusplus/ClassPointer;Z)[[Luk/ac/manchester/cs/factplusplus/IndividualPointer;
 */
JNIEXPORT jobjectArray JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_askInstancesGrouped
  (JNIEnv * env, jobject obj, jobject arg, jboolean direct)
{
	TRACE_JNI("askInstancesGrouped");
	TRACE_ARG(env,obj,arg);
	JTaxonomyActor<IndividualPolicy</*plain=*/false> > actor(env,obj);
	const TConceptExpr* p = getROConceptExpr(env,arg);
	PROCESS_ASK_QUERY ( direct ? getK(env,obj)->getDirectInstances(p,actor) : getK(env,obj)->getInstances(p,actor),"askInstances");
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
	TRACE_ARG(env,obj,arg);
	JTaxonomyActor<IndividualPolicy</*plain=*/true> > actor(env,obj);
	PROCESS_ASK_QUERY ( getK(env,obj)->getSameAs ( getROIndividualExpr(env,arg), actor ),"askSameAs");
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
	TRACE_ARG(env,obj,arg1);
	TRACE_ARG(env,obj,arg2);
	bool ret = false;
	PROCESS_ASK_QUERY ( ret=getK(env,obj)->isSameIndividuals ( getROIndividualExpr(env,arg1), getROIndividualExpr(env,arg2) ),"isSameAs");
	return ret;
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    setOperationTimeout
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_setOperationTimeout
(JNIEnv * env, jobject obj, jlong delay)
{
	TRACE_JNI("setOperationTimeout");
	getK(env,obj)->setOperationTimeout(delay > 0 ? static_cast<unsigned long>(delay) : 0);
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
	getEM(env,obj)->newArgList();
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
	TRACE_ARG(env,obj,arg);
	getEM(env,obj)->addArg(getExpr(env,arg));
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

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    startChanges
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_startChanges
  (JNIEnv * env ATTR_UNUSED, jobject obj ATTR_UNUSED)
{
	TRACE_JNI("startChanges");
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    endChanges
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_endChanges
  (JNIEnv * env ATTR_UNUSED, jobject obj ATTR_UNUSED)
{
	TRACE_JNI("endChanges");
	// do nothing for now
}

#undef PROCESS_ASK_QUERY

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    needTracing
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_needTracing
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("needTracing");
	getK(env,obj)->needTracing();
}

/*
 * Class:     uk_ac_manchester_cs_factplusplus_FaCTPlusPlus
 * Method:    getTrace
 * Signature: ()[Luk/ac/manchester/cs/factplusplus/AxiomPointer;
 */
JNIEXPORT jobjectArray JNICALL Java_uk_ac_manchester_cs_factplusplus_FaCTPlusPlus_getTrace
  (JNIEnv * env, jobject obj)
{
	TRACE_JNI("getTrace");
	return buildArray ( env, getK(env,obj)->getTrace(), AxiomPointer );
}

#ifdef __cplusplus
}
#endif
