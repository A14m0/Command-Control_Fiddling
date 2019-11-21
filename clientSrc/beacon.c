
#include "beacon.h"

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
    char *ret = NULL;
    ret = malloc(strlen(if_name) + 18);
    memset(ret, 0, sizeof(ret));
    

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;

    /* I want IP address attached to "eth0" */
    strncpy(ifr.ifr_name, if_name, IFNAMSIZ-1);

    ioctl(fd, SIOCGIFADDR, &ifr);

    close(fd);

    /* display result */
    sprintf(ret, "%s:%s", if_name, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
    return ret;
}

char *combine(char *str1, const char delim, char *str2){
    if(str1 == NULL){ 
        return str2;
    }
    if(str2 == NULL) printf("Tada you got it\n");
    int size1 = strlen(str1);
    int size2 = strlen(str2);
    char *ret = NULL;
    ret = malloc(size1+size2+2);
    memset(ret, 0, sizeof(ret));

    sprintf(ret, "%s%c%s", str1, delim, str2);
    free(str1);
    free(str2);
    return ret;
}

char *show_interfaces(){
    struct ifaddrs *addrs, *tmp;
    char *buff = NULL;
    char *ret;
    getifaddrs(&addrs);
    tmp = addrs;
    while (tmp)
    {
        if(tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_PACKET){
            ret = get_address(tmp->ifa_name);
            printf("Generated information: %s\n", ret);
            buff = combine(buff,',',ret);
            printf("Current buffer: %s\n", buff);
            /*ret = malloc(sizeof(temp) + sizeof buff);
            memset(ret, 0, sizeof(ret));
            sprintf(ret, "%s,%s", temp, buff);
            temp = ret;*/
            
        }
        tmp = tmp->ifa_next;
    }

    freeifaddrs(addrs);

    return buff;
    
}

char *show_localtime(){
    time_t t = time(NULL);
    char *ret = NULL;
    ret = malloc(20);
    memset(ret, 0, sizeof(ret));
    struct tm tm = *localtime(&t);
    sprintf(ret, "%d-%d-%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1,tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    return ret;
}

char *show_hostname(){
    char *buff = malloc(128);
    int rc = 0;
    memset(buff, 0, 128);
    rc = gethostname(buff, 128);//sizeof(buff));
    printf("RC: %d\n", rc);
    if(rc == -1) perror("");
    printf("Hostname: %s\n", buff);
    return buff;
}
char *show_procowner(){
    char *buff = NULL;
    buff = malloc(128);
    memset(buff, 0, sizeof(buff));
    struct passwd *p;
    uid_t uid;
    uid = getuid();
    p = getpwuid(uid);
    printf("Process owner: %s\n", p->pw_name);
    sprintf(buff, "%s", p->pw_name);
    return buff;
}

char *get_beacon(){
    char *ret;
    char *buff;
    char *name = malloc(16);
    memset(name, 0, sizeof(name));
    sprintf(name, "%s","TEST_BEACON");
    ret = show_interfaces();
    buff = combine(name,'\n', ret);
    ret = show_localtime();
    buff = combine(buff, '\n', ret);
    ret = show_hostname();
    buff = combine(buff, '\n', ret);
    ret = show_procowner();
    buff = combine(buff, '\n', ret);
    return buff;
}