#!/bin/bash

function usage {
	echo "collect.sh <NTHREADS> <BENCHMARK1> <BENCHMARK2> ... "
}

function single_file {
	D=$1
	T=$2
	local name=$(basename $D)
	local file=$D/bundle/summary-$T.txt
	local bwceto=$(sed -n 's/Bundle WCETO: //p' $file)
	pat="Heptane WCET: \([0-9]\+\) ($T threads)"
	str='s,'"$pat"',\1,p'
	local hwcet=$(sed -n "$str" $file)
	local bobs=$(sed -n 's/Bundle Observed Execution Time: \([0-9]\+\).*/\1/p' $file)
	local hobs=$(sed -n 's/Serial Observed Execution Time: \([0-9]\+\).*/\1/p' $file)
	echo "$name,$T,$bwceto,$hwcet,$bobs,$hobs"
}


if [ "$#" == "0" ]
then
	usage
	exit 1
fi
THREADS=$1
shift

echo "# Benchmark, Threads, Bundle WCETO, Heptane WCETO, Bundle Observed, Heptane Observed"

while (( "$#" ))
do
	DIR=$1
	shift
	single_file $DIR $THREADS
done


