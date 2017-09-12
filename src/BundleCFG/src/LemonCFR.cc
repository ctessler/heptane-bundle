#include "LemonCFR.h"

LemonCFRG::LemonCFRG(LemonCFG &cfg) : ListDigraph(), _cfg(cfg),
    _to_cfg(*this), _first_thread(*this), _second_thread(*this),
    _generation(*this) {
	_root = INVALID;
}
