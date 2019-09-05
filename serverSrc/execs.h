#pragma once

#define REQ_NONE 0
#define REQ_EXEC 1
#define REQ_TASKING 2
#define REQ_TTY 3

#define AGENT_TYPE 100
#define MANAG_TYPE 101

#define END_CONN 0

// Agent commands
#define AGENT_DOWN_FILE 10
#define AGENT_REV_SHELL 11
#define AGENT_UP_FILE 12
#define AGENT_EXEC_SC 13
#define AGENT_EXIT 0

// Management commands
#define MANAG_GET_INFO 20
#define MANAG_GET_LOOT 21
#define MANAG_UP_FILE 22
