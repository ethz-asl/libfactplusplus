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

    private long id = 0;

    private long node = 0;

    public long getId() {
        return id;
    }

    public long getNode() {
        return node;
    }

    public int hashCode() {
        return (int)(id != 0 ? id : node);
    }

    public boolean equals(Object obj) {
        if (!(obj instanceof Pointer)){
            return false;
        }

        if(id != 0) {
            return ((Pointer) obj).id == id;
        }
        else {
            return ((Pointer) obj).node == node;
        }
    }

    public String toString() {
        return "[" + id + ", " + node + "]";
    }
}
