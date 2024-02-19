CFLAGS = -O2 -g -Wall
TARGETS = wc_gnu_generic wc_gnu_generic_nochecks wc_mmap wc_simple
TESTFILE = /tmp/h.osm
TIME = @time -f "%C %Us user %Ss system %es total"

all: $(TARGETS)

bench: all
	$(TIME) ./wc_simple < $(TESTFILE)
	$(TIME) ./wc_gnu_generic < $(TESTFILE)
	$(TIME) ./wc_gnu_generic_nochecks < $(TESTFILE)
	$(TIME) ./wc_mmap $(TESTFILE)

clean:
	rm -f $(TARGETS)

.PHONY: all clean
