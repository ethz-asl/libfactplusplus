package uk.ac.manchester.cs.factplusplus.protege;

/*
 * Copyright (C) 2007, University of Manchester
 *
 * Modifications to the initial code base are copyright of their
 * respective authors, or their employers as appropriate.  Authorship
 * of the modifications may be determined from the ChangeLog placed at
 * the end of this file.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


import org.protege.editor.owl.model.inference.ProtegeOWLReasonerFactoryAdapter;
import org.semanticweb.owlapi.model.OWLOntology;
import org.semanticweb.owlapi.model.OWLRuntimeException;
import org.semanticweb.owlapi.reasoner.OWLReasoner;
import org.semanticweb.owlapi.reasoner.OWLReasonerConfiguration;
import org.semanticweb.owlapi.reasoner.ReasonerProgressMonitor;
import org.semanticweb.owlapi.reasoner.SimpleConfiguration;

import javax.swing.*;

/**
 * Author: Matthew Horridge<br>
 * The University Of Manchester<br>
 * Bio-Health Informatics Group<br>
 * Date: 11-May-2007<br><br>
 */
public class FaCTPlusPlusFactory extends ProtegeOWLReasonerFactoryAdapter {

    private uk.ac.manchester.cs.factplusplus.owlapiv3.FaCTPlusPlusReasonerFactory fac;


    public OWLReasoner createReasoner(OWLOntology owlOntology, ReasonerProgressMonitor reasonerProgressMonitor) {
        try {
            OWLReasonerConfiguration conf = new SimpleConfiguration(reasonerProgressMonitor);
            return fac.createNonBufferingReasoner(owlOntology, conf);
        }
        catch (UnsatisfiedLinkError e) {
            JOptionPane.showMessageDialog(null,
                                          "FaCT++ requires platform specific libraries which cannot be found.\n" +
                                          "Supported platforms are OS X, Windows and Linux.",
                                          "Missing libraries or platform not supported",
                                          JOptionPane.ERROR_MESSAGE);
            throw new OWLRuntimeException(e);
        }
        catch (Exception e) {
            throw new OWLRuntimeException(e);
        }
    }


    public void initialise() throws Exception {
        fac = new uk.ac.manchester.cs.factplusplus.owlapiv3.FaCTPlusPlusReasonerFactory();
    }


    public void dispose() throws Exception {
        fac = null;
    }
}
