ble_audio_android
========


android audio框架
--------
蓝牙BLE又称bluetooth smart，主打的是低功耗和快速链接，所以在支持的profile并没有audio的部分，而蓝牙语音协议A2DP只在传统蓝牙中有，本文就是提供一种利用ble数据来传输压缩语音，并最终在实现用android语音框架中的AudioRecord方法来获取语音流。
说明文档：[蓝牙ble数据转语音实现Android AudioRecord方法推荐](https://www.cnblogs.com/zozo825117/p/10272395.html)

## Features

- [x] kernel audio driver
- [x] audio deamon
- [x] framework AudioMananger config
- [x] Adpcm decode

--------

## TODO


## Project Organization
![](http://ww1.sinaimg.cn/large/98df967fgy1fz89c5z2iej20ak0h7js4.jpg)

主要文件介绍

    ├── upcm
	│   ├──upcm.c		       <- linux audio driver
	│   ├──Makefile			   
    ├── android_PCM            <- Android PCM语音通道
	│   ├── Android.mk     
	│   ├── audio_hw.cpp   
	│   ├── audio_policy.conf   
    ├── aduio_d              <- android接受处理语音工具
	│   ├── toolchain			<- 交叉编译工具
	│   ├── main.c     
	│   ├── audio.c   
	│   ├── dev_list.c 
	│   ├── ADPCMDecoder.c
	│   ├── Makefile
    ├── AudioRecordTest				<- android record test app
--------
## build

- platform:Real4418
- kernel:linux3.4.39
- android:Android 4.4.2

### upcm.ko的编译
- 修改upcm源码/Makefile 文件：

``` dts
KDIR ?= /root/Desktop/myir_4418/android/linux/kernel/kernel-3.4.39 
```

注：需要修改为当前使用的内核路径。

- 编译：

``` groovy
make ARCH=arm 	CROSS_COMPILE=/root/Desktop/myir_4418/android/prebuilts/gcc/linux-x86/arm/arm-eabi-4.7/bin/arm-ea	bi-
```

注：需要修改为当前系统所需要的编译器路径。

### Android PCM语音通道
将文件放到源码路径下，执行Android.mk编译。

### audio deamon
交叉编译工具就在文件路径下，可直接编译。
--------
## Useage
- 创建一个虚拟的PCM通道
	把upcm.ko放到文件系统/system/etc/下，在板级的init.rc里加入insmod /system/etc/upcm.ko。
	这样上电就可以加载upcm.ko的驱动。

- 安卓中间层创建PCM通道来链接底层建立的虚拟PCM通道
	1.  把`audio.LSDAudio.default.so`，放到`system/lib/hw/`下。
	2.  修改`system/etc/audio_policy.conf` 文件，把primary里的input删掉，只留output，并加上下面的内容。可参考文件包的样本，注意检查权限为644。
		`audio.LSDAudio.default.so`的加载，是靠`audio_policy.conf`里面建立PCM通道来加载的。这样就创建了一个input_mic的PCM通道。
``` nginx
				LSDAudio {
					inputs {
					LSDAudio {
						sampling_rates 8000|16000
						channel_masks AUDIO_CHANNEL_IN_MONO
						formats AUDIO_FORMAT_PCM_16_BIT
						devices AUDIO_DEVICE_IN_BUILTIN_MIC
						}
					}
				}
```
- 建立蓝牙语音数据连接
	将工具audio_d放到system/bin，加入到系统里自动启动
``` scala
			service hidraw /system/bin/audio_d
					class main
					oneshot
```
			注意，需要在加载upcm.ko之后运行。

- APP测试工具
	app测试工具：AudioRecordTest，直接安装APK即可使用。
--------
##  Reference

-  [android 音频系统/声卡驱动 codec - lp542718520的专栏 - CSDN博客](https://blog.csdn.net/lp542718520/article/details/70234312) 


