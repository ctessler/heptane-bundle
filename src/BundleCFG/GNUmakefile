
INCLS+=-Isrc -Isrc/Generic -Isrc/SharedAttributes -Isrc/CacheAnalysis
INCLS+=-Isrc/CacheAnalysis -Isrc/CodeLine -Isrc/DataAddressAnalysis
INCLS+=-Isrc/DotPrint -Isrc/DummyAnalysis -Isrc/HtmlPrint -Isrc/IPETAnalysis
INCLS+=-I../../src/HeptaneAnalysis/src -I../../src/HeptaneAnalysis/src
INCLS+=-I../../src/HeptaneAnalysis/src/Generic
INCLS+=-I../BundleWCET/src

OBJS=obj/main.o obj/BXMLCfg.o obj/Cache.o obj/DotPrint.o
OBJS+=obj/CacheLine.o obj/CacheSet.o obj/PolicyLRU.o obj/LemonCFG.o obj/LemonFactory.o
OBJS+=obj/LemonCFR.o obj/CFG.o obj/CFGFactory.o 
OBJS+=obj/CFGReadWrite.o obj/FunctionCall.o obj/DBG.o

vbin=../../bin/BundleCFG
all: $(vbin)

CXXFLAGS=-DDBG_TODOT -DDBG_LOOPDOT
CXXFLAGS=-O0 -ggdb -std=c++11

include ../makefile.common
include makefile.depends

LINKSFLAGS+=-lemon


$(vbin): $(CFGLIBDIR)/lib/libcfg.a $(OBJS) \
	$(GLOB_ATTR_DIR_OBJ)/SymbolTableAttribute.o \
	$(GLOB_ATTR_DIR_OBJ)/AddressAttribute.o \
        $(GLOB_ATTR_DIR_OBJ)/ARMWordsAttribute.o \
	$(ARCHDEP_DIR_OBJ)/MIPS.o\
        $(ARCHDEP_DIR_OBJ)/ARM.o\
        $(ARCHDEP_DIR_OBJ)/arch.o\
        $(ARCHDEP_DIR_OBJ)/InstructionType.o\
        $(ARCHDEP_DIR_OBJ)/InstructionFormat.o\
        $(ARCHDEP_DIR_OBJ)/DAAInstruction.o \
        $(ARCHDEP_DIR_OBJ)/DAAInstruction_MIPS.o \
        $(ARCHDEP_DIR_OBJ)/DAAInstruction_ARM.o \
	$(UTILITY_DIR_OBJ)/Logger.o \
	$(UTILITY_DIR_OBJ)/Utl.o \
	$(UTILITY_DIR_OBJ)/InstructionARM.o \
	../HeptaneAnalysis/obj/Analysis.o \
	../HeptaneAnalysis/obj/CallGraph.o
	$(CXX) $^ $(LINKSFLAGS) $(DEBUGFLAGS) -o $@


clean:
	$(RM) $(vbin) $(OBJS) doc/generated-doc/html/*.html makefile.depends


force: clean all doc
