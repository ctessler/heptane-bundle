#!/bin/bash

TARGETS=" \
	CROSS_COMPILERS \
	analysis.sh \
	config_files/ \
	extract.sh \
	run_benchmark.sh \
	src/Common/ArchitectureDependent/makefile.depends \
	src/Common/ArchitectureDependent/obj/ \
	src/Common/GlobalAttributes/makefile.depends \
	src/Common/GlobalAttributes/obj/ \
	src/Common/cfglib/build_dir/libcfg.a \
	src/Common/cfglib/doc/generated-doc/ \
	src/Common/cfglib/makefile.depends \
	src/Common/cfglib/obj/ \
	src/Common/utl/makefile.depends \
	src/Common/utl/obj/ \
	src/HeptaneExtract/makefile.depends \
	src/cfglib_install/ \
	src/makefile.common \
	src/makefile.config \
        bin/ \
        src/HeptaneAnalysis/makefile.depends \ 
        src/HeptaneAnalysis/obj/ \
        src/HeptaneExtract/obj/ \
"
rm -rf ${TARGETS}
