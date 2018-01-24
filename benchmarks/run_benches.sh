#!/bin/bash

for t in 1 2 4 8 16 32
do
	command="l($t)/l(2) * 55"
	c=$(echo -e "$command" | bc -l)
	for s in sim-?.cfg
	do
		make clean
		make -j3 THREADS=$t SIMCFG=../../$s CTXCOST=$c
	done
done
		  
