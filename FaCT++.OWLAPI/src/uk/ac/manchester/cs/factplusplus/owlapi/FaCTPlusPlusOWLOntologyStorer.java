package uk.ac.manchester.cs.factplusplus.owlapi;

import org.semanticweb.owl.model.*;
import org.semanticweb.owl.io.OWLOntologyOutputTarget;

import java.io.*;
import java.net.URI;
import java.util.logging.Logger;

import sun.reflect.generics.reflectiveObjects.NotImplementedException;
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
 * Date: 13-May-2007<br><br>
 */
public class FaCTPlusPlusOWLOntologyStorer implements OWLOntologyStorer {

    private Logger logger = Logger.getLogger(FaCTPlusPlusOWLOntologyStorer.class.toString());

    public boolean canStoreOntology(OWLOntologyFormat ontologyFormat) {
        return ontologyFormat.equals(new FaCTPlusPlusOntologyFormat());
    }


    public void storeOntology(OWLOntologyManager manager, OWLOntology ontology, URI physicalURI,
                              OWLOntologyFormat ontologyFormat) throws OWLOntologyStorageException {
        try {
            storeOntology(manager, ontology, new FileOutputStream(new File(physicalURI)), ontologyFormat);
        }
        catch (FileNotFoundException e) {
            throw new OWLOntologyStorageException(e);
        }
    }


    public void storeOntology(OWLOntologyManager manager, OWLOntology ontology, OWLOntologyOutputTarget outputTarget,
                              OWLOntologyFormat ontologyFormat) throws OWLOntologyStorageException {
            storeOntology(manager, ontology, outputTarget.getOutputStream(), ontologyFormat);
    }


    private void storeOntology(OWLOntologyManager manager, OWLOntology ontology, OutputStream out,
                              OWLOntologyFormat ontologyFormat) throws OWLOntologyStorageException {
        try {
            logger.info("FaCT++ ignoring ontology format requested. Not supported");
            BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(out));
            FaCTPlusPlusRenderer ren = new FaCTPlusPlusRenderer(writer);
            ontology.accept(ren);
            writer.flush();
            writer.close();
        }
        catch (IOException e) {
            throw new FaCTPlusPlusRendererException(e);
        }
    }
}
