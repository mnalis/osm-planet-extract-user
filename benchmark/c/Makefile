CFLAGS = -O3 -g -Wall
TARGETS = wc_gnu_generic wc_gnu_generic_nochecks wc_mmap wc_simple wc_gnu_avx2
TESTFILE = /tmp/h.osm
TIME = @time -f "%C %Us user %Ss system %es total"

all: $(TARGETS)

wc_gnu_avx2: wc_gnu_avx2.c
	cc $(CFLAGS) -mavx2 -o $@ $^

bench: all
	$(TIME) ./wc_simple < $(TESTFILE)
	$(TIME) ./wc_gnu_generic < $(TESTFILE)
	$(TIME) ./wc_gnu_generic_nochecks < $(TESTFILE)
	$(TIME) ./wc_mmap $(TESTFILE)
	$(TIME) ./wc_gnu_avx2 < $(TESTFILE)

clean:
	rm -f $(TARGETS)

.PHONY: all clean
