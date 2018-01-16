#!/bin/bash

for t in 1 2 4 8 16 32
do
	for s in sim-a.cfg sim-b.cfg sim-c.cfg sim-d.cfg
	do
		make clean
		make -j3 THREADS=$t SIMCFG=../../$s
	done
done
		  
