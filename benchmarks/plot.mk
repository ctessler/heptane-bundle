PLOTS=$(wildcard *.plot)
TEXS=$(PLOTS:.plot=.tex)
PDFS=$(PLOTS:.plot=.pdf)

all: $(PDFS)
	mkdir -p pdfs
	-rm *eps-converted*.pdf
	cp *.pdf pdfs/.

%.tex : %.plot
	gnuplot -e "set output '$@'" $<

%.pdf : %.tex
	latexmk -pdf $<

clean:
	-rm -f *.eps *.dvi *.aux *.log *.fls *.pdf *.fdb_latexmk
	-rm -rf pdfs
	@echo done
