#!/usr/bin/env python3
import sys
import random
import signal
import paramiko
import traceback
import time
import paletteColors

from PySide2 import QtCore, QtGui, QtWidgets, QtMultimedia, QtMultimediaWidgets
# https://www.qt.io/qt-for-python
# https://doc.qt.io/qtforpython/?hsCtaTracking=fab9e910-0b90-4caa-b6d0-e202692b6f13%7Cb9e0be8d-1d85-4644-aaa2-41de4e6d37e3
# https://matplotlib.org/gallery/user_interfaces/embedding_in_qt_sgskip.html#sphx-glr-gallery-user-interfaces-embedding-in-qt-sgskip-py

class Connect(QtWidgets.QDialog):
    def __init__(self):
        QtWidgets.QDialog.__init__(self)
        
        
        
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
        
        self.setPalette(color.pltActive)

        
        self.inputDiag = QtWidgets.QLineEdit()
        self.connectButton = QtWidgets.QPushButton("Connect")
        self.connectButton.clicked.connect(self.getConnect)
        self.progBar = QtWidgets.QProgressBar()
        self.progBar.hide()

        self.connectLayout = QtWidgets.QHBoxLayout()
        self.connectLayout.addWidget(self.inputDiag)
        self.connectLayout.addWidget(self.connectButton)

        self.info = QtWidgets.QLabel("Enter C2 Server's IP Address")

        self.initLayout = QtWidgets.QGridLayout()
        self.initLayout.addWidget(self.info, 1,0)
        self.initLayout.addLayout(self.connectLayout, 2, 0)
        
        
        self.setWindowTitle("Hunter Management Console")

        self.setLayout(self.initLayout)
        

    def getConnect(self):
        value = self.inputDiag.text()
        print("[i] Attempting to connect to ip: %s"% value)
        if value == '127.0.0.1':
            value = (value, 1337)
            try:
                ssh = paramiko.SSHClient()
                ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
                ssh.connect(hostname="localhost", username='aris', password='lala', allow_agent=False)

                channel = ssh._transport.open_session()
                channel.invoke_shell()
                
                stdin = channel.makefile('wb')
                stdout = channel.makefile('r')

                # To read from channel:
                #   recv(self, nbytes)
                # To write to channel:
                #   send(self, s)

                channel.

                
            except ValueError:
                QtWidgets.QMessageBox.information(self, "Illegal Input", "The value you passed was not a valid IP address")
                return
            except paramiko.ssh_exception.NoValidConnectionsError:
                QtWidgets.QMessageBox.information(self, "Connection Refused", "The server refused the SSH connection")
                return
            except paramiko.ssh_exception.SSHException:
                QtWidgets.QMessageBox.information(self, "Connection Failed", "Something happened. Check the console for more infromation")
                print(traceback.format_exc())
                return


            print("[+] Successfully connected!")
            self.manager = Manager("example")
            self.hide()
            self.manager.show()



class Manager(QtWidgets.QWidget):
    def __init__(self, socket):
        QtWidgets.QWidget.__init__(self)
        self.socket = socket
        
        #self.text.setAlignment(Qt.AlignCenter)

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

        

        self.color = paletteColors.Colors()
        self.setPalette(self.color.pltActive)
	



        self.createMenuBar()

        self.header = QtWidgets.QLabel("Commands:")

        self.shellTarget = QtWidgets.QPushButton("Shell")
        self.shellTarget.setToolTip("Get a reverse shell from selected target")
        #self.shellTarget.setEnabled(False)
        self.infoTarget = QtWidgets.QPushButton("Info")
        self.infoTarget.setToolTip("Get information from selected target")
        #self.infoTarget.setEnabled(False)
        self.uploadTarget = QtWidgets.QPushButton("Push File")
        self.uploadTarget.setToolTip("Push a file to target")
        #self.uploadTarget.setEnabled(False)
        self.downloadTarget = QtWidgets.QPushButton("Get Loot")
        self.downloadTarget.setToolTip("Downloads all files in loot directory")
        #self.downloadTarget.setEnabled(False)
        self.genPayload = QtWidgets.QPushButton("Generate Payload")
        self.genPayload.setToolTip("Generates a payload")
        

        self.shellTarget.clicked.connect(self.shell)
        self.infoTarget.clicked.connect(self.getInfo)
        self.uploadTarget.clicked.connect(self.upload)
        self.downloadTarget.clicked.connect(self.download)
        self.genPayload.clicked.connect(self.payload)



        
        self.leftMenuLayout = QtWidgets.QVBoxLayout()
        self.leftMenuLayout.addWidget(self.header)
        self.leftMenuLayout.addWidget(self.shellTarget)
        self.leftMenuLayout.addWidget(self.infoTarget)
        self.leftMenuLayout.addWidget(self.uploadTarget)
        self.leftMenuLayout.addWidget(self.downloadTarget)
        self.leftMenuLayout.addWidget(self.genPayload)
        self.leftMenuLayout.setAlignment(QtCore.Qt.AlignTop)
        


        self.info = QtWidgets.QLabel("Info: To begin, connect to a server...")
        self.main = QtWidgets.QListWidget()
        listex = ["This", "is", "an", "array", "input", "test"]
        for value in listex:
            example = QtWidgets.QListWidgetItem(value)
            self.main.addItem(example)
        
        
        self.mainDiagLayout = QtWidgets.QVBoxLayout()
        self.mainDiagLayout.addWidget(self.main)
        self.mainDiagLayout.addWidget(self.info)

        
        self.controlLayout = QtWidgets.QGridLayout()
        self.controlLayout.setAlignment(QtCore.Qt.AlignLeft)
        self.controlLayout.addWidget(self._menu)
        self.controlLayout.addLayout(self.leftMenuLayout, 1, 0)
        self.controlLayout.addLayout(self.mainDiagLayout, 1, 1)

        self.setWindowTitle("Hunter Management Console")

        self.setLayout(self.controlLayout)

    def has_selection(self):
        if not self.main.currentItem():
            QtWidgets.QMessageBox.information(self, "No Target Selected", "Select a target from the list to use this option")
            return False
        return True
            
    def payload(self):
        if not self.has_selection:
            return

        print("Generating payload")
    
    def shell(self):
        if not self.has_selection():
            return
            
        print("[i] Shelling target \"%s\"..." % self.main.currentItem().text())

    def getInfo(self):
        if not self.has_selection():
            return
        
        print("[i] Gathering target information...")

    def upload(self):
        if not self.has_selection():
            return
        
        data = self.openFile()
        if not data:
            return

        print("[i] Uploading file to target...")

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
        
    def connectToC2(self):
        if not self.has_selection():
            print("[-] No server selected")
            return 
        print("[i] Connecting to C2 server....")
        diag = QtWidgets.QInputDialog(self)
        
        
        print("[i] Currently selected item: %s" % self.main.currentItem().text())
        

    def openFile(self):
        obj = QtWidgets.QFileDialog()
        obj.setPalette(self.pltActive)
        filepath, filterStr = obj.getOpenFileName(self, "Select file", "", "All Files (*)")
        
        if not filepath:
            return 0

        try:
            file = open(str(filepath), 'rb')
        except IOError:
            QtWidgets.QMessageBox.information(self, "Unable to open file", "There was an error opening \"%s\"" % filepath)
            file.close()
            return 0

        data = file.read()
        file.close()
        print("[i] Successfully read data from file")
        return data

    def saveFile(self, fileData):
        obj = QtWidgets.QFileDialog()
        filepath, filterStr = obj.getSaveFileName(self, "Save file", "", "All Files (*)")

        if not filepath:
            return 1

        try:
            file = open(str(filepath), 'wb')
        except IOError:
            QtWidgets.QMessageBox.information(self, "Unable to open file", "There was an error opening \"%s\"" % filepath)
            file.close()
            return 1

        file.write(fileData)
        file.close()
        QtWidgets.QMessageBox.information(self, "Success", "Successfully wrote data to \"%s\"" % filepath)
        return 0

    def exit(self):
        print("[ ] Exiting...")
        sys.exit(0)


if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    

    connect = Connect()
    connect.show()
    
    sys.exit(app.exec_())
