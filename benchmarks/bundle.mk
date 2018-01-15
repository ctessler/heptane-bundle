THREADS?=3
SIMCFG?=simulator-${THREADS}.cfg
valgrind="valgrind --leak-check=full"
undefine valgrind
wd=$(shell pwd)
dirname=$(shell dirname ${wd})
name=$(shell basename ${dirname})
wceto=${name}-${THREADS}.wceto
obs=${name}-${THREADS}.obs
heptane=${name}.hept

.PHONY: clean

SIM=$(shell command -v simulator 2>/dev/null)

all: ${wceto} ${obs} summary-${THREADS}.txt
ifndef SIM
	$(error "simulator is not in the PATH")
endif

summary-${THREADS}.txt:	cost=`sed 's/.*: //' ${heptane}`
summary-${THREADS}.txt: tot=$(shell echo ${cost} \* ${THREADS} | bc)
summary-${THREADS}.txt: ${obs} ${wceto} ${heptane}
	@echo -n "Bundle WCETO: " > $@
	@tail -n 1 ${wceto} >> $@
	@echo -n "Heptane WCET: " >> $@
	@echo "${cost} (1 thread)" >> $@
	@echo -n "Heptane WCET: " >> $@
	@echo "${tot} (${THREADS} threads)" >> $@
	@echo -n "Bundle Observed " >> $@
	@cat ${name}-bundle.obs >> $@
	@echo -n "Serial Observed " >> $@
	@cat ${name}-serial.obs >> $@
	cat $@

${heptane}:
	../../../bin/HeptaneAnalysis ../${name}.exe configBUNDLE.xml \
	    | grep WCET > ${heptane}

${obs}: ${wceto} ${SIMCFG}
	simulator -c ${SIMCFG} -f *.entry ../${name}.exe -t ${THREADS} \
	    | grep 'Execution' > ${name}-bundle.obs
	simulator -c ${SIMCFG} ../${name}.exe -t ${THREADS} \
	    | grep 'Execution' > ${name}-serial.obs

${wceto}: ${name}.cfg
	${valgrind} ../../../bin/BundleWCETO -c ${name}.cfg -m ${THREADS} \
	    configBUNDLE.xml
	mv ${name}-level-1.wceto ${wceto}
	cat ${wceto}

${name}.cfg: 
	${valgrind} ../../../bin/BundleCFG --verbose configBUNDLE.xml

${SIMCFG}: simulator.cfg
	sed 's/threads.*/threads = ${THREADS};/' simulator.cfg > ${SIMCFG}

clean:
	rm -f ${top} ${name}.cfg *.dot *.jpg *.entry vgcore.* *.wceto simulator-*.cfg
	rm -f summary-*.txt *.obs *.hept *.log core
