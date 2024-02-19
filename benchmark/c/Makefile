CFLAGS = -O2 -g -Wall
TARGETS = wc_gnu_generic wc_gnu_generic_nochecks wc_mmap wc_simple
TESTFILE = /tmp/h.osm

all: $(TARGETS)

bench: all
	time ./wc_simple < $(TESTFILE)
	time ./wc_gnu_generic < $(TESTFILE)
	time ./wc_gnu_generic_nochecks < $(TESTFILE)
	time ./wc_mmap $(TESTFILE)

clean:
	rm -f $(TARGETS)

.PHONY: all clean
