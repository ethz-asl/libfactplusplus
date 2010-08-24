package uk.ac.manchester.cs.factplusplus.owlapi;

import org.semanticweb.owl.model.*;
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


    public void visit(OWLDisjointClassesAxiom owlDisjointClassesAxiom) {
        try {
            translateDescriptionArgList(owlDisjointClassesAxiom.getDescriptions());
            lastAxiom = faCTPlusPlus.tellDisjointClasses();
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLEquivalentClassesAxiom owlEquivalentClassesAxiom) {
        try {
            translateDescriptionArgList(owlEquivalentClassesAxiom.getDescriptions());
            lastAxiom = faCTPlusPlus.tellEquivalentClass();
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLSubClassAxiom owlSubClassAxiom) {
        try {
            lastAxiom = faCTPlusPlus.tellSubClassOf(translate(owlSubClassAxiom.getSubClass()),
                                                    translate(owlSubClassAxiom.getSuperClass()));
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


    public void visit(OWLDataPropertyRangeAxiom owlDataPropertyRangeAxiom) {
        try {
            owlDataPropertyRangeAxiom.getProperty().accept(translator);
            DataPropertyPointer p = translator.getLastDataPropertyPointer();
            owlDataPropertyRangeAxiom.getRange().accept(translator);
            lastAxiom = faCTPlusPlus.tellDataPropertyRange(p, translator.getLastDataTypeExpressionPointer());
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLEquivalentObjectPropertiesAxiom axiom) {
        try {
            translateObjectPropertyArgList(axiom.getProperties());
            lastAxiom = faCTPlusPlus.tellEquivalentObjectProperties();
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLEquivalentDataPropertiesAxiom axiom) {
        try {
            translateDataPropertyArgList(axiom.getProperties());
            lastAxiom = faCTPlusPlus.tellEquivalentDataProperties();
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLFunctionalObjectPropertyAxiom axiom) {
        try {
            axiom.getProperty().accept(translator);
            lastAxiom = faCTPlusPlus.tellFunctionalObjectProperty(translator.getLastObjectPropertyPointer());
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLInverseFunctionalObjectPropertyAxiom owlInverseFunctionalPropertyAxiom) {
        try {
            lastAxiom = faCTPlusPlus.tellInverseFunctionalObjectProperty(
                    translate(owlInverseFunctionalPropertyAxiom.getProperty()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }



    public void visit(OWLObjectPropertyRangeAxiom owlObjectPropertyRangeAxiom) {
        try {
            lastAxiom = faCTPlusPlus.tellObjectPropertyRange(translate(owlObjectPropertyRangeAxiom.getProperty()),
                                                             translate(owlObjectPropertyRangeAxiom.getRange()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLObjectPropertyDomainAxiom owlPropertyDomainAxiom) {
        try {
            lastAxiom = faCTPlusPlus.tellObjectPropertyDomain(translate(owlPropertyDomainAxiom.getProperty()),
                                                              translate(owlPropertyDomainAxiom.getDomain()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLObjectSubPropertyAxiom owlSubPropertyAxiom) {
        try {
            lastAxiom = faCTPlusPlus.tellSubObjectProperties(translate(owlSubPropertyAxiom.getSubProperty()),
                                                             translate(owlSubPropertyAxiom.getSuperProperty()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLSymmetricObjectPropertyAxiom owlSymmetricPropertyAxiom) {
        try {
            lastAxiom = faCTPlusPlus.tellSymmetricObjectProperty(translate(owlSymmetricPropertyAxiom.getProperty()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLTransitiveObjectPropertyAxiom owlTransitivePropertyAxiom) {
        try {
            lastAxiom = faCTPlusPlus.tellTransitiveObjectProperty(translate(owlTransitivePropertyAxiom.getProperty()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLInverseObjectPropertiesAxiom axiom) {
        try {
            lastAxiom = faCTPlusPlus.tellInverseProperties(translate(axiom.getFirstProperty()),
                                                           translate(axiom.getSecondProperty()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////
    //
    // Individual axioms
    //
    ////////////////////////////////////////////////////////////////////////////////////////


    public void visit(OWLDifferentIndividualsAxiom owlDifferentIndividualsAxiom) {
        try {
            translateIndividualArgList(owlDifferentIndividualsAxiom.getIndividuals());
            lastAxiom = faCTPlusPlus.tellDifferentIndividuals();
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLSameIndividualsAxiom owlSameIndividualsAxiom) {
        try {
            translateIndividualArgList(owlSameIndividualsAxiom.getIndividuals());
            lastAxiom = faCTPlusPlus.tellSameIndividuals();
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    private void translateDescriptionArgList(Set<OWLDescription> descriptions) {
        try {
            Set<ClassPointer> classPointers = new HashSet<ClassPointer>();
            for (OWLDescription desc : descriptions) {
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


    private void translateIndividualArgList(Set<? extends OWLIndividual> individuals) {
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


    private void translateObjectPropertyArgList(Set<? extends OWLObjectPropertyExpression> properties) throws
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

    private void translateDataPropertyArgList(Set<? extends OWLDataPropertyExpression> properties) throws
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

    private ClassPointer translate(OWLDescription description) {
        description.accept(translator);
        return translator.getLastClassPointer();
    }

    private DataPropertyPointer translate(OWLDataPropertyExpression dataProperty) {
        dataProperty.accept(translator);
        return translator.getLastDataPropertyPointer();
    }



    private ObjectPropertyPointer translate(OWLObjectPropertyExpression objectProperty) {
        objectProperty.accept(translator);
        return translator.getLastObjectPropertyPointer();
    }


    private IndividualPointer translate(OWLIndividual individual) {
        individual.accept(translator);
        return translator.getLastIndividualPointer();
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // OWL 1.1 Stuff
    //
    //////////////////////////////////////////////////////////////////////////////////////////////////


    public void visit(OWLDisjointUnionAxiom owlDisjointUnionAxiom) {
        try {
            // Need to expand this
            Set<ClassPointer> operandPointers = new HashSet<ClassPointer>();
            for (OWLDescription desc : owlDisjointUnionAxiom.getDescriptions()) {
                desc.accept(translator);
                operandPointers.add(translator.getLastClassPointer());
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
            for (OWLDescription desc : owlDisjointUnionAxiom.getDescriptions()) {
                desc.accept(translator);
                operandPointers.add(translator.getLastClassPointer());
            }
            faCTPlusPlus.initArgList();
            for (ClassPointer operandPointer : operandPointers) {
                faCTPlusPlus.addArg(operandPointer);
            }
            faCTPlusPlus.closeArgList();
            ClassPointer unionPointer = faCTPlusPlus.getConceptOr();
            ClassPointer clsPointer = translate(owlDisjointUnionAxiom.getOWLClass());
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


    public void visit(OWLDisjointObjectPropertiesAxiom owlDisjointObjectPropertiesAxiom) {
        try {
            Set<ObjectPropertyPointer> pointers = new HashSet<ObjectPropertyPointer>();
            for (OWLObjectPropertyExpression prop : owlDisjointObjectPropertiesAxiom.getProperties()) {
                prop.accept(translator);
                pointers.add(translator.getLastObjectPropertyPointer());
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


    public void visit(OWLDisjointDataPropertiesAxiom owlDisjointDataPropertiesAxiom) {
        try {
            Set<DataPropertyPointer> pointers = new HashSet<DataPropertyPointer>();
            for (OWLDataPropertyExpression prop : owlDisjointDataPropertiesAxiom.getProperties()) {
                prop.accept(translator);
                pointers.add(translator.getLastDataPropertyPointer());
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


    public void visit(OWLObjectPropertyChainSubPropertyAxiom owlObjectPropertyChainSubPropertyAxiom) {
        try {
            List<ObjectPropertyPointer> propertyChainPointers = new ArrayList<ObjectPropertyPointer>();
            for (OWLObjectPropertyExpression prop : owlObjectPropertyChainSubPropertyAxiom.getPropertyChain()) {
                propertyChainPointers.add(translate(prop));
            }
            faCTPlusPlus.initArgList();
            for (ObjectPropertyPointer pointer : propertyChainPointers) {
                faCTPlusPlus.addArg(pointer);
            }
            faCTPlusPlus.closeArgList();
            ObjectPropertyPointer propertyChainPointer = faCTPlusPlus.getPropertyComposition();
            ObjectPropertyPointer superPropertyPointer = translate(
                    owlObjectPropertyChainSubPropertyAxiom.getSuperProperty());
            lastAxiom = faCTPlusPlus.tellSubObjectProperties(propertyChainPointer, superPropertyPointer);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLReflexiveObjectPropertyAxiom axiom) {
        try {
            axiom.getProperty().accept(translator);
            lastAxiom = faCTPlusPlus.tellReflexiveObjectProperty(translator.getLastObjectPropertyPointer());
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLIrreflexiveObjectPropertyAxiom axiom) {
        try {
            axiom.getProperty().accept(translator);
            lastAxiom = faCTPlusPlus.tellIrreflexiveObjectProperty(translator.getLastObjectPropertyPointer());
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLAntiSymmetricObjectPropertyAxiom axiom) {
        try {
            axiom.getProperty().accept(translator);
            lastAxiom = faCTPlusPlus.tellAsymmetricObjectProperty(translator.getLastObjectPropertyPointer());
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLNegativeObjectPropertyAssertionAxiom owlObjectValueNotAxiom) {
        try {
            owlObjectValueNotAxiom.getSubject().accept(translator);
            IndividualPointer subject = translator.getLastIndividualPointer();
            owlObjectValueNotAxiom.getProperty().accept(translator);
            ObjectPropertyPointer property = translator.getLastObjectPropertyPointer();
            owlObjectValueNotAxiom.getObject().accept(translator);
            IndividualPointer object = translator.getLastIndividualPointer();
            lastAxiom = faCTPlusPlus.tellNotRelatedIndividuals(subject, property, object);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLNegativeDataPropertyAssertionAxiom owlDataValueNotAxiom) {
        try {
            IndividualPointer subject = translator.translate(owlDataValueNotAxiom.getSubject());
            DataPropertyPointer property = translator.translate(owlDataValueNotAxiom.getProperty());
            DataValuePointer object = translator.translate(owlDataValueNotAxiom.getObject());
            lastAxiom = faCTPlusPlus.tellNotRelatedIndividualValue(subject, property, object);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLDataPropertyDomainAxiom axiom) {
        try {
            axiom.getProperty().accept(translator);
            DataPropertyPointer p = translator.getLastDataPropertyPointer();
            axiom.getDomain().accept(translator);
            lastAxiom = faCTPlusPlus.tellDataPropertyDomain(p, translator.getLastClassPointer());
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLObjectPropertyAssertionAxiom axiom) {
        try {
            IndividualPointer subj = translator.translate(axiom.getSubject());
            axiom.getProperty().accept(translator);
            ObjectPropertyPointer prop = translator.getLastObjectPropertyPointer();
            IndividualPointer obj = translator.translate(axiom.getObject());
            lastAxiom = faCTPlusPlus.tellRelatedIndividuals(subj, prop, obj);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLFunctionalDataPropertyAxiom axiom) {
        try {
            lastAxiom = faCTPlusPlus.tellFunctionalDataProperty(translate(axiom.getProperty()));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLClassAssertionAxiom axiom) {
        try {
            IndividualPointer indPointer = translator.translate(axiom.getIndividual());
            ClassPointer cp = translator.translate(axiom.getDescription());
            lastAxiom = faCTPlusPlus.tellIndividualType(indPointer, cp);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLDataPropertyAssertionAxiom axiom) {
        try {
            IndividualPointer subj = translator.translate(axiom.getSubject());
            axiom.getProperty().accept(translator);
            DataPropertyPointer prop = translator.getLastDataPropertyPointer();
            DataValuePointer obj = translator.translate(axiom.getObject());
            lastAxiom = faCTPlusPlus.tellRelatedIndividualValue(subj, prop, obj);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLDataSubPropertyAxiom axiom) {
        try {
            DataPropertyPointer sub = translator.translate(axiom.getSubProperty());
            DataPropertyPointer sup = translator.translate(axiom.getSuperProperty());
            lastAxiom = faCTPlusPlus.tellSubDataProperties(sub, sup);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void visit(OWLImportsDeclaration axiom) {
        // Skip
    }


    public void visit(OWLAxiomAnnotationAxiom axiom) {
        // Skip
    }
    

    private OWLEntityVisitor declarationVisitor = new OWLEntityVisitor(){

        public void visit(OWLClass owlClass) {
            try {
                ClassPointer cls = translator.translate(owlClass);
                lastAxiom = faCTPlusPlus.tellClassDeclaration(cls);
            }
            catch (Exception e) {
                throw new FaCTPlusPlusRuntimeException(e);
            }
        }


        public void visit(OWLObjectProperty owlObjectProperty) {
            try {
                ObjectPropertyPointer op = translator.translate(owlObjectProperty);
                lastAxiom = faCTPlusPlus.tellObjectPropertyDeclaration(op);
            }
            catch (Exception e) {
                throw new FaCTPlusPlusRuntimeException(e);
            }
        }


        public void visit(OWLDataProperty owlDataProperty) {
            try {
                DataPropertyPointer op = translator.translate(owlDataProperty);
                lastAxiom = faCTPlusPlus.tellDataPropertyDeclaration(op);                
            }
            catch (Exception e) {
                throw new FaCTPlusPlusRuntimeException(e);
            }
        }


        public void visit(OWLIndividual individual) {
            try {
                IndividualPointer op = translator.translate(individual);
                lastAxiom = faCTPlusPlus.tellIndividualDeclaration(op);
            }
            catch (Exception e) {
                throw new FaCTPlusPlusRuntimeException(e);
            }
        }


        public void visit(OWLDataType owlDataType) {
            // todo
        }
    };


    public void visit(OWLDeclarationAxiom axiom) {
        axiom.getEntity().accept(declarationVisitor);
    }


    public void visit(OWLEntityAnnotationAxiom axiom) {
        // Convert every entity annotation into a declaration axiom on the subject
        // This helps us keep track of how many references to the entity exist
        OWLDeclarationAxiom decl = df.getOWLDeclarationAxiom(axiom.getSubject());
        decl.accept(this);
    }


    public void visit(SWRLRule rule) {
        // Skip
    }


    public void visit(OWLOntologyAnnotationAxiom axiom) {
        // Skip
    }
}
