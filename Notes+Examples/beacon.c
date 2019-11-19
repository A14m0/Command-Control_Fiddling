#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> /* for strncpy */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

#include <time.h>
#include <pwd.h>

void show_address(char *if_name){
    int fd;
    struct ifreq ifr;// = malloc(sizeof(struct ifreq));

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;

    /* I want IP address attached to "eth0" */
    strncpy(ifr.ifr_name, if_name, IFNAMSIZ-1);

    ioctl(fd, SIOCGIFADDR, &ifr);

    close(fd);

    /* display result */
    printf("%s\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

}

char *get_address(char *if_name){
    int fd;
    struct ifreq ifr;// = malloc(sizeof(struct ifreq));
    char *ret = malloc(strlen(if_name) + 19);
    memset(ret, 0, sizeof(ret));
    

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;

    /* I want IP address attached to "eth0" */
    strncpy(ifr.ifr_name, if_name, IFNAMSIZ-1);

    ioctl(fd, SIOCGIFADDR, &ifr);

    close(fd);

    /* display result */
    sprintf(ret, "%s: %s", if_name, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
    return ret;
}

void show_interfaces(){
    struct ifaddrs *addrs, *tmp;
    char *buff;
    char *ret = malloc(128);
    char *temp = ret;
    memset(ret, 0, sizeof(ret));
    getifaddrs(&addrs);
    tmp = addrs;
    while (tmp)
    {
        if(tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_PACKET){
            buff = get_address(tmp->ifa_name);
            printf("Generated information: %s\n", buff);
            /*ret = malloc(sizeof(temp) + sizeof buff);
            memset(ret, 0, sizeof(ret));
            sprintf(ret, "%s,%s", temp, buff);
            temp = ret;*/
            
        }
        tmp = tmp->ifa_next;
    }

    freeifaddrs(addrs);
    
}

void show_localtime(){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    printf("Current Time: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1,tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

void show_hostname(){
    char buff[BUFSIZ];
    memset(buff, 0, sizeof(buff));
    gethostname(buff, sizeof(buff));
    printf("Hostname: %s\n", buff);
}
void show_procowner(){
    struct passwd *p;
    uid_t uid;
    uid = getuid();
    p = getpwuid(uid);
    printf("Process owner: %s\n", p->pw_name);
    
}

char *get_beacon(){
    char *ret;
}

int main()
{
    show_hostname();
    show_localtime();
    show_procowner();
    show_address("wlp3s0");
    show_interfaces();
    return 0;
}