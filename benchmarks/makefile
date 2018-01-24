SIMCFG?=../../sim-a.cfg
THREADS?=3
CTXCOST?=$(shell echo 'l($(THREADS))/l(2) * 55' | bc -l)
results=results-$(shell basename $(SIMCFG))-$(THREADS)
benchmarks=  bs bsort100 crc expint fft insertsort jfdctint
benchmarks+= lcdnum matmult minver ns nsichneu qurt select simple sqrt
benchmarks+= statemate ud

MIPSCC=$(shell command -v mips-cc 2>/dev/null)
SIM=$(shell command -v simulator 2>/dev/null)

.PHONY: $(benchmarks) noresults

all: $(results)
	echo "CTX COST: $(CTXCOST)"
ifndef MIPSCC
	$(error "mips-cc is not in the PATH")
endif
ifndef SIM
	$(error "simulator is not in the PATH")
endif

$(results): $(benchmarks)
	./collect.sh $(THREADS) $(benchmarks) > $(results)

$(benchmarks):
	./upfromsim.sh $(shell basename $(SIMCFG)) $@
	make -C $@ $(tgt) THREADS=$(THREADS) SIMCFG=$(SIMCFG) CTXCOST=$(CTXCOST)

clean: tgt=clean
clean: $(benchmarks)

noresults: clean
	rm -rf results-*	

