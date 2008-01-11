package uk.ac.manchester.cs.factplusplus.owlapi;

import org.semanticweb.owl.inference.MonitorableOWLReasonerAdapter;
import org.semanticweb.owl.inference.OWLReasonerException;
import org.semanticweb.owl.inference.UnsupportedReasonerOperationException;
import org.semanticweb.owl.model.*;
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
 * <p/>
 * An OWLAPI wrapper around the FaCTPlusPlus reasoner.
 */
public class Reasoner extends MonitorableOWLReasonerAdapter implements FaCTPlusPlusProgressMonitor {

    private static final Logger logger = Logger.getLogger(Reasoner.class.getName());

    /**
     * The FaCT++ reasoner that does all the hard work *
     */
    private FaCTPlusPlus faCTPlusPlus;

    /**
     * The translator that translates back and forth between OWLAPI
     * objects and FaCT++ pointer objects.
     */
    private final Translator translator;

    /**
     * A flag which indicates if the contents of the reasoner are synchronised
     * with the current set of ontologies.
     */
    private boolean synchronised;

    private boolean sychroniseOnlyOnClassify = true;

    /**
     * There is only one kernel per reasoner.  For a new
     * kernel, a new <code>Reasoner</code> needs to be created.
     */

    /**
     * Creates a new <code>Reasoner</code>.
     */
    public Reasoner(OWLOntologyManager owlOntologyManager) throws Exception {
        super(owlOntologyManager);
        // Create an accessor interface to the FaCT++ native reasoner
        faCTPlusPlus = new FaCTPlusPlus();
        // Create the one and only translator that translates between FaCT++
        // objects and OWLAPI objects
        translator = new Translator(owlOntologyManager, faCTPlusPlus);
        synchronised = true;
    }


    /**
     * Disposes of this reasoner.  This clears the translation
     * tables, and clears and deletes the FaCT++ kernel that
     * corresponds to this reasoner.
     */
    public void disposeReasoner() {
        try {
            if (faCTPlusPlus == null) {
                return;
            }
            faCTPlusPlus.dispose();
            faCTPlusPlus = null;
        }
        catch (Exception e) {
            logger.severe(e.getMessage());
        }
    }


    public boolean isRealised() throws OWLReasonerException {
        return synchronised;
    }


    public void realise() throws OWLReasonerException {
        try {
            faCTPlusPlus.realise();
        }
        catch (FaCTPlusPlusException e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isDefined(OWLClass cls) throws OWLReasonerException {
        return containsReference(cls);
    }


    public boolean isDefined(OWLObjectProperty prop) throws OWLReasonerException {
        return containsReference(prop);
    }


    public boolean isDefined(OWLDataProperty prop) throws OWLReasonerException {
        return containsReference(prop);
    }


    public boolean isDefined(OWLIndividual ind) throws OWLReasonerException {
        return containsReference(ind);
    }


    /**
     * Determines if FaCT++ will be reloaded and the ontologies reclassified
     * on the first ask after any changes have been made, or if FaCT++ must
     * explicitly be told to reload and reclassify using the <code>classify</code>
     * method.
     * @return <code>true</code> if the <code>classify</code> method must
     *         be called to reload and classify the set of ontologies after changes
     *         have been made to the ontology, or <code>false</code> if the set of
     *         ontologies will automatically be reloaded and reclassified on the first
     *         ask operation after changes have been made to any loaded ontology.
     */
    public boolean isSychroniseOnlyOnClassify() {
        return sychroniseOnlyOnClassify;
    }


    public void setSychroniseOnlyOnClassify(boolean sychroniseOnlyOnClassify) {
        this.sychroniseOnlyOnClassify = sychroniseOnlyOnClassify;
    }


    /**
     * Checks to see if the specified ontology is consistent
     * @param ontology The ontology to check
     * @return <code>true</code> if the ontology is consistent,
     *         or <code>false</code> if the ontology is not consistent.
     */
    public boolean isConsistent(OWLOntology ontology) throws OWLReasonerException {
        try {
            return faCTPlusPlus.isKBConsistent();
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    private FaCTPlusPlus getFaCTPlusPlus() {
        return faCTPlusPlus;
    }


    private void ensureSynchronised() throws OWLReasonerException {
        if (!synchronised) {
            try {
                resync();
                synchronised = true;
            }
            catch (Exception e) {
                logger.severe(e.getMessage());
            }
        }
    }


    private void markForResyncronisation() {
        if (!sychroniseOnlyOnClassify) {
            synchronised = false;
        }
    }


    private void resync() throws Exception {
        try {
            getFaCTPlusPlus().clearKernel();
            // Reset the translator, because the pointer mappings that it holds
            // are no longer valid
            translator.reset();

            // Now reload the ontologies
            OntologyLoader loader = new OntologyLoader(getOWLOntologyManager(), translator);
            loader.loadOntologies(getLoadedOntologies(), faCTPlusPlus);
//            progressMonitor = new FaCTPPProgMon();
            faCTPlusPlus.setProgressMonitor(this);
        }
        catch (Exception e) {
            throw new OWLRuntimeException(e);
        }
    }


    public boolean isClassified() throws OWLReasonerException {
        return synchronised;
    }


    public void classify() throws OWLReasonerException {
        try {
            synchronised = false;
            ensureSynchronised();
            getReasoner().classify();
        }
        catch(InconsistentOntologyException e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    protected void ontologiesCleared() throws OWLReasonerException {
        try {
            getReasoner().clearKernel();
            synchronised = false;
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    protected void ontologiesChanged() throws OWLReasonerException {
        synchronised = false;
    }


    protected void handleOntologyChanges(List<OWLOntologyChange> changes) throws OWLReasonerException {
        try {
            markForResyncronisation();
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isSubClassOf(OWLDescription owlDescription, OWLDescription owlDescription1) throws
                                                                                               OWLReasonerException {
        try {
            ensureSynchronised();
            return getFaCTPlusPlus().isClassSubsumedBy(translator.translate(owlDescription),
                                                       translator.translate(owlDescription1));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isEquivalentClass(OWLDescription owlDescription, OWLDescription owlDescription1) throws
                                                                                                    OWLReasonerException {
        try {
            ensureSynchronised();
            return getFaCTPlusPlus().isClassEquivalentTo(translator.translate(owlDescription),
                                                         translator.translate(owlDescription1));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean containsReference(OWLClass cls) {
        return translator.contains(cls);
    }


    public boolean containsReference(OWLObjectProperty prop) {
        return translator.contains(prop);
    }


    public boolean containsReference(OWLDataProperty prop) {
        return translator.contains(prop);
    }


    public boolean containsReference(OWLIndividual ind) {
        return translator.contains(ind);
    }


    private void checkDescription(OWLDescription owlDescription) {
        if (owlDescription instanceof OWLClass) {
            if (!translator.contains((OWLClass) owlDescription)) {
                throw new FaCTPlusPlusRuntimeException("Asking about unknown class (" + ((OWLClass) owlDescription).getURI() + ").  (Check that the reasoner is synchronized)");
            }
        }
    }


    private void checkIndividual(OWLIndividual individual) {
        if (!translator.contains(individual)) {
            throw new FaCTPlusPlusRuntimeException("Asking about unknown individual (" + individual.getURI() + ") (Check that the reasoner is synchronized)");
        }
    }


    public boolean isSatisfiable(OWLDescription owlDescription) throws OWLReasonerException {
        try {
            ensureSynchronised();
            if (!faCTPlusPlus.isKBConsistent()) {
                return false;
            }
            checkDescription(owlDescription);
            return getFaCTPlusPlus().isClassSatisfiable(translator.translate(owlDescription));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLClass>> getSuperClasses(OWLDescription owlDescription) throws OWLReasonerException {
        try {
            ensureSynchronised();
            checkDescription(owlDescription);

            ClassPointer[][] result = getFaCTPlusPlus().askSuperClasses(translator.translate(owlDescription), true);
            return getOWLAPISets(result);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLClass>> getAncestorClasses(OWLDescription owlDescription) throws OWLReasonerException {
        try {
            ensureSynchronised();
            checkDescription(owlDescription);

            ClassPointer[][] result = getFaCTPlusPlus().askSuperClasses(translator.translate(owlDescription), false);
            return getOWLAPISets(result);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLClass>> getSubClasses(OWLDescription owlDescription) throws OWLReasonerException {
        try {
            ensureSynchronised();
            checkDescription(owlDescription);

            ClassPointer[][] result = getFaCTPlusPlus().askSubClasses(translator.translate(owlDescription), true);
            return getOWLAPISets(result);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLClass>> getDescendantClasses(OWLDescription owlDescription) throws OWLReasonerException {
        try {
            ensureSynchronised();
            checkDescription(owlDescription);

            ClassPointer[][] result = getFaCTPlusPlus().askSubClasses(translator.translate(owlDescription), false);
            return getOWLAPISets(result);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<OWLClass> getEquivalentClasses(OWLDescription owlDescription) throws OWLReasonerException {
        try {
            ensureSynchronised();
            checkDescription(owlDescription);

            Set<OWLClass> apiresult = new HashSet<OWLClass>();
            ClassPointer[] result = getFaCTPlusPlus().askEquivalentClasses(translator.translate(owlDescription));
            for (ClassPointer cp : result) {
                OWLClass cls = translator.getOWLClass(cp);
                if (cls != null) {
                    apiresult.add(cls);
                }
            }
            return apiresult;
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isInstanceOf(OWLIndividual owlIndividual, OWLDescription owlDescription) throws
                                                                                            OWLReasonerException {
        try {
            ensureSynchronised();
            return getFaCTPlusPlus().isInstanceOf(translator.translate(owlIndividual),
                                                  translator.translate(owlDescription));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    /**
     * A convenience methods for obtaining all classes which are inconsistent.
     * @return A set of classes which are inconsistent.
     */
    public Set<OWLClass> getInconsistentClasses() throws OWLReasonerException {
        ensureSynchronised();
        return getEquivalentClasses(getOWLDataFactory().getOWLNothing());
    }


    public Set<OWLIndividual> getIndividuals(OWLDescription owlDescription, boolean direct) throws
                                                                                            OWLReasonerException {
        try {
            ensureSynchronised();
            checkDescription(owlDescription);
            IndividualPointer[] individualPointers = getFaCTPlusPlus().askInstances(translator.translate(owlDescription));
            Set<OWLIndividual> individuals = new HashSet<OWLIndividual>();
            for (IndividualPointer individualPointer : individualPointers) {
                OWLIndividual translation = translator.getOWLIndividual(individualPointer);
                if (translation != null) {
                    individuals.add(translation);
                }
            }
            return individuals;
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLClass>> getTypes(OWLIndividual individual, boolean direct) throws OWLReasonerException {
        try {
            ensureSynchronised();
            checkIndividual(individual);
            return getOWLAPISets(getFaCTPlusPlus().askIndividualTypes(translator.translate(individual), true));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Map<OWLObjectProperty, Set<OWLIndividual>> getObjectPropertyRelationships(OWLIndividual individual) throws
                                                                                                               OWLReasonerException {
        throw new UnsupportedReasonerOperationException();
    }


    public Map<OWLDataProperty, Set<OWLConstant>> getDataPropertyRelationships(OWLIndividual invidual) throws
                                                                                                       OWLReasonerException {
        throw new UnsupportedReasonerOperationException();
    }


    public Set<OWLIndividual> getRelatedIndividuals(OWLIndividual subject, OWLObjectPropertyExpression property) throws
                                                                                                                 OWLReasonerException {
        throw new UnsupportedReasonerOperationException();
    }


    public Set<OWLConstant> getRelatedValues(OWLIndividual subject, OWLDataPropertyExpression property) throws
                                                                                                        OWLReasonerException {
        throw new UnsupportedReasonerOperationException();
    }


    public boolean hasDataPropertyRelationship(OWLIndividual subject, OWLDataPropertyExpression property,
                                               OWLConstant object) throws OWLReasonerException {
        return false;
    }


    public boolean hasObjectPropertyRelationship(OWLIndividual subject, OWLObjectPropertyExpression property,
                                                 OWLIndividual object) throws OWLReasonerException {
        return false;
    }


    public boolean hasType(OWLIndividual individual, OWLDescription type, boolean direct) throws OWLReasonerException {
        try {
            return getIndividuals(type, direct).contains(individual);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLObjectProperty>> getSuperProperties(OWLObjectProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            return getOWLAPISets(faCTPlusPlus.askSuperObjectProperties(translator.translate(property), true));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLObjectProperty>> getSubProperties(OWLObjectProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            return getOWLAPISets(faCTPlusPlus.askSubObjectProperties(translator.translate(property), true));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLObjectProperty>> getAncestorProperties(OWLObjectProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            return getOWLAPISets(faCTPlusPlus.askSuperObjectProperties(translator.translate(property), false));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLObjectProperty>> getDescendantProperties(OWLObjectProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            return getOWLAPISets(faCTPlusPlus.askSubObjectProperties(translator.translate(property), false));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLObjectProperty>> getInverseProperties(OWLObjectProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            OWLObjectProperty invProp = translator.getOWLObjectProperty(faCTPlusPlus.getInverseProperty(translator.translate(
                    property)));
            Set<Set<OWLObjectProperty>> props = new HashSet<Set<OWLObjectProperty>>();
            Set<OWLObjectProperty> equivalents = Collections.emptySet();
            if (invProp != null) {
                equivalents = getEquivalentProperties(invProp);
                equivalents.add(invProp);
            }
            props.add(equivalents);
            return props;
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<OWLObjectProperty> getEquivalentProperties(OWLObjectProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            ObjectPropertyPointer[] equivalentProperties = faCTPlusPlus.askEquivalentObjectProperties(translator.translate(
                    property));
            Set<OWLObjectProperty> props = new HashSet<OWLObjectProperty>();
            for (ObjectPropertyPointer ptr : equivalentProperties) {
                props.add(translator.getOWLObjectProperty(ptr));
            }
            return props;
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLDescription>> getDomains(OWLObjectProperty property) throws OWLReasonerException {
        throw new UnsupportedReasonerOperationException();
    }


    public Set<OWLDescription> getRanges(OWLObjectProperty property) throws OWLReasonerException {
        throw new UnsupportedReasonerOperationException();
    }


    public boolean isFunctional(OWLObjectProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            return getReasoner().isObjectPropertyFunctional(translator.translate(property));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isInverseFunctional(OWLObjectProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            return getReasoner().isObjectPropertyInverseFunctional(translator.translate(property));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isSymmetric(OWLObjectProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            return getReasoner().isObjectPropertySymmetric(translator.translate(property));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isTransitive(OWLObjectProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            return getReasoner().isObjectPropertyTransitive(translator.translate(property));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isReflexive(OWLObjectProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            return getReasoner().isObjectPropertyReflexive(translator.translate(property));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isIrreflexive(OWLObjectProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            return getReasoner().isObjectPropertyIrreflexive(translator.translate(property));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isAntiSymmetric(OWLObjectProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            return getReasoner().isObjectPropertyAntiSymmetric(translator.translate(property));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLDataProperty>> getSuperProperties(OWLDataProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            return getOWLAPISets(faCTPlusPlus.askSuperDataProperties(translator.translate(property), true));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLDataProperty>> getSubProperties(OWLDataProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            return getOWLAPISets(faCTPlusPlus.askSubDataProperties(translator.translate(property), true));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLDataProperty>> getAncestorProperties(OWLDataProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            return getOWLAPISets(faCTPlusPlus.askSuperDataProperties(translator.translate(property), false));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLDataProperty>> getDescendantProperties(OWLDataProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            return getOWLAPISets(faCTPlusPlus.askSubDataProperties(translator.translate(property), false));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<OWLDataProperty> getEquivalentProperties(OWLDataProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            DataPropertyPointer[] equivalentProperties = faCTPlusPlus.askEquivalentDataProperties(translator.translate(
                    property));
            Set<OWLDataProperty> props = new HashSet<OWLDataProperty>();
            for (DataPropertyPointer ptr : equivalentProperties) {
                props.add(translator.getOWLDataProperty(ptr));
            }
            return props;
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLDescription>> getDomains(OWLDataProperty property) throws OWLReasonerException {
        throw new UnsupportedReasonerOperationException();
    }


    public Set<OWLDataRange> getRanges(OWLDataProperty property) throws OWLReasonerException {
        throw new UnsupportedReasonerOperationException();
    }


    public boolean isFunctional(OWLDataProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            return faCTPlusPlus.isDataPropertyFunctional(translator.translate(property));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    /**
     * Given an array of arrays of class pointers, this method translates
     * them into a set of sets of OWLAPI <code>OWLClass</code> objects.
     * @param clsPointers The array of arrays of class pointers to be translated.
     */
    private Set<Set<OWLClass>> getOWLAPISets(ClassPointer[][] clsPointers) throws OWLReasonerException {
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
                            clses.add(getOWLDataFactory().getOWLThing());
                            logger.fine("Missing translation for Thing");
                        }
                        else if (clsPointer.equals(faCTPlusPlus.getNothing())) {
                            clses.add(getOWLDataFactory().getOWLNothing());
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


    private Set<Set<OWLObjectProperty>> getOWLAPISets(ObjectPropertyPointer[][] propertyPointers) throws
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


    private Set<Set<OWLDataProperty>> getOWLAPISets(DataPropertyPointer[][] propertyPointers) throws
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


    public FaCTPlusPlus getReasoner() {
        return faCTPlusPlus;
    }


    private int count;


    public OWLEntity getCurrentEntity() {
        return null;
    }


    public void setClassificationStarted(int classCount) {
        count = 0;
        getProgressMonitor().setSize(classCount);
        getProgressMonitor().setStarted();
    }


    public void setCurrentClass(String className) {
        count++;
        getProgressMonitor().setProgress(count);
    }


    public void setFinished() {
        getProgressMonitor().setFinished();
    }


    public boolean isCancelled() {
        return getProgressMonitor().isCancelled();
    }


    public String toString() {
        return "FaCT++";
    }
}
