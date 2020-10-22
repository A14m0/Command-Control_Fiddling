/* Defines constant values used throughout the program */
#pragma once

// defines module types
#define TRANSPORT 0
#define STANDALONE 1

// defines error codes that can be returned by transport functions
#define API_OK 0
#define API_ERR_GENERIC 1
#define API_ERR_WRITE 2
#define API_ERR_READ 3
#define API_ERR_LISTEN 4
#define API_ERR_BIND 5
#define API_ERR_ACCEPT 6
#define API_ERR_AUTH 7
#define API_ERR_CLIENT 8
#define API_ERR_LOCAL 9


// defines opcodes for tasks
