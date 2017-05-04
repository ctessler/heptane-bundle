#ifndef BXMLCFG
#define BXMLCFG

#include<string>
#include<stdexcept>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

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
private:
	string _xml_file;
	xmlDocPtr _doc;

	xmlNodePtr getArchNode();
};

#endif /* BXMLCFG */
