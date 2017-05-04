#include"BXMLCfg.h"
#include<iostream>
#define XPATH_ARCH	"/CONFIGURATION/ARCHITECTURE"
#define XPATH_TARGET	XPATH_ARCH "/TARGET"
#define XPATH_CACHE	XPATH_ARCH "/CACHE"
#define XPATH_MEMORY	XPATH_ARCH "/MEMORY"

BXMLCfg::BXMLCfg(void) {
	_doc = NULL;
}

BXMLCfg::BXMLCfg(string XMLFile) {
	_doc = NULL;
	read(XMLFile);
}

BXMLCfg::~BXMLCfg() {
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
	xmlNodePtr node = getArchNode();
	cout << "Node name: " << node->name << endl;
	cout << XPATH_CACHE << endl;
	
	
	xmlFreeNodeList(node);
}


/**
 * Gets the XML node of the architecture tag.
 *
 * Must be released with xmlFreeNodeList
 */
xmlNodePtr BXMLCfg::getArchNode() {
	if (!_doc) {
		throw runtime_error("etArchNode() called before read()");
	}
	xmlXPathContextPtr xpathCtx = xmlXPathNewContext(_doc);
	if (!xpathCtx) {
		throw runtime_error("Unable to allocate an xpath context");
	}

	xmlXPathObjectPtr xpathObj =
	    xmlXPathEvalExpression(BAD_CAST XPATH_ARCH, xpathCtx);
	xmlXPathFreeContext(xpathCtx);
	if (!xpathObj) {
		throw runtime_error("Unable to evaluate "
		    "/CONFIGURATION/ARCHITECTURE xpath expression");
	}

	if (xpathObj->nodesetval->nodeNr != 1) {
		xmlXPathFreeObject(xpathObj);
		throw runtime_error("XML Configuration file must have "
		    "exactly 1 /CONFIGURATION/ARCHITECTURE tag");		    
	}
	xmlNodePtr node = xpathObj->nodesetval->nodeTab[0];
	xmlNodePtr copy = xmlCopyNode(node, 1);
	xmlXPathFreeObject(xpathObj);

	return copy;
}
