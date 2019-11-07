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

from paramiko import ssh_exception
from PyQt4 import QtGui, QtCore
# https://www.qt.io/qt-for-python
# https://doc.qt.io/qtforpython/?hsCtaTracking=fab9e910-0b90-4caa-b6d0-e202692b6f13%7Cb9e0be8d-1d85-4644-aaa2-41de4e6d37e3
# https://matplotlib.org/gallery/user_interfaces/embedding_in_qt_sgskip.html#sphx-glr-gallery-user-interfaces-embedding-in-qt-sgskip-py


class TestMgr(QtGui.QMainWindow, design.Ui_MainWindow):
    def __init__(self, parent=None):
        super(TestMgr, self).__init__(parent)
        self.setupUi(self)
        self.color = paletteColors.Colors()
        self.setPalette(self.color.pltActive)

        self.PushFile.setToolTip("Push file to selected agent")
        self.PullFile.setToolTip("Download file from selected agent")
        self.ExecComm.setToolTip("Execute command on selected agent")
        self.RevSh.setToolTip("Get reverse shell on selected agent")
        self.GetLoot.setToolTip("Download all loot from selected agent")
        self.PushModule.setToolTip("Push and task a module to selected agent")
        
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

        self.session = session.Session("127.0.0.1")
        self._show_connect_dialogue()

    def has_selection(self):
        if not self.AgentList.currentItem():
            reply = QtGui.QMessageBox.information(self, "No Target Selected", "Select a target from the list to use this option")
            return False
        return True

    def _show_connect_dialogue(self):
        self.session = session.Session('127.0.0.1')
        self.AgentList.clear()
        dialog = dialogues.ConnectDialogue(self.session)
        dialog.exec()
        for entry in self.session.agents:
            item = QtGui.QListWidgetItem()
            item.setText(entry.id)
            item.setData(QtCore.Qt.UserRole, entry)
            self.AgentList.addItem(item)

    def update_info_labels(self):
        self.IP.setText("IP: " + self.AgentList.currentItem().data(QtCore.Qt.UserRole).ip)
        self.ConnectionTime.setText("Connection Time: " + self.AgentList.currentItem().data(QtCore.Qt.UserRole).connection_time)
        self.Hostname.setText("Hostname: " + self.AgentList.currentItem().data(QtCore.Qt.UserRole).hostname)
        self.Interfaces.setText("Interfaces: " + self.AgentList.currentItem().data(QtCore.Qt.UserRole).interfaces)
        self.ProcOwner.setText("Process Owner: " + self.AgentList.currentItem().data(QtCore.Qt.UserRole).process_owner)
        
    def connect_to_c2(self):
        reply = QtGui.QMessageBox.question(self, 'WARNING', 
                 'Switching C2 servers will reset all info in the console. Any active processes will be dropped. Continue?', 
                 QtGui.QMessageBox.Yes, QtGui.QMessageBox.No)
        if reply == QtGui.QMessageBox.Yes:
            self._show_connect_dialogue() 

    def open_term(self):
        diag = dialogues.TerminalWidget()
        diag.show()
        diag.exec()

    def push_file(self):
        if not self.has_selection():
            return
        print("[ ] Waiting for file selection...")
        file = misc.open_file()
        self.session.upload_file(self.AgentList.currentItem().text(), file)

    def pull_file(self):
        if not self.has_selection():
            return
        diag = dialogues.AgentDownloadDialogue(self.session, self.AgentList.currentItem().text())
        diag.exec()

    def exec_comm(self):
        if not self.has_selection():
            return
        diag = dialogues.AgentCommandDialogue(self.session, self.AgentList.currentItem().text())
        diag.exec()

    def rev_sh(self):
        if not self.has_selection():
            return
        
        print("Getting reverse shell")
    
    def get_loot(self):
        if not self.has_selection():
            return
        
        print("Getting loot")
        self.session.download_loot(self.AgentList.currentItem().text())

    def push_module(self):
        if not self.has_selection():
            return
        file = misc.open_file()
        self.session.push_module(self.AgentList.currentItem().text(), file)

    def get_client(self):
        diag = dialogues.AgentCompileDialogue(self.session)
        diag.exec()

    def reg_client(self):
        diag = dialogues.AgentRegisterDialogue(self.session)
        diag.exec()
        

if __name__ == "__main__":
    app = QtGui.QApplication(sys.argv)
    connect = TestMgr()
    connect.show()
    
    sys.exit(app.exec_())
