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
#define AGENT_EXEC_MODULE 14
#define AGENT_EXIT 0

// Management commands
#define MANAG_GET_INFO 20
#define MANAG_GET_LOOT 21
#define MANAG_UP_FILE 22
#define MANAG_DOWN_FILE 23
#define MANAG_TASK_MODULE 24
#define MANAG_TASK_SC 25
#define MANAG_REQ_RVSH 26
#define MANAG_GET_AGENT 27
#define MANAG_REG_AGENT 28
#define MANAG_EXIT 0
