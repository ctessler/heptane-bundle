.PHONY: test
tgt=../../../bin/BundleWCETO
INCLUDE=$(shell xml2-config --cflags) -I../../BundleCFG/src
LDFLAGS=-lemon $(shell xml2-config --libs)

CXXFLAGS=-DGLIBCXX_FORCE_NEW -O0 -g -std=c++11 $(INCLUDE) $(LDFLAGS)

cod=../../BundleCFG/src
cfg_srcs= BXMLCfg.cc CFG.cc CFGReadWrite.cc Cache.cc  DBG.cc
cfg_srcs+=CacheLine.cc CacheSet.cc FunctionCall.cc
cfg_srcs+=DOTFactory.cc JPGFactory.cc PolicyLRU.cc
cfg_srcs_path=$(patsubst %.cc,$(cod)/%.cc,$(cfg_srcs))
cfg_objs=$(patsubst %.cc,../objs/%.o,$(cfg_srcs))

lcl_srcs= main.cc CFRFactory.cc CFR.cc DOTfromCFRG.cc CFRG.cc
lcl_srcs+=DOTfromCFR.cc EntryFactory.cc CFRGWCETOFactory.cc CFRGDFS.cc CFRGLFS.cc
lcl_srcs+=CFGDFS.cc PQueue.cc CFGTopSort.cc
lcl_srcs+=WCETOFactory.cc ThreadWCETOMap.cc CFRWCETOMap.cc  CFRDemandMap.cc
lcl_srcs+=CFRECBs.cc
lcl_objs=$(patsubst %.cc,../objs/%.o,$(lcl_srcs))

all: $(tgt) ../objs test

test:
	$(MAKE) -C test

../objs:
	mkdir -p ../objs

cfr_test: $(test_objs)
	$(CXX) $(CXXFLAGS) -o $@ $(test_objs)

$(tgt): $(lcl_objs) $(cfg_objs)
	echo "CFG OBJS: " $(cfg_objs)
	$(CXX) $(CXXFLAGS) -o $@ $(lcl_objs) $(cfg_objs)

../objs/%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

../objs/%.o: $(cod)/%.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

../objs/%.o: test/%.cc
	$(CXX) $(CXXFLAGS) -I../ -c -o $@ $<

clean: OPT=clean
clean:
	$(MAKE) -C test clean
	rm -rf ../objs/*.o ../../bin/BundleWCETO 

