
public class Edge{
	int from;
	int to;
	float len;
	
	Edge() {
		from = -1;
		to = -1;
		len = -1;
	}
	
	Edge(int a, int b, float c) {
		from = a;
		to = b;
		len = c;
	}
	
	void print() {
		System.out.println(from+"->"+to+" : "+len);
	}	
	
}
