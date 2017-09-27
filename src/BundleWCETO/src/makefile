tgt=../../../bin/BundleWCETO
INCLUDE=$(shell xml2-config --cflags) -I../../BundleCFG/src
LDFLAGS=-lemon $(shell xml2-config --libs)

CXXFLAGS=-O0 -ggdb -std=c++11 $(INCLUDE) $(LDFLAGS)

cod=../../BundleCFG/src
cfg_srcs= BXMLCfg.cc CFG.cc CFGReadWrite.cc Cache.cc
cfg_srcs+=CacheLine.cc CacheSet.cc
cfg_srcs+=DOTFactory.cc JPGFactory.cc PolicyLRU.cc
cfg_srcs_path=$(patsubst %.cc,$(cod)/%.cc,$(cfg_srcs))
cfg_objs=$(patsubst %.cc,../objs/%.o,$(cfg_srcs))

lcl_srcs= main.cc CFRFactory.cc CFR.cc
lcl_objs=$(patsubst $.cc,../objs/%.o,$(lcl_srcs))

all: $(tgt) ../objs

../objs:
	mkdir -p ../objs

$(tgt): $(lcl_objs) $(cfg_objs)
	$(CXX) $(CXXFLAGS) -o $@ $(lcl_objs) $(cfg_objs)

$(cfg_objs): ../objs $(cfg_srcs_path)
	$(CXX) $(CXXFLAGS) $(patsubst ../objs/%.o,$(cod)/%.cc,$@) -c -o $@

