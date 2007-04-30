package uk.ac.manchester.cs.factplusplus.owlapi;

import static org.semanticweb.owl.vocab.OWLRestrictedDataRangeFacetVocabulary.*;
import org.semanticweb.owl.model.*;
import org.semanticweb.owl.apibinding.OWLManager;
import org.semanticweb.owl.vocab.OWLRDFVocabulary;
import org.semanticweb.owl.vocab.OWLRestrictedDataRangeFacetVocabulary;
import org.semanticweb.owl.vocab.XSDVocabulary;

import java.io.*;
import java.net.URI;
import java.util.Map;
import java.util.HashMap;
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

    private Map<OWLRestrictedDataRangeFacetVocabulary, String> facetRenderingMap;


    public FaCTPlusPlusRenderer(Writer writer) {
        this.writer = writer;
        facetRenderingMap = new HashMap<OWLRestrictedDataRangeFacetVocabulary, String>();
        facetRenderingMap.put(MIN_INCLUSIVE, "ge");
        facetRenderingMap.put(MAX_INCLUSIVE, "le");
        facetRenderingMap.put(MIN_EXCLUSIVE, "gt");
        facetRenderingMap.put(MAX_EXCLUSIVE, "lt");
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

    public void visit(OWLSubClassAxiom axiom) {
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

    public void visit(OWLAntiSymmetricObjectPropertyAxiom axiom) {
        write("antisymmetric");
        write(axiom.getProperty());
    }


    public void visit(OWLReflexiveObjectPropertyAxiom axiom) {
        write("reflexive");
        write(axiom.getProperty());
    }


    public void visit(OWLDisjointClassesAxiom axiom) {
        write("disjoint");
        for(OWLDescription desc : axiom.getDescriptions()) {
            write(desc);
        }
    }


    public void visit(OWLDataPropertyDomainAxiom axiom) {
        write("domain");
        write(axiom.getProperty());
        write(axiom.getDomain());
    }


    public void visit(OWLEntityAnnotationAxiom axiom) {
    }


    public void visit(OWLImportsDeclaration axiom) {
    }


    public void visit(OWLAxiomAnnotationAxiom axiom) {
    }


    public void visit(OWLObjectPropertyDomainAxiom axiom) {
        write("domain ");
        write(axiom.getProperty());
        write(axiom.getDomain());
    }


    public void visit(OWLEquivalentObjectPropertiesAxiom axiom) {
        write("equal_r");
        for(OWLObjectPropertyExpression prop : axiom.getProperties()) {
            write(prop);
        }
    }


    public void visit(OWLNegativeDataPropertyAssertionAxiom axiom) {
        write("not-related");
        write(axiom.getSubject());
        write(axiom.getProperty());
        write(axiom.getObject());
    }


    public void visit(OWLDifferentIndividualsAxiom axiom) {
        write("different ");
        for(OWLIndividual ind : axiom.getIndividuals()) {
            write(ind);
        }
    }


    public void visit(OWLDisjointDataPropertiesAxiom axiom) {
        write("disjoint_r");
        for(OWLDataPropertyExpression prop : axiom.getProperties()) {
            write(prop);
        }
    }


    public void visit(OWLDisjointObjectPropertiesAxiom axiom) {
        write("disjoint_r");
        for(OWLObjectPropertyExpression prop : axiom.getProperties()) {
            write(prop);
        }
    }


    public void visit(OWLObjectPropertyRangeAxiom axiom) {
        write("range ");
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


    public void visit(OWLObjectSubPropertyAxiom axiom) {
        write("implies_r");
        write(axiom.getSubProperty());
        write(axiom.getSuperProperty());
    }


    public void visit(OWLDisjointUnionAxiom axiom) {
        write("equal_c ");
        axiom.getOWLClass().accept(this);
        write("(disjoint-or ");
        for(OWLDescription desc : axiom.getDescriptions()) {
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
        for(OWLDataPropertyExpression prop : axiom.getProperties()) {
            write(prop);
        }
    }


    public void visit(OWLClassAssertionAxiom axiom) {
        write("instance");
        write(axiom.getIndividual());
        write(axiom.getDescription());
    }


    public void visit(OWLEquivalentClassesAxiom axiom) {
        write("equal_c");
        for(OWLDescription desc : axiom.getDescriptions()) {
            write(desc);
        }
    }


    public void visit(OWLDataPropertyAssertionAxiom axiom) {
        write("related");
        write(axiom.getSubject());
        write(axiom.getProperty());
        write(axiom.getObject());
    }


    public void visit(OWLTransitiveObjectPropertyAxiom axiom) {
        write("transitive");
        write(axiom.getProperty());
    }


    public void visit(OWLIrreflexiveObjectPropertyAxiom axiom) {
        write("irreflexive");
        write(axiom.getProperty());
    }


    public void visit(OWLDataSubPropertyAxiom axiom) {
        write("implies_r");
        write(axiom.getSubProperty());
        write(axiom.getSuperProperty());
    }


    public void visit(OWLInverseFunctionalObjectPropertyAxiom axiom) {
        write("functional (inverse");
        write(axiom.getProperty());
        write(")");
    }


    public void visit(OWLSameIndividualsAxiom axiom) {
        write("same");
        for(OWLIndividual ind : axiom.getIndividuals()) {
            write(ind);
        }
    }


    public void visit(OWLObjectPropertyChainSubPropertyAxiom axiom) {
        write("implies_r (");
        for(OWLObjectPropertyExpression prop : axiom.getPropertyChain()) {
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


    public void visit(OWLClass desc) {
        if(desc.getURI().equals(OWLRDFVocabulary.OWL_THING.getURI())) {
            write("*TOP*");
        }
        else if(desc.getURI().equals(OWLRDFVocabulary.OWL_NOTHING.getURI())) {
            write("*BOTTOM*");
        }
        else {
            write(desc.getURI().toString());
        }
    }


    public void visit(OWLObjectIntersectionOf desc) {
        write("(and");
        for(OWLDescription op : desc.getOperands()) {
            write(op);
        }
        write(")");
    }


    public void visit(OWLObjectUnionOf desc) {
        write("(or");
        for(OWLDescription op : desc.getOperands()) {
            write(op);
        }
        write(")");
    }


    public void visit(OWLObjectComplementOf desc) {
        write("(not");
        write(desc.getOperand());
        write(")");
    }


    public void visit(OWLObjectSomeRestriction desc) {
        write("(some");
        write(desc.getProperty());
        write(desc.getFiller());
        write(")");
    }


    public void visit(OWLObjectAllRestriction desc) {
        write("(all");
        write(desc.getProperty());
        write(desc.getFiller());
        write(")");
    }


    public void visit(OWLObjectValueRestriction desc) {
        write("(some");
        write(desc.getProperty());
        write(" (one-of");
        write(desc.getValue());
        write("))");
    }


    public void visit(OWLObjectMinCardinalityRestriction desc) {
        write("(atleast ");
        write(Integer.toString(desc.getCardinality()));
        write(desc.getProperty());
        write(desc.getFiller());
        write(")");
    }


    public void visit(OWLObjectExactCardinalityRestriction desc) {
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


    public void visit(OWLObjectMaxCardinalityRestriction desc) {
        write("(atmost ");
        write(Integer.toString(desc.getCardinality()));
        write(desc.getProperty());
        write(desc.getFiller());
        write(")");
    }


    public void visit(OWLObjectSelfRestriction desc) {
        write("(self-ref");
        write(desc.getProperty());
        write(")");
    }


    public void visit(OWLObjectOneOf desc) {
        write("(one-of");
        for(OWLIndividual ind : desc.getIndividuals()) {
            write(ind);
        }
        write(")");
    }


    public void visit(OWLDataSomeRestriction desc) {
        write("(some");
        write(desc.getProperty());
        write(desc.getFiller());
        write(")");
    }


    public void visit(OWLDataAllRestriction desc) {
        write("(all");
        write(desc.getProperty());
        write(desc.getFiller());
        write(")");
    }


    public void visit(OWLDataValueRestriction desc) {
        write("(some");
        write(desc.getProperty());
        write(desc.getValue());
        write(")");
    }


    public void visit(OWLDataMinCardinalityRestriction desc) {
        write("(atleast ");
        write(Integer.toString(desc.getCardinality()));
        write(desc.getProperty());
        write(desc.getFiller());
        write(")");
    }


    public void visit(OWLDataExactCardinalityRestriction desc) {
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


    public void visit(OWLDataMaxCardinalityRestriction desc) {
        write("(atmost ");
        write(Integer.toString(desc.getCardinality()));
        write(desc.getProperty());
        write(desc.getFiller());
        write(")");
    }


    public void visit(OWLDataType node) {
        if(node.getURI().equals(XSDVocabulary.STRING.getURI())) {
            write("string");
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


    public void visit(OWLDataOneOf node) {
        throw new OWLRuntimeException("Unsupported operation");
    }


    public void visit(OWLDataRangeRestriction node) {
        write("(and");
        for(OWLDataRangeFacetRestriction restriction : node.getFacetRestrictions()) {
            restriction.accept(this);
        }
        write(")");
    }


    public void visit(OWLDataRangeFacetRestriction node) {
        write("(");
            String facetRendering = facetRenderingMap.get(node.getFacet());
            if(facetRendering == null) {
                write("\n;unsupported facet " + node.getFacet());
            }
            else {
                write(facetRendering);
                node.getFacetValue().accept(this);
            }
        write(")");
    }


    public void visit(OWLTypedConstant node) {
        write("(");
        node.getDataType().accept(this);
        write(" ");
        write(node.getLiteral());
        write(")");
    }


    public void visit(OWLUntypedConstant node) {
        write("(string ");
        write(node.getLiteral());
        write(")");
    }


    public void visit(OWLObjectProperty property) {
        write(property.getURI().toString());
    }


    public void visit(OWLObjectPropertyInverse property) {
        write("(inverse");
        write(property.getInverse());
        write(")");
    }


    public void visit(OWLDataProperty property) {
        write(property.getURI().toString());
    }

    public void visit(OWLIndividual individual) {
        write(individual.getURI().toString());
    }


    public void visit(OWLOntology ontology) {
        for(OWLObjectProperty prop : ontology.getReferencedObjectProperties()) {
            write("(defprimrole");
            write(prop);
            write(")\n");
        }
        for(OWLDataProperty prop : ontology.getReferencedDataProperties()) {
            write("(defdatarole");
            write(prop);
            write(")\n");
        }
        for(OWLAxiom ax : ontology.getAxioms()) {
            if (ax.isLogicalAxiom()) {
                write("(");
                write(ax);
                write(")\n");
            }
        }
    }


    public void visit(SWRLRule rule) {
    }


    public void visit(OWLConstantAnnotation annotation) {
    }


    public void visit(OWLObjectAnnotation annotation) {
    }


    public void visit(OWLOntologyAnnotationAxiom axiom) {
    }


    public void visit(SWRLClassAtom node) {
    }


    public void visit(SWRLDataRangeAtom node) {
    }


    public void visit(SWRLObjectPropertyAtom node) {
    }


    public void visit(SWRLDataValuedPropertyAtom node) {
    }


    public void visit(SWRLBuiltInAtom node) {
    }


    public void visit(SWRLAtomDVariable node) {
    }


    public void visit(SWRLAtomIVariable node) {
    }


    public void visit(SWRLAtomIndividualObject node) {
    }


    public void visit(SWRLAtomConstantObject node) {
    }


    public void visit(SWRLDifferentFromAtom node) {
    }


    public void visit(SWRLSameAsAtom node) {
    }


    public static void main(String[] args) {
        try {
            File workingDir = new File(".");
            File mappingFile = new File(workingDir, "mapping.txt");
            final Map<URI, URI> uriMap = new HashMap<URI, URI>();
            if (mappingFile.exists()) {
                BufferedReader mappingFileReader = new BufferedReader(new InputStreamReader(new FileInputStream(mappingFile)));
                String line;
                while((line = mappingFileReader.readLine()) != null) {
                    int sepIndex = line.indexOf(" ");
                    URI ontURI = new URI(line.substring(0, sepIndex).trim());
                    URI physicalURI = new URI(line.substring(sepIndex + 1, line.length()).trim());
                    uriMap.put(ontURI, physicalURI);
                }
            }
            OWLOntologyManager man = OWLManager.createOWLOntologyManager();
            man.addURIMapper(new OWLOntologyURIMapper() {
                public URI getPhysicalURI(URI ontologyURI) {
                    URI physURI = uriMap.get(ontologyURI);
                    if(physURI != null) {
                        return physURI;
                    }
                    return ontologyURI;
                }
            });
            OWLOntology ont = man.loadOntology(URI.create(args[0]));
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
