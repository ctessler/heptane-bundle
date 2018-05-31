#!/bin/bash
CTXZERO=${CTXZERO:-0}
datespec=`date +%F.%H:%M`
odirname=${ODIRNAME:=${datespec}}
nthreads="1 2 4 8 16"

for t in ${nthreads}
do
	for s in sim-?.cfg
	do
		make clean
		if [ "${CTXZERO}" -ne "0" ] ; then
			BX=0
			TX=0
		else
			command="l($t)/l(2) * 55"
			BX=$(echo -e "$command" | bc -l)
			TX=10
		fi
			
		if [ "$t" -eq "1" ] ; then
			BX=0
			TX=0
		fi
			
		make ODIR=$odirname THREADS=$t SIMCFG=../../$s \
		     BUNDLE_CTX=${BX} THREAD_CTX=${TX}
	done
done

echo "Generating plots and data"
pushd $odirname
../bundle-vs-heptane.pl -c f -t 16
../exe-vs-sets.pl -c f -t 16
../wcetoben-vs-sets.pl -c f -t 16
../bmark-table.pl -c f -t 16
echo "Making graphs"
cp ../plot.mk makefile
mkdir data
cp *.dat data/.
make
popd

echo "----------------------------------------"
echo "Results are in $odirname"
echo "----------------------------------------"
