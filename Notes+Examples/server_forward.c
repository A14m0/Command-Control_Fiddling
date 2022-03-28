#include <libssh/libssh.h>
#include <libssh/server.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int server_direct_forwarding(ssh_session session)
{
    ssh_channel forwarding_channel;
    int rc;
    char *http_get = "GET / HTTP/1.1\nHost: www.google.com\n\n";
    int nbytes, nwritten;
    forwarding_channel = ssh_channel_new(session);
    if (forwarding_channel == NULL) {
        printf("Died creating forward channel\n");
        return rc;
    }
    rc = ssh_channel_open_forward(forwarding_channel,"www.google.com", 80,"localhost", 5555);
    if (rc != SSH_OK)
    {
        printf("Died after opening forward\n");
        ssh_channel_free(forwarding_channel);
        return rc;
    }
    nbytes = strlen(http_get);
    nwritten = ssh_channel_write(forwarding_channel,http_get,nbytes);
    if (nbytes != nwritten)
    {
        ssh_channel_free(forwarding_channel);
        printf("Channel write free\n");
        return SSH_ERROR;
    }
    printf("Done\n");
    ssh_channel_free(forwarding_channel);
    return SSH_OK;
}

int main(int argc, char **argv){
    ssh_session session;
    ssh_bind sshbind;
    int r = 0;
    ssh_message message;

    sshbind = ssh_bind_new();
    session = ssh_new();

    ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_DSAKEY, "/etc/ssh/ssh_host_dsa_key");
    ssh_bind_options_set(sshbind, SSH_BIND_OPTIONS_RSAKEY, "/etc/ssh/ssh_host_rsa_key");

/*
    if(ssh_bind_listen(sshbind)<0){
        printf("Error\n");
        exit(1);
    }
    r=ssh_bind_accept(sshbind, session);
    if(r==SSH_ERROR){
        printf("ERROR\n");
        exit(1);
    }
    if(ssh_handle_key_exchange(session)){
        printf("key error");
        exit(1);
    }
*/
    r = server_direct_forwarding(session);
    printf("%d\n", r);
    return 0;
}
