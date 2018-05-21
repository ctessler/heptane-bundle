THREADS?=3
SIMCFG?=simulator-$(THREADS).cfg
BUNDLE_CTX?=$(shell perl -e 'use POSIX; print ceil(log($(THREADS))/log(2) * 55);')
THREAD_CTX?=10
valgrind=valgrind --leak-check=full
undefine valgrind
wd=$(shell pwd)
dirname=$(shell dirname $(wd))
name=$(shell basename $(dirname))

# Simulation temporary files
bundle-sim=$(name)-bundle.sim
serial-sim=$(name)-serial.sim
nbundle-ctxs=$(name)-bundle.ctxs
nthread-ctxs=$(name)-thread.ctxs
nserial-ctxs=$(name)-serial.ctxs
bundle-cycles=$(name)-bundle.cycles
bundle-cycles-ctx=$(name)-bundle-ctx.cycles
serial-cycles=$(name)-serial.cycles
serial-cycles-ctx=$(name)-serial-ctx.cycles
cache-sets=$(name)-sets.cache
cache-brt=$(name)-brt.cache
cache-cpi=$(name)-cpi.cache
sim-done=$(name)-sim.done

# Analysis 
bundle-analysis=$(name)-bundlep.analysis
ilp-current=$(name)-ilp-current.wceto
ilp-prev=$(name)-ilp-prev.wceto
heptane=$(name)-heptane-one.wcet
heptane-total=$(name)-heptane-total.wcet
bundle-done=$(name)-bundlep.done

.PHONY: clean

SIM=$(shell command -v simulator 2>/dev/null)

all: summary.txt
ifndef SIM
	$(error "simulator is not in the PATH")
endif

summary.txt: $(bundle-done) $(sim-done) $(heptane) $(heptane-total)
	@echo -n "CACHE SETS:		" > $@
	@cat $(cache-sets) >> $@
	@echo -n "CACHE BRT:		" >> $@
	@cat $(cache-brt) >> $@
	@echo -n "CACHE CPI:		" >> $@
	@cat $(cache-cpi) >> $@
	@echo "THREADS:		$(THREADS)" >> $@
	@echo "BUNDLE CTX:		$(BUNDLE_CTX)" >> $@
	@echo "THREAD CTX:		$(THREAD_CTX)" >> $@
	@echo -n "ILP WCETO (Current):	" >> $@
	@cat $(ilp-current) >> $@
	@echo -n "ILP WCETO (Previous):	" >> $@
	@cat $(ilp-prev) >> $@
	@echo -n "Heptane WCET(1):	" >> $@
	@cat $(heptane) >> $@
	@echo -n "Hept. WCET($(THREADS) + CTXS):	" >> $@
	@cat $(heptane-total) >> $@
	@echo -n "N BUNDLE CTXs:		" >> $@
	@cat $(nbundle-ctxs) >> $@
	@echo -n "N THREAD CTXs:		" >> $@
	@cat $(nthread-ctxs) >> $@
	@echo -n "BUNDLE EXE(NO CTX):	" >> $@
	@cat $(bundle-cycles) >> $@
	@echo -n "BUNDLE EXE(W/ CTX):	" >> $@
	@cat $(bundle-cycles-ctx) >> $@
	@echo -n "Serial EXE(NO CTX):	" >> $@
	@cat $(serial-cycles) >> $@
	@echo -n "Serial EXE(W/ CTX):	" >> $@
	@cat $(serial-cycles-ctx) >> $@
	@echo ----------------------------------------
	@cat summary.txt

#
# Analytical 
#
$(heptane): configBUNDLE.xml
	../../../bin/HeptaneAnalysis ../$(name).exe configBUNDLE.xml \
	    | grep WCET | awk '{print $$2}' > $(heptane)
$(heptane-total): hwcet=$(shell cat $(heptane))
$(heptane-total): $(heptane)
	echo "($(hwcet) + $(BUNDLE_CTX)) * $(THREADS)" | bc -l > $@

# Extract the control flow graph
$(name).cfg: configBUNDLE.xml ../../../bin/BundleCFG
	$(valgrind) ../../../bin/BundleCFG configBUNDLE.xml
# Perform the BUNDLEP analysis
$(bundle-analysis): $(name).cfg ../../../bin/BundleWCETO
	$(valgrind) ../../../bin/BundleWCETO -c $(name).cfg -m $(THREADS) \
		-x $(BUNDLE_CTX) -t $(THREAD_CTX) configBUNDLE.xml
	touch $@
# ILP Result
$(ilp-current): $(bundle-analysis)
	lp_solve $(name)-level-1.lp | grep "objective function:" | \
	awk '{print $$5}' | sed s/\\..*//g > $@
$(ilp-prev): $(bundle-analysis)
	lp_solve $(name)-level-1.lp2 | grep "objective function:" | \
	awk '{print $$5}' | sed s/\\..*//g > $@
$(bundle-done): $(ilp-current) $(ilp-prev)
	touch $@

#
# Simulated execution information
#
$(SIMCFG): 
	sed 's/threads.*/threads = $(THREADS);/' simulator.cfg > $(SIMCFG)
$(bundle-sim): $(SIMCFG)
	simulator -c $(SIMCFG) -f *.entry ../$(name).exe -t $(THREADS) > $@
$(serial-sim): $(SIMCFG)
	simulator -c $(SIMCFG) ../$(name).exe -t $(THREADS) > $@
$(nbundle-ctxs): $(bundle-sim)
	grep 'BUNDLE CTX Switches:' $(bundle-sim) | awk '{print $$4}' > $@
$(nthread-ctxs): $(bundle-sim)
	grep 'THREAD CTX Switches:' $(bundle-sim) | awk '{print $$4}' > $@
$(bundle-cycles): $(bundle-sim)
	grep 'Execution Cycles:' $(bundle-sim) | awk '{print $$3}' > $@
$(bundle-cycles-ctx): lcl-cycles=$(shell cat $(bundle-cycles))
$(bundle-cycles-ctx): lcl-bctxs=$(shell cat $(nbundle-ctxs))
$(bundle-cycles-ctx): lcl-tctxs=$(shell cat $(nthread-ctxs))
$(bundle-cycles-ctx): $(bundle-cycles) $(nbundle-ctxs) $(nthread-ctxs)
	echo "$(lcl-cycles) + $(lcl-bctxs) * $(BUNDLE_CTX) + $(lcl-tctxs) * $(THREAD_CTX)" | bc -l > $@

$(nserial-ctxs): $(serial-sim)
	grep 'BUNDLE CTX Switches:' $(serial-sim) | awk '{print $$4}' > $@
$(serial-cycles): $(serial-sim)
	grep 'Execution Cycles:' $(serial-sim) | awk '{print $$3}' > $@
$(serial-cycles-ctx): lcl-cycles=$(shell cat $(serial-cycles))
$(serial-cycles-ctx): lcl-bctxs=$(shell cat $(nserial-ctxs))
$(serial-cycles-ctx): $(serial-cycles) $(serial-ctxs)
	echo "$(lcl-cycles) + $(lcl-bctxs) * $(BUNDLE_CTX)" | bc -l > $@

$(cache-sets): $(SIMCFG)
	grep 'sets =' $(SIMCFG) | awk '{print $$3}' | sed s/\;// > $@
$(cache-brt): $(SIMCFG)
	grep 'memory_latency = ' $(SIMCFG) | awk '{print $$3}' | sed s/\;// > $@
$(cache-cpi): $(SIMCFG)
	grep ' latency = ' $(SIMCFG) | awk '{print $$3}' | sed s/\;// > $@


# False target to encapsulate the others
$(sim-done): $(bundle-sim) $(serial-sim) $(nbundle-ctxs) $(nthread-ctxs) \
	$(nserial-ctxs) $(bundle-cycles) $(serial-cycles) $(bundle-cycles-ctx) \
	$(serial-cycles-ctx) $(cache-sets) $(cache-brt) $(cache-cpi)
	touch $@


datespec=$(shell date +\%F.\%H:\%M)
clean:
	@-mv summary.txt summary-$(datespec).txt
	rm -f $(name).cfg *.dot *.jpg *.entry vgcore.* *.wceto simulator-*.cfg
	rm -f *.lp *.lp2 *.wcet *.ctxs *.done *.sim *.cycles *.analysis
	rm -f *.log *.entry-w-unswitched *.cache

reallyclean: clean
	rm -f summary*.txt	
