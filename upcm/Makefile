########################
# CSR UPCM ALSA driver #
########################
KDIR ?= /root/Desktop/myir_4418/android/linux/kernel/kernel-3.4.39 

obj-m += upcm.o
upcm:
	$(MAKE) -C $(KDIR) M=`pwd` EXTRA_CLFAGS=-fno-pic modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
