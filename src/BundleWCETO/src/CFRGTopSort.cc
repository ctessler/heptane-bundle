#include "CFRGTopSort.h"

class OrderData {
public:
	OrderData(CFRGTopSort &t, ListDigraph::NodeMap<int> &d) :
		topo(t), distances(d), pqueue((NodeCompGR(distances))) {
	}
	int finish=0;
	ListDigraph::NodeMap<int> &distances;
	pqueue_gr_t pqueue;
	CFRGTopSort &topo;
};

static bool
order_dfs_fin(CFRG &cfrg, CFR *cfr, void *userdata) {
	OrderData *data = (OrderData *) userdata;
	ListDigraph::Node node = cfrg.findNode(cfr);
	pqueue_gr_t::iterator git = data->pqueue.find(node);

	data->distances[node] = data->finish;
	data->finish += 1;
	data->pqueue.insert(node);
	return true;
}

static bool
order_dfs_work(CFRG &cfrg, CFR *cfr, void *userdata) {
	OrderData *data = (OrderData *) userdata;
	ListDigraph::Node node = cfrg.findNode(cfr);	
	CFRGTS_work_t work_fn = data->topo.getWork();
	if (work_fn) {
		return work_fn(cfrg, node, data->topo.getUserData());
	}
	return true;
}

static bool
order_dfs_sel(CFRG &cfrg, CFR *cfr, void *userdata) {
	OrderData *data = (OrderData *) userdata;
	ListDigraph::Node node = cfrg.findNode(cfr);
	CFRGTS_sel_t sel_fn = data->topo.getSel();
	if (sel_fn) {
		bool r = sel_fn(cfrg, node, data->topo.getUserData());
		return r;
	}
	return false;
}

static bool
order_dfs_mask(CFRG &cfrg, CFR *cfr, void *userdata) {
	OrderData *data = (OrderData *) userdata;
	ListDigraph::Node node = cfrg.findNode(cfr);	
	CFRGTS_mask_t mask_fn = data->topo.getMask();
	if (mask_fn) {
		bool r = mask_fn(cfrg, node, data->topo.getUserData());
		return r;
	}
	return true;
}


bool
CFRGTopSort::sort(ListDigraph::Node start) {
	/* Step one perform a Depth First Search with finish times */
	OrderData data(*this, _distances);
	CFRGDFS dfs(_cfrg);
	dfs.setUserData(&data);
	dfs.setFin(order_dfs_fin);
	dfs.setMask(order_dfs_mask);
	dfs.setWork(order_dfs_work);
	dfs.setSel(order_dfs_sel);
	CFR *cfr_start = _cfrg.findCFR(start);
	dfs.recSearch(cfr_start);

	/* Step 2, present the nodes in descending finish time for */
	pqueue_gr_t::iterator git;
	int c=0;
	for (git = data.pqueue.begin(); git != data.pqueue.end(); ++git, ++c) {
		ListDigraph::Node node = *git;
		_resd[node] = c;
		result.insert(node);
	}
}
