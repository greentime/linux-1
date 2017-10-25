obj-y				:= extable.o tlb.o \
				   fault.o init.o ioremap.o mmap.o \
                                   mm-nds32.o cacheflush.o proc-n13.o

obj-$(CONFIG_ALIGNMENT_TRAP)	+= alignment.o
obj-$(CONFIG_HIGHMEM)           += highmem.o
CFLAGS_proc-n13.o		+= -fomit-frame-pointer
