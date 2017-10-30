/**
 * Instruction address type
 */
#include <map>

#include <lemon/core.h>
#include <lemon/list_graph.h>

using namespace std;
using namespace lemon;

typedef unsigned long iaddr_t;
typedef map<ListDigraph::Node, ListDigraph::Node> node_map_t;
