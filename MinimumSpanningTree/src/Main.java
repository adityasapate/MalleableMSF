import java.util.Collection;


public class Main {

	public static void main(String[] args) {
		// TODO Auto-generated method stub
		Graph graph = new Graph("12 0 1 16 0 2 12 0 3 21 1 3 17 1 4 20 2 3 28 2 5 31 3 4 18 3 5 19 3 6 23 4 6 11 5 6 27");
//		Collection<Edge> edge1 = graph.getEdgeList();
		
//		for(Edge e : edge1){
//			System.out.println(e.getWeight()+" "+ e.getU().getId() + " " + e.getV().getId());
//		}
//		
//		System.out.println();
		MST m = new MST();
		Collection<Edge> edge = m.kruskals(graph);
		
		
		if (edge == null){
			System.out.println("No valid mst");
		}else
			for(Edge e : edge){
				System.out.println(e.getWeight()+" "+ e.getU().getId() + " " + e.getV().getId());
			}
	}

}
