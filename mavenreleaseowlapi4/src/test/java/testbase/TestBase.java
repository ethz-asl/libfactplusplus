package testbase;

import org.semanticweb.owlapi.reasoner.OWLReasonerFactory;

import uk.ac.manchester.cs.factplusplus.owlapiv3.FaCTPlusPlusReasonerFactory;

@SuppressWarnings("javadoc")
public class TestBase {

    protected static OWLReasonerFactory factory() {
        return new FaCTPlusPlusReasonerFactory();
    }
}
