#!/bin/bash
#
# Updates a Heptane configBUNDLE.xml file based on the simulator parameters.
#

if [ "$#" != "2" ]
then
	echo "$#"
	echo "Usage:"
	echo "upfromsim.sh <SIMULATOR CONFIG> <BENCHMARK>"
	exit 1
fi

FILE=$1
BMARK=$2

MLAT=$(sed -n 's/memory_latency = \([0-9]\+\).*/\1/p' $FILE)
CLAT=$(sed -n 's/[ ]\+latency = \([0-9]\+\).*/\1/p' $FILE)
WAYS=$(sed -n 's/ways = \([0-9]\+\).*/\1/p' $FILE)
SETS=$(sed -n 's/sets = \([0-9]\+\).*/\1/p' $FILE)
LINES=$(sed -n 's/line_size = \([0-9]\+\).*/\1/p' $FILE)

cmd="./upheptane.sh $BMARK $SETS $WAYS $LINES $CLAT $MLAT"
echo $cmd
$cmd
