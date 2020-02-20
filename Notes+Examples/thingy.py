import paramiko

ssh = paramiko.SSHClient()
ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
ssh.connect(hostname="localhost", username='aris', password='lala')
sshin, sshout, ssherr = ssh.exec_command("test")
