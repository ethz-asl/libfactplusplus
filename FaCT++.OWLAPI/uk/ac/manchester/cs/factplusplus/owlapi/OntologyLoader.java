package uk.ac.manchester.cs.factplusplus.owlapi;

import org.semanticweb.owl.model.*;
import uk.ac.manchester.cs.factplusplus.FaCTPlusPlus;

import java.util.Set;
import java.util.logging.Logger;
import java.util.logging.Level;

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

    private OWLOntologyManager owlOntologyManager;


    public OntologyLoader(OWLOntologyManager owlOntologyManager, Translator translator) {
        this.owlOntologyManager = owlOntologyManager;
        this.translator = translator;
        logger.setLevel(Level.INFO);
    }


    public void loadOntologies(Set<OWLOntology> ontologies, FaCTPlusPlus faCTPlusPlus) throws OWLException {

        // Make sure that TOP and BOTTOM are in there
        translator.translate(owlOntologyManager.getOWLDataFactory().getOWLThing());
        translator.translate(owlOntologyManager.getOWLDataFactory().getOWLNothing());

        AxiomLoader axiomLoader = new AxiomLoader(translator, faCTPlusPlus);
        FaCTPlusPlusAxiomFilter filter = new FaCTPlusPlusAxiomFilter();

        for (OWLOntology ont : ontologies) {
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
            for (OWLAxiom axiom : ont.getAxioms()) {
                if (axiom.isLogicalAxiom()) {
                    if (filter.passes(axiom)) {
                        axiom.accept(axiomLoader);
                    }
                    else {
                        logger.info("WARNING! Ignoring axiom: " + axiom + " [" + filter.getReason() + "]");
                    }
                }
            }
        }
    }
}
