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

    private FaCTPlusPlusReasonerException lastException;

    private OntologyLoader loader;

    public enum State{
        EMPTY,
        UNCLASSIFIED_IN_SYNC,
        UNCLASSIFIED_DIRTY,
        CLASSIFIED_IN_SYNC,
        CLASSIFIED_DIRTY,
        FAIL
    }

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


    private State state = State.EMPTY;

//    /**
//     * A flag which indicates if the contents of the reasoner are synchronised
//     * with the current set of ontologies.
//     */
//    private boolean synchronised = true;

//    /**
//     * If the kernel has been classified, regardless of whether changes have been
//     * made to the ontologies since
//     */
//    private boolean classified = false;


    private TranslatorUtils translatorUtils;


    /**
     * If true, only changes since the last classify are told to the kernel
     * If false, the kernel is completely refreshed on a classify
     */
    private boolean incremental = true;


    /**
     * Changes since the last classify()
     */
    private List<OWLOntologyChange> changes = new ArrayList<OWLOntologyChange>();


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

        loader = new OntologyLoader(getOWLOntologyManager(), translator);

        translatorUtils = new TranslatorUtils(owlOntologyManager, translator);
    }


    public String toString() {
        return "FaCT++";
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


    public void setModeIncremental(boolean incremental) {
        this.incremental = incremental;
    }


    public final State getState(){
        return state;
    }


    public FaCTPlusPlus getReasoner() {
        return faCTPlusPlus;
    }


    public void classify() throws OWLReasonerException {
        while(state != State.CLASSIFIED_IN_SYNC){
            switch(state){
                case FAIL:
                    throw new FaCTPlusPlusReasonerException(lastException);
                case EMPTY:
                    load();
                    break;
                case UNCLASSIFIED_DIRTY: // fallthrough
                case CLASSIFIED_DIRTY:
                    resync();
                    break;
                case UNCLASSIFIED_IN_SYNC:
                    doClassify();
                    break;
                case CLASSIFIED_IN_SYNC: // do nothing - already in sync
            }
        }
    }


    private void load() throws OWLReasonerException {
        try {
            faCTPlusPlus.setProgressMonitor(this);

            loader.loadOntologies(getLoadedOntologies());

            state = State.UNCLASSIFIED_IN_SYNC;
        }
        catch (Exception e) {
            lastException = new FaCTPlusPlusReasonerException(e);
            state = State.FAIL;
        }
    }


    private void resync() {
        try {
            if (incremental){
                if (!changes.isEmpty()){
                    loader.applyChanges(changes);
                    changes.clear();
                    state = State.UNCLASSIFIED_IN_SYNC;
                }
            }
            else{
                clear();
                load();
            }
        }
        catch (Exception e) {
            lastException = new FaCTPlusPlusReasonerException(e);
            state = State.FAIL;
        }
    }


    private void doClassify() {
        try {
            getReasoner().classify();
            state = State.CLASSIFIED_IN_SYNC;
        }
        catch (Exception e) {
            lastException = new FaCTPlusPlusReasonerException(e);
            state = State.FAIL;
        }
    }


    private void clear(){
        try {
            getReasoner().clearKernel();
            // Reset the translator, because the pointer mappings that it holds
            // are no longer valid
            translator.reset();
            changes.clear();

            state = State.EMPTY;
        }
        catch (Exception e) {
            lastException = new FaCTPlusPlusReasonerException(e);
            state = State.FAIL;
        }
    }


    protected void ontologiesCleared() throws OWLReasonerException {
        clear();
        if (state == State.FAIL && lastException != null){
            throw lastException;
        }
    }


    /**
     * Overloaded to make sure declarations are passed to FaCT++ (as they are not considered logical axioms by the OWL API)
     * @param changes changes to the ontologies
     * @throws OWLException
     */
    public void ontologiesChanged(List<? extends OWLOntologyChange> changes) throws OWLException {
        List<OWLOntologyChange> filteredChanges = null;
        // Filter the changes so that we get the changes that only apply to the ontologies that we
        // know about and the changes which aren't addition/removal of annotation axioms
        for (OWLOntologyChange change : changes) {
            if (getLoadedOntologies().contains(change.getOntology())) {
                if (change.isAxiomChange()) {
                    OWLAxiomChange axiomChange = (OWLAxiomChange) change;
                    if (filteredChanges == null) {
                        filteredChanges = new ArrayList<OWLOntologyChange>();
                    }
                    filteredChanges.add(axiomChange);
                }
            }
        }
        if (filteredChanges != null) {
            handleOntologyChanges(filteredChanges);
        }
    }


    /**
     * Called when the set of ontologies loaded has changed
     * Just resync in this case
     * @throws OWLReasonerException
     */
    protected void ontologiesChanged() throws OWLReasonerException {
        clear();
        if (state == State.FAIL && lastException != null){
            throw lastException;
        }
    }


    protected void handleOntologyChanges(List<OWLOntologyChange> changes) throws OWLReasonerException {
        switch (state){
            case FAIL: throw lastException;

            case UNCLASSIFIED_DIRTY: // fallthrough
            case UNCLASSIFIED_IN_SYNC:
                if (incremental){
                    this.changes.addAll(changes);
                }
                state = State.UNCLASSIFIED_DIRTY;
                break;
            case CLASSIFIED_DIRTY: // fallthrough
            case CLASSIFIED_IN_SYNC:
                if (incremental){
                    this.changes.addAll(changes);
                }
                state = State.CLASSIFIED_DIRTY;
                break;
            case EMPTY: // do nothing
        }
    }


    private void autoSynchronise() throws OWLReasonerException {
        switch(state){
            case FAIL: throw lastException;
            case EMPTY:
                if (autoSync){
                    load();
                }
                break;
            case UNCLASSIFIED_DIRTY: // fallthrough
            case CLASSIFIED_DIRTY:
                if (autoSync){
                    resync();
                }
                break;
        }
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////


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


    public boolean isClassified() throws OWLReasonerException {
        return state == State.CLASSIFIED_IN_SYNC;
    }


    public boolean isRealised() throws OWLReasonerException {
        return isClassified();
    }


    public void realise() throws OWLReasonerException {
        try {
            autoSynchronise();
            faCTPlusPlus.realise();
        }
        catch (FaCTPlusPlusException e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    /**
     * Checks to see if the specified ontology is consistent
     * @param ontology The ontology to check
     * @return <code>true</code> if the ontology is consistent,
     *         or <code>false</code> if the ontology is not consistent.
     */
    public boolean isConsistent(OWLOntology ontology) throws OWLReasonerException {
        try {
            autoSynchronise();
            return faCTPlusPlus.isKBConsistent();
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isSubClassOf(OWLDescription owlDescription, OWLDescription owlDescription1) throws
            OWLReasonerException {
        try {
            autoSynchronise();
            return getReasoner().isClassSubsumedBy(translator.translate(owlDescription),
                                                   translator.translate(owlDescription1));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isEquivalentClass(OWLDescription owlDescription, OWLDescription owlDescription1) throws
            OWLReasonerException {
        try {
            autoSynchronise();
            return getReasoner().isClassEquivalentTo(translator.translate(owlDescription),
                                                     translator.translate(owlDescription1));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isSatisfiable(OWLDescription owlDescription) throws OWLReasonerException {
        try {
            autoSynchronise();
            if (!faCTPlusPlus.isKBConsistent()) {
                return false;
            }
            translatorUtils.checkParams(owlDescription);
            return getReasoner().isClassSatisfiable(translator.translate(owlDescription));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLClass>> getSuperClasses(OWLDescription owlDescription) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(owlDescription);

            ClassPointer[][] result = getReasoner().askSuperClasses(translator.translate(owlDescription), true);
            return translatorUtils.getOWLAPISets(result);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLClass>> getAncestorClasses(OWLDescription owlDescription) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(owlDescription);

            ClassPointer[][] result = getReasoner().askSuperClasses(translator.translate(owlDescription), false);
            return translatorUtils.getOWLAPISets(result);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLClass>> getSubClasses(OWLDescription owlDescription) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(owlDescription);

            ClassPointer[][] result = getReasoner().askSubClasses(translator.translate(owlDescription), true);
            return translatorUtils.getOWLAPISets(result);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLClass>> getDescendantClasses(OWLDescription owlDescription) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(owlDescription);

            ClassPointer[][] result = getReasoner().askSubClasses(translator.translate(owlDescription), false);
            return translatorUtils.getOWLAPISets(result);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<OWLClass> getEquivalentClasses(OWLDescription owlDescription) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(owlDescription);

            return translatorUtils.getOWLAPISet(getReasoner().askEquivalentClasses(translator.translate(owlDescription)));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isInstanceOf(OWLIndividual owlIndividual, OWLDescription owlDescription) throws
            OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(owlIndividual, owlDescription);

            return getReasoner().isInstanceOf(translator.translate(owlIndividual),
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
        autoSynchronise();
        return getEquivalentClasses(getOWLDataFactory().getOWLNothing());
    }


    public Set<OWLIndividual> getIndividuals(OWLDescription owlDescription, boolean direct) throws
            OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(owlDescription);

            return translatorUtils.getOWLAPISet(getReasoner().askInstances(translator.translate(owlDescription), direct));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLClass>> getTypes(OWLIndividual individual, boolean direct) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(individual);

            return translatorUtils.getOWLAPISets(getReasoner().askIndividualTypes(translator.translate(individual), direct));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Map<OWLObjectProperty, Set<OWLIndividual>> getObjectPropertyRelationships(OWLIndividual individual) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(individual);

            Map<OWLObjectProperty, Set<OWLIndividual>> results = new HashMap<OWLObjectProperty, Set<OWLIndividual>>();
            ObjectPropertyPointer[] props = getReasoner().askObjectProperties(translator.translate(individual));
            for (ObjectPropertyPointer prop : props){
                Set<OWLIndividual> inds = translatorUtils.getOWLAPISet(getReasoner().askRelatedIndividuals(translator.translate(individual), prop));
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
            autoSynchronise();
            translatorUtils.checkParams(individual);

            Map<OWLDataProperty, Set<OWLConstant>> results = new HashMap<OWLDataProperty, Set<OWLConstant>>();
            DataPropertyPointer[] props = getReasoner().askDataProperties(translator.translate(individual));
            for (DataPropertyPointer prop : props){
                Set<OWLConstant> inds = translatorUtils.getOWLAPISet(getReasoner().askRelatedValues(translator.translate(individual), prop));
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
            autoSynchronise();
            translatorUtils.checkParams(subject, property);

            return translatorUtils.getOWLAPISet(getReasoner().askRelatedIndividuals(translator.translate(subject), translator.translate(property)));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<OWLConstant> getRelatedValues(OWLIndividual subject, OWLDataPropertyExpression property) throws
            OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(subject, property);

            return translatorUtils.getOWLAPISet(getReasoner().askRelatedValues(translator.translate(subject), translator.translate(property)));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean hasDataPropertyRelationship(OWLIndividual subject, OWLDataPropertyExpression property,
                                               OWLConstant object) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(subject, property);

            return getReasoner().hasDataPropertyRelationship(translator.translate(subject), translator.translate(property), translator.translate(object));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean hasObjectPropertyRelationship(OWLIndividual subject, OWLObjectPropertyExpression property,
                                                 OWLIndividual object) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(subject, object);

            return getReasoner().hasObjectPropertyRelationship(translator.translate(subject), translator.translate(property), translator.translate(object));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean hasType(OWLIndividual individual, OWLDescription type, boolean direct) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(individual, type);

            return getIndividuals(type, direct).contains(individual);
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLObjectProperty>> getSuperProperties(OWLObjectProperty property) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(property);

            return translatorUtils.getOWLAPISets(faCTPlusPlus.askSuperObjectProperties(translator.translate(property), true));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLObjectProperty>> getSubProperties(OWLObjectProperty property) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(property);

            return translatorUtils.getOWLAPISets(faCTPlusPlus.askSubObjectProperties(translator.translate(property), true));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLObjectProperty>> getAncestorProperties(OWLObjectProperty property) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(property);

            return translatorUtils.getOWLAPISets(faCTPlusPlus.askSuperObjectProperties(translator.translate(property), false));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLObjectProperty>> getDescendantProperties(OWLObjectProperty property) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(property);

            return translatorUtils.getOWLAPISets(faCTPlusPlus.askSubObjectProperties(translator.translate(property), false));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLObjectProperty>> getInverseProperties(OWLObjectProperty property) throws OWLReasonerException {
        try {
            autoSynchronise();
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
            autoSynchronise();
            translatorUtils.checkParams(property);

            return translatorUtils.getOWLAPISet(faCTPlusPlus.askEquivalentObjectProperties(translator.translate(property)));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLDescription>> getDomains(OWLObjectProperty property) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(property);

            return translatorUtils.getOWLAPIDescriptionSets(getReasoner().askObjectPropertyDomain(translator.translate(property)));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<OWLDescription> getRanges(OWLObjectProperty property) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(property);

            return translatorUtils.getOWLAPIDescriptionSet(getReasoner().askObjectPropertyRange(translator.translate(property)));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isFunctional(OWLObjectProperty property) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(property);

            return getReasoner().isObjectPropertyFunctional(translator.translate(property));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isInverseFunctional(OWLObjectProperty property) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(property);

            return getReasoner().isObjectPropertyInverseFunctional(translator.translate(property));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isSymmetric(OWLObjectProperty property) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(property);

            return getReasoner().isObjectPropertySymmetric(translator.translate(property));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isTransitive(OWLObjectProperty property) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(property);

            return getReasoner().isObjectPropertyTransitive(translator.translate(property));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isReflexive(OWLObjectProperty property) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(property);

            return getReasoner().isObjectPropertyReflexive(translator.translate(property));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isIrreflexive(OWLObjectProperty property) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(property);

            return getReasoner().isObjectPropertyIrreflexive(translator.translate(property));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isAntiSymmetric(OWLObjectProperty property) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(property);

            return getReasoner().isObjectPropertyAntiSymmetric(translator.translate(property));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLDataProperty>> getSuperProperties(OWLDataProperty property) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(property);

            return translatorUtils.getOWLAPISets(faCTPlusPlus.askSuperDataProperties(translator.translate(property), true));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLDataProperty>> getSubProperties(OWLDataProperty property) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(property);

            return translatorUtils.getOWLAPISets(faCTPlusPlus.askSubDataProperties(translator.translate(property), true));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLDataProperty>> getAncestorProperties(OWLDataProperty property) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(property);

            return translatorUtils.getOWLAPISets(faCTPlusPlus.askSuperDataProperties(translator.translate(property), false));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLDataProperty>> getDescendantProperties(OWLDataProperty property) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(property);

            return translatorUtils.getOWLAPISets(faCTPlusPlus.askSubDataProperties(translator.translate(property), false));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<OWLDataProperty> getEquivalentProperties(OWLDataProperty property) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(property);

            return translatorUtils.getOWLAPISet(faCTPlusPlus.askEquivalentDataProperties(translator.translate(property)));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<Set<OWLDescription>> getDomains(OWLDataProperty property) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(property);

            return translatorUtils.getOWLAPIDescriptionSets(getReasoner().askDataPropertyDomain(translator.translate(property)));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public Set<OWLDataRange> getRanges(OWLDataProperty property) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(property);

            return translatorUtils.getOWLAPISet(new DataTypeExpressionPointer[]{getReasoner().askDataPropertyRange(translator.translate(property))});
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


    public boolean isFunctional(OWLDataProperty property) throws OWLReasonerException {
        try {
            autoSynchronise();
            translatorUtils.checkParams(property);

            return faCTPlusPlus.isDataPropertyFunctional(translator.translate(property));
        }
        catch (Exception e) {
            throw new FaCTPlusPlusReasonerException(e);
        }
    }


/////////////////////////////   Monitor   //////////////////////////////////////

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
}
