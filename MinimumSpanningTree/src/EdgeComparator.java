import java.util.Comparator;


public class EdgeComparator implements Comparator<Edge> {
	
	Edge e1, e2;
	
	public EdgeComparator(Edge e1, Edge e2) {
		this.e1 = e1;
		this.e2 = e2;
	}

	@Override
	public int compare(Edge e1, Edge e2) {
		// TODO Auto-generated method stub
		return e1.getWeight() - e2.getWeight();
	}

}
