THREADS?=3
valgrind="valgrind --leak-check=full"
undefine valgrind
wd=$(shell pwd)
dirname=$(shell dirname ${wd})
name=$(shell basename ${dirname})
wceto=${name}-${THREADS}.wceto
obs=${name}-${THREADS}.obs
heptane=${name}.hept
simcfg=simulator-${THREADS}.cfg

all: ${wceto} ${obs} summary-${THREADS}.txt

summary-${THREADS}.txt:	cost=`sed 's/.*: //' ${heptane}`
summary-${THREADS}.txt: tot=$(shell echo ${cost} \* ${THREADS} | bc)
summary-${THREADS}.txt: ${obs} ${wceto} ${heptane}
	@echo "Bundle WCETO: " > $@
	@echo -n "	" >> $@
	@tail -n 1 ${wceto} >> $@
	@echo "Heptane WCET (One Thread)/(Total): " >> $@
	@echo "	(${cost})/(${tot})" >> $@
	@echo "Bundle Observed: " >> $@
	@echo -n "	" >> $@
	@cat ${name}-bundle.obs >> $@
	@echo "Serial Observed: " >> $@
	@echo -n "	" >> $@
	@cat ${name}-serial.obs >> $@
	cat $@

${heptane}:
	../../../bin/HeptaneAnalysis ../bsort100.exe configBUNDLE.xml \
	    | grep WCET > ${heptane}

${obs}: ${wceto} ${simcfg}
	/home/corey/ws/wsu-sim/cache_simulator/simulator -c ${simcfg} \
	    -f *.entry ../${name}.exe \
	    | grep 'Execution' > ${name}-bundle.obs
	/home/corey/ws/wsu-sim/cache_simulator/simulator -c ${simcfg} ../${name}.exe \
	    | grep 'Execution' > ${name}-serial.obs

${wceto}: ${name}.cfg
	${valgrind} ../../../bin/BundleWCETO -c ${name}.cfg \
	    -m ${THREADS} configBUNDLE.xml
	mv ${name}-level-1.wceto ${wceto}
	cat ${wceto}

${name}.cfg: 
	${valgrind} ../../../bin/BundleCFG --verbose configBUNDLE.xml

${simcfg}: simulator.cfg
	sed 's/threads.*/threads = ${THREADS};/' simulator.cfg > ${simcfg}

clean:
	rm -f ${top} ${name}.cfg *.dot *.jpg *.entry vgcore.* *.wceto simulator-*.cfg
	rm -f summary-*.txt *.obs *.hept
