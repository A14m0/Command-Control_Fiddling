#!/usr/bin/env python
#-*- coding:utf-8 -*-

import sys
import traceback
import paletteColors
import design
from paramiko import ssh_exception
from PyQt5 import QtCore, QtGui, QtWidgets

class TerminalWidget(QtWidgets.QWidget, design.Ui_Terminal):

    def __init__(self, parent=None):
        QtWidgets.QWidget.__init__(self, parent)
        self.setupUi(self)
        self.process = QtCore.QProcess(self)
        self.terminal = QtWidgets.QWidget(self)
        layout = QtWidgets.QVBoxLayout(self)
        layout.addWidget(self.terminal)
        self.process.start('urxvt',['-embed', str(self.terminal.winId())])
        # Works also with urxvt:
        #self.process.start(
                #'urxvt',['-embed', str(self.terminal.winId())])

class ConnectDialogue(QtWidgets.QDialog, design.Ui_ManagerServerConnect):
    def __init__(self, session, parent=None):
        QtWidgets.QDialog.__init__(self, parent)
        self.setupUi(self)
        self.color = paletteColors.Colors()
        self.session = session
        self.setPalette(self.color.pltActive)

        self.Connect.clicked.connect(self.getConnect)
        self.setWindowTitle("Management Console")

    def getConnect(self):
        value = self.IP.text()
        print("[i] Attempting to connect to ip: %s"% value)
        self.session.update_addr(value)
        if self.Port.text() != '':
            self.session.update_port(int(self.Port.text()))
        try:
            self.session.init_connection()
 
        #except ValueError:
         #   QtWidgets.QMessageBox.information(self, "Illegal Input", "The value you passed was not a valid IP address")
          #  return
        except ssh_exception.NoValidConnectionsError:
            QtWidgets.QMessageBox.information(self, "Connection Refused", "The server refused the SSH connection")
            return
        except ssh_exception.SSHException:
            QtWidgets.QMessageBox.information(self, "Connection Failed", "Something happened. Check the console for more infromation")
            print(traceback.format_exc())
            return


        print("[+] Successfully connected!")
        self.close()    

class AgentCompileDialogue(QtWidgets.QDialog, design.Ui_AgentCompile):
    def __init__(self, session, parent=None):
        QtWidgets.QDialog.__init__(self, parent)
        self.setupUi(self)
        self.color = paletteColors.Colors()
        self.session = session
        self.setPalette(self.color.pltActive)

        self.buttonBox.button(QtWidgets.QDialogButtonBox.Ok).clicked.connect(self.success)
        #self.buttonBox.button(QtGui.QDialogButtonBox.Cancel).clicked.connect(self.cancel)
        

    def success(self):
        if self.ServerPort.text() == '':
            self.session.compile_agent(self.ServerIP.text(), 22)
        else:
            self.session.compile_agent(self.ServerIP.text(), int(self.ServerPort.text()))

class AgentDownloadDialogue(QtWidgets.QDialog, design.Ui_AgentDownload):
    def __init__(self, session, agent_id, parent=None):
        QtWidgets.QDialog.__init__(self, parent)
        self.setupUi(self)
        self.color = paletteColors.Colors()
        self.session = session
        self.agent_id = agent_id
        self.setPalette(self.color.pltActive)

        self.buttonBox.button(QtWidgets.QDialogButtonBox.Ok).clicked.connect(self.do_download)
        
    def do_download(self):
        self.session.do_download(self.agent_id, self.Path.text())

class AgentRegisterDialogue(QtWidgets.QDialog, design.Ui_AgentRegister):
    def __init__(self, session, parent=None):
        QtWidgets.QDialog.__init__(self, parent)
        self.setupUi(self)
        self.color = paletteColors.Colors()
        self.session = session
        self.setPalette(self.color.pltActive)

        self.buttonBox.button(QtWidgets.QDialogButtonBox.Ok).clicked.connect(self.do_register)

    def do_register(self):
        if self.Username.text() == '' or self.Password.text() == '':
            print("[-] Could not register agent credentials: Missing parameters")
        else:
            self.session.register_agent(self.Username.text(), self.Password.text())

class AgentCommandDialogue(QtWidgets.QDialog, design.Ui_CommandDialogue):
    def __init__(self, session, agent_id, parent=None):
        QtWidgets.QDialog.__init__(self, parent)
        self.setupUi(self)
        self.color = paletteColors.Colors()
        self.session = session
        self.agent_id = agent_id
        self.setPalette(self.color.pltActive)

        self.buttonBox.button(QtWidgets.QDialogButtonBox.Ok).clicked.connect(self.send_comm)

    def send_comm(self):
        if self.Input.text() =="":
            print("Found blank line")
        else:
            self.session.send_command(self.agent_id, self.Input.text())