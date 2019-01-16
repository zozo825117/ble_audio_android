/*
 * Copyright (c) 2013 DEQING DUSUN ELECTRON CO.;LTD. All rights reserved.
 */

#include <linux/types.h>
#include <linux/input.h>
#include <linux/hidraw.h>
#include <linux/netlink.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>

#include "config.h"
#include "common.h"

int handle_new_hidraw_dev(char * dev_path)
{
    int fd = -1;
    printf("Handling new hidraw dev: %s\n", dev_path);
    if ( !select_device(dev_path) )
    {
        printf("The new hidraw dev is not we want\n");
        return 0;
    }

    fd = open(dev_path, O_RDONLY);
    if (fd < 0 )
        return -1;

    if (flock(fd, LOCK_EX | LOCK_NB))
    {
        printf("hidraw dev is locked\n");
        close(fd);
        fd = -1;
        return -1;
    }

    close(fd);
    if ( !fork() )
    {
        audio_main();
        printf("audio child process exited: %d\n", getpid());
        exit(0);
    }

    return 1;

}

void resend_uevent_for_exist_hidraw_dev()
{
    struct dirent  * de;
    int len = 0;
    char  path[512];
    char * class_path = "/sys/class/hidraw" ;
    DIR * dp = opendir(class_path);

    if (dp == NULL)
    {
        printf("Failed to open dir : %s\n", class_path);
        return ;
    }

    while ( ( de = readdir(dp)) != NULL)
    {
        if (!strncmp(de->d_name, "hidraw", 6) )
        {
            sprintf(path, "%s/%s/uevent", class_path, de->d_name);
            printf("resend uevent for %s\n",  path) ;
            int fd = open(path, O_WRONLY);
            if (fd > 0 )
            {
                len = write(fd, "add", 4);
                close(fd);
            } else
            {
                printf("Failed to write add command\n");
            }
        }
    }
}

void deal_with_exist_hidraw_dev()
{
    if (fork() )
        return;
    sleep(1);
    resend_uevent_for_exist_hidraw_dev();
    exit(0);
}


int main_loop()
{
    struct sockaddr_nl nls;
    struct pollfd pfd;
    char buf[512];
    char dev_path[512];
    // Open hotplug event netlink socket
    memset(&nls,0,sizeof(struct sockaddr_nl));
    nls.nl_family = AF_NETLINK;
    nls.nl_pid = getpid();
    nls.nl_groups = -1;

    pfd.events = POLLIN;
    pfd.fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);

    if (pfd.fd < 0 )
    {
        printf("Failed to open netlink socket\n");
        return -1;
    }

    // Listen to netlink socket
    if (bind(pfd.fd, (void *)&nls, sizeof(struct sockaddr_nl)))
    {
        printf("Failed to bind socket\n");
        return -1;
    }

    deal_with_exist_hidraw_dev();

    while (1)
    {
        int res = poll(&pfd, 1, -1) ;
        if (res == -1)
        {
            if (errno == EAGAIN || errno == EINTR)
                continue;
            break;
        }

        int i = 0 ;
        int len = recv(pfd.fd, buf, sizeof(buf), MSG_DONTWAIT);
        if (len == -1 )
        {
            if (errno == EAGAIN || errno == EINTR)
                continue;
            printf("Error when recv netlink package\n");
            return -1;
        }

        i = 0 ;
        char * token = buf;
        char * action_token = NULL;
        char * devname_token = NULL;
        while ( i<len )
        {
            token = buf + i;
            i += strlen(token) + 1;
            if (!strncmp(token, "ACTION=add", 10) )
            {
                action_token = token;
            }

            if (!strncmp(token, "DEVNAME=/dev/hidraw", 19) || !strncmp(token, "DEVNAME=hidraw", 14) )
            {
                devname_token = token;
            }

            if (action_token != NULL && devname_token != NULL)
            {
                if (!strncmp(devname_token, "/dev/", 5 ) )
                    strcpy(dev_path, devname_token + 8 );
                else
                    sprintf(dev_path, "/dev/%s", devname_token + 8);
                //char * dev_path = devname_token + 8;
                printf("Found new hidraw device\n", dev_path);
                handle_new_hidraw_dev(dev_path);
                break;
            }
        }
    }

    close(pfd.fd);
}

void cleanup_child_process(int signal_number)
{
    int status;
    wait(&status);
    printf("Parent process notice child process termination\n");
}

int g_screen_width = 0;
int g_screen_height = 0;
int g_wait_before_enable_motion = 5;

/*
void connect_ble_socket()
{
    int localsocket, len;
    struct sockaddr_un remote;

    if ((localsocket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        printf("Failed to create LockSocket\n");
    }

    char *name="com.hzdusun.ble_audio.notify_socket";

    remote.sun_path[0] = '\0';
    strcpy(remote.sun_path+1, name);
    remote.sun_family = AF_UNIX;
    int nameLen = strlen(name);
    len = 1 + nameLen + offsetof(struct sockaddr_un, sun_path);

    while (1)
    {
        printf("Try to connect\n");
        if (connect(localsocket, (struct sockaddr *)&remote, len) == -1)
        {
            printf("Connected failed, sleep 1 sec, try again\n");
            sleep(1);
            continue;
        }
        break;
    }
    printf("Connected Ok\n");
    unsigned char  data = 0;
    for (data =0; data < 255; data++)
    {
       int ret =   send(localsocket, &data, 1, 0 );
       printf("ret = %d\n", ret);
    }
}*/
int main(int argc, char * argv[])
{
    struct sigaction sigchld_action;
    memset(&sigchld_action, 0, sizeof(struct sigaction) ) ;
    sigchld_action.sa_handler = &cleanup_child_process;
    sigaction(SIGCHLD, &sigchld_action, NULL);
    int c;

    while ( (c = getopt(argc, argv, "W:w:")) != -1  )
    {
        switch(c)
        {
            case 'W':
            case 'w':
                g_wait_before_enable_motion = atoi(optarg);
                break;
        }
    }

    printf("Wait before enanble motion = %d  \n", g_wait_before_enable_motion);
    argc -= optind;
    argv += optind;
    if (argc == 2 )
    {
        g_screen_width = atoi(argv[0]);
        g_screen_height = atoi(argv[1]);
        printf("User input screen width/height %d x %d\n",  g_screen_width, g_screen_height);
    }

    while(1)
    {
        //printf("BEGIN TO LOOP\n");
        main_loop();
        sleep(1);
    }
}


