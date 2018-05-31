#!/bin/sh

#---------------------------------------------------------------------
#
# Copyright IRISA, 2003-2014
#
# This file is part of Heptane, a tool for Worst-Case Execution Time (WCET)
# estimation.
# APP deposit IDDN.FR.001.510039.000.S.P.2003.000.10600
#
# Heptane is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Heptane is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details (COPYING.txt).
#
# See CREDITS.txt for credits of authorship
#
#---------------------------------------------------------------------

##################
# CONFIGURATION  BUT DEPENDS ON THE TARGET (MIPS, ARM)
##################

ENDIANNESS_MIPS=BIG
ENDIANNESS_ARM=LITTLE

XML2=/usr/include/libxml2
#XML2= /usr/local/Cellar/libxml2/2.9.1/include/libxml2

HEPTANE_ROOT=`dirname "$(readlink -f "$0")"`

####################################
# INSTALLATION PARAMETERS
####################################
# Put 0 if you do not want to
# install the corresponding part

CROSS_COMPILER_MIPS_INSTALL=${CROSS_COMPILER_MIPS_INSTALL:=0}
CROSS_COMPILER_ARM_INSTALL=${CROSS_COMPILER_ARM_INSTALL:=0}

HEPTANE_CFGLIB_INSTALL=${HEPTANE_CFGLIB_INSTALL:=1}
HEPTANE_CORE_INSTALL=${HEPTANE_CORE_INSTALL:=1}
SCRIPT_CONFIG_INSTALL=${SCRIPT_CONFIG_INSTALL:=1}

CROSS_COMPILERS_DIR=${CROSS_COMPILERS_DIR:=${HEPTANE_ROOT}/CROSS_COMPILERS}

########################################################################
#
# INSTALLATION      --- YOU DO NOT HAVE TO CHANGE LINES BELOW ---
#
########################################################################
ROOT_DIR=${HEPTANE_ROOT}

echo "Installation Parameters:"
for var in CROSS_COMPILER_MIPS_INSTALL CROSS_COMPILER_ARM_INSTALL \
    HEPTANE_CFGLIB_INSTALL HEPTANE_CORE_INSTALL SCRIPT_CONFIG_INSTALL \
    CROSS_COMPILERS_DIR HEPTANE_ROOT ROOT_DIR
do
	echo "    $var: ${!var}"
done

MODE_DEBUG=1

# Stop if any command fails
#set -e
MAKE_OPTIONS=-s
# function fail() (bash) but fail() in sh
fail()
{
    echo "$*" >&2
    exit 1
}

#set make -jX where X stands for #CPU+1
CPUS=$(getconf _NPROCESSORS_ONLN)
PARALLEL=-j$((CPUS + 1))


#detection of the OS
HOST_OS=""
case "$(uname)" in
    Linux)
    HOST_OS="LINUX"
    ;;
    Darwin)
    HOST_OS="MACOS"
    ;;
    *)
    fail "Found unknown OS. Aborting!"
    ;;
esac

cd ./src
fileConfigMakefile=makefile.config
echo "CONFIG = ${HOST_OS}" > $fileConfigMakefile
if [ $MODE_DEBUG -eq 1 ]
    then
    echo "CXX=g++ -g -Wall" >>  $fileConfigMakefile
else
    echo "CXX=g++ -O3 -Wall" >>  $fileConfigMakefile
fi
# echo "ADDR2LINE = ${ADDR2LINE}" >>  $fileConfigMakefile
echo "XML2 = ${XML2}" >>  $fileConfigMakefile
echo "RM=rm -f -v" >>  $fileConfigMakefile
echo "CXXFLAGS+=-D${HOST_OS}" >>  $fileConfigMakefile

cd "${ROOT_DIR}"
####################################
# CROSS COMPILER INSTALLATION
####################################
echo "Installing the CROSS COMPILER"
if [ "$*" != "clean" ]; then
    CROSS_COMPILERS_DIR=${ROOT_DIR}/CROSS_COMPILERS

    if [ $CROSS_COMPILER_MIPS_INSTALL -eq 1 ];
	then
	./scripts/install_cross_compiler.sh ${HOST_OS} MIPS ${CROSS_COMPILERS_DIR}
	cd "${ROOT_DIR}"
    fi
    
    if [ $CROSS_COMPILER_ARM_INSTALL -eq 1 ];
	then
	./scripts/install_cross_compiler.sh ${HOST_OS} ARM ${CROSS_COMPILERS_DIR}
	cd "${ROOT_DIR}"
    fi
fi

####################################
# CFGLIB INSTALLATION
####################################
echo "Installing Heptane CFGLIB"
if [ $HEPTANE_CFGLIB_INSTALL -eq 1 ];
    then

    pushd ./src/Common/cfglib

    hash doxygen 2> /dev/null
    [ $? -eq 0 ] || echo "Warning: CFGLIB requires doxygen to generate the doc but it's not installed."

    make ${MAKE_OPTIONS} ${PARALLEL} $*
    if [ "$*" != "clean" ]; then
	make ${MAKE_OPTIONS} install
    fi

    popd

fi


####################################
# Heptane (core) INSTALLATION
####################################
echo "Installing Heptane CORE"
if [ $HEPTANE_CORE_INSTALL -eq 1 ];
    then
    argMake=$*
    cd ./src/Common/GlobalAttributes
    make ${MAKE_OPTIONS} ${PARALLEL} $argMake
    cd "${ROOT_DIR}"

    cd ./src/Common/utl
    make ${MAKE_OPTIONS} ${PARALLEL} $argMake
    cd "${ROOT_DIR}"

    cd ./src/Common/ArchitectureDependent
    make ${MAKE_OPTIONS} ${PARALLEL} $argMake
    cd "${ROOT_DIR}"

    cd ./src

#check if lpsolve/cplex are installed
    LP_SOLVER_PRESENT=0
    hash lp_solve 2> /dev/null
    [ $? -eq 1 ] || LP_SOLVER_PRESENT=1
    hash cplex 2> /dev/null
    [ $? -eq 1 ] || LP_SOLVER_PRESENT=1

    [ $LP_SOLVER_PRESENT -eq 1 ] || echo "Warning: lpsolve and/or cplex are required for WCETanalysis but they are not installed."

    echo "include ${HEPTANE_ROOT}/src/$fileConfigMakefile" >  makefile.common
    cat ./templates/makefile.common.template >> makefile.common

    # I don't know why make doc is not accepted here!!!
    if [ "$*" == "doc" ]; then	argMake=theDoc; fi
    make ${MAKE_OPTIONS} $argMake

    cd "${ROOT_DIR}"
fi

######################
# BUNDLE Installation
######################
echo "Installing BUNDLEP Analysis"
pushd src/BundleCFG
  make ${MAKE_OPTIONS} ${PARALLEL}
popd #BundleCFG

pushd src/BundleWCETO
  make ${MAKE_OPTIONS} ${PARALLEL}
popd #BundleCFG


####################################
# template files generation
####################################
echo "Generating templates"
if [ $SCRIPT_CONFIG_INSTALL -eq 1 ];
    then
    CROSS_COMPILER_DIR=${ROOT_DIR}/CROSS_COMPILERS
    mkdir -p config_files

    #extract script
#    cp ./src/templates/extract.sh.template ./extract.sh
    sed  -e "s#_HEPTANE_ROOT_#${HEPTANE_ROOT}#g" ./src/templates/extract.sh.template >  ./extract.sh

    chmod +x ./extract.sh

    # Template for the extract script (MIPS)
    sed  -e "s#_CROSS_COMPILER_DIR_#${CROSS_COMPILER_DIR}/MIPS#g" -e "s#SELECTED_ENDIANNESS#${ENDIANNESS_MIPS}#g" ./src/templates/configExtract_MIPS.xml.template > ./config_files/configExtract_template_MIPS.xml
    # Template for the extract script (ARM)
    sed  -e "s#_CROSS_COMPILER_DIR_#${CROSS_COMPILER_DIR}/ARM#g" -e "s#SELECTED_ENDIANNESS#${ENDIANNESS_ARM}#g" ./src/templates/configExtract_ARM.xml.template > ./config_files/configExtract_template_ARM.xml

    #analysis script
    sed  -e "s#_HEPTANE_ROOT_#${HEPTANE_ROOT}#g" ./src/templates/analysis.sh.template >  ./analysis.sh
    chmod +x ./analysis.sh

    # Template for analysis script
    sed -e "s#_CROSS_COMPILER_DIR_#${CROSS_COMPILER_DIR}/MIPS#g" -e "s#BENCH_PATH#${ROOT_DIR}/benchmarks#g" -e "s#SELECTED_ENDIANNESS#${ENDIANNESS_MIPS}#g" ./src/templates/configWCET_MIPS.xml.template > ./config_files/configWCET_template_MIPS.xml
    sed -e "s#_CROSS_COMPILER_DIR_#${CROSS_COMPILER_DIR}/ARM#g" -e "s#BENCH_PATH#${ROOT_DIR}/benchmarks#g" -e "s#SELECTED_ENDIANNESS#${ENDIANNESS_ARM}#g" ./src/templates/configWCET_ARM.xml.template > ./config_files/configWCET_template_ARM.xml

    #run_benchmark script
    cp ./src/templates/run_benchmark.sh ./run_benchmark.sh
    chmod +x ./run_benchmark.sh

    cd "${ROOT_DIR}"

fi


####################################
# END MSG
####################################

#echo ""
#echo ""
#echo "HEPTANE installation done"

exit 0
