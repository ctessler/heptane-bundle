#include "PQueueTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION(PQueueTest);

void
PQueueTest::setUp()
{
}

void
PQueueTest::tearDown()
{
}

template<typename T> static void
print_queue(T &q) {
	while (!q.empty()) {
		cout << q.top() << " ";
		q.pop();
	}
	cout << endl;
}

void
PQueueTest::basic()
{
	cout << endl;
	priority_queue<int> int_default;
	int_default.push(10);
	int_default.push(-10);
	int_default.push(5);
	int_default.push(16);

	cout << "PQueueTest::basic" << endl;
	print_queue(int_default);
}

class PQTCompare {
public:
	bool operator() (const int &lhs, const int &rhs) const {
		if (lhs > rhs) {
			/* LHS is "less than" RHS */
			return true;
		}
		return false;
	}
};

void
PQueueTest::reverse()
{
	cout << endl;
	priority_queue<int, vector<int>, PQTCompare> int_rev;
	int_rev.push(10);
	int_rev.push(-10);
	int_rev.push(5);
	int_rev.push(16);

	cout << "PQueueTest::reverse" << endl;
	print_queue(int_rev);
}

class NodeComp {
public:
	NodeComp(ListDigraph::NodeMap<int> &distances) : _dist(distances) {
	}
	bool operator() (const ListDigraph::Node &lhs,
			 const ListDigraph::Node &rhs) const {
		if (_dist[lhs] < _dist[rhs]) {
			/* Greatest first */
			return true;
		}
		return false;
	}
private:
	ListDigraph::NodeMap<int> &_dist;
};

static void
print_node_queue(priority_queue<ListDigraph::Node, vector<ListDigraph::Node>,
		 NodeComp> &q, ListDigraph::NodeMap<int> &dist) {
	while (!q.empty()) {
		ListDigraph::Node node = q.top(); q.pop();
		cout << dist[node] << " ";
	}
	cout << endl;
}

	
void
PQueueTest::node()
{
	cout << endl;
	ListDigraph g;
	ListDigraph::NodeMap<int> dists(g);
	priority_queue<ListDigraph::Node, vector<ListDigraph::Node>,
		       NodeComp> basic_digraph((NodeComp(dists)));


	ListDigraph::Node node = g.addNode();
	dists[node] = 1;
	basic_digraph.push(node);

	node = g.addNode();
	dists[node] = 3;
	basic_digraph.push(node);

	node = g.addNode();
	dists[node] = -4;
	basic_digraph.push(node);

	node = g.addNode();
	dists[node] = 0;
	basic_digraph.push(node);

	cout << "PQueueTest::node" << endl;	
	print_node_queue(basic_digraph, dists);
}

