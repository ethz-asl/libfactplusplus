package uk.ac.manchester.cs.factplusplus;

/**
 * Author: Matthew Horridge<br>
 * The University Of Manchester<br>
 * Medical Informatics Group<br>
 * Date: 10-Jul-2006<br><br>
 * <p/>
 * matthew.horridge@cs.man.ac.uk<br>
 * www.cs.man.ac.uk/~horridgm<br><br>
 */
public class Pointer {

    private long node = 0;

    public long getNode() {
        return node;
    }

    public int hashCode() {
        return (int)node;
    }

    public boolean equals(Object obj) {
        if (!(obj instanceof Pointer)){
            return false;
        }

        return ((Pointer) obj).node == node;
    }

    public String toString() {
        return "[" + node + "]";
    }
}
