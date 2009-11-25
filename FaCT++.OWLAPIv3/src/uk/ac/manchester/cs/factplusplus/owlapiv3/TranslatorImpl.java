package uk.ac.manchester.cs.factplusplus.owlapiv3;

import org.semanticweb.owlapi.model.*;
import org.semanticweb.owlapi.vocab.OWLRDFVocabulary;
import org.semanticweb.owlapi.vocab.XSDVocabulary;
import org.semanticweb.owlapi.vocab.OWLFacet;
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
public class TranslatorImpl implements Translator, OWLClassExpressionVisitor, OWLEntityVisitor, OWLPropertyExpressionVisitor, OWLDataVisitor, OWLIndividualVisitor {

    private static final Logger logger = Logger.getLogger(TranslatorImpl.class.getName());


    private FaCTPlusPlus faCTPlusPlus;

    private OWLOntologyManager owlOntologyManager;

    private ClassPointer lastClassPointer;

    private ObjectPropertyPointer lastObjectPropertyPointer;

    private DataPropertyPointer lastDataPropertyPointer;

    private IndividualPointer lastIndividualPointer;

    private DataTypeExpressionPointer lastDataTypeExpressionPointer;

    private DataValuePointer lastDataValuePointer;

    private DataTypeFacet lastFacetPointer;


    private Map<ClassPointer, OWLClass> classPointerMap;
    private Map<OWLClass, ClassPointer> owlClass2ClassPointerMap;

    private Map<ObjectPropertyPointer, OWLObjectProperty> objectPropertyPointerMap;
    private Map<OWLObjectProperty, ObjectPropertyPointer> owlObjectProperty2ObjectPropertyPointerMap;

    private Map<DataPropertyPointer, OWLDataProperty> dataPropertyPointerMap;
    private Map<OWLDataProperty, DataPropertyPointer> owlDataProperty2DataPropertyPointerMap;

    private Map<IndividualPointer, OWLIndividual> individualPointerMap;
    private Map<OWLIndividual, IndividualPointer> owlIndividual2IndividualPointerMap;

    private Map<DataValuePointer, OWLLiteral> dataValuePointerMap;
    private Map<OWLLiteral, DataValuePointer> owlConstant2DataValuePointerMap;

    private Map<DataTypePointer, OWLDatatype> dataTypePointerMap;
    private Map<OWLDatatype, DataTypePointer> owlDataType2DataTypePointerMap;


    public TranslatorImpl (OWLOntologyManager owlOntologyManager, FaCTPlusPlus faCTPlusPlus) {
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

        dataValuePointerMap = new HashMap<DataValuePointer, OWLLiteral>();
        owlConstant2DataValuePointerMap = new HashMap<OWLLiteral, DataValuePointer>();

        dataTypePointerMap = new HashMap<DataTypePointer, OWLDatatype>();
        owlDataType2DataTypePointerMap = new HashMap<OWLDatatype, DataTypePointer>();
    }


    /**
     * Resets the translator by clearing out the various mappings
     * of OWLAPI objects to FaCT++ pointer objects.
     */
    public void reset() {
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
        dataTypePointerMap.clear();
        owlDataType2DataTypePointerMap.clear();

        lastClassPointer = null;
        lastObjectPropertyPointer = null;
        lastIndividualPointer = null;
        lastDataPropertyPointer = null;
        lastDataTypeExpressionPointer = null;
        lastDataValuePointer = null;
        lastFacetPointer = null;
    }

    public FaCTPlusPlus getFaCTPlusPlus(){
        return faCTPlusPlus;
    }


    public OWLOntologyManager getOWLOntologyManager(){
        return owlOntologyManager;
    }


    public void dispose() {
        reset();
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
     * A convenience method that translates an <code>OWLClassExpression</code> to
     * the FaCT++ representation of the description.
     * @param description The description to be translated.
     * @return The corresponding FaCT++ representation of the description.
     */
    public ClassPointer translate(OWLClassExpression description) throws OWLException {
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


    public DataTypeExpressionPointer translate(OWLDataRange range) throws OWLException {
        range.accept(this);
        return getLastDataTypeExpressionPointer();
    }


    public DataValuePointer translate(OWLLiteral con) throws OWLException {
        con.accept(this);
        return getLastDataValuePointer();
    }


    public void translateClassExpressionArgList(Set<OWLClassExpression> descriptions) {
        try {
            Set<ClassPointer> classPointers = new HashSet<ClassPointer>();
            for (OWLClassExpression desc : descriptions) {
                classPointers.add(translate(desc));
            }
            faCTPlusPlus.initArgList();
            for (ClassPointer cp : classPointers) {
                faCTPlusPlus.addArg(cp);
            }
            faCTPlusPlus.closeArgList();
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void translateIndividualArgList(Set<? extends OWLIndividual> individuals) {
        try {
            faCTPlusPlus.initArgList();
            for (OWLIndividual ind : individuals) {
                faCTPlusPlus.addArg(translate(ind));
            }
            faCTPlusPlus.closeArgList();
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void translateObjectPropertyArgList(Set<? extends OWLObjectPropertyExpression> properties) throws
            OWLException {
        try {
            faCTPlusPlus.initArgList();
            for (OWLObjectPropertyExpression prop : properties) {
                faCTPlusPlus.addArg(translate(prop));
            }
            faCTPlusPlus.closeArgList();
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void translateDataPropertyArgList(Set<? extends OWLDataPropertyExpression> properties) throws
            OWLException {
        try {
            faCTPlusPlus.initArgList();
            for (OWLDataPropertyExpression prop : properties) {
                faCTPlusPlus.addArg(translate(prop));
            }
            faCTPlusPlus.closeArgList();
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    ///////////////////// VISITOR


    public void visit(OWLObjectIntersectionOf owlAnd) {
        try {
            // We can't do "nested" arg lists, so translate all operands
            // and put the pointers into a set first, then create the intersection
            Set<ClassPointer> operandPointers = new HashSet<ClassPointer>();
            for (OWLClassExpression desc : owlAnd.getOperands()) {
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


    public void visit(OWLDataAllValuesFrom owlDataAllRestriction) {
        try {
            DataPropertyPointer p = translate(owlDataAllRestriction.getProperty());
            owlDataAllRestriction.getFiller().accept(this);
            lastClassPointer = faCTPlusPlus.getDataAll(p, getLastDataTypeExpressionPointer());
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLDataSomeValuesFrom owlDataSomeRestriction) {
        try {
            DataPropertyPointer p = translate(owlDataSomeRestriction.getProperty());
            owlDataSomeRestriction.getFiller().accept(this);
            lastClassPointer = faCTPlusPlus.getDataSome(p, getLastDataTypeExpressionPointer());
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLDataHasValue owlDataValueRestriction) {
        try {
            DataPropertyPointer p = translate(owlDataValueRestriction.getProperty());
            OWLLiteral val = owlDataValueRestriction.getValue();
            val.accept(this);
            lastClassPointer = faCTPlusPlus.getDataValue(p, getLastDataValuePointer());
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLObjectAllValuesFrom owlObjectAllRestriction) {
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


    public void visit(OWLObjectMinCardinality desc) {
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


    public void visit(OWLObjectExactCardinality desc) {
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


    public void visit(OWLObjectMaxCardinality desc) {
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


    public void visit(OWLObjectHasSelf desc) {
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


    public void visit(OWLDataMinCardinality desc) {
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


    public void visit(OWLDataExactCardinality desc) {
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


    public void visit(OWLDataMaxCardinality desc) {
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


    public void visit(OWLObjectSomeValuesFrom owlObjectSomeRestriction) {
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


    public void visit(OWLObjectHasValue owlObjectValueRestriction) {
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
            for (OWLClassExpression desc : owlOr.getOperands()) {
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
            lastDataTypeExpressionPointer = null;
            node.getDataRange().accept(this);
            lastDataTypeExpressionPointer = faCTPlusPlus.getNot(getLastDataTypeExpressionPointer());
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLDataIntersectionOf node) {
        try {
            // We can't do "nested" arg lists, so translate all operands
            // and put the pointers into a set first, then create the intersection
            Set<DataTypeExpressionPointer> operandPointers = new HashSet<DataTypeExpressionPointer>();
            for (OWLDataRange dr : node.getOperands()){
                dr.accept(this);
                operandPointers.add(getLastDataTypeExpressionPointer());
            }
            faCTPlusPlus.initArgList();
            for (DataTypeExpressionPointer ptr : operandPointers){
                faCTPlusPlus.addArg(ptr);
            }
            faCTPlusPlus.closeArgList();
            lastDataTypeExpressionPointer = faCTPlusPlus.getDataIntersectionOf();
        }
        catch (FaCTPlusPlusException e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLDataUnionOf node) {
        try {
            // We can't do "nested" arg lists, so translate all operands
            // and put the pointers into a set first, then create the intersection
            Set<DataTypeExpressionPointer> operandPointers = new HashSet<DataTypeExpressionPointer>();
            for (OWLDataRange dr : node.getOperands()){
                dr.accept(this);
                operandPointers.add(getLastDataTypeExpressionPointer());
            }
            faCTPlusPlus.initArgList();
            for (DataTypeExpressionPointer ptr : operandPointers){
                faCTPlusPlus.addArg(ptr);
            }
            faCTPlusPlus.closeArgList();
            lastDataTypeExpressionPointer = faCTPlusPlus.getDataUnionOf();
        }
        catch (FaCTPlusPlusException e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLDataOneOf node) {
        try {
            List<DataValuePointer> pointers = new ArrayList<DataValuePointer>();
            for (OWLLiteral con : node.getValues()) {
                con.accept(this);
                pointers.add(getLastDataValuePointer());
            }
            faCTPlusPlus.initArgList();
            for (DataValuePointer pointer : pointers) {
                faCTPlusPlus.addArg(pointer);
            }
            faCTPlusPlus.closeArgList();
            lastDataTypeExpressionPointer = faCTPlusPlus.getDataEnumeration();
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLDatatypeRestriction node) {
        try {
            node.getDatatype().accept((OWLDataVisitor)this);

            for (OWLFacetRestriction restriction : node.getFacetRestrictions()) {

                restriction.accept(this);

                // accumulate the facets
                lastDataTypeExpressionPointer = faCTPlusPlus.getRestrictedDataType(getLastDataTypeExpressionPointer(),
                                                                                   getLastFacetPointer());
            }
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLTypedLiteral node) {
        try {
            lastDataValuePointer = owlConstant2DataValuePointerMap.get(node);
            if (lastDataValuePointer == null){
                node.getDatatype().accept((OWLDataVisitor)this);
                lastDataValuePointer = faCTPlusPlus.getDataValue(node.getLiteral(), (DataTypePointer)getLastDataTypeExpressionPointer());
                owlConstant2DataValuePointerMap.put(node, lastDataValuePointer);
                dataValuePointerMap.put(lastDataValuePointer, node);
            }
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLStringLiteral node) {
        try {
            lastDataValuePointer = owlConstant2DataValuePointerMap.get(node);
            if (lastDataValuePointer == null){
                owlOntologyManager.getOWLDataFactory().getOWLDatatype(XSDVocabulary.STRING.getURI()).accept((OWLDataVisitor) this);
                lastDataValuePointer = faCTPlusPlus.getDataValue(node.getLiteral(), (DataTypePointer)getLastDataTypeExpressionPointer());
                owlConstant2DataValuePointerMap.put(node, lastDataValuePointer);
                dataValuePointerMap.put(lastDataValuePointer, node);
            }
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLFacetRestriction node) {
        try {
            node.getFacetValue().accept(this);
            DataValuePointer dv = getLastDataValuePointer();
            if (node.getFacet().equals(OWLFacet.MIN_INCLUSIVE)) {
                lastFacetPointer = faCTPlusPlus.getMinInclusiveFacet(dv);
            }
            else if (node.getFacet().equals(OWLFacet.MAX_INCLUSIVE)) {
                lastFacetPointer = faCTPlusPlus.getMaxInclusiveFacet(dv);
            }
            else if (node.getFacet().equals(OWLFacet.MIN_EXCLUSIVE)) {
                lastFacetPointer = faCTPlusPlus.getMinExclusiveFacet(dv);
            }
            else if (node.getFacet().equals(OWLFacet.MAX_EXCLUSIVE)) {
                lastFacetPointer = faCTPlusPlus.getMaxExclusiveFacet(dv);
            }
            else if (node.getFacet().equals(OWLFacet.LENGTH)) {
                lastFacetPointer = faCTPlusPlus.getLength(dv);
            }
            else if (node.getFacet().equals(OWLFacet.MIN_LENGTH)) {
                lastFacetPointer = faCTPlusPlus.getMinLength(dv);
            }
            else if (node.getFacet().equals(OWLFacet.MAX_LENGTH)) {
                lastFacetPointer = faCTPlusPlus.getMaxLength(dv);
            }
            else if (node.getFacet().equals(OWLFacet.FRACTION_DIGITS)) {
                lastFacetPointer = faCTPlusPlus.getFractionDigitsFacet(dv);
            }
            else if (node.getFacet().equals(OWLFacet.PATTERN)) {
                lastFacetPointer = faCTPlusPlus.getPattern(dv);
            }
            else if (node.getFacet().equals(OWLFacet.TOTAL_DIGITS)) {
                lastFacetPointer = faCTPlusPlus.getTotalDigitsFacet(dv);
            }
            else {
                throw new FaCTPlusPlusRuntimeException("Unsupported data type facet: " + node.getFacet());
            }
        }
        catch (FaCTPlusPlusException e) {
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
                lastDataPropertyPointer = faCTPlusPlus.getDataProperty(owlDataProperty.getIRI().toString());
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
                lastObjectPropertyPointer = faCTPlusPlus.getObjectProperty(owlObjectProperty.getIRI().toString());
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
                if (owlClass.getIRI().equals(OWLRDFVocabulary.OWL_THING.getIRI())) {
                    lastClassPointer = faCTPlusPlus.getThing();
                }
                else if (owlClass.getIRI().equals(OWLRDFVocabulary.OWL_NOTHING.getIRI())) {
                    lastClassPointer = faCTPlusPlus.getNothing();
                }
                else {
                    lastClassPointer = faCTPlusPlus.getNamedClass(owlClass.getIRI().toString());
                }
                classPointerMap.put(lastClassPointer, owlClass);
                owlClass2ClassPointerMap.put(owlClass, lastClassPointer);
            }
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLNamedIndividual individual) {
        try {
            lastIndividualPointer = owlIndividual2IndividualPointerMap.get(individual);
            if(lastIndividualPointer == null) {
                lastIndividualPointer = faCTPlusPlus.getIndividual(individual.getIRI().toString());
                individualPointerMap.put(lastIndividualPointer, individual);
                owlIndividual2IndividualPointerMap.put(individual, lastIndividualPointer);
            }
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLDatatype dataType) {
        try {
            lastDataTypeExpressionPointer = owlDataType2DataTypePointerMap.get(dataType);
            if (lastDataTypeExpressionPointer == null){
                if(owlOntologyManager.getOWLDataFactory().getTopDatatype().equals(dataType)) {
                    lastDataTypeExpressionPointer = faCTPlusPlus.getDataTop();
                }
                else if (XSDVocabulary.ALL_DATATYPES.contains(dataType.getIRI()) ||
                         OWLRDFVocabulary.RDF_XML_LITERAL.getIRI().equals(dataType.getIRI())){
                    lastDataTypeExpressionPointer = faCTPlusPlus.getBuiltInDataType(dataType.getIRI().toString());
                }
                else{
                    // unknown custom datatype
                    throw new FaCTPlusPlusRuntimeException("Unknown datatype: " + dataType);
                }
                owlDataType2DataTypePointerMap.put(dataType, (DataTypePointer)lastDataTypeExpressionPointer);
                dataTypePointerMap.put((DataTypePointer)lastDataTypeExpressionPointer, dataType);
            }
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLAnnotationProperty owlAnnotationProperty) {
        // do nothing
    }


    public void visit(OWLObjectInverseOf property) {
        try {
            property.getInverse().accept(this);
            lastObjectPropertyPointer = faCTPlusPlus.getInverseProperty(getLastObjectPropertyPointer());
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLAnonymousIndividual individual) {
        try {
            lastIndividualPointer = owlIndividual2IndividualPointerMap.get(individual);
            if(lastIndividualPointer == null) {
                // @@TODO need to avoid name clashes wrt to the referencing ontology
                lastIndividualPointer = faCTPlusPlus.getIndividual(individual.getID().toString());
                individualPointerMap.put(lastIndividualPointer, individual);
                owlIndividual2IndividualPointerMap.put(individual, lastIndividualPointer);
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
            return getOWLLiteral((DataValuePointer)pointer);
        }
        if (pointer instanceof DataTypePointer){
            return getOWLDataType((DataTypePointer)pointer);
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


    public OWLLiteral getOWLLiteral(DataValuePointer constantPointer) {
        return dataValuePointerMap.get(constantPointer);
    }


    public OWLDatatype getOWLDataType(DataTypePointer pointer) {
        return dataTypePointerMap.get(pointer);
    }


    //////////////////////// convenience methods

    /**
     * Gets a pointer that corresponds to the last class that was translated.
     * @return A class pointer that corresponds to the last class that was translated,
     *         or <code>null</code> if no class has been translated or the translator was reset.
     */
    private ClassPointer getLastClassPointer() {
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
    private ObjectPropertyPointer getLastObjectPropertyPointer() {
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
    private IndividualPointer getLastIndividualPointer() {
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
    private DataPropertyPointer getLastDataPropertyPointer() {
        if(lastDataPropertyPointer == null) {
            throw new RuntimeException("Last data property pointer is null!");
        }
        return lastDataPropertyPointer;
    }


    private DataTypeExpressionPointer getLastDataTypeExpressionPointer() {
        if(lastDataTypeExpressionPointer == null) {
            throw new RuntimeException("Last data type expression pointer is null");
        }
        return lastDataTypeExpressionPointer;
    }


    private DataValuePointer getLastDataValuePointer() {
        if(lastDataValuePointer == null) {
            throw new RuntimeException("Last data value pointer is null");
        }
        return lastDataValuePointer;
    }


    private DataTypeFacet getLastFacetPointer() {
        if(lastFacetPointer == null) {
            throw new RuntimeException("Last data facet pointer is null");
        }
        return lastFacetPointer;
    }
}
