#ifndef LEMON_CFR
#define LEMON_CFR
#include "LemonCFG.h"

class LemonCFRG : public ListDigraph {
public:
	LemonCFRG(LemonCFG &cfg);
	void setRoot(ListDigraph::Node root) { _root = root; }
	ListDigraph::Node getRoot() { return _root; }

	string getStartString(ListDigraph::Node node) {
		return _cfg.getStartString(getCFGNode(node));
	}
	unsigned long getStartLong(ListDigraph::Node node) {
		return _cfg.getStartLong(getCFGNode(node));
	}
	ListDigraph::Node getCFGNode(ListDigraph::Node node) {
		return _to_cfg[node];
	}

	/**
	 * Adds a CFR to the CFRG
	 *
	 * There must be an associated initial instruction of the CFR
	 * in the originating CFG. 
	 *
	 * @param[in] from_cfg the node from the CFG (used in the
	 * constructor) which is the initial instruction of the CFR
	 * being added to the CFRG
	 *
	 * @return the node added to the CFRG
	 */
	ListDigraph::Node addNode(ListDigraph::Node from_cfg) {
		ListDigraph::Node rv = ListDigraph::addNode();
		_to_cfg[rv] = from_cfg;
		_from_cfg[from_cfg] = rv;

		_first_thread[rv] = 0;
		_second_thread[rv] = 0;
		_generation[rv] = 0;
		return rv;
	}

	/**
	 * @override To protect from incorrect addition
	 */
	ListDigraph::Node addNode() {
		throw runtime_error("Cannot add a node with a CFG associate");
	}

	/**
	 * Finds a CFR given an initial instruction from the CFRG
	 *
	 * @param[in] from_cfg the node from the CFG which starts a
	 * CFR in this CFRG
	 *
	 * @return the CFR in this CFRG if it exists, INVALID
	 * otherwise. 
	 */
	ListDigraph::Node findNode(ListDigraph::Node from_cfg) {
		map<ListDigraph::Node, ListDigraph::Node>::iterator mit;
		mit = _from_cfg.find(from_cfg);
		if (mit != _from_cfg.end()) {
			return mit->second;
		}
		return INVALID;
	}

	/**
	 * @override To ensure state is cleaned up.
	 */
	void erase(ListDigraph::Node node) {
		map<ListDigraph::Node, ListDigraph::Node>::iterator mit;
		mit = _from_cfg.find(getCFGNode(node));
		if (mit != _from_cfg.end()) {
			_from_cfg.erase(mit);
		}
		_to_cfg[node] = INVALID;
		_first_thread[node] = 0;
		_second_thread[node] = 0;
		_generation[node] = 0;

		ListDigraph::erase(node);
	}

	/**
	 * Sets the cost of adding a thread, not the total cost.
	 *
	 * @param[in] thread the number of the thread, starting at 1.
	 * @param[in] wcet the cost of adding the thread in addition
	 * to all the other threads before it
	 *
	 */
	void setWCET(ListDigraph::Node node, unsigned int thread, unsigned int wcet) {
		switch(thread) {
		case 1:
			_first_thread[node] = wcet;
			break;
		case 2:
			_second_thread[node] = wcet;
			break;
		default:
			throw runtime_error("Not yet");
		}
	}

	/**
	 * Gets the cost of threads over the CFR
	 *
	 * @param[in] node the CFR
	 * @param[in] threads the number of threads
	 *
	 * @return the wcet of threads over the CFR
	 */
	unsigned int getWCET(ListDigraph::Node node, unsigned int threads) {
		if (threads <= 0) {
			return 0;
		}
		unsigned int rv = _first_thread[node] +
			(threads - 1) * _second_thread[node];
		return rv;
	}

	void dump() {
		ListDigraph::NodeMap<bool> visited(*this);
		stack<ListDigraph::Node> kids;
		kids.push(getRoot());
		while (!kids.empty()) {
			ListDigraph::Node u = kids.top(); kids.pop();
			visited[u] = true;

			cout << "Node [" << getStartString(u) << "] --> ( ";
			for (ListDigraph::OutArcIt ait(*this, u); ait != INVALID; ++ait) {
				ListDigraph::Node kid = runningNode(ait);
				cout << getStartString(kid) << " ";
				if (visited[kid]) {
					continue;
				}
				kids.push(kid);
			}
			cout << ")" << endl;
		}
	}
	
private:
	LemonCFG &_cfg;
	ListDigraph::Node _root;
	/* CFG node --> CFRG node */
	map<ListDigraph::Node, ListDigraph::Node> _from_cfg;
	/* CFRG node --> CFG node */
	ListDigraph::NodeMap<ListDigraph::Node> _to_cfg;
	ListDigraph::NodeMap<unsigned int> _first_thread;
	ListDigraph::NodeMap<unsigned int> _second_thread;
	ListDigraph::NodeMap<unsigned int> _generation;
};

#endif /* LEMON_CFR */
