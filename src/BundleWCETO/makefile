subdirs=src
.PHONY: clean all $(subdirs)

all: $(subdirs)

$(subdirs):
	make -C $@ $(opt)

clean: opt=clean
clean: $(subdirs)
