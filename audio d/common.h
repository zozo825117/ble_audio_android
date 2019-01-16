#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

typedef int (*device_audio_main) (int fd);


struct device
{
    char * name;
    int vid;
    int pid;
    int desc_size;
    char dev_path[256];
   // device_motion_init  motion_init;
   // device_motion_get_input motion_get_input;
    //device_motion_sensor_sleep_cb motion_sensor_sleep_cb;
    device_audio_main  audio_main;
};

inline struct device * get_current_device();
inline bool select_device(char * dev_path);
bool open_upcm_dev();
void close_upcm_dev();
bool write_upcm_dev(unsigned char * data, int size);
//extern struct device * current_device;
#endif
