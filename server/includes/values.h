/* Defines constant values used throughout the program */
#pragma once


// defines module types
#define TRANSPORT 1
#define STANDALONE 2

// defines connection types
#define AGENT_TYPE 100
#define MANAG_TYPE 101


// defines error codes that can be returned by transport functions
#define API_OK 0          // No problems encountered
#define API_ERR_GENERIC 1 // Generic problem encountered
#define API_ERR_WRITE 2   // Failed to write something
#define API_ERR_READ 3    // Failed to read something
#define API_ERR_LISTEN 4  // Failed to listen on a network port
#define API_ERR_BIND 5    // Failed to bind to a network port
#define API_ERR_ACCEPT 6  // Failed to accept an inbound connection
#define API_ERR_AUTH 7    // Failed to authenticate client
#define API_ERR_CLIENT 8  // Encountered client-side error
#define API_ERR_LOCAL 9   // Encountered local-side error


// defines task type vales
#define TASK_NULL 0             // NOP tasking
#define TASK_AUTH 1             // Request to authenticate credentials
#define TASK_NEW_NETINST 2      // Request to create new NetInst
#define TASK_PUSH_BEACON 3      // Request to write beacon data for an agent
#define TASK_SAVE_FILE 4        // Request to save a file from an agent
#define TASK_TERM_NETINST 5     // Request to terminate the sending NetInst


// defines manager operation types
#define MANAGER_RETRIEVE_AGENT 0xff-1           // Manager wants all beacons
#define MANAGER_RETRIEVE_LOOT 0xff-2            // Manager wants the loot of an agent
#define MANAGER_UPLOAD_FILE 0xff-3              // Manager wants to upload a file to an agent
#define MANAGER_DOWNLOAD_FILE 0xff-4            // Manager wants to download a file from an agent
#define MANAGER_PUSH_MODULE 0xff-5              // Manager wants to push shellcode to the agent
#define MANAGER_RUN_COMMAND 0xff-6              // Manager wants to send a command to agent
#define MANAGER_REQUEST_REVERSESHELL 0xff-7     // Manager wants a reverse shell
#define MANAGER_REGISTER_AGENT 0xff-8           // Manager wants to register credentials
#define MANAGER_REVIEW_TRANSPORTS 0xff-0x9      // Manager wants a list of transports
#define MANAGER_START_TRANSPORT 0xff-0xa        // Manager wants to start a transport
#define MANAGER_EXIT 0xff-0xb                   // Manager is terminating