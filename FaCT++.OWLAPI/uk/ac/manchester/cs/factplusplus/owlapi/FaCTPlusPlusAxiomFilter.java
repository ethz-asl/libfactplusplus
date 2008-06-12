package uk.ac.manchester.cs.factplusplus.owlapi;

import org.semanticweb.owl.model.*;
import org.semanticweb.owl.util.OWLAxiomFilter;
import org.semanticweb.owl.vocab.OWLRDFVocabulary;
import org.semanticweb.owl.vocab.XSDVocabulary;

import java.net.URI;
import java.util.HashSet;
import java.util.Set;
/*
 * Copyright (C) 2006, University of Manchester
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
 * Date: 29-Nov-2006<br><br>
 */
public class FaCTPlusPlusAxiomFilter implements OWLAxiomFilter, OWLAxiomVisitor, OWLDescriptionVisitor, OWLEntityVisitor, OWLDataVisitor {

    private boolean passes = false;

    private boolean passDataStuff = false;

    private Set<URI> supportedDataTypes;

    private String reason;


    public FaCTPlusPlusAxiomFilter() {
        supportedDataTypes = new HashSet<URI>();
        supportedDataTypes.add(XSDVocabulary.INT.getURI());
        supportedDataTypes.add(XSDVocabulary.INTEGER.getURI());
        supportedDataTypes.add(XSDVocabulary.NON_NEGATIVE_INTEGER.getURI());
        supportedDataTypes.add(XSDVocabulary.DOUBLE.getURI());
        supportedDataTypes.add(XSDVocabulary.FLOAT.getURI());
        supportedDataTypes.add(XSDVocabulary.INTEGER.getURI());
        supportedDataTypes.add(XSDVocabulary.STRING.getURI());
        supportedDataTypes.add(OWLRDFVocabulary.RDFS_LITERAL.getURI());
    }


    public boolean isSupportedDatatype(URI datatypeURI) {
        return supportedDataTypes.contains(datatypeURI);
    }


    public boolean passes(OWLAxiom axiom) {
        reason = "";
        passes = true;
        axiom.accept(this);
        return passes;
    }


    public String getReason() {
        return reason;
    }


    public void visit(OWLSubClassAxiom axiom) {
        axiom.getSubClass().accept(this);
        axiom.getSuperClass().accept(this);
    }


    public void visit(OWLNegativeObjectPropertyAssertionAxiom axiom) {
    }


    public void visit(OWLAntiSymmetricObjectPropertyAxiom axiom) {
    }


    public void visit(OWLReflexiveObjectPropertyAxiom axiom) {
    }


    public void visit(OWLDisjointClassesAxiom axiom) {
        if (axiom.getDescriptions().size() < 2){
            passes = false;
            reason = "Disjoint classes axiom with less than 2 classes detected.";
        }
        else{
            for (OWLDescription desc : axiom.getDescriptions()) {
                desc.accept(this);
            }
        }
    }


    public void visit(OWLDataPropertyDomainAxiom axiom) {
        axiom.getDomain().accept(this);
    }


    public void visit(OWLImportsDeclaration axiom) {

    }


    public void visit(OWLAxiomAnnotationAxiom axiom) {
    }


    public void visit(OWLObjectPropertyDomainAxiom axiom) {
        axiom.getDomain().accept(this);
    }


    public void visit(OWLEquivalentObjectPropertiesAxiom axiom) {
    }


    public void visit(OWLNegativeDataPropertyAssertionAxiom axiom) {
        axiom.getObject().accept(this);
    }


    public void visit(OWLDifferentIndividualsAxiom axiom) {
    }


    public void visit(OWLDisjointDataPropertiesAxiom axiom) {
    }


    public void visit(OWLDisjointObjectPropertiesAxiom axiom) {
    }


    public void visit(OWLObjectPropertyRangeAxiom axiom) {
        axiom.getRange().accept(this);
    }


    public void visit(OWLObjectPropertyAssertionAxiom axiom) {

    }


    public void visit(OWLFunctionalObjectPropertyAxiom axiom) {
    }


    public void visit(OWLObjectSubPropertyAxiom axiom) {
    }


    public void visit(OWLDisjointUnionAxiom axiom) {
        for (OWLDescription desc : axiom.getDescriptions()) {
            desc.accept(this);
        }
    }


    public void visit(OWLDeclarationAxiom axiom) {
    }


    public void visit(OWLEntityAnnotationAxiom axiom) {
    }


    public void visit(OWLSymmetricObjectPropertyAxiom axiom) {
    }


    public void visit(OWLDataPropertyRangeAxiom axiom) {
        axiom.getRange().accept(this);
    }


    public void visit(OWLFunctionalDataPropertyAxiom axiom) {
    }


    public void visit(OWLEquivalentDataPropertiesAxiom axiom) {
    }


    public void visit(OWLClassAssertionAxiom axiom) {
        axiom.getDescription().accept(this);
    }


    public void visit(OWLEquivalentClassesAxiom axiom) {
        for (OWLDescription desc : axiom.getDescriptions()) {
            desc.accept(this);
        }
    }


    public void visit(OWLDataPropertyAssertionAxiom axiom) {
        axiom.getObject().accept(this);
    }


    public void visit(OWLTransitiveObjectPropertyAxiom axiom) {
    }


    public void visit(OWLIrreflexiveObjectPropertyAxiom axiom) {
    }


    public void visit(OWLDataSubPropertyAxiom axiom) {
    }


    public void visit(OWLInverseFunctionalObjectPropertyAxiom axiom) {
    }


    public void visit(OWLSameIndividualsAxiom axiom) {
    }


    public void visit(OWLObjectPropertyChainSubPropertyAxiom axiom) {
    }


    public void visit(OWLInverseObjectPropertiesAxiom axiom) {
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////


    public void visit(OWLClass desc) {
    }


    public void visit(OWLObjectIntersectionOf desc) {
        for (OWLDescription op : desc.getOperands()) {
            op.accept(this);
        }
    }


    public void visit(OWLObjectUnionOf desc) {
        for (OWLDescription op : desc.getOperands()) {
            op.accept(this);
        }
    }


    public void visit(OWLObjectComplementOf desc) {
        desc.getOperand().accept(this);
    }


    public void visit(OWLObjectSomeRestriction desc) {
        desc.getFiller().accept(this);
    }


    public void visit(OWLObjectAllRestriction desc) {
        desc.getFiller().accept(this);
    }


    public void visit(OWLObjectValueRestriction desc) {
    }


    public void visit(OWLObjectMinCardinalityRestriction desc) {
        desc.getFiller().accept(this);
    }


    public void visit(OWLObjectExactCardinalityRestriction desc) {
        desc.getFiller().accept(this);
    }


    public void visit(OWLObjectMaxCardinalityRestriction desc) {
        desc.getFiller().accept(this);
    }


    public void visit(OWLObjectSelfRestriction desc) {
    }


    public void visit(OWLObjectOneOf desc) {
    }


    public void visit(OWLDataSomeRestriction desc) {
        desc.getFiller().accept(this);
    }


    public void visit(OWLDataAllRestriction desc) {
        desc.getFiller().accept(this);
    }


    public void visit(OWLDataValueRestriction desc) {
        desc.getValue().accept(this);
    }


    public void visit(OWLDataMinCardinalityRestriction desc) {
        desc.getFiller().accept(this);
    }


    public void visit(OWLDataExactCardinalityRestriction desc) {
        desc.getFiller().accept(this);
    }


    public void visit(OWLDataMaxCardinalityRestriction desc) {
        desc.getFiller().accept(this);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////


    public void visit(OWLObjectProperty property) {
    }


    public void visit(OWLDataProperty property) {

    }


    public void visit(OWLIndividual individual) {
    }


    public void visit(OWLDataType dataType) {
        passes = isSupportedDatatype(dataType.getURI());
        if (!passes) {
            reason = dataType.getURI() + " is not supported";
        }
    }


    public void visit(SWRLRule rule) {
    }


    public void visit(OWLOntologyAnnotationAxiom axiom) {
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////


    public void visit(OWLDataComplementOf node) {
        node.getDataRange().accept(this);
    }


    public void visit(OWLDataOneOf node) {
        for (OWLConstant con : node.getValues()) {
            con.accept(this);
        }
    }


    public void visit(OWLDataRangeRestriction node) {
        node.getDataRange().accept(this);
    }


    public void visit(OWLTypedConstant node) {
        node.getDataType().accept((OWLDataVisitor) this);
    }


    public void visit(OWLUntypedConstant node) {
    }


    public void visit(OWLDataRangeFacetRestriction node) {
    }
}
