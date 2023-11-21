# comple all plugins in subfolders as shared libraries

# get all subfolders which have Makefile
SUBDIRS := $(wildcard */Makefile)

# filter out the folder name from SUBDIRS
SUBDIRS := $(patsubst %/Makefile, %, $(SUBDIRS))

# make each subfolders
TARGS= all clean

$(TARGS): $(SUBDIRS)
	for dir in $(SUBDIRS); do \
        make -C $$dir $(MAKECMDGOALS); \
    done	


.PHONY: $(TARGS)