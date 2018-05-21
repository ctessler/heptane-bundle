odirname=$(shell date +\%F.\%H:\%M)
ODIR?=$(odirname)
SIMCFG?=../../sim-a.cfg
THREADS?=3
BUNDLE_CTX?=$(shell perl -e 'use POSIX; print ceil(log($(THREADS))/log(2) * 55);')
THREAD_CTX?=10
results=$(ODIR)/results-$(shell basename $(SIMCFG))-$(THREADS)
benchmarks=  bs bsort100 crc expint fft insertsort jfdctint
benchmarks+= lcdnum matmult minver ns nsichneu qurt select simple sqrt
benchmarks+= statemate ud

MIPSCC=$(shell command -v mips-cc 2>/dev/null)
SIM=$(shell command -v simulator 2>/dev/null)

.PHONY: $(benchmarks) noresults

all: $$(results)
	@echo "BUNDLE_CTX : $(BUNDLE_CTX)"
	@echo "THREAD_CTX : $(THREAD_CTX)"
ifndef MIPSCC
	$(error "mips-cc is not in the PATH")
endif
ifndef SIM
	$(error "simulator is not in the PATH")
endif

$$(results): $(benchmarks) $$(ODIR)
	./collect.pl -t $(THREADS) $(benchmarks) -o $(results)

# To run an individual benchmark for a given simulatior configuration and threads
# issue the following
# ./upfromsim.sh sim-a.cfg bs
# make -C bs THREADS=8 SIMCFG=../../sim-a.cfg BUNDLE_CTX=135 THREAD_CTX=10
$(benchmarks):
	./upfromsim.sh $(shell basename $(SIMCFG)) $@
	make -C $@ $(tgt) THREADS=$(THREADS) SIMCFG=$(SIMCFG) BUNDLE_CTX=$(BUNDLE_CTX) THREAD_CTX=$(THREAD_CTX)

$$(ODIR):
	mkdir -p $(ODIR)

clean: tgt=clean
clean: $(benchmarks)

noresults: clean
	rm -rf results-* *-comp.txt

