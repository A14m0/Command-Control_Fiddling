#!/usr/bin/env python3
import sys
import random
import signal
import traceback
import time
import paletteColors
import session
import design
import dialogues
import misc
import os

from paramiko import ssh_exception
from PyQt5 import QtGui, QtCore, QtWidgets
# https://www.qt.io/qt-for-python
# https://doc.qt.io/qtforpython/?hsCtaTracking=fab9e910-0b90-4caa-b6d0-e202692b6f13%7Cb9e0be8d-1d85-4644-aaa2-41de4e6d37e3
# https://matplotlib.org/gallery/user_interfaces/embedding_in_qt_sgskip.html#sphx-glr-gallery-user-interfaces-embedding-in-qt-sgskip-py


class Manager(QtWidgets.QMainWindow, design.Ui_MainWindow):
    def __init__(self, parent=None):
        # Initialize the GUI
        super(Manager, self).__init__(parent)
        self.setupUi(self)

        # Set color palette
        self.color = paletteColors.Colors()
        self.setPalette(self.color.pltActive)

        # Set button tooltips
        self.PushFile.setToolTip("Push file to selected agent")
        self.PullFile.setToolTip("Download file from selected agent")
        self.ExecComm.setToolTip("Execute command on selected agent")
        self.RevSh.setToolTip("Get reverse shell on selected agent")
        self.GetLoot.setToolTip("Download all loot from selected agent")
        self.PushModule.setToolTip("Push and task a module to selected agent")
        
        # Connect buttons to functions
        self.PushFile.clicked.connect(self.push_file)
        self.PullFile.clicked.connect(self.pull_file)
        self.ExecComm.clicked.connect(self.exec_comm)
        self.RevSh.clicked.connect(self.rev_sh)
        self.GetLoot.clicked.connect(self.get_loot)
        self.PushModule.clicked.connect(self.push_module)
        self.actionCompile_Client.triggered.connect(self.get_client)
        self.actionRegister_Client_Creds.triggered.connect(self.reg_client)
        self.actionOpen_Terminal.triggered.connect(self.open_term)
        self.actionChange_C2_Server.triggered.connect(self.connect_to_c2)
        self.AgentList.currentItemChanged.connect(self.update_info_labels)

        self.winList = []
        self.winListIndex = 0

        # Initialize the session and attempt to start a connection
        self.session = session.Session("127.0.0.1")
        self._show_connect_dialogue()

    def closeEvent(self, evnt):
        print("[ ] Closing window...")
        self.session.clean_exit()
        super(Manager, self).closeEvent(evnt)

    def has_selection(self):
        """ Determines if an agent is selected from AgentList"""
        if not self.AgentList.currentItem():
            reply = QtWidgets.QMessageBox.information(self, "No Target Selected", "Select a target from the list to use this option")
            return False
        return True

    def _show_connect_dialogue(self):
        """Shows the server connection dialogue and initializes the agent list"""
        self.session = session.Session('127.0.0.1')
        self.AgentList.clear()
        dialog = dialogues.ConnectDialogue(self.session)
        dialog.exec()
        for entry in self.session.agents:
            item = QtWidgets.QListWidgetItem()
            item.setText(entry.id)
            item.setData(QtCore.Qt.UserRole, entry)
            self.AgentList.addItem(item)

    def update_info_labels(self):
        """Updates the info labels when a new agent is clicked"""
        try:
            self.ConnectionTime.setText("Connection Time: " + self.AgentList.currentItem().data(QtCore.Qt.UserRole).connection_time)
            self.Hostname.setText("Hostname: " + self.AgentList.currentItem().data(QtCore.Qt.UserRole).hostname)
            self.Interfaces.setText("Interfaces: " + self.AgentList.currentItem().data(QtCore.Qt.UserRole).interfaces)
            self.ProcOwner.setText("Process Owner: " + self.AgentList.currentItem().data(QtCore.Qt.UserRole).process_owner)
        except AttributeError:
            # Resetting the dialogue stuff
            self.ConnectionTime.setText("Connection Time: ")
            self.Hostname.setText("Hostname: ")
            self.Interfaces.setText("Interfaces: ")
            self.ProcOwner.setText("Process Owner: ")
        
    def connect_to_c2(self):
        """Wrapper for _show_connect_dialogue which checks if the user really wants to"""
        reply = QtWidgets.QMessageBox.question(self, 'WARNING', 
                 'Switching C2 servers will reset all info in the console. Any active processes will be dropped. Continue?', 
                 QtWidgets.QMessageBox.Yes, QtWidgets.QMessageBox.No)
        if reply == QtWidgets.QMessageBox.Yes:
            self._show_connect_dialogue() 

    def open_term(self):
        """Opens a local terminal"""
        if sys.platform == "linux":
            os.system("urxvt")
        else:
            os.system("cmd.exe")
        
    def push_file(self):
        """Dialogue to push a file to server"""
        if not self.has_selection():
            return
        print("[ ] Waiting for file selection...")
        file = misc.open_file()
        self.session.upload_file(self.AgentList.currentItem().text(), file)

    def pull_file(self):
        """Dialogue to pull a file from an agent"""
        if not self.has_selection():
            return
        diag = dialogues.AgentDownloadDialogue(self.session, self.AgentList.currentItem().text())
        diag.exec()

    def exec_comm(self):
        """Dialogue to task command for agent"""
        if not self.has_selection():
            return
        diag = dialogues.AgentCommandDialogue(self.session, self.AgentList.currentItem().text())
        diag.show()
        return diag

    def rev_sh(self):
        """Dialogue to request reverse shell from agent"""
        if not self.has_selection():
            return
        
        print("Getting reverse shell")
    
    def get_loot(self):
        """Dialogue to get all loot from an agent"""
        if not self.has_selection():
            return
        
        print("Getting loot")
        self.session.download_loot(self.AgentList.currentItem().text())

    def push_module(self):
        """Dialogue to push a module to agent"""
        if not self.has_selection():
            return
        file = misc.open_file()
        self.session.push_module(self.AgentList.currentItem().text(), file)

    def get_client(self):
        """Dialogue to get a compiled agent from server"""
        diag = dialogues.AgentCompileDialogue(self.session)
        diag.show()
        return diag

    def reg_client(self):
        """Dialogue to register credentials with the server"""
        diag = dialogues.AgentRegisterDialogue(self.session)
        diag.show()
        return diag
        

if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    connect = Manager()
    connect.show()
    
    sys.exit(app.exec_())
