#include "shell.h"
#include "b64.h"

int shell_unix(ssh_channel chan){
    int rc = 0;
    printf("Starting shell init\n");
    char *args[] = {"/bin/bash", "-i", NULL};
    char input[150];
    int sockfd = ssh_get_fd(ssh_channel_get_session(chan));

    int fdm = posix_openpt(O_RDWR);
    if(fdm < 0){
        perror("Woops on posix_openpt()\n");
        exit(EXIT_FAILURE);
    }

    rc = grantpt(fdm);
    if(rc != 0){
        perror("Woops on grantpt()");
        exit(EXIT_FAILURE);
    }

    rc = unlockpt(fdm);
    if (rc != 0)
    {
        perror("woops on unlockpt()");
        exit(EXIT_FAILURE);
    }

    int fds = open(ptsname(fdm), O_RDWR);
    if (fork())
    {
        fd_set fd_in;

        // FATHER

        // Close the slave side of the PTY
        close(fds);

        while (1)
        {
            // Wait for data from standard input and master side of PTY
            FD_ZERO(&fd_in);
            FD_SET(sockfd, &fd_in);
            FD_SET(fdm, &fd_in);

            rc = select(fdm + 1, &fd_in, NULL, NULL, NULL);
            int sz = 0;
            char inp_buff[BUFSIZ];
            memset(inp_buff, 0, sizeof(inp_buff));
            switch(rc)
            {
                case -1 : fprintf(stderr, "Error %d on select()\n", errno);
                    exit(1);

                default :
                {
                    // If data on standard input
                    if (FD_ISSET(sockfd, &fd_in))
                    {
                        
                        rc = ssh_channel_read(chan, input, sizeof(input), 0);
                        
                        if (rc > 0)
                        {
                            // Send data on the master side of PTY
                            //write(sock, input, rc);
                            sz = b64_decoded_size(input);
                            rc = b64_decode(input, inp_buff, sz);
                            if(rc != 0){
                                write(fdm, inp_buff, sz);
                            }
                        
                            
                        }
                        else
                        {
                            if (rc < 0)
                            {
                                fprintf(stderr, "Error %d on read standard input\n", errno);
                                exit(1);
                            }
                        }
                    }

                    // If data on master side of PTY
                    if (FD_ISSET(fdm, &fd_in))
                    {
                        rc = read(fdm, input, sizeof(input));
                        if (rc > 0)
                        {
                            sz = b64_encoded_size(rc);
                            rc = b64_encode(input, inp_buff);
                            if(rc != 0){
                                ssh_channel_write(chan, inp_buff, sz);
                            }
                            // Send data on standard output
                            
                        }
                        else
                        {
                            if (rc < 0)
                            {
                                fprintf(stderr, "Error %d on read master PTY\n", errno);
                                exit(1);
                            }
                        }
                    }
                }
            } // End switch
        } // End while
    }
    else
    {
        struct termios slave_orig_term_settings; // Saved terminal settings
        struct termios new_term_settings; // Current terminal settings

        // CHILD

        // Close the master side of the PTY
        close(fdm);

        // Save the defaults parameters of the slave side of the PTY
        rc = tcgetattr(fds, &slave_orig_term_settings);

        // Set RAW mode on slave side of PTY
        new_term_settings = slave_orig_term_settings;
        new_term_settings.c_lflag &= ~ICANON & ECHO;
        //cfmakeraw (&new_term_settings);
        tcsetattr (fds, TCSANOW, &new_term_settings);

        // The slave side of the PTY becomes the standard input and outputs of the child process
        close(0); // Close standard input (current terminal)
        close(1); // Close standard output (current terminal)
        close(2); // Close standard error (current terminal)

        dup(fds); // PTY becomes standard input (0)
        dup(fds); // PTY becomes standard output (1)
        dup(fds); // PTY becomes standard error (2)

        // Now the original file descriptor is useless
        close(fds);

        // Make the current process a new session leader
        setsid();

        // As the child is a session leader, set the controlling terminal to be the slave side of the PTY
        // (Mandatory for programs like the shell to make them manage correctly their outputs)
        ioctl(0, TIOCSCTTY, 1);

        // Execution of the program
        {
            rc = execve("/bin/bash", args, NULL);
        }

        // if Error...
        return 1;
    }
        
    return 0;
}

