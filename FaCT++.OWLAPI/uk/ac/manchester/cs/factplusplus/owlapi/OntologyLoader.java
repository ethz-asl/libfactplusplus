package uk.ac.manchester.cs.factplusplus.owlapi;

import org.semanticweb.owl.model.*;
import uk.ac.manchester.cs.factplusplus.FaCTPlusPlus;
import uk.ac.manchester.cs.factplusplus.AxiomPointer;
import uk.ac.manchester.cs.factplusplus.FaCTPlusPlusException;

import java.util.Set;
import java.util.List;
import java.util.Map;
import java.util.HashMap;
import java.util.logging.Level;
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
public class OntologyLoader {

    private static final Logger logger = Logger.getLogger(OntologyLoader.class.getName());

    private Translator translator;

    private AxiomLoader axiomLoader;

    private FaCTPlusPlusAxiomFilter filter;

    private Map<OWLAxiom, AxiomPointer> axiom2PtrMap = new HashMap<OWLAxiom, AxiomPointer>();

    private OWLOntologyChangeVisitor changeVisitor = new OWLOntologyChangeVisitor() {

        public void visit(AddAxiom addAxiom) {
            try {
                loadAxiom(addAxiom.getAxiom());
            }
            catch (FaCTPlusPlusException e) {
                throw new FaCTPlusPlusRuntimeException(e);
            }
        }


        public void visit(RemoveAxiom removeAxiom) {
            try {
                retractAxiom(removeAxiom.getAxiom());
            }
            catch (FaCTPlusPlusException e) {
                throw new FaCTPlusPlusRuntimeException(e);
            }
        }


        public void visit(SetOntologyURI setOntologyURI) {
        }
    };


    public OntologyLoader(OWLOntologyManager owlOntologyManager, Translator translator) {
        this.translator = translator;
        this.axiomLoader = new AxiomLoader(translator);
        this.filter = new FaCTPlusPlusAxiomFilter();

        try {
            // Make sure that TOP and BOTTOM are in there
            translator.translate(owlOntologyManager.getOWLDataFactory().getOWLThing());
            translator.translate(owlOntologyManager.getOWLDataFactory().getOWLNothing());
        }
        catch (OWLException e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }

        logger.setLevel(Level.INFO);
    }


    public void loadOntologies(Set<OWLOntology> ontologies) throws FaCTPlusPlusException {
        try {
            for (OWLOntology ont : ontologies) {
                // make sure entities that do not occur in logical axioms are still understood by FaCT++
                for (OWLClass cls : ont.getReferencedClasses()) {
                    translator.translate(cls);
                }
                for (OWLObjectProperty prop : ont.getReferencedObjectProperties()) {
                    translator.translate(prop);
                }
                for (OWLDataProperty prop : ont.getReferencedDataProperties()) {
                    translator.translate(prop);
                }
                for (OWLIndividual ind : ont.getReferencedIndividuals()) {
                    translator.translate(ind);
                }
                loadAxioms(ont.getAxioms());
            }
        }
        catch (OWLException e) {
            throw new FaCTPlusPlusRuntimeException(e);
        }
    }


    public void loadAxioms(Set<OWLAxiom> axioms) throws FaCTPlusPlusException {
        for (OWLAxiom axiom : axioms) {
            loadAxiom(axiom);
        }
    }


    public void loadAxiom(OWLAxiom axiom) throws FaCTPlusPlusException {
        boolean added = false;
        if (axiom.isLogicalAxiom()) {
            if (filter.passes(axiom)) {
                axiom2PtrMap.put(axiom, axiomLoader.load(axiom));
                added = true;
            }
            else {
                logger.info("WARNING! Ignoring axiom: " + axiom + " [" + filter.getReason() + "]");
            }
        }

        if (!added){
            // TODO translate entities in case they don't participate in any logical axioms?
        }
    }


    public void retractAxiom(OWLAxiom axiom) throws FaCTPlusPlusException {
        AxiomPointer ptr = axiom2PtrMap.get(axiom);
        if (ptr != null){
            translator.getFaCTPlusPlus().retract(ptr);
            axiom2PtrMap.remove(axiom);
        }

        // @@TODO what about entities that FaCT++ still knows about but are no longer known by the model
    }

    
    public void applyChanges(List<OWLOntologyChange> changes) throws OWLException {
        for (OWLOntologyChange change : changes){
            change.accept(changeVisitor);
        }
    }
}
