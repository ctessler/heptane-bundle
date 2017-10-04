tgt=unit_test
INCLUDE=$(shell xml2-config --cflags) -I../../../BundleCFG/src -I../ 
LDFLAGS=-lemon $(shell xml2-config --libs) -lcppunit
.PHONY=run_valgrind obj ext

CXXFLAGS=-DGLIBCXX_FORCE_NEW -O0 -g -std=c++11 $(INCLUDE) $(LDFLAGS)

cfg_srcs= FunctionCall.cc CFG.cc PolicyLRU.cc Cache.cc CacheLine.cc
cfg_srcs+=CacheSet.cc BXMLCfg.cc
cod=../../../BundleCFG/src
cfg_srcs_path=$(patsubst %.cc,$(cod)/%.cc,$(cfg_srcs))
cfg_objs=$(patsubst %.cc,ext/%.o,$(cfg_srcs))


wcet_srcs=CFR.cc CFRFactory.cc
wcet=../
wcet_srcs_path=$(patsubst %.cc,$(wcet)/%.cc,$(wcet_srcs))
wcet_objs=$(patsubst %.cc,wcet/%.o,$(wcet_srcs))

test_srcs= unit_test.cc CFRTest.cc CacheTest.cc BXMLCFGTest.cc
test_srcs+=CFRFactoryTest.cc
#test_srcs_path=$(patsubst %.cc,test/%.cc,$(test_srcs))
test_objs=$(patsubst %.cc,obj/%.o,$(test_srcs))

all: run_valgrind $(tgt)

run_valgrind: $(tgt)
	valgrind --leak-check=full ./$(tgt)

$(tgt): obj $(test_objs) $(cfg_objs) $(wcet_objs)
	$(CXX) $(CXXFLAGS) -o $@ $(test_objs) $(cfg_objs) $(wcet_objs)

obj:
	mkdir obj

obj/%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(wcet_objs): | wcet
wcet:
	mkdir wcet

wcet/%.o: ../%.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

ext/%.o: $(cod)/%.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(cfg_objs): | ext
ext:
	mkdir ext

%.cc: %.h

clean:
	rm -rf wcet obj ext $(tgt) vgcore.*

