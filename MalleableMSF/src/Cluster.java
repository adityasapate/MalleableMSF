import java.util.Iterator;
import java.util.LinkedHashSet;
import java.util.Set;


public class Cluster {
	int root;
	Set<Integer> vertices;
	Set<Edge> out_edges;
	int status;
	
	Cluster() {
		root = -1;
		vertices = new LinkedHashSet<Integer>();
		out_edges = new LinkedHashSet<Edge>();
	}
	
	Cluster(int i) {
		root = i;
		vertices = new LinkedHashSet<Integer>();
		out_edges = new LinkedHashSet<Edge>();
		vertices.add(new Integer(i));
	}
	
	void printOutEdges() {
		synchronized (out_edges) {
			Iterator<Edge> it = out_edges.iterator();
			while(it.hasNext()) {
				Edge e = it.next();
				System.out.println("Out edges of cluster : ");
				e.print();
			}
		}
	}
	
	void print() {
		System.out.print(root+":{");
		synchronized (vertices) {
			Iterator<Integer> it = vertices.iterator();
			while(it.hasNext()) {
				System.out.print(it.next()+" ");
			}
			System.out.println("}");
		}
	}
}
