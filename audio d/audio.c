/*
 * Copyright (c) 2013 DEQING DUSUN ELECTRON CO.;LTD. All rights reserved.
 */

#include "common.h"

static int  upcm_fd = -1;

bool open_upcm_dev()
{
    if (upcm_fd > 0 )
        return true;
    upcm_fd = open("/dev/upcm", O_WRONLY);
    if (upcm_fd > 0 )
        return true;
    else
        return false;
}

bool write_upcm_dev(unsigned char * data , int size)
{
    if (upcm_fd < 0 )
        return false;
    int ret = write(upcm_fd, data, size);
    if (ret == size)
        return true;
    else
        return false;
}

void close_upcm_dev()
{
    if (upcm_fd > 0 )
    {
        close(upcm_fd);
        upcm_fd = -1;
    }
}


int audio_main()
{
    struct device *  dev = get_current_device();
    if (dev == NULL)
        return -1;

    if (dev->audio_main == NULL )
        return -1;

    int fd = open(dev->dev_path, O_RDWR);
    if (fd < 0)
    {
        fprintf(stderr, "unable to open hidraw device %s\n", dev->dev_path);
        return -1;
    }

    dev->audio_main(fd);
    close(fd);
    return 0;
}