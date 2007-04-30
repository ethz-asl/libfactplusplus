package uk.ac.manchester.cs.factplusplus;

/**
 * Author: Matthew Horridge<br>
 * The University Of Manchester<br>
 * Medical Informatics Group<br>
 * Date: 10-Jul-2006<br><br>
 * <p/>
 * matthew.horridge@cs.man.ac.uk<br>
 * www.cs.man.ac.uk/~horridgm<br><br>
 *
 * An interface to the native FaCT++ reasoner.  Use of this
 * class requires the FaCT++ JNI library for the appropriate
 * platform.
 */
public class FaCTPlusPlus {

    static {
        // Load the FaCT++ JNI library
        System.loadLibrary("FaCTPlusPlusJNI");
    }

    /**
     * Set internally on the native side - DO NOT ALTER!
     */
    private long KernelId;

    public FaCTPlusPlus() throws Exception {
        initKernel();
    }

    /**
     * Use this method to dispose of native resources.  This method
     * MUST be called when the reasoner is no longer required.  Failure
     * to call dispose, may result in memory leaks!
     */
    public void dispose() throws Exception {
        deleteKernel();
    }

    private native void initKernel() throws Exception;

    private native void deleteKernel() throws Exception;

    /**
     * Clears told and any cached information from the kernel.
     * @throws Exception
     */
    public native void clearKernel() throws Exception;

    /**
     * Gets the class corresponding to TOP
     * @throws Exception
     */
    public native ClassPointer getThing() throws Exception;

    /**
     * Gets the class corresponding to BOTTOM
     * @throws Exception
     */
    public native ClassPointer getNothing() throws Exception;

    /**
     * Gets a pointer to a named class.
     * @param name The name of the class.
     * @return A <code>ClassPointer</code>
     * @throws Exception
     */
    public native ClassPointer getNamedClass(String name) throws Exception;

    /**
     * Gets a pointer to an object property.
     * @param name The name of the property.
     * @return A pointer to the object property that has the specified name.
     * @throws Exception
     */
    public native ObjectPropertyPointer getObjectProperty(String name) throws Exception;

    public native DataPropertyPointer getDataProperty(String name) throws Exception;

    public native IndividualPointer getIndividual(String name) throws Exception;


    ///////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Datatype stuff

    /**
     * Gets a pointer to a built in data type
     * @param name The name of the datatype. e.g. string, int, float etc.
     */
    public native DataTypePointer getBuiltInDataType(String name) throws Exception;

    /**
     * Assigns a name to a datatype expression
     * @param name The name to assign to the datatype
     * @param datatypeExpression
     */
    public native DataTypeExpressionPointer getDataSubType(String name, DataTypeExpressionPointer datatypeExpression) throws Exception;

    /**
     * Gets a data enumeration using previously added arguments
     * (initArgList, addArg, closeArgList)
     */
    public native DataTypeExpressionPointer getDataEnumeration() throws Exception;

    public native DataTypeExpressionPointer getRestrictedDataType(DataTypeExpressionPointer d, DataTypeFacet facet) throws Exception;

    public native DataTypeFacet getLength(DataValuePointer dv) throws Exception;

    public native DataTypeFacet getMinLength(DataValuePointer dv) throws Exception;

    public native DataTypeFacet getMaxLength(DataValuePointer dv) throws Exception;

    public native DataTypeFacet getPattern(DataValuePointer dv) throws Exception;

    public native DataTypeFacet getEnumeration(DataValuePointer dv) throws Exception;

    public native DataTypeFacet getMinExclusiveFacet(DataValuePointer dv) throws Exception;

    public native DataTypeFacet getMaxExclusiveFacet(DataValuePointer dv) throws Exception;

    public native DataTypeFacet getMinInclusiveFacet(DataValuePointer dv) throws Exception;

    public native DataTypeFacet getMaxInclusiveFacet(DataValuePointer dv) throws Exception;

    public native DataTypeFacet getTotalDigitsFacet(DataValuePointer dv) throws Exception;

    public native DataTypeFacet getFractionDigitsFacet(DataValuePointer dv) throws Exception;




    public native DataTypeExpressionPointer getNot(DataTypeExpressionPointer d) throws Exception;

    public native DataValuePointer getDataValue(String literal, DataTypePointer type) throws Exception;



    ////////////////////////////////////////////////////////////////////////////////////////////////////



    /**
     * Gets an intersection whose operands are in the last
     * closed arg list.
     */
    public native ClassPointer getConceptAnd() throws Exception;

    /**
     * Gets a union whose operands are in the last
     * closed arg list.
     */
    public native ClassPointer getConceptOr() throws Exception;

    public native ClassPointer getConceptNot(ClassPointer c) throws Exception;

    public native ClassPointer getObjectSome(ObjectPropertyPointer r, ClassPointer c) throws Exception;

    public native ClassPointer getObjectAll(ObjectPropertyPointer r, ClassPointer c) throws Exception;

    public native ClassPointer getObjectValue(ObjectPropertyPointer r, IndividualPointer i) throws Exception;

    public native ClassPointer getDataSome(DataPropertyPointer r, DataTypeExpressionPointer d) throws Exception;

    public native ClassPointer getDataAll(DataPropertyPointer r, DataTypeExpressionPointer d) throws Exception;

    public native ClassPointer getDataValue(DataPropertyPointer r, DataValuePointer d) throws Exception;

    public native ClassPointer getObjectAtLeast(int num, ObjectPropertyPointer r, ClassPointer c) throws Exception;

    public native ClassPointer getObjectExact(int num, ObjectPropertyPointer r, ClassPointer c) throws Exception;

    public native ClassPointer getObjectAtMost(int num, ObjectPropertyPointer r, ClassPointer c) throws Exception;

    public native ClassPointer getDataAtLeast(int num, DataPropertyPointer r, DataTypePointer d) throws Exception;

    public native ClassPointer getDataExact(int num, DataPropertyPointer r, DataTypePointer d) throws Exception;

    public native ClassPointer getDataAtMost(int num, DataPropertyPointer r, DataTypePointer d) throws Exception;

    public native ObjectPropertyPointer getInverseProperty(ObjectPropertyPointer r) throws Exception;

    /**
     * Gets a property chain whose properties are in the last
     * closed arg list.
     */
    public native ObjectPropertyPointer getPropertyComposition() throws Exception;

    /**
     * Gets an enumeration whose individuals are in the last
     * closed arg list.
     */
    public native ClassPointer getOneOf() throws Exception;

    public native ClassPointer getSelf(ObjectPropertyPointer r) throws Exception;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Axioms

    public native void tellSubClassOf(ClassPointer c, ClassPointer d) throws Exception;

    /**
     * Tells an equivalent classes axiom, whose classes are
     * in the last closed arg list.
     */
    public native void tellEquivalentClass() throws Exception;

    /**
     * Tells a disjoint classes axiom, whose classes are
     * in the last closed arg list.
     */
    public native void tellDisjointClasses() throws Exception;


    public native void tellSubObjectProperties(ObjectPropertyPointer s, ObjectPropertyPointer r) throws Exception;

    /**
     * Tells an equivalent object properties axiom, whose properties are
     * in the last closed arg list.
     */
    public native void tellEquivalentObjectProperties() throws Exception;

    public native void tellInverseProperties(ObjectPropertyPointer s, ObjectPropertyPointer r) throws Exception;

    public native void tellObjectPropertyRange(ObjectPropertyPointer s, ClassPointer c) throws Exception;

    public native void tellDataPropertyRange(DataPropertyPointer s, DataTypePointer d) throws Exception;

    public native void tellObjectPropertyDomain(ObjectPropertyPointer s, ClassPointer c) throws Exception;

    public native void tellDataPropertyDomain(DataPropertyPointer s, ClassPointer c) throws Exception;

    /**
     * Tells a disjoint object properties axiom, whose properties are
     * in the last closed arg list.
     */
    public native void tellDisjointObjectProperties() throws Exception;

    public native void tellFunctionalObjectProperty(ObjectPropertyPointer s) throws Exception;

    public native void tellInverseFunctionalObjectProperty(ObjectPropertyPointer s) throws Exception;

    public native void tellSymmetricObjectProperty(ObjectPropertyPointer s) throws Exception;

    public native void tellAntiSymmetricObjectProperty(ObjectPropertyPointer s) throws Exception;

    public native void tellReflexiveObjectProperty(ObjectPropertyPointer s) throws Exception;

    public native void tellIrreflexiveObjectProperty(ObjectPropertyPointer s) throws Exception;

    public native void tellTransitiveObjectProperty(ObjectPropertyPointer s) throws Exception;



    public native void tellSubDataProperties(DataPropertyPointer s, DataPropertyPointer r) throws Exception;

    /**
     * Tells an equivalent data properties axiom, whose properties are
     * in the last closed arg list.
     */
    public native void tellEquivalentDataProperties(DataPropertyPointer s, DataPropertyPointer r) throws Exception;

    /**
     * Tells a disjoint data properties axiom, whose properties are
     * in the last closed arg list.
     */
    public native void tellDisjointDataProperties(DataPropertyPointer s, DataPropertyPointer r) throws Exception;

    public native void tellFunctionalDataProperty(DataPropertyPointer s) throws Exception;


    public native void tellIndividualType(IndividualPointer i, ClassPointer c) throws Exception;

    public native void tellRelatedIndividuals(IndividualPointer i, ObjectPropertyPointer r, IndividualPointer j) throws Exception;

    public native void tellNotRelatedIndividuals(IndividualPointer i, ObjectPropertyPointer r, IndividualPointer j) throws Exception;

    public native void tellRelatedIndividualValue(IndividualPointer i, DataPropertyPointer r, DataValuePointer dv) throws Exception;

    public native void tellNotRelatedIndividualValue(IndividualPointer i, DataPropertyPointer r, DataValuePointer dv) throws Exception;

    /**
     * Tells a same individuals axiom, whose individuals are
     * in the last closed arg list.
     */
    public native void tellSameIndividuals() throws Exception;

    /**
     * Tells a different individuals axiom, whose individuals are
     * in the last closed arg list.
     */
    public native void tellDifferentIndividuals() throws Exception;


    //////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Asks

    public native boolean isKBConsistent() throws Exception;

    public native boolean isClassSatisfiable(ClassPointer c) throws Exception;

    public native boolean isClassSubsumedBy(ClassPointer c, ClassPointer d) throws Exception;

    public native boolean isClassEquivalentTo(ClassPointer c, ClassPointer d) throws Exception;

    public native boolean isClassDisjointWith(ClassPointer c, ClassPointer d) throws Exception;

    public native ClassPointer [][] askSubClasses(ClassPointer c, boolean direct) throws Exception;

    public native ClassPointer [][] askSuperClasses(ClassPointer c, boolean direct) throws Exception;

    public native ClassPointer [] askEquivalentClasses(ClassPointer c) throws Exception;



    public native ObjectPropertyPointer [][] askSuperObjectProperties(ObjectPropertyPointer r, boolean direct) throws Exception;

    public native ObjectPropertyPointer [][] askSubObjectProperties(ObjectPropertyPointer r, boolean direct) throws Exception;

    public native ObjectPropertyPointer [] askEquivalentObjectProperties(ObjectPropertyPointer r) throws Exception;

    public native ClassPointer askObjectPropertDomain(ObjectPropertyPointer r) throws Exception;

    public native ClassPointer askObjectPropertyRange(ObjectPropertyPointer r) throws Exception;

    public native boolean isObjectPropertyFunctional(ObjectPropertyPointer r) throws Exception;

    public native boolean isObjectPropertyInverseFunctional(ObjectPropertyPointer r) throws Exception;

    public native boolean isObjectPropertySymmetric(ObjectPropertyPointer r) throws Exception;

    public native boolean isObjectPropertyAntiSymmetric(ObjectPropertyPointer r) throws Exception;

    public native boolean isObjectPropertyTransitive(ObjectPropertyPointer r) throws Exception;

    public native boolean isObjectPropertyReflexive(ObjectPropertyPointer r) throws Exception;

    public native boolean isObjectPropertyIrreflexive(ObjectPropertyPointer r) throws Exception;


    public native DataPropertyPointer [][] askSuperDataProperties(DataPropertyPointer r, boolean direct) throws Exception;

    public native DataPropertyPointer [][] askSubDataProperties(DataPropertyPointer r, boolean direct) throws Exception;

    public native DataPropertyPointer [] askEquivalentDataProperties(DataPropertyPointer r) throws Exception;

    public native ClassPointer askDataPropertyDomain(DataPropertyPointer r) throws Exception;

    public native DataTypeExpressionPointer askDataPropertyRange(DataPropertyPointer r) throws Exception;

    public native boolean isDataPropertyFunctional(DataPropertyPointer r) throws Exception;


    public native ClassPointer [][] askIndividualTypes(IndividualPointer i, boolean direct) throws Exception;

    public native boolean isInstanceOf(IndividualPointer i, ClassPointer c) throws Exception;

    public native IndividualPointer [] askInstances(ClassPointer c) throws Exception;

    public native IndividualPointer [] askSameAs(IndividualPointer i) throws Exception;

    public native boolean isSameAs(IndividualPointer i, IndividualPointer j) throws Exception;


    /////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Options




    ///////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Utilitity stuff

    /**
     * Starts an arg list.  Note that only ONE arg list may be
     * created at any given time.  For example, it is illegal
     * to call initArgList for a second time without closing
     * calling closeArgList first.
     * @throws Exception
     */
    public native void initArgList() throws Exception;

    /**
     * Adds an argument to the currently open arg list.
     * @param p A pointer to the argument to be added.
     * @throws Exception
     */
    public native void addArg(Pointer p) throws Exception;

    /**
     * Closes the currently open arg list.  It is illegal to
     * close an empty arg list.  It is also illegal to
     * call this method without calling initArgList first.
     * @throws Exception
     */
    public native void closeArgList() throws Exception;

    public native void setProgressMonitor(FaCTPlusPlusProgressMonitor progressMonitor) throws Exception; 


}
