#include "CFGTopSort.h"

class OrderData {
public:
	OrderData(CFGTopSort &t, ListDigraph::NodeMap<int> &d) :
		topo(t), distances(d), pqueue((NodeCompGR(d))) {
	}
	int finish=0;
	ListDigraph::NodeMap<int> &distances;
	pqueue_gr_t pqueue;
	CFGTopSort &topo;
};

static bool
order_dfs_fin(CFG &cfg, ListDigraph::Node node, void *userdata) {
	OrderData *data = (OrderData *) userdata;
	data->distances[node] = data->finish;
	data->finish += 1;
	data->pqueue.insert(node);
	return true;
}

static bool
order_dfs_work(CFG &cfg, ListDigraph::Node node, void *userdata) {
	OrderData *data = (OrderData *) userdata;
	TS_work_t work_fn = data->topo.getWork();
	if (work_fn) {
		return work_fn(cfg, node, data->topo.getUserData());
	}
	return true;
}

static bool
order_dfs_sel(CFG &cfg, ListDigraph::Node node, void *userdata) {
	OrderData *data = (OrderData *) userdata;
	TS_sel_t sel_fn = data->topo.getSel();
	if (sel_fn) {
		bool r = sel_fn(cfg, node, data->topo.getUserData());
		return r;
	}
	return false;
}

static bool
order_dfs_mask(CFG &cfg, ListDigraph::Node node, void *userdata) {
	OrderData *data = (OrderData *) userdata;
	TS_mask_t mask_fn = data->topo.getMask();
	if (mask_fn) {
		bool r = mask_fn(cfg, node, data->topo.getUserData());
		return r;
	}
	return true;
}


bool
CFGTopSort::sort(ListDigraph::Node start) {
	/* Step one perform a Depth First Search with finish times */
	OrderData data(*this, _distances);
	CFGDFS dfs(_cfg);
	dfs.setUserData(&data);
	dfs.setFin(order_dfs_fin);
	dfs.setMask(order_dfs_mask);
	dfs.setWork(order_dfs_work);
	dfs.setSel(order_dfs_sel);
	dfs.recSearch(start);

	/* Step 2, present the nodes in descending finish time for */
	pqueue_gr_t::iterator git;
	int c=0;
	for (git = data.pqueue.begin(); git != data.pqueue.end(); ++git, ++c) {
		ListDigraph::Node node = *git;
		_distances[node] = c;
		result.insert(node);
	}
}
