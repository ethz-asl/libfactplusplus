package uk.ac.manchester.cs.factplusplus.protege;

import org.semanticweb.owl.inference.MonitorableOWLReasoner;
import org.semanticweb.owl.model.OWLEntity;
import org.semanticweb.owl.util.OWLReasonerMediator;
import org.semanticweb.owl.util.ProgressMonitor;
import uk.ac.manchester.cs.factplusplus.owlapi.Reasoner;
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


/**
 * Author: Matthew Horridge<br>
 * The University Of Manchester<br>
 * Bio-Health Informatics Group<br>
 * Date: 11-May-2007<br><br>
 *
 * No need to add ontology listeners:
 * When ontologies have changed since classification, FaCT++ reports isClassified() = true
 */
public class FaCTPlusPlusReasonerWrapper extends OWLReasonerMediator implements MonitorableOWLReasoner {


    public FaCTPlusPlusReasonerWrapper(Reasoner factPlusPlus) throws Exception {
        super(factPlusPlus);
        setSilentUndefinedEntityHandling(true);
    }


    public void setProgressMonitor(ProgressMonitor progressMonitor) {
        if (getKernel() instanceof MonitorableOWLReasoner) {
            ((MonitorableOWLReasoner) getKernel()).setProgressMonitor(progressMonitor);
        }
    }


    public OWLEntity getCurrentEntity() {
        return null;
    }


    public String toString() {
        return "FaCT++";
    }
}
