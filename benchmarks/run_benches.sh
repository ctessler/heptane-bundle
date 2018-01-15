#!/bin/bash

for t in 1 2 4 8 16 32
do
	for s in sim-a.cfg sim-b.cfg
	do
		make clean
		make THREADS=$t SIMCFG=../../$s
	done
done
		  
