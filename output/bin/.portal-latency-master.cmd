cmd_output/bin/portal-latency-master := /usr/local/k1tools//bin/k1-gcc -o output/bin/portal-latency-master  output/build/portal-latency-master_build//home/lig-ext/podesta/bluedragon-system/benchmark/portal-latency/master.c.o     -mcluster=ioddr -L/home/lig-ext/podesta/bluedragon-system/output/lib/io/ -Wl,--defsym=_LIBNOC_DISABLE_FIFO_FULL_CHECK=0   -march=k1b -mboard=developer -mos=rtems -lmppaipc  
