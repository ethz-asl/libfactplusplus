package uk.ac.manchester.cs.factplusplus.owlapi;

import org.semanticweb.owl.model.*;
import org.semanticweb.owl.inference.OWLReasonerException;
import org.semanticweb.owl.util.OWLObjectVisitorAdapter;

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

    private Translator translator;

    private FaCTPlusPlus faCTPlusPlus;

    private OWLOntologyManager mngr;


    /**
     * @deprecated use <code>TranslatorUtils(OWLOntologyManager mngr, Translator translator)</code>
     * @param mngr
     * @param faCTPlusPlus ignored - this is taken from the translator
     * @param translator
     */
    public TranslatorUtils(OWLOntologyManager mngr, FaCTPlusPlus faCTPlusPlus, Translator translator) {
        this(mngr, translator);
    }


    public TranslatorUtils(OWLOntologyManager mngr, Translator translator) {
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


    public Set<Set<OWLDescription>> getOWLAPIDescriptionSets(ClassPointer[][] clsPointers) throws OWLReasonerException {
        Set<Set<OWLDescription>> clsSets = new HashSet<Set<OWLDescription>>();
        for (ClassPointer[] clsPointArray : clsPointers) {
            Set<OWLDescription> clses = new HashSet<OWLDescription>();
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


    public Set<OWLDescription> getOWLAPIDescriptionSet(ClassPointer[][] pointers) {
        Set<OWLDescription> apiresult = new HashSet<OWLDescription>();
        for (ClassPointer[] equivs : pointers) {
            for (ClassPointer pointer : equivs){
                OWLDescription translation = translator.getOWLClass(pointer);
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
            throw new FaCTPlusPlusRuntimeException("Asking about unknown class (" + owlClass.getURI() + ").  (Check that the reasoner is synchronized)");
        }
    }

    public void visit(OWLObjectProperty property) {
        if (!translator.contains(property)) {
            throw new FaCTPlusPlusRuntimeException("Asking about unknown object property (" + property.getURI() + ").  (Check that the reasoner is synchronized)");
        }
    }

    public void visit(OWLDataProperty property) {
        if (!translator.contains(property)) {
            throw new FaCTPlusPlusRuntimeException("Asking about unknown data property (" + property.getURI() + ").  (Check that the reasoner is synchronized)");
        }
    }

    public void visit(OWLIndividual individual) {
        if (!translator.contains(individual)) {
            throw new FaCTPlusPlusRuntimeException("Asking about unknown individual (" + individual.getURI() + ") (Check that the reasoner is synchronized)");
        }
    }

}
