tgt=cfr_test
INCLUDE=$(shell xml2-config --cflags) -I../../../BundleCFG/src
LDFLAGS=-lemon $(shell xml2-config --libs)
.PHONY=run_valgrind obj ext

CXXFLAGS=-DGLIBCXX_FORCE_NEW -O0 -g -std=c++11 $(INCLUDE) $(LDFLAGS)

cfg_srcs=FunctionCall.cc
cod=../../../BundleCFG/src
cfg_srcs_path=$(patsubst %.cc,$(cod)/%.cc,$(cfg_srcs))
cfg_objs=$(patsubst %.cc,ext/%.o,$(cfg_srcs))

test_srcs=cfr_test.cc test_cfr.cc test_cfg.cc
#test_srcs_path=$(patsubst %.cc,test/%.cc,$(test_srcs))
test_objs=$(patsubst %.cc,obj/%.o,$(test_srcs))

all: run_valgrind $(tgt)

run_valgrind: $(tgt)
	valgrind --leak-check=full ./$(tgt)

$(tgt): obj $(test_objs) $(cfg_objs)
	$(CXX) $(CXXFLAGS) -o $@ $(test_objs) $(cfg_objs)

obj:
	mkdir obj

obj/%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

ext/%.o: $(cod)/%.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(cfg_objs): ext
ext:
	mkdir ext

%.cc: %.h

clean:
	rm -rf obj ext $(tgt) vgcore.*

