#include "SetTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION(SetTest);

void
SetTest::setUp()
{
}

void
SetTest::tearDown()
{
}

template<typename T> static void
print_set(T &s) {
	typename T::iterator it;

	for (it = s.begin(); it != s.end(); it++) {
		cout << *it << " ";
	}
	cout << endl;
}

void
SetTest::basic()
{
	cout << endl;
	set<int> int_default;
	int_default.insert(10);
	int_default.insert(-10);
	int_default.insert(5);
	int_default.insert(16);

	print_set(int_default);
}


class Reverse {
public:
	bool operator() (const int &lhs, const int &rhs) const {
		if (lhs > rhs) {
			/* Left is ahead of Right */
			return true;
		}
		return false;
	}
};

void
SetTest::reverse()
{
	cout << endl;
	set<int, Reverse> int_default;
	int_default.insert(10);
	int_default.insert(-10);
	int_default.insert(5);
	int_default.insert(16);

	print_set(int_default);
}


class SetNodeComp {
public:
	SetNodeComp(ListDigraph::NodeMap<int> &dist) : _dist(dist) {

	}
	bool operator()(const ListDigraph::Node &lhs,
			const ListDigraph::Node &rhs) const {
		if (_dist[lhs] < _dist[rhs]) {
			/* Least first */
			return true;
		}
		return false;
	}
private:
	ListDigraph::NodeMap<int> &_dist;
};

static void
print_node_set(set<ListDigraph::Node, SetNodeComp> &s,
	       ListDigraph::NodeMap<int> &dist) {
	set<ListDigraph::Node, SetNodeComp>::iterator it;

	for (it = s.begin(); it != s.end(); it++) {
		ListDigraph::Node node = *it;
		cout << dist[node] << " ";
	}
	cout << endl;
	
}

void
SetTest::node()
{
	cout << endl;
	ListDigraph g;
	ListDigraph::NodeMap<int> dists(g);
	set<ListDigraph::Node, SetNodeComp> basic_set((SetNodeComp(dists)));

	ListDigraph::Node node = g.addNode();
	dists[node] = 1;
	basic_set.insert(node);

	node = g.addNode();
	dists[node] = 3;
	basic_set.insert(node);

	node = g.addNode();
	dists[node] = -4;
	basic_set.insert(node);

	node = g.addNode();
	dists[node] = 0;
	basic_set.insert(node);

	print_node_set(basic_set, dists);
}


static void
addNode(ListDigraph &g, set<ListDigraph::Node> &s) {
	ListDigraph::NodeIt nit(g);
	ListDigraph::Node n = nit;
	s.insert(n);
}

void
SetTest::stdSet() {
	set<ListDigraph::Node> myset;

	ListDigraph g;
	g.addNode();
	g.addNode();
	g.addNode();

	addNode(g, myset);
	cout << "SetTest::stdSet added a node" << endl;
}
