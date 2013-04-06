/*
 * Copyright (C) 2009-2010, University of Manchester
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
package uk.ac.manchester.cs.factplusplus.owlapiv3;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Set;

import org.semanticweb.owlapi.model.AxiomType;
import org.semanticweb.owlapi.model.OWLAxiom;
import org.semanticweb.owlapi.model.OWLClass;
import org.semanticweb.owlapi.model.OWLClassExpression;
import org.semanticweb.owlapi.model.OWLDataProperty;
import org.semanticweb.owlapi.model.OWLDataPropertyExpression;
import org.semanticweb.owlapi.model.OWLDataRange;
import org.semanticweb.owlapi.model.OWLException;
import org.semanticweb.owlapi.model.OWLLiteral;
import org.semanticweb.owlapi.model.OWLNamedIndividual;
import org.semanticweb.owlapi.model.OWLObjectProperty;
import org.semanticweb.owlapi.model.OWLObjectPropertyExpression;
import org.semanticweb.owlapi.model.OWLOntology;
import org.semanticweb.owlapi.model.OWLOntologyChange;
import org.semanticweb.owlapi.reasoner.AxiomNotInProfileException;
import org.semanticweb.owlapi.reasoner.BufferingMode;
import org.semanticweb.owlapi.reasoner.ClassExpressionNotInProfileException;
import org.semanticweb.owlapi.reasoner.FreshEntitiesException;
import org.semanticweb.owlapi.reasoner.FreshEntityPolicy;
import org.semanticweb.owlapi.reasoner.InconsistentOntologyException;
import org.semanticweb.owlapi.reasoner.IndividualNodeSetPolicy;
import org.semanticweb.owlapi.reasoner.InferenceType;
import org.semanticweb.owlapi.reasoner.Node;
import org.semanticweb.owlapi.reasoner.NodeSet;
import org.semanticweb.owlapi.reasoner.OWLReasonerConfiguration;
import org.semanticweb.owlapi.reasoner.ReasonerInterruptedException;
import org.semanticweb.owlapi.reasoner.TimeOutException;
import org.semanticweb.owlapi.reasoner.UnsupportedEntailmentTypeException;
import org.semanticweb.owlapi.reasoner.knowledgeexploration.OWLKnowledgeExplorerReasoner;
import org.semanticweb.owlapi.util.Version;

import uk.ac.manchester.cs.factplusplus.NodePointer;


/**wrapper class for the new interface in the OWL API; it decouples the rest of the reasoner from relying on an OWL API > 3.2.4*/
public class OWLKnowledgeExplorationReasonerWrapper implements
		OWLKnowledgeExplorerReasoner {
	private class RootNodeImpl implements RootNode{
		private final NodePointer pointer;
		public RootNodeImpl(NodePointer p) {
		this.pointer=p;
		}
		public <T> T getNode() {

			return (T)pointer;
		}
		@Override
		public int hashCode() {

			return pointer.hashCode();
		}
		@Override
		public boolean equals(Object arg0) {
			if(arg0==null) {
				return false;
			}if(this==arg0) {
				return true;
			}
			if(arg0 instanceof RootNode) {
				return this.pointer.equals(((RootNode) arg0).getNode());
			}
			return false;
		}
	}
	private final FaCTPlusPlusReasoner r;

	public OWLKnowledgeExplorationReasonerWrapper(FaCTPlusPlusReasoner r) {
		this.r = r;
	}

	public void ontologiesChanged(List<? extends OWLOntologyChange> changes)
			throws OWLException {
		this.r.ontologiesChanged(changes);
	}

	public OWLReasonerConfiguration getReasonerConfiguration() {
		return this.r.getReasonerConfiguration();
	}

	public BufferingMode getBufferingMode() {
		return this.r.getBufferingMode();
	}

	public long getTimeOut() {
		return this.r.getTimeOut();
	}

	public OWLOntology getRootOntology() {
		return this.r.getRootOntology();
	}

	public List<OWLOntologyChange> getPendingChanges() {
		return this.r.getPendingChanges();
	}

	public Set<OWLAxiom> getPendingAxiomAdditions() {
		return this.r.getPendingAxiomAdditions();
	}

	public Set<OWLAxiom> getPendingAxiomRemovals() {
		return this.r.getPendingAxiomRemovals();
	}

	public FreshEntityPolicy getFreshEntityPolicy() {
		return this.r.getFreshEntityPolicy();
	}

	public IndividualNodeSetPolicy getIndividualNodeSetPolicy() {
		return this.r.getIndividualNodeSetPolicy();
	}

	public String getReasonerName() {
		return this.r.getReasonerName();
	}

	public Version getReasonerVersion() {
		return this.r.getReasonerVersion();
	}

	public void interrupt() {
		this.r.interrupt();
	}

	public void precomputeInferences(InferenceType... inferenceTypes)
			throws ReasonerInterruptedException, TimeOutException,
			InconsistentOntologyException {
		this.r.precomputeInferences(inferenceTypes);
	}

	public boolean isPrecomputed(InferenceType inferenceType) {
		return this.r.isPrecomputed(inferenceType);
	}

	public Set<InferenceType> getPrecomputableInferenceTypes() {
		return this.r.getPrecomputableInferenceTypes();
	}

	public boolean isConsistent() throws ReasonerInterruptedException, TimeOutException {
		return this.r.isConsistent();
	}

	public boolean isSatisfiable(OWLClassExpression classExpression)
			throws ReasonerInterruptedException, TimeOutException,
			ClassExpressionNotInProfileException, FreshEntitiesException,
			InconsistentOntologyException {
		return this.r.isSatisfiable(classExpression);
	}

	public Node<OWLClass> getUnsatisfiableClasses() throws ReasonerInterruptedException,
			TimeOutException, InconsistentOntologyException {
		return this.r.getUnsatisfiableClasses();
	}

	public boolean isEntailed(OWLAxiom axiom) throws ReasonerInterruptedException,
			UnsupportedEntailmentTypeException, TimeOutException,
			AxiomNotInProfileException, FreshEntitiesException,
			InconsistentOntologyException {
		return this.r.isEntailed(axiom);
	}

	public boolean isEntailed(Set<? extends OWLAxiom> axioms)
			throws ReasonerInterruptedException, UnsupportedEntailmentTypeException,
			TimeOutException, AxiomNotInProfileException, FreshEntitiesException,
			InconsistentOntologyException {
		return this.r.isEntailed(axioms);
	}

	public boolean isEntailmentCheckingSupported(AxiomType<?> axiomType) {
		return this.r.isEntailmentCheckingSupported(axiomType);
	}

	public Set<OWLAxiom> getTrace(OWLAxiom axiom) {
		return this.r.getTrace(axiom);
	}

	public Node<OWLClass> getTopClassNode() {
		return this.r.getTopClassNode();
	}

	public Node<OWLClass> getBottomClassNode() {
		return this.r.getBottomClassNode();
	}

	public NodeSet<OWLClass> getSubClasses(OWLClassExpression ce, boolean direct)
			throws ReasonerInterruptedException, TimeOutException,
			FreshEntitiesException, InconsistentOntologyException {
		return this.r.getSubClasses(ce, direct);
	}

	public NodeSet<OWLClass> getSuperClasses(OWLClassExpression ce, boolean direct)
			throws InconsistentOntologyException, ClassExpressionNotInProfileException,
			ReasonerInterruptedException, TimeOutException {
		return this.r.getSuperClasses(ce, direct);
	}

	public Node<OWLClass> getEquivalentClasses(OWLClassExpression ce)
			throws InconsistentOntologyException, ClassExpressionNotInProfileException,
			ReasonerInterruptedException, TimeOutException {
		return this.r.getEquivalentClasses(ce);
	}

	public Node<OWLObjectPropertyExpression> getTopObjectPropertyNode() {
		return this.r.getTopObjectPropertyNode();
	}

	public Node<OWLObjectPropertyExpression> getBottomObjectPropertyNode() {
		return this.r.getBottomObjectPropertyNode();
	}

	public NodeSet<OWLObjectPropertyExpression> getSubObjectProperties(
			OWLObjectPropertyExpression pe, boolean direct)
			throws InconsistentOntologyException, ReasonerInterruptedException,
			TimeOutException {
		return this.r.getSubObjectProperties(pe, direct);
	}

	public NodeSet<OWLObjectPropertyExpression> getSuperObjectProperties(
			OWLObjectPropertyExpression pe, boolean direct)
			throws InconsistentOntologyException, ReasonerInterruptedException,
			TimeOutException {
		return this.r.getSuperObjectProperties(pe, direct);
	}

	public Node<OWLObjectPropertyExpression> getEquivalentObjectProperties(
			OWLObjectPropertyExpression pe) throws InconsistentOntologyException,
			ReasonerInterruptedException, TimeOutException {
		return this.r.getEquivalentObjectProperties(pe);
	}

	public NodeSet<OWLObjectPropertyExpression> getDisjointObjectProperties(
			OWLObjectPropertyExpression pe) throws InconsistentOntologyException,
			ReasonerInterruptedException, TimeOutException {
		return this.r.getDisjointObjectProperties(pe);
	}

	public Node<OWLObjectPropertyExpression> getInverseObjectProperties(
			OWLObjectPropertyExpression pe) throws InconsistentOntologyException,
			ReasonerInterruptedException, TimeOutException {
		return this.r.getInverseObjectProperties(pe);
	}

	public NodeSet<OWLClass> getObjectPropertyDomains(OWLObjectPropertyExpression pe,
			boolean direct) throws InconsistentOntologyException,
			ReasonerInterruptedException, TimeOutException {
		return this.r.getObjectPropertyDomains(pe, direct);
	}

	public NodeSet<OWLClass> getObjectPropertyRanges(OWLObjectPropertyExpression pe,
			boolean direct) throws InconsistentOntologyException,
			ReasonerInterruptedException, TimeOutException {
		return this.r.getObjectPropertyRanges(pe, direct);
	}

	public Node<OWLDataProperty> getTopDataPropertyNode() {
		return this.r.getTopDataPropertyNode();
	}

	public Node<OWLDataProperty> getBottomDataPropertyNode() {
		return this.r.getBottomDataPropertyNode();
	}

	public NodeSet<OWLDataProperty> getSubDataProperties(OWLDataProperty pe,
			boolean direct) throws InconsistentOntologyException,
			ReasonerInterruptedException, TimeOutException {
		return this.r.getSubDataProperties(pe, direct);
	}

	public NodeSet<OWLDataProperty> getSuperDataProperties(OWLDataProperty pe,
			boolean direct) throws InconsistentOntologyException,
			ReasonerInterruptedException, TimeOutException {
		return this.r.getSuperDataProperties(pe, direct);
	}

	public Node<OWLDataProperty> getEquivalentDataProperties(OWLDataProperty pe)
			throws InconsistentOntologyException, ReasonerInterruptedException,
			TimeOutException {
		return this.r.getEquivalentDataProperties(pe);
	}

	public NodeSet<OWLClass> getTypes(OWLNamedIndividual ind, boolean direct)
			throws InconsistentOntologyException, ReasonerInterruptedException,
			TimeOutException {
		return this.r.getTypes(ind, direct);
	}

	public NodeSet<OWLNamedIndividual> getInstances(OWLClassExpression ce, boolean direct)
			throws InconsistentOntologyException, ClassExpressionNotInProfileException,
			ReasonerInterruptedException, TimeOutException {
		return this.r.getInstances(ce, direct);
	}

	public NodeSet<OWLNamedIndividual> getObjectPropertyValues(OWLNamedIndividual ind,
			OWLObjectPropertyExpression pe) throws InconsistentOntologyException,
			ReasonerInterruptedException, TimeOutException {
		return this.r.getObjectPropertyValues(ind, pe);
	}

	public Node<OWLNamedIndividual> getSameIndividuals(OWLNamedIndividual ind)
			throws InconsistentOntologyException, ReasonerInterruptedException,
			TimeOutException {
		return this.r.getSameIndividuals(ind);
	}

	public void dispose() {
		this.r.dispose();
	}

	public void flush() {
		this.r.flush();
	}

	public NodeSet<OWLClass> getDisjointClasses(OWLClassExpression ce) {
		return this.r.getDisjointClasses(ce);
	}

	public NodeSet<OWLDataProperty> getDisjointDataProperties(OWLDataPropertyExpression pe)
			throws InconsistentOntologyException, ReasonerInterruptedException,
			TimeOutException {
		return this.r.getDisjointDataProperties(pe);
	}

	public NodeSet<OWLClass> getDataPropertyDomains(OWLDataProperty pe, boolean direct)
			throws InconsistentOntologyException, ReasonerInterruptedException,
			TimeOutException {
		return this.r.getDataPropertyDomains(pe, direct);
	}

	public Set<OWLLiteral> getDataPropertyValues(OWLNamedIndividual ind,
			OWLDataProperty pe) throws InconsistentOntologyException,
			ReasonerInterruptedException, TimeOutException {
		return this.r.getDataPropertyValues(ind, pe);
	}

	public NodeSet<OWLNamedIndividual> getDifferentIndividuals(OWLNamedIndividual ind)
			throws InconsistentOntologyException, ReasonerInterruptedException,
			TimeOutException {
		return this.r.getDifferentIndividuals(ind);
	}

	public RootNode getRoot(OWLClassExpression expression) {
		return new RootNodeImpl(this.r.getRoot(expression));
	}

	public Node<? extends OWLObjectPropertyExpression> getObjectNeighbours(
			RootNode object, boolean deterministicOnly) {
		return this.r.getObjectNeighbours((NodePointer)object.getNode(), deterministicOnly);
	}

	public Node<OWLDataProperty> getDataNeighbours(RootNode object, boolean deterministicOnly) {
		return this.r.getDataNeighbours((NodePointer)object.getNode(), deterministicOnly);
	}

	public Collection<RootNode> getObjectNeighbours(RootNode object, OWLObjectProperty property) {
		Collection<RootNode> toReturn=new ArrayList<RootNode>();
		for(NodePointer p:this.r.getObjectNeighbours((NodePointer)object.getNode(), property)) {
			toReturn.add(new RootNodeImpl(p));
		}

		return toReturn;
	}

	public Collection<RootNode> getDataNeighbours(RootNode object, OWLDataProperty property) {
		Collection<RootNode> toReturn=new ArrayList<RootNode>();
		for(NodePointer p:this.r.getDataNeighbours((NodePointer)object.getNode(), property)) {
			toReturn.add(new RootNodeImpl(p));
		}
		return toReturn;
	}

	public Node<? extends OWLClassExpression> getObjectLabel(RootNode object, boolean deterministicOnly) {
		return this.r.getObjectLabel((NodePointer)object.getNode(), deterministicOnly);
	}

	public Node<? extends OWLDataRange> getDataLabel(RootNode object, boolean deterministicOnly) {
		return this.r.getDataLabel((NodePointer)object.getNode(), deterministicOnly);
	}

	public RootNode getBlocker(RootNode object) {
		return new RootNodeImpl(this.r.getBlocker((NodePointer)object.getNode()));
	}

}
