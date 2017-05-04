
INCLS+=-Isrc -Isrc/Generic -Isrc/SharedAttributes -Isrc/CacheAnalysis
INCLS+=-Isrc/CacheAnalysis -Isrc/CodeLine -Isrc/DataAddressAnalysis
INCLS+=-Isrc/DotPrint -Isrc/DummyAnalysis -Isrc/HtmlPrint -Isrc/IPETAnalysis
INCLS+=-Isrc/PipelineAnalysis -Isrc/SimplePrint

OBJS=obj/main.o obj/BXMLCfg.o

vbin=../../bin/BundleCFG
all: $(vbin)

include ../makefile.common
include makefile.depends

$(vbin): $(CFGLIBDIR)/lib/libcfg.a $(OBJS)
	$(CXX) $^ $(LINKSFLAGS) -o $@


clean:
	$(RM) $(vbin) $(OBJS) doc/generated-doc/html/*.html makefile.depends


force: clean all doc

