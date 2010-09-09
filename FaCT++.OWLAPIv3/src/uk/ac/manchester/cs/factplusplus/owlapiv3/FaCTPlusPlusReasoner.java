package uk.ac.manchester.cs.factplusplus.owlapiv3;

import org.semanticweb.owlapi.model.*;
import org.semanticweb.owlapi.reasoner.*;
import org.semanticweb.owlapi.reasoner.InconsistentOntologyException;
import org.semanticweb.owlapi.reasoner.impl.*;
import org.semanticweb.owlapi.util.Version;
import org.semanticweb.owlapi.vocab.OWLFacet;
import uk.ac.manchester.cs.factplusplus.*;

import java.io.PrintStream;
import java.io.PrintWriter;
import java.util.*;
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

/**
 * Author: Matthew Horridge<br>
 * The University of Manchester<br>
 * Information Management Group<br>
 * Date: 29-Dec-2009
 */
public class FaCTPlusPlusReasoner extends OWLReasonerBase {

    public static final String REASONER_NAME = "FaCT++";

    public static final Version VERSION = new Version(1, 4, 0, 0);

    private boolean interrupted = false;

    private FaCTPlusPlus kernel = new FaCTPlusPlus();

    private AxiomTranslator axiomTranslator = new AxiomTranslator();

    private ClassExpressionTranslator classExpressionTranslator = new ClassExpressionTranslator();

    private DataRangeTranslator dataRangeTranslator = new DataRangeTranslator();

    private ObjectPropertyTranslator objectPropertyTranslator = new ObjectPropertyTranslator();

    private DataPropertyTranslator dataPropertyTranslator = new DataPropertyTranslator();

    private IndividualTranslator individualTranslator = new IndividualTranslator();

    private EntailmentChecker entailmentChecker = new EntailmentChecker();

	private Map<OWLAxiom, AxiomPointer> axiom2PtrMap = new HashMap<OWLAxiom, AxiomPointer>();

    public FaCTPlusPlusReasoner(OWLOntology rootOntology, OWLReasonerConfiguration configuration, BufferingMode bufferingMode) {
        super(rootOntology, configuration, bufferingMode);
	kernel.setTopBottomPropertyNames(
		"http://www.w3.org/2002/07/owl#topObjectProperty",
		"http://www.w3.org/2002/07/owl#bottomObjectProperty",
		"http://www.w3.org/2002/07/owl#topDataProperty",
		"http://www.w3.org/2002/07/owl#bottomDataProperty");
        loadReasonerAxioms();
        kernel.setProgressMonitor(new ProgressMonitorAdapter());
        kernel.setOperationTimeout(configuration.getTimeOut());
    }

	///////////////////////////////////////////////////////////////////////////
	//
	//  load/retract axioms
	//
	///////////////////////////////////////////////////////////////////////////

	private void loadAxiom(OWLAxiom axiom) {
		final AxiomPointer axiomPointer = axiom.accept(axiomTranslator);
		
		if (axiomPointer != null) {
                axiom2PtrMap.put(axiom, axiomPointer);
        }
    }
	
	private void retractAxiom(OWLAxiom axiom) {
		final AxiomPointer ptr = axiom2PtrMap.get(axiom);
		if (ptr != null) {
			kernel.retract(ptr);
			axiom2PtrMap.remove(axiom);
		}
    }
	
	@Override
    protected void handleChanges(Set<OWLAxiom> addAxioms, Set<OWLAxiom> removeAxioms) {
		kernel.startChanges();
		for ( OWLAxiom ax_a: addAxioms )
			loadAxiom(ax_a);
		for ( OWLAxiom ax_r: removeAxioms )
			retractAxiom(ax_r);
        kernel.endChanges();
    }

    private void loadReasonerAxioms() {
        getReasonerConfiguration().getProgressMonitor().reasonerTaskStarted(ReasonerProgressMonitor.LOADING);
        getReasonerConfiguration().getProgressMonitor().reasonerTaskBusy();
        kernel.clearKernel();
        axiomTranslator = new AxiomTranslator();
        classExpressionTranslator = new ClassExpressionTranslator();
        dataRangeTranslator = new DataRangeTranslator();
        objectPropertyTranslator = new ObjectPropertyTranslator();
        dataPropertyTranslator = new DataPropertyTranslator();
        individualTranslator = new IndividualTranslator();
		axiom2PtrMap.clear();
        
        for (OWLAxiom ax : getReasonerAxioms()) {
            loadAxiom(ax);
        }
        getReasonerConfiguration().getProgressMonitor().reasonerTaskStopped();
    }


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////
    //////
    //////  Implementation of reasoner interfaces
    //////
    //////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    public String getReasonerName() {
        return REASONER_NAME;
    }

    public Version getReasonerVersion() {
        return VERSION;
    }

    public void interrupt() {
        interrupted = true;
    }

	// precompute inferences

	public void precomputeInferences(InferenceType... inferenceTypes) throws ReasonerInterruptedException, TimeOutException, InconsistentOntologyException {	
		// FIXME!! later
        kernel.realise();
    }

	public boolean isPrecomputed(InferenceType inferenceType) {
		// FIXME!! later
		return true;
	}
	
	public Set<InferenceType> getPrecomputableInferenceTypes() {
		// FIXME!! later
		return new HashSet<InferenceType>();
	}


	// consistency
	
	public boolean isConsistent() throws ReasonerInterruptedException, TimeOutException {
        return kernel.isKBConsistent();
    }

    private void checkConsistency() {
        if (!isConsistent()) {
            throw new InconsistentOntologyException();
        }
    }

    public boolean isSatisfiable(OWLClassExpression classExpression) throws ReasonerInterruptedException, TimeOutException, ClassExpressionNotInProfileException, FreshEntitiesException, InconsistentOntologyException {
        checkConsistency();
        return kernel.isClassSatisfiable(toClassPointer(classExpression));
    }

    public Node<OWLClass> getUnsatisfiableClasses() throws ReasonerInterruptedException, TimeOutException, InconsistentOntologyException {
        checkConsistency();
        return getBottomClassNode();
    }

	// entailments

    public boolean isEntailed(OWLAxiom axiom) throws ReasonerInterruptedException, UnsupportedEntailmentTypeException, TimeOutException, AxiomNotInProfileException, FreshEntitiesException, InconsistentOntologyException {
        checkConsistency();
        Boolean entailed = axiom.accept(entailmentChecker);
        if (entailed == null) {
            throw new UnsupportedEntailmentTypeException(axiom);
        }
        return entailed;
    }

    public boolean isEntailed(Set<? extends OWLAxiom> axioms) throws ReasonerInterruptedException, UnsupportedEntailmentTypeException, TimeOutException, AxiomNotInProfileException, FreshEntitiesException, InconsistentOntologyException {
        checkConsistency();
        for (OWLAxiom ax : axioms) {
            if (!isEntailed(ax)) {
                return false;
            }
        }
        return true;
    }

    public boolean isEntailmentCheckingSupported(AxiomType<?> axiomType) {
		// FIXME!! check later
        return true;
    }

	// classes

    public Node<OWLClass> getTopClassNode() {
        checkConsistency();
        return getEquivalentClasses(getOWLDataFactory().getOWLThing());
    }

    public Node<OWLClass> getBottomClassNode() {
        checkConsistency();
        return getEquivalentClasses(getOWLDataFactory().getOWLNothing());
    }

    public NodeSet<OWLClass> getSubClasses(OWLClassExpression ce, boolean direct) throws ReasonerInterruptedException, TimeOutException, FreshEntitiesException, InconsistentOntologyException {
        checkConsistency();
        return classExpressionTranslator.getNodeSetFromPointers(kernel.askSubClasses(toClassPointer(ce), direct));
    }

    public NodeSet<OWLClass> getSuperClasses(OWLClassExpression ce, boolean direct) throws InconsistentOntologyException, ClassExpressionNotInProfileException, ReasonerInterruptedException, TimeOutException {
        checkConsistency();
        return classExpressionTranslator.getNodeSetFromPointers(kernel.askSuperClasses(toClassPointer(ce), direct));
    }

    public Node<OWLClass> getEquivalentClasses(OWLClassExpression ce) throws InconsistentOntologyException, ClassExpressionNotInProfileException, ReasonerInterruptedException, TimeOutException {
        checkConsistency();
        ClassPointer[] pointers = kernel.askEquivalentClasses(toClassPointer(ce));
        return classExpressionTranslator.getNodeFromPointers(pointers);
    }

    public NodeSet<OWLClass> getDisjointClasses(OWLClassExpression ce) {
        checkConsistency();
        // TODO:
        // Not supported directly by FaCT++
        return new OWLClassNodeSet();
    }

	// object properties

    public Node<OWLObjectPropertyExpression> getTopObjectPropertyNode() {
        checkConsistency();
        return getEquivalentObjectProperties(getOWLDataFactory().getOWLTopObjectProperty());
    }

    public Node<OWLObjectPropertyExpression> getBottomObjectPropertyNode() {
        checkConsistency();
        return getEquivalentObjectProperties(getOWLDataFactory().getOWLBottomObjectProperty());
    }

    public NodeSet<OWLObjectPropertyExpression> getSubObjectProperties(OWLObjectPropertyExpression pe, boolean direct) throws InconsistentOntologyException, ReasonerInterruptedException, TimeOutException {
        checkConsistency();
        return objectPropertyTranslator.getNodeSetFromPointers(kernel.askSubObjectProperties(toObjectPropertyPointer(pe), direct));
    }

    public NodeSet<OWLObjectPropertyExpression> getSuperObjectProperties(OWLObjectPropertyExpression pe, boolean direct) throws InconsistentOntologyException, ReasonerInterruptedException, TimeOutException {
        checkConsistency();
        return objectPropertyTranslator.getNodeSetFromPointers(kernel.askSuperObjectProperties(toObjectPropertyPointer(pe), direct));
    }

    public Node<OWLObjectPropertyExpression> getEquivalentObjectProperties(OWLObjectPropertyExpression pe) throws InconsistentOntologyException, ReasonerInterruptedException, TimeOutException {
        checkConsistency();
        return objectPropertyTranslator.getNodeFromPointers(kernel.askEquivalentObjectProperties(toObjectPropertyPointer(pe)));
    }

    public NodeSet<OWLObjectPropertyExpression> getDisjointObjectProperties(OWLObjectPropertyExpression pe) throws InconsistentOntologyException, ReasonerInterruptedException, TimeOutException {
        checkConsistency();
        // TODO:
        return new OWLObjectPropertyNodeSet();
    }

    public Node<OWLObjectPropertyExpression> getInverseObjectProperties(OWLObjectPropertyExpression pe) throws InconsistentOntologyException, ReasonerInterruptedException, TimeOutException {
        checkConsistency();
        return objectPropertyTranslator.getNodeFromPointers(kernel.askEquivalentObjectProperties(toObjectPropertyPointer(pe.getInverseProperty())));
    }

    public NodeSet<OWLClass> getObjectPropertyDomains(OWLObjectPropertyExpression pe, boolean direct) throws InconsistentOntologyException, ReasonerInterruptedException, TimeOutException {
        checkConsistency();
        ClassPointer subClass = toClassPointer(getOWLDataFactory().getOWLObjectSomeValuesFrom(pe, getOWLDataFactory().getOWLThing()));
        return classExpressionTranslator.getNodeSetFromPointers(kernel.askSuperClasses(subClass, direct));
    }

    public NodeSet<OWLClass> getObjectPropertyRanges(OWLObjectPropertyExpression pe, boolean direct) throws InconsistentOntologyException, ReasonerInterruptedException, TimeOutException {
        checkConsistency();
        return getSuperClasses(getOWLDataFactory().getOWLObjectSomeValuesFrom(pe.getInverseProperty(), getOWLDataFactory().getOWLThing()), direct);
    }

	// data properties
	
    public Node<OWLDataProperty> getTopDataPropertyNode() {
        checkConsistency();
        return getEquivalentDataProperties(getOWLDataFactory().getOWLTopDataProperty());
    }

    public Node<OWLDataProperty> getBottomDataPropertyNode() {
        checkConsistency();
        return getEquivalentDataProperties(getOWLDataFactory().getOWLBottomDataProperty());
    }

    public NodeSet<OWLDataProperty> getSubDataProperties(OWLDataProperty pe, boolean direct) throws InconsistentOntologyException, ReasonerInterruptedException, TimeOutException {
        checkConsistency();
        return dataPropertyTranslator.getNodeSetFromPointers(kernel.askSubDataProperties(toDataPropertyPointer(pe), direct));
    }

    public NodeSet<OWLDataProperty> getSuperDataProperties(OWLDataProperty pe, boolean direct) throws InconsistentOntologyException, ReasonerInterruptedException, TimeOutException {
        checkConsistency();
        return dataPropertyTranslator.getNodeSetFromPointers(kernel.askSuperDataProperties(toDataPropertyPointer(pe), direct));
    }

    public Node<OWLDataProperty> getEquivalentDataProperties(OWLDataProperty pe) throws InconsistentOntologyException, ReasonerInterruptedException, TimeOutException {
        checkConsistency();
        return dataPropertyTranslator.getNodeFromPointers(kernel.askEquivalentDataProperties(toDataPropertyPointer(pe)));
    }

    public NodeSet<OWLDataProperty> getDisjointDataProperties(OWLDataPropertyExpression pe) throws InconsistentOntologyException, ReasonerInterruptedException, TimeOutException {
        checkConsistency();
        // TODO:
        return new OWLDataPropertyNodeSet();
    }

    public NodeSet<OWLClass> getDataPropertyDomains(OWLDataProperty pe, boolean direct) throws InconsistentOntologyException, ReasonerInterruptedException, TimeOutException {
        checkConsistency();
        return getSuperClasses(getOWLDataFactory().getOWLDataSomeValuesFrom(getOWLDataFactory().getOWLTopDataProperty(), getOWLDataFactory().getTopDatatype()), direct);
    }

	// individuals
	
    public NodeSet<OWLClass> getTypes(OWLNamedIndividual ind, boolean direct) throws InconsistentOntologyException, ReasonerInterruptedException, TimeOutException {
        checkConsistency();
        return classExpressionTranslator.getNodeSetFromPointers(kernel.askIndividualTypes(toIndividualPointer(ind), direct));
    }

    public NodeSet<OWLNamedIndividual> getInstances(OWLClassExpression ce, boolean direct) throws InconsistentOntologyException, ClassExpressionNotInProfileException, ReasonerInterruptedException, TimeOutException {
        checkConsistency();
        return translateIndividualPointersToNodeSet(kernel.askInstances(toClassPointer(ce), direct));
    }

    public NodeSet<OWLNamedIndividual> getObjectPropertyValues(OWLNamedIndividual ind, OWLObjectPropertyExpression pe) throws InconsistentOntologyException, ReasonerInterruptedException, TimeOutException {
        checkConsistency();
        return translateIndividualPointersToNodeSet(kernel.askRelatedIndividuals(toIndividualPointer(ind), toObjectPropertyPointer(pe)));
    }

    public Set<OWLLiteral> getDataPropertyValues(OWLNamedIndividual ind, OWLDataProperty pe) throws InconsistentOntologyException, ReasonerInterruptedException, TimeOutException {
        // TODO:
        checkConsistency();
        return Collections.emptySet();
    }

    public Node<OWLNamedIndividual> getSameIndividuals(OWLNamedIndividual ind) throws InconsistentOntologyException, ReasonerInterruptedException, TimeOutException {
        checkConsistency();
        return individualTranslator.getNodeFromPointers(kernel.askSameAs(toIndividualPointer(ind)));
    }

    public NodeSet<OWLNamedIndividual> getDifferentIndividuals(OWLNamedIndividual ind) throws InconsistentOntologyException, ReasonerInterruptedException, TimeOutException {
        checkConsistency();
        OWLClassExpression ce = getOWLDataFactory().getOWLObjectOneOf(ind).getObjectComplementOf();
        return getInstances(ce, false);
    }


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////
    ////   Translation to FaCT++ structures and back
    ////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    private abstract class OWLEntityTranslator<E extends OWLObject, P extends Pointer> {

        private Map<E, P> entity2PointerMap = new HashMap<E, P>();

        protected Map<P, E> pointer2EntityMap = new HashMap<P, E>();

        protected void fillEntityPointerMaps ( E entity, P pointer ) {
            entity2PointerMap.put(entity, pointer);
            pointer2EntityMap.put(pointer, entity);
        }

        protected OWLEntityTranslator() {
            E topEntity = getTopEntity();
            if (topEntity != null) {
                fillEntityPointerMaps(topEntity, getTopEntityPointer());
            }
            E bottomEntity = getBottomEntity();
            if (bottomEntity != null) {
                fillEntityPointerMaps(bottomEntity, getBottomEntityPointer());
            }
        }

        protected P registerNewEntity(E entity) {
            P pointer = createPointerForEntity(entity);
            fillEntityPointerMaps(entity, pointer);
            return pointer;
        }

        public E getEntityFromPointer(P pointer) {
            return pointer2EntityMap.get(pointer);
        }

        public P getPointerFromEntity(E entity) {
            if (entity.isTopEntity()) {
                return getTopEntityPointer();
            }
            else if (entity.isBottomEntity()) {
                return getBottomEntityPointer();
            }
            else {
                P pointer = entity2PointerMap.get(entity);
                if (pointer == null) {
                    pointer = registerNewEntity(entity);
                }
                return pointer;
            }
        }

        public Node<E> getNodeFromPointers(P[] pointers) {
            DefaultNode<E> node = createDefaultNode();
            for (P pointer : pointers) {
                node.add(getEntityFromPointer(pointer));
            }
            return node;
        }

        public NodeSet<E> getNodeSetFromPointers(P[][] pointers) {
            DefaultNodeSet<E> nodeSet = createDefaultNodeSet();
            for (P[] pointerArray : pointers) {
                nodeSet.addNode(getNodeFromPointers(pointerArray));
            }
            return nodeSet;
        }

        protected abstract DefaultNode<E> createDefaultNode();

        protected abstract DefaultNodeSet<E> createDefaultNodeSet();

        protected abstract P getTopEntityPointer();

        protected abstract P getBottomEntityPointer();

        protected abstract P createPointerForEntity(E entity);

        protected abstract E getTopEntity();

        protected abstract E getBottomEntity();
    }


    private ClassPointer toClassPointer(OWLClassExpression classExpression) {
        return classExpression.accept(classExpressionTranslator);
    }

    private DataTypeExpressionPointer toDataTypeExpressionPointer(OWLDataRange dataRange) {
        return dataRange.accept(dataRangeTranslator);
    }

//    private OWLClassNode toOWLClassNode(ClassPointer [] classPointers) {
//        OWLClassNode node = new OWLClassNode();
//        for(ClassPointer cp : classPointers) {
//            node.add(classExpressionTranslator.getEntityFromPointer(cp));
//        }
//        return node;
//    }

//    private OWLClassNodeSet toOWLClassNodeSet(ClassPointer [][] classPointers) {
//
//        for(ClassPointer [] pointers : classPointers) {
//
//        }
//    }

//    private OWLObjectPropertyNode toOWLObjectPropertyNode(ObjectPropertyPointer [] propertyPointers) {
//
//    }
//
//    private OWLObjectPropertyNodeSet toOWLObjectPropertyNodeSet(ObjectPropertyPointer [][] propertyPointers) {
//
//    }
//
//
//    private OWLDataPropertyNodeSet toOWLDataPropertyNodeSet(DataPropertyPointer[][] dataPropertyPointers) {
//
//
//    }
//
//    private OWLDataPropertyNode toOWLDataPropertyNode(DataPropertyPointer [] dataPropertyPointers) {
//
//    }

    private ObjectPropertyPointer toObjectPropertyPointer(OWLObjectPropertyExpression propertyExpression) {
        OWLObjectPropertyExpression simp = propertyExpression.getSimplified();
        if (simp.isAnonymous()) {
            OWLObjectInverseOf inv = (OWLObjectInverseOf) simp;
            return kernel.getInverseProperty(objectPropertyTranslator.getPointerFromEntity(inv.getInverse().asOWLObjectProperty()));
        }
        else {
            return objectPropertyTranslator.getPointerFromEntity(simp.asOWLObjectProperty());
        }
    }

    protected DataPropertyPointer toDataPropertyPointer(OWLDataPropertyExpression propertyExpression) {
        return dataPropertyTranslator.getPointerFromEntity(propertyExpression.asOWLDataProperty());
    }

    protected IndividualPointer toIndividualPointer(OWLIndividual individual) {
//        return kernel.getIndividual(individual.toStringID());
        if (!individual.isAnonymous()) {
            return individualTranslator.getPointerFromEntity(individual.asOWLNamedIndividual());
        }
        else {
            return kernel.getIndividual(individual.toStringID());
//            throw new RuntimeException("Anonymous individuals not supported");
        }
    }

    protected DataTypePointer toDataTypePointer(OWLDatatype datatype) {
        if(datatype == null) {
            throw new NullPointerException();
        }
        return kernel.getBuiltInDataType(datatype.toStringID());
    }

    protected DataValuePointer toDataValuePointer(OWLLiteral literal) {
		String value = literal.getLiteral();
		if (literal.isRDFPlainLiteral()) {
			value = value + "@" + literal.getLang();
		}

		return kernel.getDataValue(value, toDataTypePointer(literal.getDatatype()));
		//  else {
		//      return kernel.getDataValue(literal.getLiteral(), toDataTypePointer(getOWLDataFactory().getTopDatatype()));
		//  }
    }

    private NodeSet<OWLNamedIndividual> translateIndividualPointersToNodeSet(IndividualPointer[] pointers) {
//        if (getIndividualNodeSetPolicy().equals(IndividualNodeSetPolicy.BY_SAME_AS)) {
//            OWLNamedIndividualNodeSet ns = new OWLNamedIndividualNodeSet();
//            for (IndividualPointer pointer : pointers) {
//                if (pointer != null) {
//                    IndividualPointer[] sameAsPointers = kernel.askSameAs(pointer);
//                    if (sameAsPointers != null && sameAsPointers.length > 0) {
//                        ns.addNode(individualTranslator.getNodeFromPointers(sameAsPointers));
//                    }
//                }
//            }
//            return ns;
//        }
//        else {
            OWLNamedIndividualNodeSet ns = new OWLNamedIndividualNodeSet();
            for (IndividualPointer pointer : pointers) {
                if (pointer != null) {
                    OWLNamedIndividual ind = individualTranslator.getEntityFromPointer(pointer);
                    ns.addEntity(ind);
                }
            }
            return ns;
//        }
    }

    private void translateIndividualSet(Set<OWLIndividual> inds) {
        kernel.initArgList();
        for (OWLIndividual ind : inds) {
            IndividualPointer ip = toIndividualPointer(ind);
            kernel.addArg(ip);
        }
        kernel.closeArgList();
    }

    private class ClassExpressionTranslator extends OWLEntityTranslator<OWLClass, ClassPointer> implements OWLClassExpressionVisitorEx<ClassPointer> {


        @Override
        protected ClassPointer getTopEntityPointer() {
            return kernel.getThing();
        }

        @Override
        protected ClassPointer getBottomEntityPointer() {
            return kernel.getNothing();
        }

        @Override
        protected OWLClass getTopEntity() {
            return getOWLDataFactory().getOWLThing();
        }

        @Override
        protected OWLClass getBottomEntity() {
            return getOWLDataFactory().getOWLNothing();
        }

        @Override
        protected ClassPointer createPointerForEntity(OWLClass entity) {
            return kernel.getNamedClass(entity.toStringID());
        }

        @Override
        protected DefaultNode<OWLClass> createDefaultNode() {
            return new OWLClassNode();
        }

        @Override
        protected DefaultNodeSet<OWLClass> createDefaultNodeSet() {
            return new OWLClassNodeSet();
        }

        public ClassPointer visit(OWLClass desc) {
            return getPointerFromEntity(desc);
        }

        public ClassPointer visit(OWLObjectIntersectionOf desc) {
            translateClassExpressionSet(desc.getOperands());
            return kernel.getConceptAnd();
        }

        private void translateClassExpressionSet(Set<OWLClassExpression> classExpressions) {
            kernel.initArgList();
            for (OWLClassExpression ce : classExpressions) {
                ClassPointer cp = ce.accept(this);
                kernel.addArg(cp);
            }
            kernel.closeArgList();
        }

        public ClassPointer visit(OWLObjectUnionOf desc) {
            translateClassExpressionSet(desc.getOperands());
            return kernel.getConceptOr();
        }

        public ClassPointer visit(OWLObjectComplementOf desc) {
            return kernel.getConceptNot(desc.getOperand().accept(this));
        }

        public ClassPointer visit(OWLObjectSomeValuesFrom desc) {
            return kernel.getObjectSome(toObjectPropertyPointer(desc.getProperty()), desc.getFiller().accept(this));
        }

        public ClassPointer visit(OWLObjectAllValuesFrom desc) {
            return kernel.getObjectAll(toObjectPropertyPointer(desc.getProperty()), desc.getFiller().accept(this));
        }

        public ClassPointer visit(OWLObjectHasValue desc) {
            return kernel.getObjectValue(toObjectPropertyPointer(desc.getProperty()), toIndividualPointer(desc.getValue()));
        }

        public ClassPointer visit(OWLObjectMinCardinality desc) {
            return kernel.getObjectAtLeast(desc.getCardinality(), toObjectPropertyPointer(desc.getProperty()), desc.getFiller().accept(this));
        }

        public ClassPointer visit(OWLObjectExactCardinality desc) {
            return kernel.getObjectExact(desc.getCardinality(), toObjectPropertyPointer(desc.getProperty()), desc.getFiller().accept(this));
        }

        public ClassPointer visit(OWLObjectMaxCardinality desc) {
            return kernel.getObjectAtMost(desc.getCardinality(), toObjectPropertyPointer(desc.getProperty()), desc.getFiller().accept(this));
        }

        public ClassPointer visit(OWLObjectHasSelf desc) {
            return kernel.getSelf(toObjectPropertyPointer(desc.getProperty()));
        }

        public ClassPointer visit(OWLObjectOneOf desc) {
            translateIndividualSet(desc.getIndividuals());
            return kernel.getOneOf();
        }


        public ClassPointer visit(OWLDataSomeValuesFrom desc) {
            return kernel.getDataSome(toDataPropertyPointer(desc.getProperty()), toDataTypeExpressionPointer(desc.getFiller()));
        }

        public ClassPointer visit(OWLDataAllValuesFrom desc) {
            return kernel.getDataAll(toDataPropertyPointer(desc.getProperty()), toDataTypeExpressionPointer(desc.getFiller()));
        }

        public ClassPointer visit(OWLDataHasValue desc) {
            return kernel.getDataValue(toDataPropertyPointer(desc.getProperty()), toDataValuePointer(desc.getValue()));
        }

        public ClassPointer visit(OWLDataMinCardinality desc) {
            return kernel.getDataAtLeast(desc.getCardinality(), toDataPropertyPointer(desc.getProperty()), toDataTypeExpressionPointer(desc.getFiller()));
        }

        public ClassPointer visit(OWLDataExactCardinality desc) {
            return kernel.getDataExact(desc.getCardinality(), toDataPropertyPointer(desc.getProperty()), toDataTypeExpressionPointer(desc.getFiller()));
        }

        public ClassPointer visit(OWLDataMaxCardinality desc) {
            return kernel.getDataAtMost(desc.getCardinality(), toDataPropertyPointer(desc.getProperty()), toDataTypeExpressionPointer(desc.getFiller()));
        }
    }

    private class DataRangeTranslator extends OWLEntityTranslator<OWLDatatype, DataTypePointer> implements OWLDataRangeVisitorEx<DataTypeExpressionPointer> {

        @Override
        protected DataTypePointer getTopEntityPointer() {
            return kernel.getDataTop();
        }

        @Override
        protected DataTypePointer getBottomEntityPointer() {
            return null;
        }

        @Override
        protected DefaultNode<OWLDatatype> createDefaultNode() {
            return new OWLDatatypeNode();
        }

        @Override
        protected OWLDatatype getTopEntity() {
            return getOWLDataFactory().getTopDatatype();
        }

        @Override
        protected OWLDatatype getBottomEntity() {
            return null;
        }

        @Override
        protected DefaultNodeSet<OWLDatatype> createDefaultNodeSet() {
            return new OWLDatatypeNodeSet();
        }

        @Override
        protected DataTypePointer createPointerForEntity(OWLDatatype entity) {
            return kernel.getBuiltInDataType(entity.toStringID());
        }

        public DataTypeExpressionPointer visit(OWLDatatype node) {
            return kernel.getBuiltInDataType(node.getIRI().toString());
        }

        public DataTypeExpressionPointer visit(OWLDataOneOf node) {
            kernel.initArgList();
            for (OWLLiteral literal : node.getValues()) {
                DataValuePointer dvp = toDataValuePointer(literal);
                kernel.addArg(dvp);
            }
            kernel.closeArgList();
            return kernel.getDataEnumeration();
        }

        public DataTypeExpressionPointer visit(OWLDataComplementOf node) {
            return kernel.getDataNot(node.getDataRange().accept(this));
        }

        public DataTypeExpressionPointer visit(OWLDataIntersectionOf node) {
            translateDataRangeSet(node.getOperands());
            return kernel.getDataIntersectionOf();
        }

        private void translateDataRangeSet(Set<OWLDataRange> dataRanges) {
            kernel.initArgList();
            for (OWLDataRange op : dataRanges) {
                DataTypeExpressionPointer dtp = op.accept(this);
                kernel.addArg(dtp);
            }
            kernel.closeArgList();
        }

        public DataTypeExpressionPointer visit(OWLDataUnionOf node) {
            translateDataRangeSet(node.getOperands());
            return kernel.getDataUnionOf();
        }

        public DataTypeExpressionPointer visit(OWLDatatypeRestriction node) {
            DataTypeExpressionPointer dte = node.getDatatype().accept(this);
            for (OWLFacetRestriction restriction : node.getFacetRestrictions()) {
                DataValuePointer dv = toDataValuePointer(restriction.getFacetValue());
                DataTypeFacet facet;
                if (restriction.getFacet().equals(OWLFacet.MIN_INCLUSIVE)) {
                    facet = kernel.getMinInclusiveFacet(dv);
                }
                else if (restriction.getFacet().equals(OWLFacet.MAX_INCLUSIVE)) {
                    facet = kernel.getMaxInclusiveFacet(dv);
                }
                else if (restriction.getFacet().equals(OWLFacet.MIN_EXCLUSIVE)) {
                    facet = kernel.getMinExclusiveFacet(dv);
                }
                else if (restriction.getFacet().equals(OWLFacet.MAX_EXCLUSIVE)) {
                    facet = kernel.getMaxExclusiveFacet(dv);
                }
                else if (restriction.getFacet().equals(OWLFacet.LENGTH)) {
                    facet = kernel.getLength(dv);
                }
                else if (restriction.getFacet().equals(OWLFacet.MIN_LENGTH)) {
                    facet = kernel.getMinLength(dv);
                }
                else if (restriction.getFacet().equals(OWLFacet.MAX_LENGTH)) {
                    facet = kernel.getMaxLength(dv);
                }
                else if (restriction.getFacet().equals(OWLFacet.FRACTION_DIGITS)) {
                    facet = kernel.getFractionDigitsFacet(dv);
                }
                else if (restriction.getFacet().equals(OWLFacet.PATTERN)) {
                    facet = kernel.getPattern(dv);
                }
                else if (restriction.getFacet().equals(OWLFacet.TOTAL_DIGITS)) {
                    facet = kernel.getTotalDigitsFacet(dv);
                }
                else {
                    throw new OWLRuntimeException("Unsupported facet: " + restriction.getFacet());
                }
                dte = kernel.getRestrictedDataType(dte, facet);
            }
            return dte;
        }
    }


    private class IndividualTranslator extends OWLEntityTranslator<OWLNamedIndividual, IndividualPointer> {
        @Override
        protected IndividualPointer getTopEntityPointer() {
            return null;
        }

        @Override
        protected IndividualPointer getBottomEntityPointer() {
            return null;
        }

        @Override
        protected IndividualPointer createPointerForEntity(OWLNamedIndividual entity) {
            return kernel.getIndividual(entity.toStringID());
        }

        @Override
        protected OWLNamedIndividual getTopEntity() {
            return null;
        }

        @Override
        protected OWLNamedIndividual getBottomEntity() {
            return null;
        }

        @Override
        protected DefaultNode<OWLNamedIndividual> createDefaultNode() {
            return new OWLNamedIndividualNode();
        }

        @Override
        protected DefaultNodeSet<OWLNamedIndividual> createDefaultNodeSet() {
            return new OWLNamedIndividualNodeSet();
        }
    }

    private class ObjectPropertyTranslator extends OWLEntityTranslator<OWLObjectPropertyExpression, ObjectPropertyPointer> {
        @Override
        protected ObjectPropertyPointer getTopEntityPointer() {
            return kernel.getTopObjectProperty();
        }

        @Override
        protected ObjectPropertyPointer getBottomEntityPointer() {
            return kernel.getBottomObjectProperty();
        }
		// TODO: add implementation of registerNewEntity
		@Override
		protected ObjectPropertyPointer registerNewEntity(OWLObjectPropertyExpression entity) {
			ObjectPropertyPointer pointer = createPointerForEntity(entity);
			fillEntityPointerMaps(entity, pointer);
			entity = entity.getInverseProperty().getSimplified();
			fillEntityPointerMaps(entity,createPointerForEntity(entity));
			return pointer;
		}
        @Override
        protected ObjectPropertyPointer createPointerForEntity(OWLObjectPropertyExpression entity) {
			// FIXME!! think later!!
			ObjectPropertyPointer p = kernel.getObjectProperty(entity.getNamedProperty().toStringID());
			if ( entity.isAnonymous() )	// inverse!
				p = kernel.getInverseProperty(p);
            return p;
        }

        @Override
        protected OWLObjectProperty getTopEntity() {
            return getOWLDataFactory().getOWLTopObjectProperty();
        }

        @Override
        protected OWLObjectProperty getBottomEntity() {
            return getOWLDataFactory().getOWLBottomObjectProperty();
        }

        @Override
        protected DefaultNode<OWLObjectPropertyExpression> createDefaultNode() {
            return new OWLObjectPropertyNode();
        }

        @Override
        protected DefaultNodeSet<OWLObjectPropertyExpression> createDefaultNodeSet() {
            return new OWLObjectPropertyNodeSet();
        }
    }

    private class DataPropertyTranslator extends OWLEntityTranslator<OWLDataProperty, DataPropertyPointer> {

        @Override
        protected DataPropertyPointer getTopEntityPointer() {
            return kernel.getTopDataProperty();
        }

        @Override
        protected DataPropertyPointer getBottomEntityPointer() {
            return kernel.getBottomDataProperty();
        }

        @Override
        protected DataPropertyPointer createPointerForEntity(OWLDataProperty entity) {
            return kernel.getDataProperty(entity.toStringID());
        }

        @Override
        protected OWLDataProperty getTopEntity() {
            return getOWLDataFactory().getOWLTopDataProperty();
        }

        @Override
        protected OWLDataProperty getBottomEntity() {
            return getOWLDataFactory().getOWLBottomDataProperty();
        }

        @Override
        protected DefaultNode<OWLDataProperty> createDefaultNode() {
            return new OWLDataPropertyNode();
        }

        @Override
        protected DefaultNodeSet<OWLDataProperty> createDefaultNodeSet() {
            return new OWLDataPropertyNodeSet();
        }
    }

    private class AxiomTranslator implements OWLAxiomVisitorEx<AxiomPointer> {

        public AxiomPointer visit(OWLSubClassOfAxiom axiom) {
            return kernel.tellSubClassOf(toClassPointer(axiom.getSubClass()), toClassPointer(axiom.getSuperClass()));
        }

        public AxiomPointer visit(OWLNegativeObjectPropertyAssertionAxiom axiom) {
            return kernel.tellNotRelatedIndividuals(toIndividualPointer(axiom.getSubject()), toObjectPropertyPointer(axiom.getProperty()), toIndividualPointer(axiom.getObject()));
        }

        public AxiomPointer visit(OWLAsymmetricObjectPropertyAxiom axiom) {
            return kernel.tellAsymmetricObjectProperty(toObjectPropertyPointer(axiom.getProperty()));
        }

        public AxiomPointer visit(OWLReflexiveObjectPropertyAxiom axiom) {
            return kernel.tellReflexiveObjectProperty(toObjectPropertyPointer(axiom.getProperty()));
        }

        public AxiomPointer visit(OWLDisjointClassesAxiom axiom) {
            translateClassExpressionSet(axiom.getClassExpressions());
            return kernel.tellDisjointClasses();
        }

        private void translateClassExpressionSet(Set<OWLClassExpression> classExpressions) {
            kernel.initArgList();
            for (OWLClassExpression ce : classExpressions) {
                ClassPointer cp = toClassPointer(ce);
                kernel.addArg(cp);
            }
            kernel.closeArgList();
        }

        public AxiomPointer visit(OWLDataPropertyDomainAxiom axiom) {
            return kernel.tellDataPropertyDomain(toDataPropertyPointer(axiom.getProperty()), toClassPointer(axiom.getDomain()));
        }

        public AxiomPointer visit(OWLObjectPropertyDomainAxiom axiom) {
            return kernel.tellObjectPropertyDomain(toObjectPropertyPointer(axiom.getProperty()), toClassPointer(axiom.getDomain()));
        }

        public AxiomPointer visit(OWLEquivalentObjectPropertiesAxiom axiom) {
            translateObjectPropertySet(axiom.getProperties());
            return kernel.tellEquivalentObjectProperties();
        }

        private void translateObjectPropertySet(Collection<OWLObjectPropertyExpression> properties) {
            kernel.initArgList();
            for (OWLObjectPropertyExpression property : properties) {
                ObjectPropertyPointer opp = toObjectPropertyPointer(property);
                kernel.addArg(opp);
            }
            kernel.closeArgList();
        }

        public AxiomPointer visit(OWLNegativeDataPropertyAssertionAxiom axiom) {
            return kernel.tellNotRelatedIndividualValue(toIndividualPointer(axiom.getSubject()), toDataPropertyPointer(axiom.getProperty()), toDataValuePointer(axiom.getObject()));
        }

        public AxiomPointer visit(OWLDifferentIndividualsAxiom axiom) {
            translateIndividualSet(axiom.getIndividuals());
            return kernel.tellDifferentIndividuals();
        }

        public AxiomPointer visit(OWLDisjointDataPropertiesAxiom axiom) {
            translateDataPropertySet(axiom.getProperties());
            return kernel.tellDisjointDataProperties();
        }

        private void translateDataPropertySet(Set<OWLDataPropertyExpression> properties) {
            kernel.initArgList();
            for (OWLDataPropertyExpression property : properties) {
                DataPropertyPointer dpp = toDataPropertyPointer(property);
                kernel.addArg(dpp);
            }
            kernel.closeArgList();
        }

        public AxiomPointer visit(OWLDisjointObjectPropertiesAxiom axiom) {
            translateObjectPropertySet(axiom.getProperties());
            return kernel.tellDisjointObjectProperties();
        }

        public AxiomPointer visit(OWLObjectPropertyRangeAxiom axiom) {
            return kernel.tellObjectPropertyRange(toObjectPropertyPointer(axiom.getProperty()), toClassPointer(axiom.getRange()));
        }

        public AxiomPointer visit(OWLObjectPropertyAssertionAxiom axiom) {
            return kernel.tellRelatedIndividuals(toIndividualPointer(axiom.getSubject()), toObjectPropertyPointer(axiom.getProperty()), toIndividualPointer(axiom.getObject()));
        }

        public AxiomPointer visit(OWLFunctionalObjectPropertyAxiom axiom) {
            return kernel.tellFunctionalObjectProperty(toObjectPropertyPointer(axiom.getProperty()));
        }

        public AxiomPointer visit(OWLSubObjectPropertyOfAxiom axiom) {
            return kernel.tellSubObjectProperties(toObjectPropertyPointer(axiom.getSubProperty()), toObjectPropertyPointer(axiom.getSuperProperty()));
        }

        public AxiomPointer visit(OWLDisjointUnionAxiom axiom) {
            axiom.getOWLEquivalentClassesAxiom().accept(this);
            axiom.getOWLDisjointClassesAxiom().accept(this);
            return null;
        }

        public AxiomPointer visit(OWLDeclarationAxiom axiom) {
            OWLEntity entity = axiom.getEntity();
            if (entity.isOWLClass()) {
                return kernel.tellClassDeclaration(toClassPointer(entity.asOWLClass()));
            }
            else if (entity.isOWLObjectProperty()) {
                return kernel.tellObjectPropertyDeclaration(toObjectPropertyPointer(entity.asOWLObjectProperty()));
            }
            else if (entity.isOWLDataProperty()) {
                return kernel.tellDataPropertyDeclaration(toDataPropertyPointer(entity.asOWLDataProperty()));
            }
            else if (entity.isOWLNamedIndividual()) {
                return kernel.tellIndividualDeclaration(toIndividualPointer(entity.asOWLNamedIndividual()));
            }
            else if (entity.isOWLDatatype()) {
                return kernel.tellDatatypeDeclaration(toDataTypePointer(entity.asOWLDatatype()));
            }
            return null;
        }

        public AxiomPointer visit(OWLAnnotationAssertionAxiom axiom) {
            // Ignore
            return null;
        }

        public AxiomPointer visit(OWLSymmetricObjectPropertyAxiom axiom) {
            return kernel.tellSymmetricObjectProperty(toObjectPropertyPointer(axiom.getProperty()));
        }

        public AxiomPointer visit(OWLDataPropertyRangeAxiom axiom) {
            return kernel.tellDataPropertyRange(toDataPropertyPointer(axiom.getProperty()), toDataTypeExpressionPointer(axiom.getRange()));
        }

        public AxiomPointer visit(OWLFunctionalDataPropertyAxiom axiom) {
            return kernel.tellFunctionalDataProperty(toDataPropertyPointer(axiom.getProperty()));
        }

        public AxiomPointer visit(OWLEquivalentDataPropertiesAxiom axiom) {
            translateDataPropertySet(axiom.getProperties());
            return kernel.tellEquivalentDataProperties();
        }

        public AxiomPointer visit(OWLClassAssertionAxiom axiom) {
            return kernel.tellIndividualType(toIndividualPointer(axiom.getIndividual()), toClassPointer(axiom.getClassExpression()));
        }

        public AxiomPointer visit(OWLEquivalentClassesAxiom axiom) {
            translateClassExpressionSet(axiom.getClassExpressions());
            return kernel.tellEquivalentClass();
        }

        public AxiomPointer visit(OWLDataPropertyAssertionAxiom axiom) {
            return kernel.tellRelatedIndividualValue(toIndividualPointer(axiom.getSubject()), toDataPropertyPointer(axiom.getProperty()), toDataValuePointer(axiom.getObject()));
        }

        public AxiomPointer visit(OWLTransitiveObjectPropertyAxiom axiom) {
            return kernel.tellTransitiveObjectProperty(toObjectPropertyPointer(axiom.getProperty()));
        }

        public AxiomPointer visit(OWLIrreflexiveObjectPropertyAxiom axiom) {
            return kernel.tellIrreflexiveObjectProperty(toObjectPropertyPointer(axiom.getProperty()));
        }

        public AxiomPointer visit(OWLSubDataPropertyOfAxiom axiom) {
            return kernel.tellSubDataProperties(toDataPropertyPointer(axiom.getSubProperty()), toDataPropertyPointer(axiom.getSuperProperty()));
        }

        public AxiomPointer visit(OWLInverseFunctionalObjectPropertyAxiom axiom) {
            return kernel.tellInverseFunctionalObjectProperty(toObjectPropertyPointer(axiom.getProperty()));
        }

        public AxiomPointer visit(OWLSameIndividualAxiom axiom) {
            translateIndividualSet(axiom.getIndividuals());
            return kernel.tellSameIndividuals();
        }

        public AxiomPointer visit(OWLSubPropertyChainOfAxiom axiom) {
            translateObjectPropertySet(axiom.getPropertyChain());
            return kernel.tellSubObjectProperties(kernel.getPropertyComposition(), toObjectPropertyPointer(axiom.getSuperProperty()));
        }

        public AxiomPointer visit(OWLInverseObjectPropertiesAxiom axiom) {
            return kernel.tellInverseProperties(toObjectPropertyPointer(axiom.getFirstProperty()), toObjectPropertyPointer(axiom.getSecondProperty()));
        }

        public AxiomPointer visit(OWLHasKeyAxiom axiom) {
            translateObjectPropertySet(axiom.getObjectPropertyExpressions());
            ObjectPropertyPointer objectPropertyPointer = kernel.getObjectPropertyKey();
            translateDataPropertySet(axiom.getDataPropertyExpressions());
            DataPropertyPointer dataPropertyPointer = kernel.getDataPropertyKey();
            return kernel.tellHasKey(toClassPointer(axiom.getClassExpression()), dataPropertyPointer, objectPropertyPointer);
        }

        public AxiomPointer visit(OWLDatatypeDefinitionAxiom axiom) {
            kernel.getDataSubType(axiom.getDatatype().getIRI().toString(), toDataTypeExpressionPointer(axiom.getDataRange()));
            return null;
        }

        public AxiomPointer visit(SWRLRule rule) {
            // Ignore
            return null;
        }

        public AxiomPointer visit(OWLSubAnnotationPropertyOfAxiom axiom) {
            // Ignore
            return null;
        }

        public AxiomPointer visit(OWLAnnotationPropertyDomainAxiom axiom) {
            // Ignore
            return null;
        }

        public AxiomPointer visit(OWLAnnotationPropertyRangeAxiom axiom) {
            // Ignore
            return null;
        }
    }


    private class EntailmentChecker implements OWLAxiomVisitorEx<Boolean> {

        public Boolean visit(OWLSubClassOfAxiom axiom) {
            return kernel.isClassSubsumedBy(toClassPointer(axiom.getSubClass()), toClassPointer(axiom.getSuperClass()));
        }

        public Boolean visit(OWLNegativeObjectPropertyAssertionAxiom axiom) {
            return axiom.asOWLSubClassOfAxiom().accept(this);
        }

        public Boolean visit(OWLAsymmetricObjectPropertyAxiom axiom) {
            return kernel.isObjectPropertyAsymmetric(toObjectPropertyPointer(axiom.getProperty()));
        }

        public Boolean visit(OWLReflexiveObjectPropertyAxiom axiom) {
            return kernel.isObjectPropertyReflexive(toObjectPropertyPointer(axiom.getProperty()));
        }

        public Boolean visit(OWLDisjointClassesAxiom axiom) {
            Set<OWLClassExpression> classExpressions = axiom.getClassExpressions();
            if (classExpressions.size() == 2) {
                Iterator<OWLClassExpression> it = classExpressions.iterator();
                return kernel.isClassDisjointWith(toClassPointer(it.next()), toClassPointer(it.next()));
            }
            else {
                for (OWLAxiom ax : axiom.asOWLSubClassOfAxioms()) {
                    if (!ax.accept(this)) {
                        return false;
                    }
                }
                return true;
            }
        }

        public Boolean visit(OWLDataPropertyDomainAxiom axiom) {
            return axiom.asOWLSubClassOfAxiom().accept(this);
        }

        public Boolean visit(OWLObjectPropertyDomainAxiom axiom) {
            return axiom.asOWLSubClassOfAxiom().accept(this);
        }

        public Boolean visit(OWLEquivalentObjectPropertiesAxiom axiom) {
            for (OWLAxiom ax : axiom.asSubObjectPropertyOfAxioms()) {
                if (!ax.accept(this)) {
                    return false;
                }
            }
            return true;
        }

        public Boolean visit(OWLNegativeDataPropertyAssertionAxiom axiom) {
            return axiom.asOWLSubClassOfAxiom().accept(this);
        }

        public Boolean visit(OWLDifferentIndividualsAxiom axiom) {
            for (OWLSubClassOfAxiom ax : axiom.asOWLSubClassOfAxioms()) {
                if (!ax.accept(this)) {
                    return false;
                }
            }
            return true;
        }

		// TODO: this check is incomplete
        public Boolean visit(OWLDisjointDataPropertiesAxiom axiom) {
			kernel.initArgList();
			for (OWLDataPropertyExpression p : axiom.getProperties())	{
				kernel.addArg(toDataPropertyPointer(p));
			}
			kernel.closeArgList();
			return kernel.arePropertiesDisjoint();
        }

        public Boolean visit(OWLDisjointObjectPropertiesAxiom axiom) {
			kernel.initArgList();
			for (OWLObjectPropertyExpression p : axiom.getProperties())	{
				kernel.addArg(toObjectPropertyPointer(p));
			}
			kernel.closeArgList();
			return kernel.arePropertiesDisjoint();
        }

        public Boolean visit(OWLObjectPropertyRangeAxiom axiom) {
            return axiom.asOWLSubClassOfAxiom().accept(this);
        }

        public Boolean visit(OWLObjectPropertyAssertionAxiom axiom) {
            return axiom.asOWLSubClassOfAxiom().accept(this);
        }

        public Boolean visit(OWLFunctionalObjectPropertyAxiom axiom) {
            return kernel.isObjectPropertyFunctional(toObjectPropertyPointer(axiom.getProperty()));
        }

        public Boolean visit(OWLSubObjectPropertyOfAxiom axiom) {
            return kernel.isObjectSubPropertyOf(toObjectPropertyPointer(axiom.getSubProperty()), toObjectPropertyPointer(axiom.getSuperProperty()));
        }

        public Boolean visit(OWLDisjointUnionAxiom axiom) {
            return axiom.getOWLEquivalentClassesAxiom().accept(this) && axiom.getOWLDisjointClassesAxiom().accept(this);
        }

        public Boolean visit(OWLDeclarationAxiom axiom) {
            return false;
        }

        public Boolean visit(OWLAnnotationAssertionAxiom axiom) {
            return false;
        }

        public Boolean visit(OWLSymmetricObjectPropertyAxiom axiom) {
            return kernel.isObjectPropertySymmetric(toObjectPropertyPointer(axiom.getProperty()));
        }

        public Boolean visit(OWLDataPropertyRangeAxiom axiom) {
            return axiom.asOWLSubClassOfAxiom().accept(this);
        }

        public Boolean visit(OWLFunctionalDataPropertyAxiom axiom) {
            return kernel.isDataPropertyFunctional(toDataPropertyPointer(axiom.getProperty()));
        }

        public Boolean visit(OWLEquivalentDataPropertiesAxiom axiom) {
/*	this is not implemented in OWL API
			for (OWLAxiom ax : axiom.asSubDataPropertyOfAxioms()) {
                if (!ax.accept(this)) {
                    return false;
                }
            }
            return true;
*/
			return null;
        }

        public Boolean visit(OWLClassAssertionAxiom axiom) {
            return kernel.isInstanceOf(toIndividualPointer(axiom.getIndividual()), toClassPointer(axiom.getClassExpression()));
        }

        public Boolean visit(OWLEquivalentClassesAxiom axiom) {
            Set<OWLClassExpression> classExpressionSet = axiom.getClassExpressions();
            if (classExpressionSet.size() == 2) {
                Iterator<OWLClassExpression> it = classExpressionSet.iterator();
                return kernel.isClassEquivalentTo(toClassPointer(it.next()), toClassPointer(it.next()));
            }
            else {
                for (OWLAxiom ax : axiom.asOWLSubClassOfAxioms()) {
                    if (!ax.accept(this)) {
                        return false;
                    }
                }
                return true;
            }
        }

        public Boolean visit(OWLDataPropertyAssertionAxiom axiom) {
            return axiom.asOWLSubClassOfAxiom().accept(this);
        }

        public Boolean visit(OWLTransitiveObjectPropertyAxiom axiom) {
            return kernel.isObjectPropertyTransitive(toObjectPropertyPointer(axiom.getProperty()));
        }

        public Boolean visit(OWLIrreflexiveObjectPropertyAxiom axiom) {
            return kernel.isObjectPropertyIrreflexive(toObjectPropertyPointer(axiom.getProperty()));
        }

		// TODO: this is incomplete
        public Boolean visit(OWLSubDataPropertyOfAxiom axiom) {
            return kernel.isDataSubPropertyOf(toDataPropertyPointer(axiom.getSubProperty()), toDataPropertyPointer(axiom.getSuperProperty()));
        }

        public Boolean visit(OWLInverseFunctionalObjectPropertyAxiom axiom) {
            return kernel.isObjectPropertyInverseFunctional(toObjectPropertyPointer(axiom.getProperty()));
        }

        public Boolean visit(OWLSameIndividualAxiom axiom) {
            for (OWLSameIndividualAxiom ax : axiom.asPairwiseAxioms()) {
                Iterator<OWLIndividual> it = ax.getIndividuals().iterator();
                OWLIndividual indA = it.next();
                OWLIndividual indB = it.next();
                if (!kernel.isSameAs(toIndividualPointer(indA), toIndividualPointer(indB))) {
                    return false;
                }
            }
            return true;
        }

        public Boolean visit(OWLSubPropertyChainOfAxiom axiom) {
            kernel.initArgList();
			for (OWLObjectPropertyExpression p : axiom.getPropertyChain())	{
				kernel.addArg(toObjectPropertyPointer(p));
			}
			kernel.closeArgList();
			return kernel.isSubPropertyChainOf(toObjectPropertyPointer(axiom.getSuperProperty()));
        }

        public Boolean visit(OWLInverseObjectPropertiesAxiom axiom) {
            for (OWLAxiom ax : axiom.asSubObjectPropertyOfAxioms()) {
                if (!ax.accept(this)) {
                    return false;
                }
            }
            return true;
        }

        public Boolean visit(OWLHasKeyAxiom axiom) {
			// FIXME!! unsupported by FaCT++ ATM
            return null;
        }

        public Boolean visit(OWLDatatypeDefinitionAxiom axiom) {
			// FIXME!! unsupported by FaCT++ ATM
            return null;
        }

        public Boolean visit(SWRLRule rule) {
			// FIXME!! unsupported by FaCT++ ATM
            return null;
        }

        public Boolean visit(OWLSubAnnotationPropertyOfAxiom axiom) {
            return false;
        }

        public Boolean visit(OWLAnnotationPropertyDomainAxiom axiom) {
            return false;
        }

        public Boolean visit(OWLAnnotationPropertyRangeAxiom axiom) {
            return false;
        }
    }

    @Override
    public void dispose() {
        super.dispose();
        axiomTranslator = new AxiomTranslator();
        classExpressionTranslator = new ClassExpressionTranslator();
        dataRangeTranslator = new DataRangeTranslator();
        objectPropertyTranslator = new ObjectPropertyTranslator();
        dataPropertyTranslator = new DataPropertyTranslator();
        individualTranslator = new IndividualTranslator();
        kernel.dispose();
    }

    private class ProgressMonitorAdapter implements FaCTPlusPlusProgressMonitor {

        private int count = 0;

        private int total = 0;

        public void setClassificationStarted(int classCount) {
            count = 0;
            total = classCount;
            getReasonerConfiguration().getProgressMonitor().reasonerTaskStarted(ReasonerProgressMonitor.CLASSIFYING);
            getReasonerConfiguration().getProgressMonitor().reasonerTaskProgressChanged(count, classCount);
        }

        public void nextClass() {
            count++;
            getReasonerConfiguration().getProgressMonitor().reasonerTaskProgressChanged(count, total);

        }

        public void setFinished() {
            getReasonerConfiguration().getProgressMonitor().reasonerTaskStopped();
        }

        public boolean isCancelled() {
            return interrupted;
        }

    }


    public void dumpClassHierarchy(PrintStream pw, boolean includeBottomNode) {
        dumpSubClasses(getTopClassNode(), pw, 0, includeBottomNode);
    }


    private void dumpSubClasses(Node<OWLClass> node, PrintStream pw, int depth, boolean includeBottomNode) {
        if (includeBottomNode || !node.isBottomNode()) {
            for (int i = 0; i < depth; i++) {
                pw.print("    ");
            }
            pw.println(node);
            for (Node<OWLClass> sub : getSubClasses(node.getRepresentativeElement(), true)) {
                dumpSubClasses(sub, pw, depth + 1, includeBottomNode);
            }
        }
    }
}
