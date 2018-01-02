name=$(shell basename `pwd`)
exe=${name}.exe
MIPSCC=$(shell command -v mips-cc 2>/dev/null)
subdirs=bundle
.PHONY: all clean $(subdirs) precheck

all: $(subdirs)
ifndef MIPSCC
	$(error "mips-cc is not in the PATH")
endif

$(subdirs): ${name}.xml
	make -C $@ $(opt)

${name}.xml: configExtract.xml
	../../bin/HeptaneExtract configExtract.xml

clean: opt=clean
clean: $(subdirs)
clean:
	rm -rf res*.xml *.pdf ${name}.xml *.log
	rm -rf ${name}.dot ${name}.html ${name}.jpg core
	rm -rf ${name}_*.xml annot.xml

