name=$(shell basename `pwd`)
exe=$(name).exe
MIPSCC=$(shell command -v mips-cc 2>/dev/null)
subdirs=bundle
.PHONY: all clean $(subdirs) precheck

all: $(name).xml $(subdirs)
ifndef MIPSCC
	$(error "mips-cc is not in the PATH")
endif

$(subdirs): 
	make -C $@ $(opt)

$(name).xml: configExtract.xml
	- ../../bin/HeptaneExtract configExtract.xml
	@echo "Ignore 'Unknown annotation type found in binary 48' errors"

$(name).exe: $(name).c
	mips-gcc -fomit-frame-pointer -ggdb -c -S $(name).c -o $(name).s
	mips-as $(name).s -o $(name).o
	mips-ld -e main -o $(name).exe $(name).o -T ../simul.ld
	mips-readelf -S $(name).exe > $(name).readelf
	mips-objdump -t -d -z $(name).exe > $(name).objdump

clean: opt=clean
clean: $(subdirs)
	rm -rf res*.xml *.pdf $(name).xml *.log
	rm -rf $(name).dot $(name).html $(name).jpg core
	rm -rf $(name)_*.xml annot.xml $(name).o $(name).s $(name).objdump
	rm -rf $(name).readelf $(name).exe

