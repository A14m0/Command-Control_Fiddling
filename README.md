# Command-Control_Fiddling
A C2 server/client/management project written in C, C++, and Python 3.

We are working on making the server more modular in order to include varied agent functions, 
transports, and agent implementations.

As it stands right now, the only transport is over an SSH connection.

## Disclaimer
We are not responsible for illegal or malicious activity done with this software. 
Please don't be stupid and have fun with this.

## To Compile
Ensure that you have a Linux machine with `gcc`, `g++`, `PyQt5` (and related tool `pyuic5`), 
`LibSSH`, and `Nikito` installed. Then run `make` or `make debug` to build the server, 
C agent, and python UI files.

## Files
There are 3 main files and 1 directory created with the make.
### Executable: `out/server.out`
This is the main server file. Because it listens on the default SSH port 22, it requires 
sudo to run. Some CLI arguments and stuff is coming at some point, but we will see...

### Directory: `out/shared/`
This is the location for all of the shared object libraries used by the server.
Here is where you will place all of the modules and network backend interfaces
you may use.

### Executable: `out/client.out`
This is an example client executable written in C. Its basically just there to provide 
an example for other clients. Note that it may not be registered with the server after 
initial clone of this repo, so make sure you add its credentials to the server database 
(gonna add a way of adding this at some point, so in the meantime, add the following to 
the server's main function, `make`, run it, remove the line, `make` again: 
`AgentInformationHandler::register_agent("NAME_OF_AGENT", "PASSWORD")`

It will be added to the server's database of known agents and saved from here on out).

### Executable: `python/management.py`
This is the QT-based GUI manager designed to interact with the server remotely. 
It's credentials may also not be registered with the server, so make sure you add its 
credentials to the server database.

Through this interface, you can schedule tasks for each registered agent to complete 
on their next beacon connection. Whenever agents connect, telemetry data is updated 
and relayed to this interface.


## About the module/network transport interfaces
The server utilizes a modular design for ease and flexibility in agent creation
and usage. In order to create your own modules and backends, there are a few
things to know. Firstly is that all modules and network backends are compiled as
Linux shared-object libraries. Depending on which kind of thing you are
creating, you must define a few things. All valid modules contain a defined
global integer variable named `type`, a global integer variable named `id`, and a global
character pointer variable named `name`. The `type`
variable identifies to the server if the shared-object is a network backend or a
standalone module. Set this value to `69` if you are making a module, and to `99` if
you are making a network backend. The `id` variable allows the server to uniquely
identify modules, so be creative with this one :), and the `name` variable is the 
string name of the module for use in the manager interface. There are some additional
things you must define in order for the server to use your custom program.

### Modules
Modules are single-shot programs that are executed on the server and run in a
separate thread. In addition to the three previous global variables that are
required in both modules and transports, you must define a void function named
`entrypoint`. This will be the equivalent of the `main` function of a normal C/C++ 
program. Any other included functions can be defined and called from there.

### Network transport interfaces
Like modules, you need to define a few more things in order to create a valid
network transport interface. The first thing to do is make sure that you
implement all the functions required by the `transport_t` (defined in
`serverSrc/transport.h`).

#### Initialization Requirements
There are a hand-full of things that you may want to keep track of during the
backend's execution, and this is where transport initialization comes in. You
can define a custom `void\*` structure that will house all of the required variables 
for your transport to work. The server calls `get_dat_siz()`, which returns the size
of the structure required. It will allocate a spot in heap memory for that data 
and then call `init()`, passing the heap pointer to the method, so you can set up
variables and whatnot. When the thread is completed and the transport is no longer
needed, the server will call `end()`, pass the heap pointer and you can uninitialize 
special structs or whatever.

#### Accessor/Mutator Functions
There are a few functions that must be implemented in order for the server to
work. The first is `get_name()` which should return the string name of the
backend. There is also `get_id()`, which is rather self-explanatory. The next one,
`get_agent_name()`, returns the string name of the remote agent when connected.
This data is ideally stored in the void structure you defined. There is also 
`set_port()` which defines where the server wants your module to listen for 
connections. 

#### Generic protocol methods
There are a handfull of generic methods that you must define in order to have a 
functional network backend. Most of these are trivial, like `send_ok()` or `read()` 
or `write()`. Pretty simple stuff, but required.

#### Special protocol methods
There are some methods that you must define that are much more exciting. These 
include the `get_loot`, `upload_file`, `init_reverse_shell`, and `determine_handler`
functions. The `get_loot` function is a manager-specific command, which will
take all of the downloaded files a particular agent has provided and send it to 
the manager. `upload_file` is an agent function which just pushes a file to said 
agent. The `init_reverse_shell` is optional, but helpful when you want your agent
to provide a full TTY shell session over your transport. The last one is the
`determine_handler` function, which just lets the server know if the connection 
it got is either an agent (returns `AGENT_TYPE`, defined in `serverSrc/execs.h`) or 
is a manager (returns `MANAGER_TYPE`, defined in the same file).
