#ifndef BXMLCFG
#define BXMLCFG

#include<string>
#include<stdexcept>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "Cache.h"

using namespace std;

class BXMLCfg {
public:
	/**
	 * Default empty constructor
	 */
	BXMLCfg(void);
	/**
	 * File name constructor, equivalent to
	 * BXMLCfg cfg();
	 * cfg.read("config.xml");
	 */
	BXMLCfg(string XMLFile);
	/**
	 * Destructor
	 */
	~BXMLCfg();
	/**
	 * Reads the XMLFile as the configuration for this BUNDLE
	 * extraction. 
	 */
	void read(string XMLFile);

	enum ARCH {ADFLT, MIPS, ARM};
	enum ENDIAN {EDFLT, BIG, LITTLE};
	enum CACHE_TYPE {DCACHE, ICACHE};

	ARCH getArch() { return _arch; }
	ENDIAN getEndian() { return _endian; }
	int getLoadLatency() { return _load_latency; }
	int getStoreLatency() { return _store_latency; }
	int maxICacheLevel() {
		map<int, Cache *>::iterator it = _ins_cache.end();
		it--;
		return it->first;
	}
	int maxDCacheLevel() {
		map<int, Cache *>::iterator it = _dat_cache.end();
		it--;
		return it->first;
	}
	void copyCaches(map<int, Cache *>& iCache, map<int, Cache *>& dCache);
	string getWorkDir() { return _dir; }
	string getCfgFile() { return _dir + "/" + _cfg; }

private:
	string _xml_file;
	xmlDocPtr _doc;
	ARCH _arch;
	ENDIAN _endian;
	int _load_latency;
	int _store_latency;
	string _dir, _cfg, _entry;

	map<int, Cache *> _ins_cache;
	map<int, Cache *> _dat_cache;

	xmlNodePtr getArchNode();
	xmlNodePtr getTargetNode();
	xmlNodePtr getCacheNodes();
	xmlNodePtr getMemoryNode();
	xmlNodePtr getDirNode();
	xmlNodePtr getEntryNode();
	xmlNodePtr getSingletonNode(const char* xpath);

	void addCacheLevel(xmlNodePtr node);
};

#endif /* BXMLCFG */
