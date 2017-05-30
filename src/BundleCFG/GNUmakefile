
INCLS+=-Isrc -Isrc/Generic -Isrc/SharedAttributes -Isrc/CacheAnalysis
INCLS+=-Isrc/CacheAnalysis -Isrc/CodeLine -Isrc/DataAddressAnalysis
INCLS+=-Isrc/DotPrint -Isrc/DummyAnalysis -Isrc/HtmlPrint -Isrc/IPETAnalysis
INCLS+=-I../../src/HeptaneAnalysis/src -I../../src/HeptaneAnalysis/src
INCLS+=-I../../src/HeptaneAnalysis/src/Generic

OBJS=obj/main.o obj/BXMLCfg.o obj/Cache.o obj/DotPrint.o obj/CFRFactory.o

vbin=../../bin/BundleCFG
all: $(vbin)

include ../makefile.common
include makefile.depends

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
	$(CXX) $^ $(LINKSFLAGS) -o $@


clean:
	$(RM) $(vbin) $(OBJS) doc/generated-doc/html/*.html makefile.depends


force: clean all doc


# $(GLOB_ATTR_DIR_OBJ)/ARMWordsAttribute.o \

# ../../src/HeptaneAnalysis/obj/Context.o \
# ../../src/HeptaneAnalysis/obj/StackInfoAttribute.o \
# ../../src/HeptaneAnalysis/obj/CodeLineAttribute.o
