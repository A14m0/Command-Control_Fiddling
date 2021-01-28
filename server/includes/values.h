/* Defines constant values used throughout the program */
#pragma once

// defines error codes that can be returned by transport functions
#define API_OK 0          // No problems encountered
#define API_ERR_GENERIC 1 // Generic problem encountered
#define API_ERR_WRITE 2   // Failed to write somethign
#define API_ERR_READ 3    // Failed to read something
#define API_ERR_LISTEN 4  // Failed to listen on a network port
#define API_ERR_BIND 5    // Failed to bind to a network port
#define API_ERR_ACCEPT 6  // Failed to accept an inbound connection
#define API_ERR_AUTH 7    // Failed to authenticate client
#define API_ERR_CLIENT 8  // Encountered client-side error
#define API_ERR_LOCAL 9   // Encountered local-side error



// defines task type vales
#define TASK_NULL 0x0           // NOP tasking
#define TASK_AUTH 0x1           // Request to authenticate credentials
#define TASK_NEW_NETINST 0x2    // Request to create new NetInst
#define TASK_WRITE_BEACON 0x3   // Request to write beacon data