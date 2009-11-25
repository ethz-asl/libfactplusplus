package uk.ac.manchester.cs.factplusplus.owlapiv3;

import org.semanticweb.owlapi.model.*;
import org.semanticweb.owlapi.inference.OWLReasonerException;
import org.semanticweb.owlapi.util.OWLObjectVisitorAdapter;

import java.util.Set;
import java.util.HashSet;
import java.util.logging.Logger;

import uk.ac.manchester.cs.factplusplus.*;

/**
 * Author: Nick Drummond<br>
 * http://www.cs.man.ac.uk/~drummond/<br><br>
 * <p/>
 * The University Of Manchester<br>
 * Bio Health Informatics Group<br>
 * Date: Oct 3, 2008<br><br>
 *
 * Set translation and parameter checking utilities
 */
public class TranslatorUtils extends OWLObjectVisitorAdapter {

    private static final Logger logger = Logger.getLogger(TranslatorUtils.class.getName());

    private TranslatorImpl translator;

    private FaCTPlusPlus faCTPlusPlus;

    private OWLOntologyManager mngr;


    public TranslatorUtils(OWLOntologyManager mngr, TranslatorImpl translator) {
        this.mngr = mngr;
        this.faCTPlusPlus = translator.getFaCTPlusPlus();
        this.translator = translator;
    }


    /**
     * Given an array of arrays of class pointers, this method translates
     * them into a set of sets of OWLAPI <code>OWLClass</code> objects.
     * @param clsPointers The array of arrays of class pointers to be translated.
     */
    public Set<Set<OWLClass>> getOWLAPISets(ClassPointer[][] clsPointers) throws OWLReasonerException {
        Set<Set<OWLClass>> clsSets = new HashSet<Set<OWLClass>>();
        for (ClassPointer[] clsPointArray : clsPointers) {
            Set<OWLClass> clses = new HashSet<OWLClass>();
            for (ClassPointer clsPointer : clsPointArray) {
                OWLClass cls = translator.getOWLClass(clsPointer);
                if (cls != null) {
                    clses.add(cls);
                }
                else {
                    try {
                        if (clsPointer.equals(faCTPlusPlus.getThing())) {
                            clses.add(mngr.getOWLDataFactory().getOWLThing());
                            logger.fine("Missing translation for Thing");
                        }
                        else if (clsPointer.equals(faCTPlusPlus.getNothing())) {
                            clses.add(mngr.getOWLDataFactory().getOWLNothing());
                            logger.fine("Missing translation for Nothing");
                        }
                        else {
                            logger.severe("Translation failed for class pointer: " + clsPointer);
                        }
                    }
                    catch (Exception e) {
                        throw new FaCTPlusPlusReasonerException(e);
                    }
                }
            }
            clsSets.add(clses);
        }
        return clsSets;
    }


    public Set<Set<OWLClassExpression>> getOWLAPIClassExpressionSets(ClassPointer[][] clsPointers) throws OWLReasonerException {
        Set<Set<OWLClassExpression>> clsSets = new HashSet<Set<OWLClassExpression>>();
        for (ClassPointer[] clsPointArray : clsPointers) {
            Set<OWLClassExpression> clses = new HashSet<OWLClassExpression>();
            for (ClassPointer clsPointer : clsPointArray) {
                OWLClass cls = translator.getOWLClass(clsPointer);
                if (cls != null) {
                    clses.add(cls);
                }
                else {
                    try {
                        if (clsPointer.equals(faCTPlusPlus.getThing())) {
                            clses.add(mngr.getOWLDataFactory().getOWLThing());
                            logger.fine("Missing translation for Thing");
                        }
                        else if (clsPointer.equals(faCTPlusPlus.getNothing())) {
                            clses.add(mngr.getOWLDataFactory().getOWLNothing());
                            logger.fine("Missing translation for Nothing");
                        }
                        else {
                            logger.severe("Translation failed for class pointer: " + clsPointer);
                        }
                    }
                    catch (Exception e) {
                        throw new FaCTPlusPlusReasonerException(e);
                    }
                }
            }
            clsSets.add(clses);
        }
        return clsSets;
    }


    public Set<Set<OWLObjectProperty>> getOWLAPISets(ObjectPropertyPointer[][] propertyPointers) throws
            OWLReasonerException {
        Set<Set<OWLObjectProperty>> propSets = new HashSet<Set<OWLObjectProperty>>();
        for (ObjectPropertyPointer[] propertyPointerArray : propertyPointers) {
            Set<OWLObjectProperty> props = new HashSet<OWLObjectProperty>();
            for (ObjectPropertyPointer propPointer : propertyPointerArray) {
                OWLObjectProperty prop = translator.getOWLObjectProperty(propPointer);
                if (prop != null) {
                    props.add(prop);
                }
                else {
                    logger.severe("Translation failed for object pointer: " + propPointer);
                }
            }
            propSets.add(props);
        }
        return propSets;
    }


    public Set<Set<OWLDataProperty>> getOWLAPISets(DataPropertyPointer[][] propertyPointers) throws
            OWLReasonerException {
        Set<Set<OWLDataProperty>> propSets = new HashSet<Set<OWLDataProperty>>();
        for (DataPropertyPointer[] propertyPointerArray : propertyPointers) {
            Set<OWLDataProperty> props = new HashSet<OWLDataProperty>();
            for (DataPropertyPointer propPointer : propertyPointerArray) {
                OWLDataProperty prop = translator.getOWLDataProperty(propPointer);
                if (prop != null) {
                    props.add(prop);
                }
                else {
                    logger.severe("Translation failed for object pointer: " + propPointer);
                }
            }
            propSets.add(props);
        }
        return propSets;
    }


    public <N extends OWLObject, P extends Pointer> Set<N> getOWLAPISet(P[] pointers) {
        Set<N> apiresult = new HashSet<N>();
        for (P pointer : pointers) {
            N translation = (N)translator.getOWLObject(pointer);
            if (translation != null) {
                apiresult.add(translation);
            }
        }
        return apiresult;
    }


    public Set<OWLClassExpression> getOWLAPIClassExpressionSet(ClassPointer[][] pointers) {
        Set<OWLClassExpression> apiresult = new HashSet<OWLClassExpression>();
        for (ClassPointer[] equivs : pointers) {
            for (ClassPointer pointer : equivs){
                OWLClassExpression translation = translator.getOWLClass(pointer);
                if (translation != null) {
                    apiresult.add(translation);
                }
            }
        }
        return apiresult;
    }



    public void checkParams(OWLObject... owlObjects){
        for (OWLObject owlObject : owlObjects){
            owlObject.accept(this);
        }
    }


    public void visit(OWLClass owlClass) {
        if (!translator.contains(owlClass)) {
            throw new FaCTPlusPlusRuntimeException("Asking about unknown class (" + owlClass.getIRI() + ").  (Check that the reasoner is synchronized)");
        }
    }

    public void visit(OWLObjectProperty property) {
        if (!translator.contains(property)) {
            throw new FaCTPlusPlusRuntimeException("Asking about unknown object property (" + property.getIRI() + ").  (Check that the reasoner is synchronized)");
        }
    }

    public void visit(OWLDataProperty property) {
        if (!translator.contains(property)) {
            throw new FaCTPlusPlusRuntimeException("Asking about unknown data property (" + property.getIRI() + ").  (Check that the reasoner is synchronized)");
        }
    }

    public void visit(OWLNamedIndividual individual) {
        if (!translator.contains(individual)) {
            throw new FaCTPlusPlusRuntimeException("Asking about unknown individual (" + individual.getIRI() + ") (Check that the reasoner is synchronized)");
        }
    }

    public void visit(OWLAnonymousIndividual individual) {
        if (!translator.contains(individual)) {
            throw new FaCTPlusPlusRuntimeException("Asking about unknown anonymous individual (" + individual.getID() + ") (Check that the reasoner is synchronized)");
        }
    }
}
