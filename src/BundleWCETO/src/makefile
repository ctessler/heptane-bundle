tgt=../../../bin/BundleWCETO
INCLUDE=$(shell xml2-config --cflags) -I../../BundleCFG/src
LDFLAGS=-lemon $(shell xml2-config --libs)

CXXFLAGS=-DGLIBCXX_FORCE_NEW -O0 -g -std=c++11 $(INCLUDE) $(LDFLAGS)

cod=../../BundleCFG/src
cfg_srcs= BXMLCfg.cc CFG.cc CFGReadWrite.cc Cache.cc
cfg_srcs+=CacheLine.cc CacheSet.cc
cfg_srcs+=DOTFactory.cc JPGFactory.cc PolicyLRU.cc
cfg_srcs_path=$(patsubst %.cc,$(cod)/%.cc,$(cfg_srcs))
cfg_objs=$(patsubst %.cc,../objs/%.o,$(cfg_srcs))

lcl_srcs= main.cc CFRFactory.cc CFRFactory_test.cc CFR.cc CFR_test.cc
lcl_srcs+=DOTfromCFR.cc CFR_class_test.cc
lcl_objs=$(patsubst %.cc,../objs/%.o,$(lcl_srcs))

test_srcs=cfr_test.cc test_cfr.cc test_cfg.cc
test_srcs_path=$(patsubst %.cc,test/%.cc,$(test_srcs))
test_objs=$(patsubst %.cc,../objs/%.o,$(test_srcs))

all: $(tgt) ../objs cfr_test

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

clean:
	rm -rf ../objs/*.o ../../bin/BundleWCETO cfr_test

