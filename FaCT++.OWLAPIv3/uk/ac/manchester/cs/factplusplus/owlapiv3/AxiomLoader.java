package uk.ac.manchester.cs.factplusplus.owlapiv3;

import org.semanticweb.owlapi.model.*;
import uk.ac.manchester.cs.factplusplus.*;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
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
public class AxiomLoader implements OWLAxiomVisitor {

    private static final Logger logger = Logger.getLogger(AxiomLoader.class.getName());

    private FaCTPlusPlus faCTPlusPlus;

    private Translator translator;

    private AxiomPointer lastAxiom;

    private OWLDataFactory df;


    public AxiomLoader(Translator translator) {
        this.translator = translator;
        this.faCTPlusPlus = translator.getFaCTPlusPlus();
        this.df = translator.getOWLOntologyManager().getOWLDataFactory();
    }


    public AxiomPointer load(OWLAxiom ax) {
        lastAxiom = null;
        ax.accept(this);
        return lastAxiom;
    }


    ////////////////////////////////////////////////////////////////////////////////////////
    //
    // Class axioms
    //
    ////////////////////////////////////////////////////////////////////////////////////////


    public void visit(OWLDisjointClassesAxiom axiom) {
        try {
            translator.translateClassExpressionArgList(axiom.getClassExpressions());
            lastAxiom = faCTPlusPlus.tellDisjointClasses();
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLEquivalentClassesAxiom axiom) {
        try {
            translator.translateClassExpressionArgList(axiom.getClassExpressions());
            lastAxiom = faCTPlusPlus.tellEquivalentClass();
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLSubClassOfAxiom axiom) {
        try {
            lastAxiom = faCTPlusPlus.tellSubClassOf(translator.translate(axiom.getSubClass()),
                                                    translator.translate(axiom.getSuperClass()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////
    //
    // Property axioms
    //
    ////////////////////////////////////////////////////////////////////////////////////////


    public void visit(OWLDataPropertyRangeAxiom axiom) {
        try {
            lastAxiom = faCTPlusPlus.tellDataPropertyRange(translator.translate(axiom.getProperty()),
                                                           translator.translate(axiom.getRange()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLEquivalentObjectPropertiesAxiom axiom) {
        try {
            translator.translateObjectPropertyArgList(axiom.getProperties());
            lastAxiom = faCTPlusPlus.tellEquivalentObjectProperties();
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLEquivalentDataPropertiesAxiom axiom) {
        try {
            translator.translateDataPropertyArgList(axiom.getProperties());
            lastAxiom = faCTPlusPlus.tellEquivalentDataProperties();
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLFunctionalObjectPropertyAxiom axiom) {
        try {
            lastAxiom = faCTPlusPlus.tellFunctionalObjectProperty(translator.translate(axiom.getProperty()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLInverseFunctionalObjectPropertyAxiom axiom) {
        try {
            lastAxiom = faCTPlusPlus.tellInverseFunctionalObjectProperty(translator.translate(axiom.getProperty()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }



    public void visit(OWLObjectPropertyRangeAxiom axiom) {
        try {
            lastAxiom = faCTPlusPlus.tellObjectPropertyRange(translator.translate(axiom.getProperty()),
                                                             translator.translate(axiom.getRange()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLObjectPropertyDomainAxiom axiom) {
        try {
            lastAxiom = faCTPlusPlus.tellObjectPropertyDomain(translator.translate(axiom.getProperty()),
                                                              translator.translate(axiom.getDomain()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLSubObjectPropertyOfAxiom axiom) {
        try {
            lastAxiom = faCTPlusPlus.tellSubObjectProperties(translator.translate(axiom.getSubProperty()),
                                                             translator.translate(axiom.getSuperProperty()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLSymmetricObjectPropertyAxiom axiom) {
        try {
            lastAxiom = faCTPlusPlus.tellSymmetricObjectProperty(translator.translate(axiom.getProperty()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLTransitiveObjectPropertyAxiom axiom) {
        try {
            lastAxiom = faCTPlusPlus.tellTransitiveObjectProperty(translator.translate(axiom.getProperty()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLInverseObjectPropertiesAxiom axiom) {
        try {
            lastAxiom = faCTPlusPlus.tellInverseProperties(translator.translate(axiom.getFirstProperty()),
                                                           translator.translate(axiom.getSecondProperty()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLHasKeyAxiom axiom) {
        try {
            List<DataPropertyPointer> dataKeyPropertiesPointers = new ArrayList<DataPropertyPointer>();
            for (OWLDataPropertyExpression prop : axiom.getDataPropertyExpressions()) {
                dataKeyPropertiesPointers.add(translator.translate(prop));
            }
            faCTPlusPlus.initArgList();
            for (DataPropertyPointer pointer : dataKeyPropertiesPointers) {
                faCTPlusPlus.addArg(pointer);
            }
            faCTPlusPlus.closeArgList();

            DataPropertyPointer dataKey = faCTPlusPlus.getDataPropertyKey();

            List<ObjectPropertyPointer> objectPropertyPointers = new ArrayList<ObjectPropertyPointer>();
            for (OWLObjectPropertyExpression prop : axiom.getObjectPropertyExpressions()) {
                objectPropertyPointers.add(translator.translate(prop));
            }
            faCTPlusPlus.initArgList();
            for (ObjectPropertyPointer pointer : objectPropertyPointers) {
                faCTPlusPlus.addArg(pointer);
            }
            faCTPlusPlus.closeArgList();

            ObjectPropertyPointer objectKey = faCTPlusPlus.getObjectPropertyKey();

            lastAxiom = faCTPlusPlus.tellHasKey(translator.translate(axiom.getClassExpression()),
                                                dataKey,
                                                objectKey);
        }
        catch (Exception e){
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLDatatypeDefinitionAxiom axiom) {
        throw new FaCTPlusPlusRuntimeException("Datatype definitions not currently supported");
//        try {
//            lastAxiom = faCTPlusPlus.tellDatatypeDefinition(translator.translate(axiom.getDatatype()),
//                                                            translator.translate(axiom.getDataRange()));
//        }
//        catch (Exception e){
//            throw new FaCTPlusPlusRuntimeException(e);
//        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////
    //
    // Individual axioms
    //
    ////////////////////////////////////////////////////////////////////////////////////////


    public void visit(OWLDifferentIndividualsAxiom axiom) {
        try {
            translator.translateIndividualArgList(axiom.getIndividuals());
            lastAxiom = faCTPlusPlus.tellDifferentIndividuals();
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLSameIndividualAxiom axiom) {
        try {
            translator.translateIndividualArgList(axiom.getIndividuals());
            lastAxiom = faCTPlusPlus.tellSameIndividuals();
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }




    //////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // OWL 1.1 Stuff
    //
    //////////////////////////////////////////////////////////////////////////////////////////////////


    public void visit(OWLDisjointUnionAxiom axiom) {
        try {
            // Need to expand this
            Set<ClassPointer> operandPointers = new HashSet<ClassPointer>();
            for (OWLClassExpression desc : axiom.getClassExpressions()) {
                operandPointers.add(translator.translate(desc));
            }
            // Disjoint
            faCTPlusPlus.initArgList();
            for (ClassPointer operandPointer : operandPointers) {
                faCTPlusPlus.addArg(operandPointer);
            }
            faCTPlusPlus.closeArgList();
            lastAxiom = faCTPlusPlus.tellDisjointClasses();
            // Union
            operandPointers.clear();
            for (OWLClassExpression desc : axiom.getClassExpressions()) {
                operandPointers.add(translator.translate(desc));
            }
            faCTPlusPlus.initArgList();
            for (ClassPointer operandPointer : operandPointers) {
                faCTPlusPlus.addArg(operandPointer);
            }
            faCTPlusPlus.closeArgList();
            ClassPointer unionPointer = faCTPlusPlus.getConceptOr();
            ClassPointer clsPointer = translator.translate(axiom.getOWLClass());
            faCTPlusPlus.initArgList();
            faCTPlusPlus.addArg(clsPointer);
            faCTPlusPlus.addArg(unionPointer);
            faCTPlusPlus.closeArgList();
            lastAxiom = faCTPlusPlus.tellEquivalentClass();
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLDisjointObjectPropertiesAxiom axiom) {
        try {
            Set<ObjectPropertyPointer> pointers = new HashSet<ObjectPropertyPointer>();
            for (OWLObjectPropertyExpression prop : axiom.getProperties()) {
                pointers.add(translator.translate(prop));
            }
            faCTPlusPlus.initArgList();
            for (ObjectPropertyPointer pointer : pointers) {
                faCTPlusPlus.addArg(pointer);
            }
            faCTPlusPlus.closeArgList();
            lastAxiom = faCTPlusPlus.tellDisjointObjectProperties();
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLDisjointDataPropertiesAxiom axiom) {
        try {
            Set<DataPropertyPointer> pointers = new HashSet<DataPropertyPointer>();
            for (OWLDataPropertyExpression prop : axiom.getProperties()) {
                pointers.add(translator.translate(prop));
            }
            faCTPlusPlus.initArgList();
            for (DataPropertyPointer pointer : pointers) {
                faCTPlusPlus.addArg(pointer);
            }
            faCTPlusPlus.closeArgList();
            lastAxiom = faCTPlusPlus.tellDisjointDataProperties();
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLSubPropertyChainOfAxiom axiom) {
        try {
            List<ObjectPropertyPointer> propertyChainPointers = new ArrayList<ObjectPropertyPointer>();
            for (OWLObjectPropertyExpression prop : axiom.getPropertyChain()) {
                propertyChainPointers.add(translator.translate(prop));
            }
            faCTPlusPlus.initArgList();
            for (ObjectPropertyPointer pointer : propertyChainPointers) {
                faCTPlusPlus.addArg(pointer);
            }
            faCTPlusPlus.closeArgList();
            ObjectPropertyPointer propertyChainPointer = faCTPlusPlus.getPropertyComposition();
            ObjectPropertyPointer superPropertyPointer = translator.translate(axiom.getSuperProperty());
            lastAxiom = faCTPlusPlus.tellSubObjectProperties(propertyChainPointer, superPropertyPointer);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLReflexiveObjectPropertyAxiom axiom) {
        try {
            lastAxiom = faCTPlusPlus.tellReflexiveObjectProperty(translator.translate(axiom.getProperty()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLIrreflexiveObjectPropertyAxiom axiom) {
        try {
            lastAxiom = faCTPlusPlus.tellIrreflexiveObjectProperty(translator.translate(axiom.getProperty()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLAsymmetricObjectPropertyAxiom axiom) {
        try {
            lastAxiom = faCTPlusPlus.tellAntiSymmetricObjectProperty(translator.translate(axiom.getProperty()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLNegativeObjectPropertyAssertionAxiom axiom) {
        try {
            lastAxiom = faCTPlusPlus.tellNotRelatedIndividuals(translator.translate(axiom.getSubject()),
                                                               translator.translate(axiom.getProperty()),
                                                               translator.translate(axiom.getObject()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLNegativeDataPropertyAssertionAxiom axiom) {
        try {
            lastAxiom = faCTPlusPlus.tellNotRelatedIndividualValue(translator.translate(axiom.getSubject()),
                                                                   translator.translate(axiom.getProperty()),
                                                                   translator.translate(axiom.getObject()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLDataPropertyDomainAxiom axiom) {
        try {
            lastAxiom = faCTPlusPlus.tellDataPropertyDomain(translator.translate(axiom.getProperty()),
                                                            translator.translate(axiom.getDomain()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLObjectPropertyAssertionAxiom axiom) {
        try {
            lastAxiom = faCTPlusPlus.tellRelatedIndividuals(translator.translate(axiom.getSubject()),
                                                            translator.translate(axiom.getProperty()),
                                                            translator.translate(axiom.getObject()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLFunctionalDataPropertyAxiom axiom) {
        try {
            lastAxiom = faCTPlusPlus.tellFunctionalDataProperty(translator.translate(axiom.getProperty()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLClassAssertionAxiom axiom) {
        try {
            lastAxiom = faCTPlusPlus.tellIndividualType(translator.translate(axiom.getIndividual()),
                                                        translator.translate(axiom.getClassExpression()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLDataPropertyAssertionAxiom axiom) {
        try {
            lastAxiom = faCTPlusPlus.tellRelatedIndividualValue(translator.translate(axiom.getSubject()),
                                                                translator.translate(axiom.getProperty()),
                                                                translator.translate(axiom.getObject()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLSubDataPropertyOfAxiom axiom) {
        try {
            lastAxiom = faCTPlusPlus.tellSubDataProperties(translator.translate(axiom.getSubProperty()),
                                                           translator.translate(axiom.getSuperProperty()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    private OWLEntityVisitor declarationVisitor = new OWLEntityVisitor(){

        public void visit(OWLClass owlClass) {
            try {
                lastAxiom = faCTPlusPlus.tellClassDeclaration(translator.translate(owlClass));
            }
            catch (Exception e) {
                throw new FaCTPlusPlusRuntimeException(e);
            }
        }


        public void visit(OWLObjectProperty owlObjectProperty) {
            try {
                lastAxiom = faCTPlusPlus.tellObjectPropertyDeclaration(translator.translate(owlObjectProperty));
            }
            catch (Exception e) {
                throw new FaCTPlusPlusRuntimeException(e);
            }
        }


        public void visit(OWLDataProperty owlDataProperty) {
            try {
                lastAxiom = faCTPlusPlus.tellDataPropertyDeclaration(translator.translate(owlDataProperty));
            }
            catch (Exception e) {
                throw new FaCTPlusPlusRuntimeException(e);
            }
        }


        public void visit(OWLNamedIndividual individual) {
            try {
                lastAxiom = faCTPlusPlus.tellIndividualDeclaration(translator.translate(individual));
            }
            catch (Exception e) {
                throw new FaCTPlusPlusRuntimeException(e);
            }
        }


        public void visit(OWLDatatype owlDataType) {
            try {
                lastAxiom = faCTPlusPlus.tellDatatypeDeclaration((DataTypePointer)translator.translate(owlDataType));
            }
            catch (Exception e) {
                throw new FaCTPlusPlusRuntimeException(e);
            }
        }


        public void visit(OWLAnnotationProperty owlAnnotationProperty) {
            // do nothing
        }
    };


    public void visit(OWLDeclarationAxiom axiom) {
        axiom.getEntity().accept(declarationVisitor);
    }


    public void visit(OWLAnnotationAssertionAxiom axiom) {
        // skip
    }


    public void visit(OWLSubAnnotationPropertyOfAxiom axiom) {
        // skip
    }


    public void visit(OWLAnnotationPropertyDomainAxiom axiom) {
        // skip
    }


    public void visit(OWLAnnotationPropertyRangeAxiom axiom) {
        // skip
    }


    public void visit(SWRLRule rule) {
        // Skip
    }
}
