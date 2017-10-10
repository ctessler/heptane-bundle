#include"BXMLCfg.h"
#define XPATH_CONFIG	"/CONFIGURATION"
#define XPATH_ANALYSIS	XPATH_CONFIG "/ANALYSIS"
#define XPATH_ARCH	XPATH_CONFIG "/ARCHITECTURE"
#define XPATH_TARGET	XPATH_ARCH "/TARGET"
#define XPATH_CACHE	XPATH_ARCH "/CACHE"
#define XPATH_MEMORY	XPATH_ARCH "/MEMORY"
#define XPATH_DIR	XPATH_CONFIG "/INPUTOUTPUTDIR"
#define XPATH_PROG	XPATH_ANALYSIS "/ENTRYPOINT"


BXMLCfg::BXMLCfg(void) {
	_doc = NULL;
	_arch = ADFLT;
	_endian = EDFLT;
}

BXMLCfg::BXMLCfg(string XMLFile) {
	_doc = NULL;
	_arch = ADFLT;
	_endian = EDFLT;
	read(XMLFile);
}

BXMLCfg::~BXMLCfg() {
	map<int, Cache*>::iterator mit;
	for (mit=_ins_cache.begin(); mit != _ins_cache.end(); ++mit) {
		delete mit->second;
	}
	for (mit=_dat_cache.begin(); mit != _dat_cache.end(); ++mit) {
		delete mit->second;
	}
	_ins_cache.clear();
	_dat_cache.clear();
	if (_doc) {
		xmlFreeDoc(_doc);
	}
}

void BXMLCfg::read(string XMLFile) {
	_doc = xmlParseFile(XMLFile.c_str());
	if (!_doc) {
		string error = "Could not load file: ";
		error += XMLFile.c_str();
		throw runtime_error(error);
	}

	//xmlNodePtr node = xmlDocGetRootElement(_doc);
	xmlNodePtr archNode = getArchNode();
	xmlNodePtr targetNode = getTargetNode();
	xmlNodePtr cacheNodes = getCacheNodes();
	xmlNodePtr memoryNode = getMemoryNode();
	xmlNodePtr dirNode = getDirNode();
	xmlNodePtr entryNode = getEntryNode();
	
	xmlFreeNodeList(archNode);
	xmlFreeNodeList(targetNode);
	xmlFreeNodeList(cacheNodes);
	xmlFreeNodeList(memoryNode);
	xmlFreeNodeList(dirNode);
	xmlFreeNodeList(entryNode);	
}


/**
 * Returns the XML node of a tag by its xpath, if and only if, there is a single
 * instance of the tag in the document
 *
 * Returned node must be freed with xmlFreeNodeList()
 *
 * @param[in] xpath to the tag (ie, /CONFIGURATION/ARCHITECTURE)
 *
 * @return the node, otherwise an exception is thrown.
 *
 */
xmlNodePtr BXMLCfg::getSingletonNode(const char* xpath) {
	xmlXPathContextPtr xpathCtx = xmlXPathNewContext(_doc);
	if (!xpathCtx) {
		throw runtime_error("Unable to allocate an xpath context");
	}

	xmlXPathObjectPtr xpathObj =
	    xmlXPathEvalExpression(BAD_CAST xpath, xpathCtx);
	xmlXPathFreeContext(xpathCtx);
	if (!xpathObj) {
		string error = "Unable to evaluate ";
		error += xpath;
		error += " xpath expression";
		throw runtime_error(error);
	}

	if (xpathObj->nodesetval->nodeNr != 1) {
		xmlXPathFreeObject(xpathObj);
		string error = "XML Configuration file must have exactly 1 ";
		error += xpath;
		error += " tag";
		throw runtime_error(error);
	}
	xmlNodePtr node = xpathObj->nodesetval->nodeTab[0];
	xmlNodePtr copy = xmlCopyNode(node, 1);
	xmlXPathFreeObject(xpathObj);

	return copy;
}


/**
 * Gets the XML node of the architecture tag.
 *
 * Must be released with xmlFreeNodeList()
 *
 * @return an individual xmlNodePtr that is the ARCHITECTURE node.
 */
xmlNodePtr BXMLCfg::getArchNode() {
	if (!_doc) {
		throw runtime_error("getArchNode() called before read()");
	}
	return getSingletonNode(XPATH_ARCH);
}

/**
 * Gets the XML node of the target tag.
 *
 * Must be released with xmlFreeNodeList()
 *
 * @return an individual xmlNodePtr that is the TARGET node.
 */
xmlNodePtr BXMLCfg::getTargetNode() {
	if (!_doc) {
		throw runtime_error("getTargetNode() called before read()");
	}
	xmlNodePtr node = getSingletonNode(XPATH_TARGET);
	xmlChar *prop;
	prop = xmlGetProp(node, (const xmlChar*) "NAME");
	if (!prop) {
		throw runtime_error("TARGET tag has no NAME attribute");
	}

	if (string ("MIPS") == (const char *) prop) {
		_arch = MIPS;
	}
	if (string ("ARM") == (const char*) prop) {
		_arch = ARM;
	}
	xmlFree(prop);
	switch (_arch) {
	case ARM:
	case MIPS:
		break;
	default:
		throw runtime_error("No supported architecture in TARGET tag");
	}
	
	prop = xmlGetProp(node, (const xmlChar*) "ENDIANNESS");
	if (!prop) {
		throw runtime_error("TARGET tag has no ENDIANNESS attribute");
	}
	if (string ("BIG") == (const char *) prop) {
		_endian = BIG;
	}
	if (string ("LITTLE") == (const char *) prop) {
		_endian = LITTLE;
	}
	xmlFree(prop);
	switch (_endian) {
	case BIG:
	case LITTLE:
		break;
	default:
		throw runtime_error("No endianness TARGET tag");
	}
	
	return node;
}

/**
 * Gets the XML node list of all the CACHE tags
 *
 * @return the list of cache nodes
 */
xmlNodePtr BXMLCfg::getCacheNodes() {
	if (!_doc) {
		throw runtime_error("getCacheNodes() called before read()");
	}
	xmlXPathContextPtr xpathCtx = xmlXPathNewContext(_doc);
	if (!xpathCtx) {
		throw runtime_error("Unable to allocate an xpath context");
	}

	xmlXPathObjectPtr xpathObj =
	    xmlXPathEvalExpression(BAD_CAST XPATH_CACHE, xpathCtx);
	xmlXPathFreeContext(xpathCtx);
	if (!xpathObj) {
		string error = "Unable to evaluate ";
		error += XPATH_CACHE;
		error += " xpath expression";
		throw runtime_error(error);
	}
	
	int cache_count = xpathObj->nodesetval->nodeNr;
	if (cache_count < 1) {
		xmlXPathFreeObject(xpathObj);
		string error = "XML Configuration file must have at least one ";
		error += XPATH_CACHE;
		error += " tag";
		throw runtime_error(error);
	}

	for (int i=0; i < cache_count; i++) {
		addCacheLevel(xpathObj->nodesetval->nodeTab[i]);
	}
	/* Frees everything */
	xmlXPathFreeObject(xpathObj);
	return NULL;
}

void
BXMLCfg::addCacheLevel(xmlNodePtr node) {
	if (!xmlHasProp(node, (xmlChar *) "level")) {
		throw runtime_error("CACHE tag has no level");
	}
	if (!xmlHasProp(node, (xmlChar *) "nbsets")) {
		throw runtime_error("CACHE has no nbsets (number of sets)");
	}

	if (!xmlHasProp(node, (xmlChar *) "nbways")) {
		throw runtime_error("CACHE has no nbways (associativity)");
	}
	if (!xmlHasProp(node, (xmlChar *) "cachelinesize")) {
		throw runtime_error("CACHE has no cachelinesize");
	}
	if (!xmlHasProp(node, (xmlChar *) "type")) {
		throw runtime_error("CACHE tag has no type");
	}
	if (!xmlHasProp(node, (xmlChar *) "replacement_policy")) {
		throw runtime_error("CACHE has no replacement_policy");
	}
	if (!xmlHasProp(node, (xmlChar *) "latency")) {
		throw runtime_error("CACHE has no replacement_policy");
	}
	int level, nsets, nways, latency, linesize;
	bool valid_policy = false;
	string iord;
	
	char *buff;
	buff = (char *)xmlGetProp(node, (xmlChar *) "level");
	sscanf(buff, "%i", &level);
	free(buff);

	buff = (char *)xmlGetProp(node, (xmlChar *) "nbsets");
	sscanf(buff, "%i", &nsets);
	free(buff);

	buff = (char *)xmlGetProp(node, (xmlChar *) "nbways");
	sscanf(buff, "%i", &nways);
	free(buff);

	buff = (char *)xmlGetProp(node, (xmlChar *) "cachelinesize");
	sscanf(buff, "%i", &linesize);
	free(buff);

	buff = (char *)xmlGetProp(node, (xmlChar *) "latency");
	sscanf(buff, "%i", &latency);
	free(buff);

	buff = (char *)xmlGetProp(node, (xmlChar *) "type");
	Cache *new_cache = new Cache(nsets, nways, linesize, latency);
	if (string ("icache") == buff) {
		if (_ins_cache.find(level) != _ins_cache.end()) {
			delete new_cache;
			throw runtime_error(
			    "Multiple CACHE tags at the same level and type");
		}
		iord = "instruction";
		_ins_cache.insert(make_pair(level, new_cache));
	}
	if (string ("dcache") == buff) {
		if (_dat_cache.find(level) != _dat_cache.end()) {
			delete new_cache;
			throw runtime_error(
			    "Multiple CACHE tags at the same level and type");
		}
		iord = "data";
		_dat_cache.insert(make_pair(level, new_cache));
	}
	free(buff);

	buff = (char *)xmlGetProp(node, (xmlChar *) "replacement_policy");
	if (string ("LRU") == buff) {
		new_cache->setPolicy(&lru);
		valid_policy = true;
	}
	free(buff);
	if (!valid_policy) {
		throw runtime_error("CACHE has no supported replacement policy");
	}
}

/**
 * Gets the XML node of the memory tag.
 *
 * Must be released with xmlFreeNodeList()
 *
 * @return an individual xmlNodePtr that is the MEMORY node.
 */
xmlNodePtr BXMLCfg::getMemoryNode() {
	if (!_doc) {
		throw runtime_error("getMemoryNode() called before read()");
	}
	xmlNodePtr node = getSingletonNode(XPATH_MEMORY);
	if (!xmlHasProp(node, (xmlChar *) "store_latency")) {
		throw runtime_error("CACHE tag has no store_latency");
	}

	if (!xmlHasProp(node, (xmlChar *) "load_latency")) {
		throw runtime_error("CACHE tag has no load_latency");
	}
	char *buff;
	buff = (char *)xmlGetProp(node, (xmlChar *) "store_latency");
	sscanf(buff, "%i", &_store_latency);
	free(buff);

	buff = (char *)xmlGetProp(node, (xmlChar *) "load_latency");
	sscanf(buff, "%i", &_load_latency);
	free(buff);

	return node;
}

/**
 * Gets the XML node of the input output directory tag.
 *
 * Must be released with xmlFreeNodeList()
 *
 * @return an individual xmlNodePtr that is the INPUTOUTPUTDIR node.
 */
xmlNodePtr BXMLCfg::getDirNode() {
	if (!_doc) {
		throw runtime_error("getDirNode() called before read()");
	}
	xmlNodePtr node = getSingletonNode(XPATH_DIR);
	if (!xmlHasProp(node, (xmlChar *) "name")) {
		throw runtime_error("INPUTOUTPUTDIR tag has no name");
	}
	char *buff;
	buff = (char *)xmlGetProp(node, (xmlChar *) "name");
	_dir = buff;
	free(buff);

	return node;
}

/**
 * Gets the XML node of the entrypoint tag
 *
 * Must be released with xmlFreeNodeList()
 *
 * @return an individual xmlNodePtr that is the ENTRYPOINT node.
 */
xmlNodePtr BXMLCfg::getEntryNode() {
	if (!_doc) {
		throw runtime_error("getEntryNode() called before read()");
	}
	xmlNodePtr node = getSingletonNode(XPATH_PROG);
	if (!xmlHasProp(node, (xmlChar *) "input_file")) {
		throw runtime_error("INPUTOUTPUTDIR tag has no input_file");
	}
	if (!xmlHasProp(node, (xmlChar *) "entrypointname")) {
		throw runtime_error("INPUTOUTPUTDIR tag has no entrypointname");
	}
	char *buff;
	buff = (char *)xmlGetProp(node, (xmlChar *) "input_file");
	_cfg = buff;
	free(buff);

	buff = (char *)xmlGetProp(node, (xmlChar *) "entrypointname");
	_entry = buff;
	free(buff);

	return node;

}


void BXMLCfg::copyCaches(map<int, Cache *>& icache_map, map<int, Cache *>& dcache_map) {
	map<int, Cache *>::iterator it;

	icache_map.clear();
	for (it = _ins_cache.begin(); it != _ins_cache.end(); it++) {
		int index = it->first;
		Cache *cache = it->second;
		Cache *copy = new Cache(*cache);
		icache_map.insert(pair<int, Cache*>(index, copy));
	}
	
	dcache_map.clear();
 	for (it = _dat_cache.begin(); it != _dat_cache.end(); it++) {
		int index = it->first;
		Cache *cache = it->second;
		Cache *copy = new Cache(*cache);
		dcache_map.insert(pair<int, Cache*>(index, copy));
	}
}
