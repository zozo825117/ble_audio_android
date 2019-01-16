#include <linux/types.h>
#include <linux/input.h>
#include <linux/hidraw.h>
#include <linux/netlink.h>
#include "common.h"

static struct device * current_device = NULL;
extern struct device dev_default;
//extern struct device dev_tcl_broadcom;
//extern struct device dev_0502;

static struct device *  dev_list[] =
{
    &dev_default,
    NULL
};

inline struct device * get_current_device()
{
    return current_device;
}


inline bool select_device(char * dev_path)
{
    int fd,res,desc_size,i;
    struct hidraw_devinfo info;
    struct hidraw_report_descriptor rpt_desc;
    char buf[100];
    fd = open(dev_path, O_RDWR);
    if (fd < 0 )
    {
        sleep(1);
        fd = open(dev_path, O_RDWR);
        if (fd < 0 )
            return false;
    }

    ioctl(fd, HIDIOCGRDESCSIZE, &desc_size);
    ioctl(fd, HIDIOCGRAWINFO, &info);
    close(fd);

    int vid = info.vendor & 0xFFFF;
    int pid = info.product & 0xFFFF;

    for (i=0;;i++)
    {
        struct device * dev = dev_list[i];
        if (dev == NULL)
            break;

        if ( (dev->vid <= 0 || dev->vid == vid)
            &&( dev->pid <=0 || dev->pid == pid)
            && (dev->desc_size <=0 || dev->desc_size == desc_size ) )
        {
            current_device = dev;
            strncpy(current_device->dev_path, dev_path, sizeof(current_device->dev_path));
            return true;
        }
    }

    return false;
}
