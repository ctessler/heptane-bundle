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
	local bctxs=$(sed -n 's/Bundle Thread Level Switches: \([0-9]\+\).*/\1/p' $file)
	local sctxs=$(sed -n 's/Serial Thread Level Switches: \([0-9]\+\).*/\1/p' $file)

	local aben;
	(( aben = $hwcet - $bwceto ))
	
	local oben;
	(( oben = $hobs - $bobs ))
	if [ "$den" == "0" ]
	   then
		   den=1
	fi

	local ctol;
	local den;
	(( den = $bctxs - $sctxs ))
	if [ "$den" != "0" ]
	then
		(( ctol = ($hobs - $bobs) / $den ))
	else
		ctol=0
	fi
	printf "$FMT" \
	       $name, $T, $bwceto, $hwcet, $aben, $bobs, $hobs, $bctxs, $sctxs, $oben, \
	       $ctol
	
}


if [ "$#" == "0" ]
then
	usage
	exit 1
fi
THREADS=$1
shift



echo "# Terms: "
echo "#     bmark = benchmark, t = threads, Bu = bundle, He = heptane, obs = observed"
echo "#     X = thread level context switch, B+ = Bundle Benefit "
echo "#     ctol = additional cycle tolerance"
echo "#     Note: observed times don't include context switch cost cycles" 
FMT="%-8s %3s %10s %10s %9s %10s %10s %4s %4s %6s %3s\n"
printf "# $FMT" bmark, t, BuWCETO, HeWCETO, B+WCETO, BuObs, HeObs, BuX, HeX, B+Obs, CTOL
FMT="%-10s %3s %10s %10s %9s %10s %10s %4s %4s %6s %3s\n" 


while (( "$#" ))
do
	DIR=$1
	shift
	single_file $DIR $THREADS
done


