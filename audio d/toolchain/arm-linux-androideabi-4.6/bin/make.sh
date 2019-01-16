#!/bin/bash

CFLAGS="-march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16  "

./arm-linux-androideabi-gcc  $CFLAGS -o test test.c 
