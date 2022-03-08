#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
 
// #include "query_ioctl.h"
#include "event_ioctl.h"
 
void get_vars(int fd)
{
    // query_arg_t q;
 
    gpio_event_t e;

    //if (ioctl(fd, EVENT_GET_VARIABLES, &q) == -1)
    if (ioctl(fd, EVENT_GET_VARIABLES, &e) == -1)
    {
        perror("query_apps ioctl get");
    }
    else
    {
	/*
        printf("Status : %d\n", q.status);
        printf("Dignity: %d\n", q.dignity);
        printf("Ego    : %d\n", q.ego);
	*/

        printf("Status : %d\n", e.gpio_num);
        printf("Dignity: %d\n", e.edge);
        printf("Ego    : %d\n", e.bouncetime);
    }
}
void clr_vars(int fd)
{
    if (ioctl(fd, EVENT_CLR_VARIABLES) == -1)
    {
        perror("query_apps ioctl clr");
    }
}
void set_vars(int fd)
{
    int v;
   // query_arg_t q;
    gpio_event_t e;

    printf("Enter Status: ");
    scanf("%d", &v);
    getchar();
    // q.status = v;
    e.gpio_num = v;

    printf("Enter Dignity: ");
    scanf("%d", &v);
    getchar();
    //q.dignity = v;
    e.edge = v;

    printf("Enter Ego: ");
    scanf("%d", &v);
    getchar();
    // q.ego = v;
    e.bouncetime = v;
 
    //if (ioctl(fd, EVENT_SET_VARIABLES, &q) == -1)
    if (ioctl(fd, EVENT_SET_VARIABLES, &e) == -1)
    {
        perror("query_apps ioctl set");
    }
}
 
int main(int argc, char *argv[])
{
    //char *file_name = "/dev/query";
    char *file_name = "/dev/ls2k_gpio_device";
    int fd;
    enum
    {
        e_get,
        e_clr,
        e_set
    } option;
 
    if (argc == 1)
    {
        option = e_get;
    }
    else if (argc == 2)
    {
        if (strcmp(argv[1], "-g") == 0)
        {
            option = e_get;
        }
        else if (strcmp(argv[1], "-c") == 0)
        {
            option = e_clr;
        }
        else if (strcmp(argv[1], "-s") == 0)
        {
            option = e_set;
        }
        else
        {
            fprintf(stderr, "Usage: %s [-g | -c | -s]\n", argv[0]);
            return 1;
        }
    }
    else
    {
        fprintf(stderr, "Usage: %s [-g | -c | -s]\n", argv[0]);
        return 1;
    }
    fd = open(file_name, O_RDWR);
    if (fd == -1)
    {
        perror("query_apps open");
        return 2;
    }
 
    switch (option)
    {
        case e_get:
            get_vars(fd);
            break;
        case e_clr:
            clr_vars(fd);
            break;
        case e_set:
            set_vars(fd);
            break;
        default:
            break;
    }
 
    close (fd);
 
    return 0;
}
