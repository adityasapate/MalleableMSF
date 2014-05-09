import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.Iterator;
import java.util.LinkedHashSet;
import java.util.LinkedList;
import java.util.ListIterator;
import java.util.Queue;
import java.util.Random;
import java.util.Set;


public class Main {
	
	static int no_of_cores;
	static Set<Cluster> cluster_set;
	static int no_of_vertices;
	static int vertices_finished;
	static Queue<WorkPacket> queue;
	static int barrier;
	static Set<Edge> spanning_forest;
	
	static class Argument {
		Cluster c1,c2;
		Edge e;
		Argument(Cluster c1, Cluster c2, Edge e) {
			this.c1 = c1;
			this.c2 = c2;
			this.e = e;
		}
	}
	
	static class ExtendCluster implements Function {
		
		@Override
		public void execute(Object data) {
			// TODO Auto-generated method stub
			System.out.println("Starting extend cluster function");
			Cluster c = (Cluster)data;
			
			//Check if the cluster has out-edges
			if(c.out_edges.isEmpty()) {
				c.status = 1;
				vertices_finished += c.vertices.size();
				return;
			}
			
			Iterator<Edge> it = c.out_edges.iterator();
			float min_len = Float.MAX_VALUE;
			Edge min_edge = null;
			while(it.hasNext()) {
				Edge e = it.next();
				if(e.len < min_len) {
					min_len = e.len;
					min_edge = e;
				}
			}
//			spanning_forest.add(min_edge);
//			c.print();
//			c.printOutEdges();
//			getCluster(min_edge.to).print();
//			getCluster(min_edge.to).printOutEdges();
			Argument arg = new Argument(c, getCluster(min_edge.to), min_edge);
			WorkPacket w = new WorkPacket(new MergeCluster(), arg);
			queue.add(w);
		}
		
	}
	
	static class MergeCluster implements Function {

		@Override
		public void execute(Object data) {
			// TODO Auto-generated method stub
			System.out.println("Starting merge cluster function");
			Argument arg = (Argument)data;
			int v1, v2;
			Iterator<Integer> iter = arg.c1.vertices.iterator();
			v1 = iter.next();
			Cluster c1 = getCluster(v1);
			iter = arg.c2.vertices.iterator();
			v2 = iter.next();
			Cluster c2 = getCluster(v2);
			
			synchronized (c1) {
				synchronized (c2) {
					//Validation
					if(getCluster(arg.e.from) == getCluster(arg.e.to)) {
						System.out.println("Same clusters..not merging");
						return;
					}
					spanning_forest.add(arg.e);
					//Add edges to c1
					Iterator<Edge> it = c2.out_edges.iterator();
					while(it.hasNext()) {
						Edge e = it.next();
						if(getCluster(e.to) != c1) {
							getCluster(v1).out_edges.add(e);
						}
					}
					//Remove common edges from c1
					it = c1.out_edges.iterator();
					while(it.hasNext()) {
						Edge e = it.next();
						if(getCluster(e.to) == c2) {
							it.remove();
						}
					}
					//Merge vertices
					c1.vertices.addAll(c2.vertices);
				}
			}
			synchronized (cluster_set) {
				cluster_set.remove(c2);
			}
//			c1.print();
//			c1.printOutEdges();
		}
		
	}
	
	public static class WorkerThread implements Runnable{

		int threadid;
		public WorkerThread(int i) {
			// TODO Auto-generated constructor stub
			threadid = i;
		}
		@Override
		public void run() {
			// TODO Auto-generated method stub
			System.out.println("Starting thread "+threadid);
			while(true) {
				if(queue.isEmpty()) {
					if(barrier == 1) 
						break;
					else
						continue;
				}
				WorkPacket obj;
				synchronized (queue) {
					if(queue.isEmpty())
						continue;
					obj = queue.poll();
				}
				System.out.println("Thread "+threadid+" : Getting the work packet and starting the necessary work");
				obj.function.execute(obj.args);
			}
		}

	}
	
	public static class Schedule implements Runnable {

		@Override
		public void run() {
			// TODO Auto-generated method stub
			System.out.println("Scheduler thread started");
			int worker_cores = no_of_cores - 1;
			Thread[] t = new Thread[worker_cores];
			for(int i = 0; i < worker_cores; i++) {
				WorkerThread w = new WorkerThread(i);
				t[i] = new Thread(w);
				t[i].start();
			}
			for(int i = 0; i < worker_cores; i++)
				try {
					t[i].join();
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
		}
		
	}
	
	static void setNoOfCores(int cores) {
		int physical_cores = Runtime.getRuntime().availableProcessors();
		if(cores > physical_cores) 
			no_of_cores = physical_cores;
		else
			no_of_cores = cores;
	}
	
	static Cluster getCluster(int vertex) {
		synchronized (cluster_set) {
			Iterator<Cluster> it = cluster_set.iterator();
			while(it.hasNext()) {
				Cluster c = it.next();
				if(c.vertices.contains(Integer.valueOf(vertex)))
					return c;
			}
		}
		return null;
	}
	
	static void printClusterSet() {
		Iterator<Cluster> it = cluster_set.iterator();
		while(it.hasNext()) {
			Cluster c = it.next();
			c.print();
			c.printOutEdges();
		}
	}
	
	static void init(String args[]) throws NumberFormatException, IOException {
		
		int cores = Integer.parseInt(args[0]);
		setNoOfCores(cores);
		BufferedReader reader = null;
		
		try {
			reader = new BufferedReader(new FileReader(args[1]));
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			System.exit(-1);
		}
		cluster_set = new LinkedHashSet<Cluster>();
		no_of_vertices = Integer.parseInt(reader.readLine());
		
		for (int i = 0; i < no_of_vertices; i++) {
			Cluster cluster = new Cluster(i);
			cluster.status = 0;
			cluster_set.add(cluster);
		}
		String str;
		while((str = reader.readLine()) != null) {
			String[] tokens = str.split(" ");
			int v1 = Integer.parseInt(tokens[0]);
			int v2 = Integer.parseInt(tokens[1]);
			Edge e1 = new Edge(v1, v2, Float.parseFloat(tokens[2]));
			Edge e2 = new Edge(v2, v1, Float.parseFloat(tokens[2]));
			getCluster(v1).out_edges.add(e1);
			getCluster(v2).out_edges.add(e2);
		}
		
	}
	
	public static void main(String[] args) throws NumberFormatException, IOException, InterruptedException {
		init(args);
//		printClusterSet();
		queue = new LinkedList<WorkPacket>();
		spanning_forest = new LinkedHashSet<Edge>();
		Thread t = new Thread(new Schedule());
		t.start();
		Random ran = new Random();
		for(int i = 0; i < no_of_vertices; i++) {
			Cluster c = getCluster(3-i);
			WorkPacket w = new WorkPacket(new ExtendCluster(), c);
			queue.add(w);
		}
		barrier = 1;
		t.join();
		Iterator<Edge> it = spanning_forest.iterator();
		while(it.hasNext()) {
			Edge e = it.next();
			e.print();
		}
	}
}
