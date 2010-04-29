package uk.ac.manchester.cs.factplusplus.owlapi;

import org.semanticweb.owl.model.*;
import org.semanticweb.owl.vocab.OWLRDFVocabulary;
import org.semanticweb.owl.vocab.OWLRestrictedDataRangeFacetVocabulary;
import org.semanticweb.owl.vocab.XSDVocabulary;
import uk.ac.manchester.cs.factplusplus.*;

import java.util.*;
import java.util.logging.Logger;

/**
 * Author: Matthew Horridge<br>
 * The University Of Manchester<br>
 * Medical Informatics Group<br>
 * Date: 06-Sep-2006<br><br>
 * <p/>
 * matthew.horridge@cs.man.ac.uk<br>
 * www.cs.man.ac.uk/~horridgm<br><br>
 */
public class Translator implements OWLDescriptionVisitor, OWLEntityVisitor, OWLPropertyExpressionVisitor, OWLDataVisitor {

    private static final Logger logger = Logger.getLogger(Translator.class.getName());


    private FaCTPlusPlus faCTPlusPlus;

    private OWLOntologyManager owlOntologyManager;

    private ClassPointer lastClassPointer;

    private ObjectPropertyPointer lastObjectPropertyPointer;

    private DataPropertyPointer lastDataPropertyPointer;

    private IndividualPointer lastIndividualPointer;

    private DataTypePointer lastDataTypePointer;

    private DataTypeExpressionPointer lastDataTypeExpressionPointer;

    private DataValuePointer lastDataValuePointer;


    private Map<ClassPointer, OWLClass> classPointerMap;

    private Map<OWLClass, ClassPointer> owlClass2ClassPointerMap;

    private Map<ObjectPropertyPointer, OWLObjectProperty> objectPropertyPointerMap;

    private Map<OWLObjectProperty, ObjectPropertyPointer> owlObjectProperty2ObjectPropertyPointerMap;

    private Map<DataPropertyPointer, OWLDataProperty> dataPropertyPointerMap;

    private Map<OWLDataProperty, DataPropertyPointer> owlDataProperty2DataPropertyPointerMap;

    private Map<IndividualPointer, OWLIndividual> individualPointerMap;

    private Map<OWLIndividual, IndividualPointer> owlIndividual2IndividualPointerMap;

    private Map<DataValuePointer, OWLConstant> dataValuePointerMap;

    private Map<OWLConstant, DataValuePointer> owlConstant2DataValuePointerMap;

    private Map<DataTypeExpressionPointer, OWLDataRange> dataTypeExpressionPointerMap;

    private Map<OWLDataRange, DataTypeExpressionPointer> owlDataRange2DataTypeExpressionPointerMap;


    public Translator(OWLOntologyManager owlOntologyManager, FaCTPlusPlus faCTPlusPlus) {
        this.owlOntologyManager = owlOntologyManager;
        this.faCTPlusPlus = faCTPlusPlus;
        classPointerMap = new HashMap<ClassPointer, OWLClass>();
        owlClass2ClassPointerMap = new HashMap<OWLClass, ClassPointer>();
        objectPropertyPointerMap = new HashMap<ObjectPropertyPointer, OWLObjectProperty>();
        owlObjectProperty2ObjectPropertyPointerMap = new HashMap<OWLObjectProperty, ObjectPropertyPointer>();
        dataPropertyPointerMap = new HashMap<DataPropertyPointer, OWLDataProperty>();
        owlDataProperty2DataPropertyPointerMap = new HashMap<OWLDataProperty, DataPropertyPointer>();
        individualPointerMap = new HashMap<IndividualPointer, OWLIndividual>();
        owlIndividual2IndividualPointerMap = new HashMap<OWLIndividual, IndividualPointer>();
        dataValuePointerMap = new HashMap<DataValuePointer, OWLConstant>();
        owlConstant2DataValuePointerMap = new HashMap<OWLConstant, DataValuePointer>();
        dataTypeExpressionPointerMap = new HashMap<DataTypeExpressionPointer, OWLDataRange>();
        owlDataRange2DataTypeExpressionPointerMap = new HashMap<OWLDataRange, DataTypeExpressionPointer>();
    }


    /**
     * Resets the translator by clearing out the various mappings
     * of OWLAPI objects to FaCT++ pointer objects.
     */
    public void reset() throws OWLException {
        try {
            classPointerMap.clear();
            owlClass2ClassPointerMap.clear();
            objectPropertyPointerMap.clear();
            owlObjectProperty2ObjectPropertyPointerMap.clear();
            owlDataProperty2DataPropertyPointerMap.clear();
            dataPropertyPointerMap.clear();
            individualPointerMap.clear();
            owlIndividual2IndividualPointerMap.clear();
            dataValuePointerMap.clear();
            owlConstant2DataValuePointerMap.clear();
            dataTypeExpressionPointerMap.clear();
            owlDataRange2DataTypeExpressionPointerMap.clear();

            lastClassPointer = null;
            lastObjectPropertyPointer = null;
            lastIndividualPointer = null;
            lastDataPropertyPointer = null;
            lastDataTypePointer = null;
            lastDataTypeExpressionPointer = null;
            lastDataValuePointer = null;
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }

    public FaCTPlusPlus getFaCTPlusPlus(){
           return faCTPlusPlus;
    }


    public OWLOntologyManager getOWLOntologyManager(){
        return owlOntologyManager;
    }

    
    public void dispose() {
        try {
            reset();
        }
        catch (OWLException e) {
            logger.severe(e.getMessage());
        }
    }


    public boolean contains(OWLClass cls) {
        if (cls.equals(owlOntologyManager.getOWLDataFactory().getOWLThing())) {
            return true;
        }
        else if (cls.equals(owlOntologyManager.getOWLDataFactory().getOWLNothing())) {
            return true;
        }
        return owlClass2ClassPointerMap.containsKey(cls);
    }


    public boolean contains(OWLObjectProperty prop) {
        if (prop.equals(owlOntologyManager.getOWLDataFactory().getOWLThing())) {
            return true;
        }
        else if (prop.equals(owlOntologyManager.getOWLDataFactory().getOWLNothing())) {
            return true;
        }
        return owlObjectProperty2ObjectPropertyPointerMap.containsKey(prop);
    }


    public boolean contains(OWLDataProperty prop) {
        if (prop.equals(owlOntologyManager.getOWLDataFactory().getOWLThing())) {
            return true;
        }
        else if (prop.equals(owlOntologyManager.getOWLDataFactory().getOWLNothing())) {
            return true;
        }
        return owlDataProperty2DataPropertyPointerMap.containsKey(prop);
    }


    public boolean contains(OWLIndividual ind) {
        return owlIndividual2IndividualPointerMap.containsKey(ind);
    }


    /**
     * A convenience method that translates an <code>OWLDescription</code> to
     * the FaCT++ representation of the description.
     * @param description The description to be translated.
     * @return The corresponding FaCT++ representation of the description.
     */
    public ClassPointer translate(OWLDescription description) throws OWLException {
        description.accept(this);
        return getLastClassPointer();
    }


    public ObjectPropertyPointer translate(OWLObjectPropertyExpression property) throws OWLException {
        property.accept(this);
        return getLastObjectPropertyPointer();
    }


    public DataPropertyPointer translate(OWLDataPropertyExpression property) throws OWLException {
        property.accept(this);
        return getLastDataPropertyPointer();
    }


    public IndividualPointer translate(OWLIndividual individual) throws OWLException {
        individual.accept(this);
        return getLastIndividualPointer();
    }


    public DataTypePointer translate(OWLDataRange range) throws OWLException {
        range.accept(this);
        return getLastDataTypePointer();
    }


    public DataValuePointer translate(OWLConstant con) throws OWLException {
        con.accept(this);
        return getLastDataValuePointer();
    }


    /**
     * Gets a pointer that corresponds to the last class that was translated.
     * @return A class pointer that corresponds to the last class that was translated,
     *         or <code>null</code> if no class has been translated or the translator was reset.
     */
    public ClassPointer getLastClassPointer() {
        if(lastClassPointer == null) {
            throw new RuntimeException("Last class pointer is null!");
        }
        return lastClassPointer;
    }


    /**
     * Gets a pointer that corresponds to the last object property that was translated.
     * @return An object property pointer that corresponds to the last object property that was translated,
     *         or <code>null</code> if no object property has been translated or the translator was reset.
     */
    public ObjectPropertyPointer getLastObjectPropertyPointer() {
        if(lastObjectPropertyPointer == null) {
            throw new RuntimeException("Last object property pointer is null");
        }
        return lastObjectPropertyPointer;
    }


    /**
     * Gets a pointer that corresponds to the last data property that was translated.
     * @return An data property pointer that corresponds to the last data property that was translated,
     *         or <code>null</code> if no data property has been translated or the translator was reset.
     */
    public IndividualPointer getLastIndividualPointer() {
        if(lastIndividualPointer == null) {
            throw new RuntimeException("Last individual pointer is null!");
        }
        return lastIndividualPointer;
    }


    /**
     * Gets a pointer that corresponds to the last individual that was translated.
     * @return An individual pointer that corresponds to the last individual that was translated,
     *         or <code>null</code> if no class has been translated or the translater was reset.
     */
    public DataPropertyPointer getLastDataPropertyPointer() {
        if(lastDataPropertyPointer == null) {
            throw new RuntimeException("Last data property pointer is null!");
        }
        return lastDataPropertyPointer;
    }


    public DataTypePointer getLastDataTypePointer() {
        if(lastDataTypePointer == null) {
            throw new RuntimeException("Last data type pointer is null");
        }
        return lastDataTypePointer;
    }

    public DataTypeExpressionPointer getLastDataTypeExpressionPointer() {
        if(lastDataTypeExpressionPointer != null) {
            return lastDataTypeExpressionPointer;
        }
        else {
            return getLastDataTypePointer();
        }
    }

    public DataValuePointer getLastDataValuePointer() {
        if(lastDataValuePointer == null) {
            throw new RuntimeException("Last data value pointer is null");
        }
        return lastDataValuePointer;
    }


    public void visit(OWLObjectIntersectionOf owlAnd) {
        try {
            // We can't do "nested" arg lists, so translate all operands
            // and put the pointers into a set first, then create the intersection
            Set<ClassPointer> operandPointers = new HashSet<ClassPointer>();
            for (OWLDescription desc : owlAnd.getOperands()) {
                desc.accept(this);
                ClassPointer cp = getLastClassPointer();
                operandPointers.add(cp);
            }
            // Now create the actual class pointer for the OWLAnd.
            faCTPlusPlus.initArgList();
            for (ClassPointer operandPointer : operandPointers) {
                faCTPlusPlus.addArg(operandPointer);
            }
            faCTPlusPlus.closeArgList();
            lastClassPointer = faCTPlusPlus.getConceptAnd();
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLObjectComplementOf owlNot) {
        try {
            owlNot.getOperand().accept(this);
            ClassPointer cp = getLastClassPointer();
            lastClassPointer = faCTPlusPlus.getConceptNot(cp);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLDataAllRestriction owlDataAllRestriction) {
        try {
            DataPropertyPointer p = translate(owlDataAllRestriction.getProperty());
            owlDataAllRestriction.getFiller().accept(this);
            lastClassPointer = faCTPlusPlus.getDataAll(p, getLastDataTypeExpressionPointer());
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLDataSomeRestriction owlDataSomeRestriction) {
        try {
            DataPropertyPointer p = translate(owlDataSomeRestriction.getProperty());
            owlDataSomeRestriction.getFiller().accept(this);
            lastClassPointer = faCTPlusPlus.getDataSome(p, getLastDataTypeExpressionPointer());
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLDataValueRestriction owlDataValueRestriction) {
        try {
            DataPropertyPointer p = translate(owlDataValueRestriction.getProperty());
            OWLConstant val = owlDataValueRestriction.getValue();
            val.accept(this);
            lastClassPointer = faCTPlusPlus.getDataValue(p, getLastDataValuePointer());
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLObjectAllRestriction owlObjectAllRestriction) {
        try {
            owlObjectAllRestriction.getFiller().accept(this);
            ClassPointer classPointer = getLastClassPointer();
            owlObjectAllRestriction.getProperty().accept(this);
            ObjectPropertyPointer propertyPointer = getLastObjectPropertyPointer();
            lastClassPointer = faCTPlusPlus.getObjectAll(propertyPointer, classPointer);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLObjectMinCardinalityRestriction desc) {
        try {
            desc.getFiller().accept(this);
            ClassPointer classPointer = getLastClassPointer();
            desc.getProperty().accept(this);
            ObjectPropertyPointer propertyPointer = getLastObjectPropertyPointer();
            lastClassPointer = faCTPlusPlus.getObjectAtLeast(desc.getCardinality(), propertyPointer, classPointer);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLObjectExactCardinalityRestriction desc) {
        try {
            desc.getFiller().accept(this);
            ClassPointer classPointer = getLastClassPointer();
            desc.getProperty().accept(this);
            ObjectPropertyPointer propertyPointer = getLastObjectPropertyPointer();
            lastClassPointer = faCTPlusPlus.getObjectExact(desc.getCardinality(), propertyPointer, classPointer);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLObjectMaxCardinalityRestriction desc) {
        try {
            desc.getFiller().accept(this);
            ClassPointer classPointer = getLastClassPointer();
            desc.getProperty().accept(this);
            ObjectPropertyPointer propertyPointer = getLastObjectPropertyPointer();
            lastClassPointer = faCTPlusPlus.getObjectAtMost(desc.getCardinality(), propertyPointer, classPointer);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLObjectSelfRestriction desc) {
        try {
            desc.getProperty().accept(this);
            lastClassPointer = faCTPlusPlus.getSelf(getLastObjectPropertyPointer());
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLObjectOneOf desc) {
        try {
            faCTPlusPlus.initArgList();
            for (OWLIndividual ind : desc.getIndividuals()) {
                ind.accept(this);
                faCTPlusPlus.addArg(getLastIndividualPointer());
            }
            faCTPlusPlus.closeArgList();
            lastClassPointer = faCTPlusPlus.getOneOf();
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLDataMinCardinalityRestriction desc) {
        try {
            desc.getProperty().accept(this);
            DataPropertyPointer p = getLastDataPropertyPointer();
            desc.getFiller().accept(this);
            lastClassPointer = faCTPlusPlus.getDataAtLeast(desc.getCardinality(), p, getLastDataTypeExpressionPointer());
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLDataExactCardinalityRestriction desc) {
        try {
            desc.getProperty().accept(this);
            DataPropertyPointer p = getLastDataPropertyPointer();
            desc.getFiller().accept(this);
            lastClassPointer = faCTPlusPlus.getDataExact(desc.getCardinality(), p, getLastDataTypeExpressionPointer());
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLDataMaxCardinalityRestriction desc) {
        try {
            desc.getProperty().accept(this);
            DataPropertyPointer p = getLastDataPropertyPointer();
            desc.getFiller().accept(this);
            lastClassPointer = faCTPlusPlus.getDataAtMost(desc.getCardinality(), p, getLastDataTypeExpressionPointer());
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLObjectSomeRestriction owlObjectSomeRestriction) {
        try {
            owlObjectSomeRestriction.getFiller().accept(this);
            ClassPointer classPointer = getLastClassPointer();
            owlObjectSomeRestriction.getProperty().accept(this);
            ObjectPropertyPointer propertyPointer = getLastObjectPropertyPointer();
            lastClassPointer = faCTPlusPlus.getObjectSome(propertyPointer, classPointer);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLObjectValueRestriction owlObjectValueRestriction) {
        try {
            owlObjectValueRestriction.getValue().accept(this);
            IndividualPointer individualPointer = getLastIndividualPointer();
            owlObjectValueRestriction.getProperty().accept(this);
            ObjectPropertyPointer propertyPointer = getLastObjectPropertyPointer();
            lastClassPointer = faCTPlusPlus.getObjectValue(propertyPointer, individualPointer);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLObjectUnionOf owlOr) {
        try {
            // We can't do "nested" arg lists, so translate all operands
            // and put the pointers into a set first, then create the intersection
            Set<ClassPointer> operandPointers = new HashSet<ClassPointer>();
            for (OWLDescription desc : owlOr.getOperands()) {
                desc.accept(this);
                operandPointers.add(getLastClassPointer());
            }
            faCTPlusPlus.initArgList();
            for (ClassPointer operandPointer : operandPointers) {
                faCTPlusPlus.addArg(operandPointer);
            }
            faCTPlusPlus.closeArgList();
            lastClassPointer = faCTPlusPlus.getConceptOr();
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////
    //
    // Data stuff
    //
    ////////////////////////////////////////////////////////////////////////////////////////


    public void visit(OWLDataComplementOf node) {
        try {
            lastDataTypeExpressionPointer = owlDataRange2DataTypeExpressionPointerMap.get(node);
            if (lastDataTypeExpressionPointer == null){
                node.getDataRange().accept(this);
                lastDataTypeExpressionPointer = faCTPlusPlus.getDataNot(getLastDataTypeExpressionPointer());
                owlDataRange2DataTypeExpressionPointerMap.put(node, lastDataTypeExpressionPointer);
                dataTypeExpressionPointerMap.put(lastDataTypeExpressionPointer, node);
            }
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLDataOneOf node) {
        try {
            lastDataTypeExpressionPointer = owlDataRange2DataTypeExpressionPointerMap.get(node);
            if (lastDataTypeExpressionPointer == null){

                List<DataValuePointer> pointers = new ArrayList<DataValuePointer>();
                for (OWLConstant con : node.getValues()) {
                    con.accept(this);
                    pointers.add(getLastDataValuePointer());
                }
                faCTPlusPlus.initArgList();
                for (DataValuePointer pointer : pointers) {
                    faCTPlusPlus.addArg(pointer);
                }
                faCTPlusPlus.closeArgList();
                lastDataTypeExpressionPointer = faCTPlusPlus.getDataEnumeration();
                owlDataRange2DataTypeExpressionPointerMap.put(node, lastDataTypeExpressionPointer);
                dataTypeExpressionPointerMap.put(lastDataTypeExpressionPointer, node);
            }
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLDataRangeRestriction node) {
        try {
            lastDataTypeExpressionPointer = owlDataRange2DataTypeExpressionPointerMap.get(node);
            if (lastDataTypeExpressionPointer == null){
                lastDataTypeExpressionPointer = null;
                lastDataTypePointer = null;
                node.getDataRange().accept(this);
                DataTypeExpressionPointer dt = getLastDataTypeExpressionPointer();

                for (OWLDataRangeFacetRestriction restriction : node.getFacetRestrictions()) {

                    restriction.getFacetValue().accept(this);
                    DataValuePointer dv = getLastDataValuePointer();
                    DataTypeFacet facet = null;
                    if (restriction.getFacet().equals(OWLRestrictedDataRangeFacetVocabulary.MIN_INCLUSIVE)) {
                        facet = faCTPlusPlus.getMinInclusiveFacet(dv);
                    }
                    else if (restriction.getFacet().equals(OWLRestrictedDataRangeFacetVocabulary.MAX_INCLUSIVE)) {
                        facet = faCTPlusPlus.getMaxInclusiveFacet(dv);
                    }
                    else if (restriction.getFacet().equals(OWLRestrictedDataRangeFacetVocabulary.MIN_EXCLUSIVE)) {
                        facet = faCTPlusPlus.getMinExclusiveFacet(dv);
                    }
                    else if (restriction.getFacet().equals(OWLRestrictedDataRangeFacetVocabulary.MAX_EXCLUSIVE)) {
                        facet = faCTPlusPlus.getMaxExclusiveFacet(dv);
                    }
                    else if (restriction.getFacet().equals(OWLRestrictedDataRangeFacetVocabulary.LENGTH)) {
                        facet = faCTPlusPlus.getLength(dv);
                    }
                    else if (restriction.getFacet().equals(OWLRestrictedDataRangeFacetVocabulary.MIN_LENGTH)) {
                        facet = faCTPlusPlus.getMinLength(dv);
                    }
                    else if (restriction.getFacet().equals(OWLRestrictedDataRangeFacetVocabulary.MAX_LENGTH)) {
                        facet = faCTPlusPlus.getMaxLength(dv);
                    }
                    else if (restriction.getFacet().equals(OWLRestrictedDataRangeFacetVocabulary.FRACTION_DIGITS)) {
                        facet = faCTPlusPlus.getFractionDigitsFacet(dv);
                    }
                    else if (restriction.getFacet().equals(OWLRestrictedDataRangeFacetVocabulary.PATTERN)) {
                        facet = faCTPlusPlus.getPattern(dv);
                    }
                    else if (restriction.getFacet().equals(OWLRestrictedDataRangeFacetVocabulary.TOTAL_DIGITS)) {
                        facet = faCTPlusPlus.getTotalDigitsFacet(dv);
                    }
                    else {
                        throw new FaCTPlusPlusReasonerException("Unsupported data type facet: " + restriction.getFacet());
                    }
                    dt = lastDataTypeExpressionPointer = faCTPlusPlus.getRestrictedDataType(dt, facet);
                }
                owlDataRange2DataTypeExpressionPointerMap.put(node, lastDataTypeExpressionPointer);
                dataTypeExpressionPointerMap.put(lastDataTypeExpressionPointer, node);
            }
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLDataRangeFacetRestriction node) {

    }


    public void visit(OWLTypedConstant node) {
        try {
            lastDataValuePointer = owlConstant2DataValuePointerMap.get(node);
            if (lastDataValuePointer == null){
                node.getDataType().accept((OWLDataVisitor) this);
                lastDataValuePointer = faCTPlusPlus.getDataValue(node.getLiteral(), getLastDataTypePointer());
                owlConstant2DataValuePointerMap.put(node, lastDataValuePointer);
                dataValuePointerMap.put(lastDataValuePointer, node);
            }
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLUntypedConstant node) {
        try {
            lastDataValuePointer = owlConstant2DataValuePointerMap.get(node);
            if (lastDataValuePointer == null){
                owlOntologyManager.getOWLDataFactory().getOWLDataType(XSDVocabulary.STRING.getURI()).accept((OWLDataVisitor) this);
                lastDataValuePointer = faCTPlusPlus.getDataValue(node.getLiteral(), getLastDataTypePointer());
                owlConstant2DataValuePointerMap.put(node, lastDataValuePointer);
                dataValuePointerMap.put(lastDataValuePointer, node);
            }
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////
    //
    // Entities
    //
    ////////////////////////////////////////////////////////////////////////////////////////


    public void visit(OWLDataProperty owlDataProperty) {
        try {
            lastDataPropertyPointer = owlDataProperty2DataPropertyPointerMap.get(owlDataProperty);
            if (lastDataPropertyPointer == null) {
                lastDataPropertyPointer = faCTPlusPlus.getDataProperty(owlDataProperty.getURI().toString());
                dataPropertyPointerMap.put(lastDataPropertyPointer, owlDataProperty);
                owlDataProperty2DataPropertyPointerMap.put(owlDataProperty, lastDataPropertyPointer);
            }
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLObjectProperty owlObjectProperty) {
        try {
            lastObjectPropertyPointer = owlObjectProperty2ObjectPropertyPointerMap.get(owlObjectProperty);
            if (lastObjectPropertyPointer == null) {
                lastObjectPropertyPointer = faCTPlusPlus.getObjectProperty(owlObjectProperty.getURI().toString());
                objectPropertyPointerMap.put(lastObjectPropertyPointer, owlObjectProperty);
                owlObjectProperty2ObjectPropertyPointerMap.put(owlObjectProperty, lastObjectPropertyPointer);
            }
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLClass owlClass) {
        try {
            lastClassPointer = owlClass2ClassPointerMap.get(owlClass);
            // Cache if not in map
            if (lastClassPointer == null) {
                if (owlClass.getURI().equals(OWLRDFVocabulary.OWL_THING.getURI())) {
                    lastClassPointer = faCTPlusPlus.getThing();
                }
                else if (owlClass.getURI().equals(OWLRDFVocabulary.OWL_NOTHING.getURI())) {
                    lastClassPointer = faCTPlusPlus.getNothing();
                }
                else {
                    lastClassPointer = faCTPlusPlus.getNamedClass(owlClass.getURI().toString());
                }
                classPointerMap.put(lastClassPointer, owlClass);
                owlClass2ClassPointerMap.put(owlClass, lastClassPointer);
            }
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLObjectPropertyInverse property) {
        try {
            property.getInverse().accept(this);
            lastObjectPropertyPointer = faCTPlusPlus.getInverseProperty(getLastObjectPropertyPointer());
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLIndividual individual) {
        try {
            lastIndividualPointer = owlIndividual2IndividualPointerMap.get(individual);
            if(lastIndividualPointer == null) {
                lastIndividualPointer = faCTPlusPlus.getIndividual(individual.getURI().toString());
                individualPointerMap.put(lastIndividualPointer, individual);
                owlIndividual2IndividualPointerMap.put(individual, lastIndividualPointer);
            }
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLDataType dataType) {
        try {
            lastDataTypeExpressionPointer = null;
            lastDataTypePointer = (DataTypePointer)owlDataRange2DataTypeExpressionPointerMap.get(dataType);
            if (lastDataTypePointer == null){
                if(owlOntologyManager.getOWLDataFactory().getTopDataType().equals(dataType)) {
                    lastDataTypePointer = faCTPlusPlus.getDataTop();
                }
                else {
                    lastDataTypePointer = faCTPlusPlus.getBuiltInDataType(dataType.getURI().toString());
                }
                owlDataRange2DataTypeExpressionPointerMap.put(dataType, lastDataTypePointer);
                dataTypeExpressionPointerMap.put(lastDataTypePointer, dataType);
            }
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////
    //
    // Pointer to entity
    //
    ////////////////////////////////////////////////////////////////////////////////////////


    public OWLObject getOWLObject(Pointer pointer) {
        if (pointer instanceof ClassPointer){
            return getOWLClass((ClassPointer)pointer);
        }
        if (pointer instanceof ObjectPropertyPointer){
            return getOWLObjectProperty((ObjectPropertyPointer)pointer);
        }
        if (pointer instanceof DataPropertyPointer){
            return getOWLDataProperty((DataPropertyPointer)pointer);
        }
        if (pointer instanceof IndividualPointer){
            return getOWLIndividual((IndividualPointer)pointer);
        }
        if (pointer instanceof DataValuePointer){
            return getOWLConstant((DataValuePointer)pointer);
        }
        if (pointer instanceof DataTypeExpressionPointer){
            return getOWLDataRange((DataTypeExpressionPointer)pointer);
        }
        return null;
    }


    public OWLClass getOWLClass(ClassPointer classPointer) {
        return classPointerMap.get(classPointer);
    }


    public OWLObjectProperty getOWLObjectProperty(ObjectPropertyPointer objectPropertyPointer) {
        return objectPropertyPointerMap.get(objectPropertyPointer);
    }

    public OWLDataProperty getOWLDataProperty(DataPropertyPointer dataPropertyPointer) {
        return dataPropertyPointerMap.get(dataPropertyPointer);
    }


    public OWLIndividual getOWLIndividual(IndividualPointer individualPointer) {
        return individualPointerMap.get(individualPointer);
    }


    public OWLConstant getOWLConstant(DataValuePointer constantPointer) {
        return dataValuePointerMap.get(constantPointer);
    }


    public OWLDataRange getOWLDataRange(DataTypeExpressionPointer pointer) {
        return dataTypeExpressionPointerMap.get(pointer);
    }
}
