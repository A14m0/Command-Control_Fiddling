# Command-Control_Fiddling
A C2 server/client/management project written in C, C++, and Python 3.

We are working on making the server more modular in order to include varied agent functions, transports, and agent implementations. 

As it stands right now, the only transport is over an SSH connection. 


## To Compile
Ensure that you have a Linux machine with gcc, g++, PyQt5 (and related tool pyuic5), LibSSH, and Nikito installed. Then run 'make' or 'make debug' to build the server, C agent, and python UI files.

## Files
There are 3 main files created with the make.
### serverSrc/server.out
This is the main server file. Because it listens on the default SSH port 22, it requires sudo to run. You must specify a bind address that the server is to listen on. All other options can be seen with the '-h' option.
### clientSrc/client.out
This is an example client executable written in C. Its basically just there to provide an example for other clients. Note that it may not be registered with the server after initial clone of this repo, so make sure you add its credentials to the server database (see '-h' for server.out to see how).
### python/management.py
This is the QT-based GUI manager designed to interact with the server remotely. It's credentials may also not be registered with the server, so make sure you add its credentials to the server database. 

Through this interface, you can schedule tasks for each registered agent to complete on their next beacon connection. Whenever agents connect, telemetry data is updated and relayed to this interface.

## Disclaimer
We are not responsible for illegal or malicious activity done with this software. Please don't be stupid and have fun with this.