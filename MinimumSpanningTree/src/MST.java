import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class MST {

    /**
     * Using Disjoint set(s), run Kruskal's algorithm on the given graph and return the MST, return
     * null if no MST exists for the graph
     * 
     * @param g
     *            The graph, g will never be null
     * @return the MST of the graph, null if no valid MST exists
     */
    public static Collection<Edge> kruskals(Graph g) {
        // TODO
    	
    	Set<Edge> mst = new HashSet<Edge>(); 
    	Set<Vertex> vertices = g.getVertices(); 
    	
    	DisjointSets<Vertex> disjoint_set = new DisjointSets<Vertex>(vertices);
    	
    	
    	
    	while(mst.size() < g.getVertices().size()) {
        	int minWeight = Integer.MAX_VALUE;
    		Edge min_e = null;
    	
    		for(Edge e : g.getEdgeList()) {
    			if(e.getWeight() < minWeight && !disjoint_set.sameSet(e.getU(), e.getV())) {
					min_e = e;
					
					minWeight = e.getWeight();
    			}
    		}
    	
    		if(mst.size() ==g.getVertices().size() -1 )
    			return mst;
    		
    		if(min_e == null)
    			return null;
    		
    		mst.add(min_e);
    		
//    		for(Edge e : mst){
//    			System.out.println(e.getU().getId() + " " + e.getV().getId() +"  " + disjoint_set.disjointSets.get(e.getU()).getId() +" " + disjoint_set.disjointSets.get(e.getU()).getId());
//    		}

    		disjoint_set.merge(min_e.getU(), min_e.getV());
    			
    		
    	}
    	
    	
        return mst;
    }

    /**
     * Run Prim's algorithm on the given graph and return the minimum spanning tree
     * If no MST exists, return null
     * 
     * @param g 
     * 				The graph to be processed.  Will never be null
     * @param start 
     * 				The ID of the start node.  Will always exist in the graph
     * @return the MST of the graph, null if no valid MST exists
     */
    public static Collection<Edge> prims(Graph g, int start){
    	
    	Set<Vertex> T = new HashSet<Vertex>();
    	Collection<Edge> edges = new HashSet<Edge>();
    	
    	Vertex startNode = null;
    	
    	for(Vertex v : g.getVertices()) {
    		if(v.getId() == start)
    			startNode = v;
    	}
    	
    	T.add(startNode);
    
    	while(T.size() < g.getVertices().size()) {
        	int minWeight = Integer.MAX_VALUE;
    		Edge min_e = null;
    		
    		for(Edge e : g.getEdgeList()) {
    			if((T.contains(e.getU()) && !T.contains(e.getV())) || (T.contains(e.getV()) && !T.contains(e.getU()))) {
    				if(e.getWeight() < minWeight) {
    					min_e = e;    	
    					minWeight = e.getWeight();
    				}
    			}
    		}
    		
    		if(min_e == null)
    			return null;
    		
    		T.add(min_e.getV());
    		T.add(min_e.getU());
    		edges.add(min_e);
    		
    	}
    	
    	
    	
        return edges;
    }
}