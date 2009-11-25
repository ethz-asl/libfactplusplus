package uk.ac.manchester.cs.factplusplus.owlapiv3;

import org.semanticweb.owlapi.apibinding.OWLManager;
import org.semanticweb.owlapi.model.*;
import org.semanticweb.owlapi.vocab.OWLRDFVocabulary;
import org.semanticweb.owlapi.vocab.XSDVocabulary;
import org.semanticweb.owlapi.vocab.OWLFacet;

import java.io.*;
import java.net.URI;
import java.util.HashMap;
import java.util.Map;
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
 * Date: 04-Dec-2006<br><br>
 */
public class FaCTPlusPlusRenderer implements OWLObjectVisitor {

    private Writer writer;

    private Map<OWLFacet, String> facetRenderingMap;


    public FaCTPlusPlusRenderer(Writer writer) {
        this.writer = writer;
        facetRenderingMap = new HashMap<OWLFacet, String>();
        facetRenderingMap.put(OWLFacet.MIN_INCLUSIVE, "ge");
        facetRenderingMap.put(OWLFacet.MAX_INCLUSIVE, "le");
        facetRenderingMap.put(OWLFacet.MIN_EXCLUSIVE, "gt");
        facetRenderingMap.put(OWLFacet.MAX_EXCLUSIVE, "lt");
    }


    private void write(String s) {
        try {
            writer.write(s);
        }
        catch (IOException e) {
            e.printStackTrace();
        }
    }


    private void write(OWLObject obj) {
        write(" ");
        obj.accept(this);
    }


    public void visit(OWLSubClassOfAxiom axiom) {
        write("implies_c");
        write(axiom.getSubClass());
        write(axiom.getSuperClass());
    }


    public void visit(OWLNegativeObjectPropertyAssertionAxiom axiom) {
        write("not-related");
        write(axiom.getSubject());
        write(axiom.getProperty());
        write(axiom.getObject());
    }


    public void visit(OWLAsymmetricObjectPropertyAxiom axiom) {
        write("antisymmetric");
        write(axiom.getProperty());
    }


    public void visit(OWLReflexiveObjectPropertyAxiom axiom) {
        write("reflexive");
        write(axiom.getProperty());
    }


    public void visit(OWLDisjointClassesAxiom axiom) {
        write("disjoint");
        for (OWLClassExpression desc : axiom.getClassExpressions()) {
            write(desc);
        }
    }


    public void visit(OWLDataPropertyDomainAxiom axiom) {
        write("domain");
        write(axiom.getProperty());
        write(axiom.getDomain());
    }


    public void visit(OWLObjectPropertyDomainAxiom axiom) {
        write("domain ");
        write(axiom.getProperty());
        write(axiom.getDomain());
    }


    public void visit(OWLEquivalentObjectPropertiesAxiom axiom) {
        write("equal_r");
        for (OWLObjectPropertyExpression prop : axiom.getProperties()) {
            write(prop);
        }
    }


    public void visit(OWLNegativeDataPropertyAssertionAxiom axiom) {
        write("instance");
        write(axiom.getSubject());
        write("(all ");
        write(axiom.getProperty());
        write("(not ");
        write(axiom.getObject());
        write("))");
    }


    public void visit(OWLDifferentIndividualsAxiom axiom) {
        write("different ");
        for (OWLIndividual ind : axiom.getIndividuals()) {
            write(ind);
        }
    }


    public void visit(OWLDisjointDataPropertiesAxiom axiom) {
        write("disjoint_r");
        for (OWLDataPropertyExpression prop : axiom.getProperties()) {
            write(prop);
        }
    }


    public void visit(OWLDisjointObjectPropertiesAxiom axiom) {
        write("disjoint_r");
        for (OWLObjectPropertyExpression prop : axiom.getProperties()) {
            write(prop);
        }
    }


    public void visit(OWLObjectPropertyRangeAxiom axiom) {
        write("range");
        write(axiom.getProperty());
        write(axiom.getRange());
    }


    public void visit(OWLObjectPropertyAssertionAxiom axiom) {
        write("related");
        write(axiom.getSubject());
        write(axiom.getProperty());
        write(axiom.getObject());
    }


    public void visit(OWLFunctionalObjectPropertyAxiom axiom) {
        write("functional");
        write(axiom.getProperty());
    }


    public void visit(OWLSubObjectPropertyOfAxiom axiom) {
        write("implies_r");
        write(axiom.getSubProperty());
        write(axiom.getSuperProperty());
    }


    public void visit(OWLDisjointUnionAxiom axiom) {
        write("equal_c ");
        axiom.getOWLClass().accept(this);
        write("(disjoint-or ");
        for (OWLClassExpression desc : axiom.getClassExpressions()) {
            desc.accept(this);
            write(" ");
        }
        write(")");
    }


    public void visit(OWLDeclarationAxiom axiom) {

    }


    public void visit(OWLSymmetricObjectPropertyAxiom axiom) {
        write("symmetric");
        write(axiom.getProperty());
    }


    public void visit(OWLDataPropertyRangeAxiom axiom) {
        if (!(axiom.getRange() instanceof OWLDataOneOf)) {
            write("range");
            write(axiom.getProperty());
            write(axiom.getRange());
        }
    }


    public void visit(OWLFunctionalDataPropertyAxiom axiom) {
        write("functional");
        write(axiom.getProperty());
    }


    public void visit(OWLEquivalentDataPropertiesAxiom axiom) {
        write("equal_r");
        for (OWLDataPropertyExpression prop : axiom.getProperties()) {
            write(prop);
        }
    }


    public void visit(OWLClassAssertionAxiom axiom) {
        write("instance");
        write(axiom.getIndividual());
        write(axiom.getClassExpression());
    }


    public void visit(OWLEquivalentClassesAxiom axiom) {
        write("equal_c");
        for (OWLClassExpression desc : axiom.getClassExpressions()) {
            write(desc);
        }
    }


    public void visit(OWLDataPropertyAssertionAxiom axiom) {
        write("instance");
        write(axiom.getSubject());
        write("(some ");
        write(axiom.getProperty());
        write(axiom.getObject());
        write(")");
    }


    public void visit(OWLTransitiveObjectPropertyAxiom axiom) {
        write("transitive");
        write(axiom.getProperty());
    }


    public void visit(OWLIrreflexiveObjectPropertyAxiom axiom) {
        write("irreflexive");
        write(axiom.getProperty());
    }


    public void visit(OWLSubDataPropertyOfAxiom axiom) {
        write("implies_r");
        write(axiom.getSubProperty());
        write(axiom.getSuperProperty());
    }


    public void visit(OWLInverseFunctionalObjectPropertyAxiom axiom) {
        write("functional (inverse");
        write(axiom.getProperty());
        write(")");
    }


    public void visit(OWLSameIndividualAxiom axiom) {
        write("same");
        for (OWLIndividual ind : axiom.getIndividuals()) {
            write(ind);
        }
    }


    public void visit(OWLSubPropertyChainOfAxiom axiom) {
        write("implies_r (compose ");
        for (OWLObjectPropertyExpression prop : axiom.getPropertyChain()) {
            write(prop);
        }
        write(")");
        write(axiom.getSuperProperty());
    }


    public void visit(OWLInverseObjectPropertiesAxiom axiom) {
        write("equal_r");
        write(axiom.getFirstProperty());
        write(" (inverse");
        write(axiom.getSecondProperty());
        write(")");
    }


    public void visit(OWLHasKeyAxiom owlHasKeyAxiom) {
        // @@TODO
    }


    public void visit(OWLDatatypeDefinitionAxiom owlDatatypeDefinitionAxiom) {
        // @@TODO
    }


    public void visit(OWLClass desc) {
        if (desc.isOWLThing()) {
            write("*TOP*");
        }
        else if (desc.isOWLNothing()) {
            write("*BOTTOM*");
        }
        else {
            write(desc.getIRI().toString());
        }
    }


    public void visit(OWLObjectIntersectionOf desc) {
        write("(and");
        for (OWLClassExpression op : desc.getOperands()) {
            write(op);
        }
        write(")");
    }


    public void visit(OWLObjectUnionOf desc) {
        write("(or");
        for (OWLClassExpression op : desc.getOperands()) {
            write(op);
        }
        write(")");
    }


    public void visit(OWLObjectComplementOf desc) {
        write("(not");
        write(desc.getOperand());
        write(")");
    }


    public void visit(OWLObjectSomeValuesFrom desc) {
        write("(some");
        write(desc.getProperty());
        write(desc.getFiller());
        write(")");
    }


    public void visit(OWLObjectAllValuesFrom desc) {
        write("(all");
        write(desc.getProperty());
        write(desc.getFiller());
        write(")");
    }


    public void visit(OWLObjectHasValue desc) {
        write("(some");
        write(desc.getProperty());
        write(" (one-of");
        write(desc.getValue());
        write("))");
    }


    public void visit(OWLObjectMinCardinality desc) {
        write("(atleast ");
        write(Integer.toString(desc.getCardinality()));
        write(desc.getProperty());
        write(desc.getFiller());
        write(")");
    }


    public void visit(OWLObjectExactCardinality desc) {
        write("(and (atleast ");
        write(Integer.toString(desc.getCardinality()));
        write(desc.getProperty());
        write(desc.getFiller());
        write(") (atmost ");
        write(Integer.toString(desc.getCardinality()));
        write(desc.getProperty());
        write(desc.getFiller());
        write("))");
    }


    public void visit(OWLObjectMaxCardinality desc) {
        write("(atmost ");
        write(Integer.toString(desc.getCardinality()));
        write(desc.getProperty());
        write(desc.getFiller());
        write(")");
    }


    public void visit(OWLObjectHasSelf desc) {
        write("(self-ref");
        write(desc.getProperty());
        write(")");
    }


    public void visit(OWLObjectOneOf desc) {
        write("(one-of");
        for (OWLIndividual ind : desc.getIndividuals()) {
            write(ind);
        }
        write(")");
    }


    public void visit(OWLDataSomeValuesFrom desc) {
        write("(some");
        write(desc.getProperty());
        write(desc.getFiller());
        write(")");
    }


    public void visit(OWLDataAllValuesFrom desc) {
        write("(all");
        write(desc.getProperty());
        write(desc.getFiller());
        write(")");
    }


    public void visit(OWLDataHasValue desc) {
        write("(some");
        write(desc.getProperty());
        write(desc.getValue());
        write(")");
    }


    public void visit(OWLDataMinCardinality desc) {
        write("(atleast ");
        write(Integer.toString(desc.getCardinality()));
        write(desc.getProperty());
        write(desc.getFiller());
        write(")");
    }


    public void visit(OWLDataExactCardinality desc) {
        write("(and (atleast ");
        write(Integer.toString(desc.getCardinality()));
        write(desc.getProperty());
        write(desc.getFiller());
        write(") (atmost ");
        write(Integer.toString(desc.getCardinality()));
        write(desc.getProperty());
        write(desc.getFiller());
        write("))");
    }


    public void visit(OWLDataMaxCardinality desc) {
        write("(atmost ");
        write(Integer.toString(desc.getCardinality()));
        write(desc.getProperty());
        write(desc.getFiller());
        write(")");
    }


    public void visit(OWLDatatype node) {
        if (node.getIRI().equals(XSDVocabulary.STRING.getIRI())) {
            write("string");
        }
        else if (node.getIRI().equals(XSDVocabulary.DOUBLE.getIRI())) {
            write("real");
        }
        else if (node.getIRI().equals(XSDVocabulary.FLOAT.getIRI())) {
            write("real");
        }
        else {
            write("number");
        }
    }


    public void visit(OWLDataComplementOf node) {
        write("(not ");
        node.getDataRange().accept(this);
        write(")");
    }


    public void visit(OWLDataIntersectionOf owlDataIntersectionOf) {
        // @@TODO
    }


    public void visit(OWLDataUnionOf owlDataUnionOf) {
        // @@TODO
    }


    public void visit(OWLDataOneOf node) {
        throw new OWLRuntimeException("Unsupported operation");
    }


    public void visit(OWLDatatypeRestriction node) {
        write("(and");
        for (OWLFacetRestriction restriction : node.getFacetRestrictions()) {
            restriction.accept(this);
        }
        write(")");
    }


    public void visit(OWLFacetRestriction node) {
        write("(");
        String facetRendering = facetRenderingMap.get(node.getFacet());
        if (facetRendering == null) {
            write("\n;unsupported facet " + node.getFacet());
        }
        else {
            write(facetRendering);
            node.getFacetValue().accept(this);
        }
        write(")");
    }


    public void visit(OWLTypedLiteral node) {
        write("(");
        node.getDatatype().accept(this);
        write(" ");
        if (node.getDatatype().getIRI().equals(XSDVocabulary.STRING.getIRI())) {
            write("\"");
            write(node.getLiteral());
            write("\"");
        }
        else {
            write(node.getLiteral());
        }

        write(")");
    }


    public void visit(OWLStringLiteral node) {
        write("(string \"");
        write(node.getLiteral());
        write("\")");
    }


    public void visit(OWLObjectProperty property) {
        write(property.getIRI().toString());
    }


    public void visit(OWLObjectInverseOf property) {
        write("(inverse");
        write(property.getInverse());
        write(")");
    }


    public void visit(OWLDataProperty property) {
        write(property.getIRI().toString());
    }


    public void visit(OWLNamedIndividual individual) {
        write(individual.getIRI().toString());
    }


    public void visit(IRI iri) {
        write(iri.toString());
    }


    public void visit(OWLAnonymousIndividual owlAnonymousIndividual) {
        // todo?
    }


    public void visit(OWLOntology ontology) {
        for (OWLObjectProperty prop : ontology.getReferencedObjectProperties()) {
            write("(defprimrole");
            write(prop);
            write(")\n");
        }
        for (OWLDataProperty prop : ontology.getReferencedDataProperties()) {
            write("(defdatarole");
            write(prop);
            write(")\n");
        }
        for (OWLAxiom ax : ontology.getAxioms()) {
            if (ax.isLogicalAxiom()) {
                write("(");
                write(ax);
                write(")\n");
            }
        }
    }


    public void visit(SWRLRule rule) {
    }


    public void visit(SWRLClassAtom node) {
    }


    public void visit(SWRLDataRangeAtom node) {
    }


    public void visit(SWRLObjectPropertyAtom node) {
    }


    public void visit(SWRLDataPropertyAtom node) {
    }


    public void visit(SWRLBuiltInAtom node) {
    }

    public void visit(SWRLVariable node) {
    }

    public void visit(SWRLIndividualArgument swrlIndividualArgument) {
    }


    public void visit(SWRLLiteralArgument swrlLiteralArgument) {
    }


    public void visit(SWRLSameIndividualAtom swrlSameIndividualAtom) {
    }


    public void visit(SWRLDifferentIndividualsAtom swrlDifferentIndividualsAtom) {
    }


    public void visit(OWLAnnotationProperty owlAnnotationProperty) {
    }


    public void visit(OWLAnnotationAssertionAxiom axiom) {
    }


    public void visit(OWLSubAnnotationPropertyOfAxiom owlSubAnnotationPropertyOfAxiom) {
    }


    public void visit(OWLAnnotationPropertyDomainAxiom owlAnnotationPropertyDomainAxiom) {
    }


    public void visit(OWLAnnotationPropertyRangeAxiom owlAnnotationPropertyRangeAxiom) {
    }


    public void visit(OWLAnnotation owlAnnotation) {
    }

    public static void main(String[] args) {
        try {
            File workingDir = new File(".");
            File mappingFile = new File(workingDir, "mapping.txt");
            final Map<IRI, URI> uriMap = new HashMap<IRI, URI>();
            if (mappingFile.exists()) {
                BufferedReader mappingFileReader = new BufferedReader(new InputStreamReader(new FileInputStream(
                        mappingFile)));
                String line;
                while ((line = mappingFileReader.readLine()) != null) {
                    int sepIndex = line.indexOf(" ");
                    IRI ontURI = IRI.create(new URI(line.substring(0, sepIndex).trim()));
                    URI physicalURI = new URI(line.substring(sepIndex + 1, line.length()).trim());
                    uriMap.put(ontURI, physicalURI);
                }
            }
            OWLOntologyManager man = OWLManager.createOWLOntologyManager();
            man.addIRIMapper(new OWLOntologyIRIMapper() {
                public URI getPhysicalURI(IRI ontologyIRI) {
                    URI physURI = uriMap.get(ontologyIRI);
                    if (physURI != null) {
                        return physURI;
                    }
                    return ontologyIRI.toURI();
                }
            });
            OWLOntology ont = man.loadOntology(IRI.create(args[0]));
            Writer w = new BufferedWriter(new OutputStreamWriter(System.out));
            FaCTPlusPlusRenderer ren = new FaCTPlusPlusRenderer(w);
            ont.accept(ren);
            w.flush();
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }
}
