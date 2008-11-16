package uk.ac.manchester.cs.factplusplus.owlapi;

import org.semanticweb.owl.inference.MonitorableOWLReasonerAdapter;
import org.semanticweb.owl.inference.OWLReasonerException;
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
     * if true, the reasoner will try to resync automatically when query methods are called
     * (when required)
     */
    private boolean autoSync = false;

    /**
     * A flag which indicates if the contents of the reasoner are synchronised
     * with the current set of ontologies.
     */
    private boolean synchronised = true;

    /**
     * If the kernel has been classified, regardless of whether changes have been
     * made to the ontologies since
     */
    private boolean classified = false;


    private TranslatorUtils translatorUtils;

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

        translatorUtils = new TranslatorUtils(owlOntologyManager, faCTPlusPlus, translator);
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
        return classified && synchronised;
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
        return !autoSync;
    }


    public void setSychroniseOnlyOnClassify(boolean sychroniseOnlyOnClassify) {
        this.autoSync = !sychroniseOnlyOnClassify;
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


    private void ensureSynchronised(boolean force) throws OWLReasonerException {
        if (!synchronised &&
            (autoSync || force)) {
            synchronised = true; // set sync first in case of errors in classify
            resync();
        }
    }


    private void ensureSynchronised() throws OWLReasonerException {
        ensureSynchronised(false);
    }


    private void markForResyncronisation() {
        synchronised = false;
    }


    private void resync() throws OWLReasonerException {
        try {
            getFaCTPlusPlus().clearKernel();
            classified = false;
            // Reset the translator, because the pointer mappings that it holds
            // are no longer valid

            translator.reset();

            faCTPlusPlus.setProgressMonitor(this);
            // Now reload the ontologies
            OntologyLoader loader = new OntologyLoader(getOWLOntologyManager(), translator);
            loader.loadOntologies(getLoadedOntologies(), faCTPlusPlus);
//            progressMonitor = new FaCTPPProgMon();
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isClassified() throws OWLReasonerException {
        return classified && synchronised;
    }


    public void classify() throws OWLReasonerException {
        try {
            markForResyncronisation();
            ensureSynchronised(true);
            getReasoner().classify();
            classified = true;
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
            classified = false;
            translator.reset();
            markForResyncronisation();
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    protected void ontologiesChanged() throws OWLReasonerException {
        try {
            markForResyncronisation();
            translator.reset();
        }
        catch (OWLException e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
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


    public boolean isSatisfiable(OWLDescription owlDescription) throws OWLReasonerException {
        try {
            ensureSynchronised();
            if (!faCTPlusPlus.isKBConsistent()) {
                return false;
            }
            translatorUtils.checkParams(owlDescription);
            return getFaCTPlusPlus().isClassSatisfiable(translator.translate(owlDescription));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLClass>> getSuperClasses(OWLDescription owlDescription) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(owlDescription);

            ClassPointer[][] result = getFaCTPlusPlus().askSuperClasses(translator.translate(owlDescription), true);
            return translatorUtils.getOWLAPISets(result);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLClass>> getAncestorClasses(OWLDescription owlDescription) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(owlDescription);

            ClassPointer[][] result = getFaCTPlusPlus().askSuperClasses(translator.translate(owlDescription), false);
            return translatorUtils.getOWLAPISets(result);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLClass>> getSubClasses(OWLDescription owlDescription) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(owlDescription);

            ClassPointer[][] result = getFaCTPlusPlus().askSubClasses(translator.translate(owlDescription), true);
            return translatorUtils.getOWLAPISets(result);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLClass>> getDescendantClasses(OWLDescription owlDescription) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(owlDescription);

            ClassPointer[][] result = getFaCTPlusPlus().askSubClasses(translator.translate(owlDescription), false);
            return translatorUtils.getOWLAPISets(result);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<OWLClass> getEquivalentClasses(OWLDescription owlDescription) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(owlDescription);

            return translatorUtils.getOWLAPISet(getFaCTPlusPlus().askEquivalentClasses(translator.translate(owlDescription)));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isInstanceOf(OWLIndividual owlIndividual, OWLDescription owlDescription) throws
            OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(owlIndividual, owlDescription);

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
            translatorUtils.checkParams(owlDescription);

            return translatorUtils.getOWLAPISet(getFaCTPlusPlus().askInstances(translator.translate(owlDescription), direct));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLClass>> getTypes(OWLIndividual individual, boolean direct) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(individual);

            return translatorUtils.getOWLAPISets(getFaCTPlusPlus().askIndividualTypes(translator.translate(individual), direct));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Map<OWLObjectProperty, Set<OWLIndividual>> getObjectPropertyRelationships(OWLIndividual individual) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(individual);

            Map<OWLObjectProperty, Set<OWLIndividual>> results = new HashMap<OWLObjectProperty, Set<OWLIndividual>>();
            ObjectPropertyPointer[] props = getFaCTPlusPlus().askObjectProperties(translator.translate(individual));
            for (ObjectPropertyPointer prop : props){
                Set<OWLIndividual> inds = translatorUtils.getOWLAPISet(getFaCTPlusPlus().askRelatedIndividuals(translator.translate(individual), prop));
                if(!inds.isEmpty()){
                    results.put(translator.getOWLObjectProperty(prop), inds);
                }
            }
            return results;
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Map<OWLDataProperty, Set<OWLConstant>> getDataPropertyRelationships(OWLIndividual individual) throws
            OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(individual);

            Map<OWLDataProperty, Set<OWLConstant>> results = new HashMap<OWLDataProperty, Set<OWLConstant>>();
            DataPropertyPointer[] props = getFaCTPlusPlus().askDataProperties(translator.translate(individual));
            for (DataPropertyPointer prop : props){
                Set<OWLConstant> inds = translatorUtils.getOWLAPISet(getFaCTPlusPlus().askRelatedValues(translator.translate(individual), prop));
                if(!inds.isEmpty()){
                    results.put(translator.getOWLDataProperty(prop), inds);
                }
            }
            return results;
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<OWLIndividual> getRelatedIndividuals(OWLIndividual subject, OWLObjectPropertyExpression property) throws
            OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(subject, property);

            return translatorUtils.getOWLAPISet(getFaCTPlusPlus().askRelatedIndividuals(translator.translate(subject), translator.translate(property)));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<OWLConstant> getRelatedValues(OWLIndividual subject, OWLDataPropertyExpression property) throws
            OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(subject, property);

            return translatorUtils.getOWLAPISet(getFaCTPlusPlus().askRelatedValues(translator.translate(subject), translator.translate(property)));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean hasDataPropertyRelationship(OWLIndividual subject, OWLDataPropertyExpression property,
                                               OWLConstant object) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(subject, property);

            return getFaCTPlusPlus().hasDataPropertyRelationship(translator.translate(subject), translator.translate(property), translator.translate(object));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean hasObjectPropertyRelationship(OWLIndividual subject, OWLObjectPropertyExpression property,
                                                 OWLIndividual object) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(subject, object);

            return getFaCTPlusPlus().hasObjectPropertyRelationship(translator.translate(subject), translator.translate(property), translator.translate(object));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean hasType(OWLIndividual individual, OWLDescription type, boolean direct) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(individual, type);

            return getIndividuals(type, direct).contains(individual);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLObjectProperty>> getSuperProperties(OWLObjectProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(property);

            return translatorUtils.getOWLAPISets(faCTPlusPlus.askSuperObjectProperties(translator.translate(property), true));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLObjectProperty>> getSubProperties(OWLObjectProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(property);

            return translatorUtils.getOWLAPISets(faCTPlusPlus.askSubObjectProperties(translator.translate(property), true));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLObjectProperty>> getAncestorProperties(OWLObjectProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(property);

            return translatorUtils.getOWLAPISets(faCTPlusPlus.askSuperObjectProperties(translator.translate(property), false));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLObjectProperty>> getDescendantProperties(OWLObjectProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(property);

            return translatorUtils.getOWLAPISets(faCTPlusPlus.askSubObjectProperties(translator.translate(property), false));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLObjectProperty>> getInverseProperties(OWLObjectProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(property);

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
            translatorUtils.checkParams(property);

            return translatorUtils.getOWLAPISet(faCTPlusPlus.askEquivalentObjectProperties(translator.translate(property)));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLDescription>> getDomains(OWLObjectProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(property);

            return translatorUtils.getOWLAPIDescriptionSets(getFaCTPlusPlus().askObjectPropertyDomain(translator.translate(property)));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<OWLDescription> getRanges(OWLObjectProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(property);

            return translatorUtils.getOWLAPIDescriptionSet(getFaCTPlusPlus().askObjectPropertyRange(translator.translate(property)));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isFunctional(OWLObjectProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(property);

            return getReasoner().isObjectPropertyFunctional(translator.translate(property));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isInverseFunctional(OWLObjectProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(property);

            return getReasoner().isObjectPropertyInverseFunctional(translator.translate(property));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isSymmetric(OWLObjectProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(property);

            return getReasoner().isObjectPropertySymmetric(translator.translate(property));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isTransitive(OWLObjectProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(property);

            return getReasoner().isObjectPropertyTransitive(translator.translate(property));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isReflexive(OWLObjectProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(property);

            return getReasoner().isObjectPropertyReflexive(translator.translate(property));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isIrreflexive(OWLObjectProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(property);

            return getReasoner().isObjectPropertyIrreflexive(translator.translate(property));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isAntiSymmetric(OWLObjectProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(property);

            return getReasoner().isObjectPropertyAntiSymmetric(translator.translate(property));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLDataProperty>> getSuperProperties(OWLDataProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(property);

            return translatorUtils.getOWLAPISets(faCTPlusPlus.askSuperDataProperties(translator.translate(property), true));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLDataProperty>> getSubProperties(OWLDataProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(property);

            return translatorUtils.getOWLAPISets(faCTPlusPlus.askSubDataProperties(translator.translate(property), true));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLDataProperty>> getAncestorProperties(OWLDataProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(property);

            return translatorUtils.getOWLAPISets(faCTPlusPlus.askSuperDataProperties(translator.translate(property), false));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLDataProperty>> getDescendantProperties(OWLDataProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(property);

            return translatorUtils.getOWLAPISets(faCTPlusPlus.askSubDataProperties(translator.translate(property), false));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<OWLDataProperty> getEquivalentProperties(OWLDataProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(property);

            return translatorUtils.getOWLAPISet(faCTPlusPlus.askEquivalentDataProperties(translator.translate(property)));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLDescription>> getDomains(OWLDataProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(property);

            return translatorUtils.getOWLAPIDescriptionSets(getFaCTPlusPlus().askDataPropertyDomain(translator.translate(property)));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<OWLDataRange> getRanges(OWLDataProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(property);

            return translatorUtils.getOWLAPISet(new DataTypeExpressionPointer[]{getFaCTPlusPlus().askDataPropertyRange(translator.translate(property))});
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isFunctional(OWLDataProperty property) throws OWLReasonerException {
        try {
            ensureSynchronised();
            translatorUtils.checkParams(property);
                        
            return faCTPlusPlus.isDataPropertyFunctional(translator.translate(property));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
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
