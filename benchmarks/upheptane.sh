#!/bin/bash

function usage {
	echo "upheptane.sh <BENCHMARK> <# SETS> <# WAYS> <LINE SIZE> <CACHE LATENCY> <MEM LATENCY>"
}

if [ "$#" != "6" ]
then
	usage
	exit 1
fi

B=$1
SETS=$2
WAYS=$3
LINES=$4
CLAT=$5
MLAT=$6

BEGIN="Cache Begin"
END="Cache End"

FILE=$B/bundle/configBUNDLE.xml
echo "Updating Heptane XML Analysis file ${FILE} to: "
echo "Sets: $SETS Ways: $WAYS Lines: $LINES Cache Latency: $CLAT Memory Latency: $MLAT"

sed -i -n "/${BEGIN}/{:a;N;/${END}/!ba;N;s/.*\n/<!-- Cache Begin -->\n\
<CACHE nbsets=\"$SETS\" nbways=\"$WAYS\" cachelinesize=\"$LINES\" level=\"1\" \n\
       replacement_policy=\"LRU\" type=\"icache\" latency=\"$CLAT\" \/>\n\
<CACHE nbsets=\"$SETS\" nbways=\"$WAYS\" cachelinesize=\"$LINES\" level=\"1\" \n\
       replacement_policy=\"LRU\" type=\"dcache\" latency=\"$CLAT\" \/>\n\
<MEMORY load_latency=\"$MLAT\" store_latency=\"$MLAT\" \/>\n\
<!-- Cache End -->\n/};p" $FILE

