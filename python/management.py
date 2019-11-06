#!/usr/bin/env python3
import sys
import random
import signal
import traceback
import time
import paletteColors
import session
import design
import terminal

from paramiko import ssh_exception
from PyQt4 import QtGui
# https://www.qt.io/qt-for-python
# https://doc.qt.io/qtforpython/?hsCtaTracking=fab9e910-0b90-4caa-b6d0-e202692b6f13%7Cb9e0be8d-1d85-4644-aaa2-41de4e6d37e3
# https://matplotlib.org/gallery/user_interfaces/embedding_in_qt_sgskip.html#sphx-glr-gallery-user-interfaces-embedding-in-qt-sgskip-py


class File():
    def __init__(self, filename, size, data):
        self.filename = filename
        self.filesize = size
        self.data = data
        

class Connect(QtGui.QDialog):
    def __init__(self, session, parent=None):
        QtGui.QDialog.__init__(self, parent)
        
        
        
        # Fun side note: 
        # you can use "fbs" to make standalone/installers for your programs

        # Some example widgets:
        # QLabel    --> Displays text
        # QComboBox --> Select all that apply button
        # QCheckBox --> On/Off option check box
        # QRadioButton  --> A,B,C,or D
        # QPushButton   --> On/Off button
        # QTableWidget  --> Shows an input table
        # QLineEdit --> Value Entry field
        # QProgressBar  --> (funny enough) a progress bar

        # Positioning
        # app = QApplication([])    <-- Defines the application object
        # app.setStyle("Fusion")    <-- Defines the application's theme
        #
        # window = QWidget()    <-- Defines the window basically
        # layout = QVBoxLayout()    <-- This defines a layout object
        # layout.addWidget(QPushButton('Top'))  <-- This tells the layout to add a button at the top of the window
        # window.setLayout(layout)  <-- Selects the given layout
        # window.show() <-- Shows the window
        # app.exec_()   <-- Executes the application

        color = paletteColors.Colors()
        print("Created thing")
        self.session = session
        
        self.setPalette(color.pltActive)

        
        self.inputDiag = QtGui.QLineEdit()
        self.connectButton = QtGui.QPushButton("Connect")
        self.connectButton.clicked.connect(self.getConnect)
        self.progBar = QtGui.QProgressBar()
        self.progBar.hide()

        self.connectLayout = QtGui.QHBoxLayout()
        self.connectLayout.addWidget(self.inputDiag)
        self.connectLayout.addWidget(self.connectButton)

        self.info = QtGui.QLabel("Enter C2 Server's IP Address")

        self.initLayout = QtGui.QGridLayout()
        self.initLayout.addWidget(self.info, 1,0)
        self.initLayout.addLayout(self.connectLayout, 2, 0)
        
        
        self.setWindowTitle("Management Console")

        self.setLayout(self.initLayout)
        

    def getConnect(self):
        value = self.inputDiag.text()
        print("[i] Attempting to connect to ip: %s"% value)
        if value == '127.0.0.1':
            value = (value, 1337)
            try:
                self.session.init_connection()
 
            except ValueError:
                QtGui.QMessageBox.information(self, "Illegal Input", "The value you passed was not a valid IP address")
                return
            except ssh_exception.NoValidConnectionsError:
                QtGui.QMessageBox.information(self, "Connection Refused", "The server refused the SSH connection")
                return
            except ssh_exception.SSHException:
                QtGui.QMessageBox.information(self, "Connection Failed", "Something happened. Check the console for more infromation")
                print(traceback.format_exc())
                return


            print("[+] Successfully connected!")
            self.close()            


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

        self.session = session.Session("127.0.0.1")
        self._show_connect_dialogue()

    def _has_selection(self):
        if not self.main.currentItem():
            QtGui.QMessageBox.information(self, "No Target Selected", "Select a target from the list to use this option")
            return False
        return True

    def _show_connect_dialogue(self):
        dialog = Connect(self.session)
        dialog.exec()

    def connectToC2(self):
        if not self.has_selection():
            print("[-] No server selected")
            return 
        print("[i] Connecting to C2 server....")
        diag = QtGui.QInputDialog(self)
        print("[i] Currently selected item: %s" % self.main.currentItem().text())
     
    def openFile(self):
        obj = QtGui.QFileDialog()
        obj.setPalette(self.color.pltActive)
        filepath, filterStr = obj.getOpenFileName(self, "Select file", "", "All Files (*)")
        
        if not filepath:
            return 0

        try:
            file = open(str(filepath), 'rb')
        except IOError:
            QtGui.QMessageBox.information(self, "Unable to open file", "There was an error opening \"%s\"" % filepath)
            file.close()
            return 0

        data = file.read()
        file.close()
        print("[i] Successfully read data from file")
        return data

    def saveFile(self, fileData):
        obj = QtGui.QFileDialog()
        filepath, filterStr = obj.getSaveFileName(self, "Save file", "", "All Files (*)")

        if not filepath:
            return 1

        try:
            file = open(str(filepath), 'wb')
        except IOError:
            QtGui.QMessageBox.information(self, "Unable to open file", "There was an error opening \"%s\"" % filepath)
            file.close()
            return 1

        file.write(fileData)
        file.close()
        QtGui.QMessageBox.information(self, "Success", "Successfully wrote data to \"%s\"" % filepath)
        return 0

    

    def push_file(self):
        print("Select a file")

    def pull_file(self):
        print("Downloading file")

    def exec_comm(self):
        print("Executing command")

    def rev_sh(self):
        print("Getting reverse shell")
    
    def get_loot(self):
        print("Getting loot")

    def push_module(self):
        print("Pushing module")

    def get_client(self):
        print("Getting client")

    def reg_client(self):
        print("Registering client")
        

"""
        self.leftMenuLayout = QtWidgets.QVBoxLayout()
        self.leftMenuLayout.addWidget(self.header)
        self.leftMenuLayout.addWidget(self.shellTarget)
        self.leftMenuLayout.addWidget(self.infoTarget)
        self.leftMenuLayout.addWidget(self.uploadTarget)
        self.leftMenuLayout.addWidget(self.downloadTarget)
        self.leftMenuLayout.addWidget(self.genPayload)
        self.leftMenuLayout.setAlignment(QtCore.Qt.AlignTop)
        

        self.mainDiagLayout = QtWidgets.QVBoxLayout()
        
        self.info = QtWidgets.QLabel("Info: To begin, connect to a server...")
        #self.main = QtWidgets.QListWidget()

        self.main = QtWidgets.QScrollArea()

        self.mainDiagLayout.addWidget(self.main)
        widget = QtWidgets.QWidget()
        
        for value in self.session.agents:
            print(value.id)
            item = QtWidgets.QFrame()
            item.setObjectName(value.id)
            item.setStyleSheet('background-color: red')
            self.mainDiagLayout.addWidget(item)
        #    example = QtWidgets.QListWidgetItem(value.id)
        #    self.main.addItem(example)
        
        self.main.setWidget(widget)
        
        self.mainDiagLayout.addWidget(self.info)
        self._lastpos = None


        self.infoBox = QtWidgets.QLabel("")
        #if self.has_selection():
         #   text = "Agent ID: %s\nAgent Last Connection Time: %s\nHostname: %s\nIP Address: %s\nInterfaces: %s\nProcess owner: %s" % (self.main.currentItem.id, self.main.currentItem.ip, self.main.currentItem.connection_time, self.main.currentItem.hostname, self.main.currentItem.ip_address,self.main.currentItem.interfaces,self.main.currentItem.process_owner)
          #  self.infoBox.setText(text)
        #else:
        self.infoBox.setText("No Agent Selected")
        self.infoLayout = QtWidgets.QVBoxLayout()
        self.infoLayout.addWidget(self.infoBox)

        
        self.controlLayout = QtWidgets.QGridLayout()
        #self.controlLayout = QtWidgets.QLayout()
        self.controlLayout.setAlignment(QtCore.Qt.AlignLeft)
        self.controlLayout.addWidget(self._menu)
        self.controlLayout.addLayout(self.leftMenuLayout, 1, 0)
        self.controlLayout.addLayout(self.mainDiagLayout, 1, 1)
        self.controlLayout.addLayout(self.infoLayout, 1, 2)

        self.setWindowTitle("Management Console")

        self.setLayout(self.controlLayout)

            
    def payload(self):
        if not self.has_selection:
            return

        print("Generating payload")
    
    def shell(self):
        if not self.has_selection():
            return
            
        print("[i] Getting shell from target \"%s\"..." % self.main.currentItem().text())

    def upload(self):
        if not self.has_selection():
            return
        
        data = self.openFile()
        if not data:
            return

        print("[i] Uploading file to target...")

        # Commands here are the same for agents 
        # So 10 -> Download file from server (but has info regarding agent too)
        # 11 -> Request reverse shell from agent
        # 12 -> Upload file to server (with agent info)
        # 13 -> agent exec shell code
        # 14 -> agent exec module
        # 0 -> exit connection

        self.channel.sendall("10")

    def download(self):
        if not self.has_selection():
            return
        
        self.saveFile(self.data)
        
        print("[i] Downloading loot from target...")

    def createMenuBar(self):
        self._menu = QtWidgets.QMenuBar()
        self._menu.setNativeMenuBar(True)

        
        fileMenu = self._menu.addMenu("File")
        fileMenu.setPalette(self.color.pltActive)
        fileMenu.addAction("New")
        fileMenu.addAction("Open", self.openFile)
        fileMenu.addAction("Save", self.openFile)
        fileMenu.addAction("Save As")
        fileMenu.addSeparator()
        fileMenu.addAction("Exit", self.exit)
        
        self._menu.addAction("Connect",self.connectToC2)
        
       

    def exit(self):
        print("[ ] Exiting...")
        sys.exit(0)
"""

if __name__ == "__main__":
    app = QtGui.QApplication(sys.argv)
    

    connect = TestMgr()
    connect.show()
    
    sys.exit(app.exec_())
