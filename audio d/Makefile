#!/bin/sh

# The following definitions should be modified to match the user's installation and toolchain
TOOLCHAIN_PREFIX=./toolchain/arm-linux-androideabi-4.6/bin/arm-linux-androideabi-

CC=${TOOLCHAIN_PREFIX}gcc
CFLAGS=-march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -pie -fPIE
LDFLAGS=-pie -fPIE

SRCS =  audio.c dev_list.c main.c

SRCS += dev_default.c
SRCS += ADPCMDecoder.c
hidraw_daemon: FORCE
	$(CC) $(LDFLAGS) $(CFLAGS)  -o audio_d $(SRCS)  $(LIBS)
#	adb push hidraw_daemon /data/
#	adb shell /data/hidraw_daemon

capture_audio: FORCE
	$(CC)  -o  capture_audio adpcm_decoder.c audiod.c

clearpair: FORCE
	$(CC) -o clearpair clearpair.c

FORCE:

clean:
	rm -rf audio_d *.o

