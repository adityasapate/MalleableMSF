import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

public class DisjointSets<T> {

	public Map<T, T> disjointSets = new HashMap<T, T>();
	
	/**
	 * @param setItems
	 *            the initial items (sameSet and merge will never be called with
	 *            items not in this set, this set will never contain null
	 *            elements)
	 */
	public DisjointSets(Set<T> setItems) {
		// TODO
		
		for(T t : setItems) {
			disjointSets.put(t, t);
		}
	}

	/**
	 * @param u
	 * @param v
	 * @return true if the items given are in the same set, false otherwise
	 */
	public boolean sameSet(T u, T v) {
		// TODO
		
		int lead_u = ((Vertex)disjointSets.get(u)).getId();
		int lead_v = ((Vertex)disjointSets.get(v)).getId();
		if(lead_u == lead_v){
			return true;
		}
		
		return false;
	}

	/**
	 * merges the sets u and v are in, do nothing if they are in the same set
	 * You are required to implement the following in this method:
	 * 	Path compression: every node points to its root
	 *  Merge by rank: Let the rank (estimate tree depth) of each set initially be 0. 
	 *  When merging to set with different ranks, make the smaller ranked root point to the larger root.
	 *  If the two ranks are the same, choose one to point to the other, and increment the rank of the new set
	 *
	 * @param u
	 * @param v
	 */
	public void merge(T u, T v) {
		// TODO
		
		T u_map = disjointSets.get(u);
		T v_map = disjointSets.get(v);
		
		
				
		
		for(T var : disjointSets.keySet()){
			if(disjointSets.get(var).equals(v_map)){
				disjointSets.put(var, u_map);
			}
		}
		
	}
}
