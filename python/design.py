
# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file './python/ui/agent_command_dialogue.ui'
#
# Created by: PyQt5 UI code generator 5.13.2
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_CommandDialogue(object):
    def setupUi(self, CommandDialogue):
        CommandDialogue.setObjectName("CommandDialogue")
        CommandDialogue.resize(400, 173)
        self.buttonBox = QtWidgets.QDialogButtonBox(CommandDialogue)
        self.buttonBox.setGeometry(QtCore.QRect(-70, 120, 341, 32))
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtWidgets.QDialogButtonBox.Cancel|QtWidgets.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName("buttonBox")
        self.Title = QtWidgets.QLabel(CommandDialogue)
        self.Title.setGeometry(QtCore.QRect(10, 10, 131, 19))
        self.Title.setObjectName("Title")
        self.Info = QtWidgets.QLabel(CommandDialogue)
        self.Info.setGeometry(QtCore.QRect(20, 50, 221, 19))
        self.Info.setObjectName("Info")
        self.Input = QtWidgets.QLineEdit(CommandDialogue)
        self.Input.setGeometry(QtCore.QRect(20, 80, 351, 25))
        self.Input.setObjectName("Input")

        self.retranslateUi(CommandDialogue)
        self.buttonBox.accepted.connect(CommandDialogue.accept)
        self.buttonBox.rejected.connect(CommandDialogue.reject)
        QtCore.QMetaObject.connectSlotsByName(CommandDialogue)

    def retranslateUi(self, CommandDialogue):
        _translate = QtCore.QCoreApplication.translate
        CommandDialogue.setWindowTitle(_translate("CommandDialogue", "Dialog"))
        self.Title.setText(_translate("CommandDialogue", "Command Dialogue"))
        self.Info.setText(_translate("CommandDialogue", "Enter Command to Send to Agent"))
# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file './python/ui/agent_compile_dialogue.ui'
#
# Created by: PyQt5 UI code generator 5.13.2
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_AgentCompile(object):
    def setupUi(self, AgentCompile):
        AgentCompile.setObjectName("AgentCompile")
        AgentCompile.resize(357, 227)
        self.buttonBox = QtWidgets.QDialogButtonBox(AgentCompile)
        self.buttonBox.setGeometry(QtCore.QRect(-10, 170, 341, 32))
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtWidgets.QDialogButtonBox.Cancel|QtWidgets.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName("buttonBox")
        self.Title = QtWidgets.QLabel(AgentCompile)
        self.Title.setGeometry(QtCore.QRect(10, 10, 191, 19))
        self.Title.setObjectName("Title")
        self.ServerIP = QtWidgets.QLineEdit(AgentCompile)
        self.ServerIP.setGeometry(QtCore.QRect(20, 80, 181, 25))
        self.ServerIP.setObjectName("ServerIP")
        self.ServerPort = QtWidgets.QLineEdit(AgentCompile)
        self.ServerPort.setGeometry(QtCore.QRect(20, 140, 101, 25))
        self.ServerPort.setObjectName("ServerPort")
        self.IPLable = QtWidgets.QLabel(AgentCompile)
        self.IPLable.setGeometry(QtCore.QRect(20, 50, 61, 19))
        self.IPLable.setObjectName("IPLable")
        self.PortLabel = QtWidgets.QLabel(AgentCompile)
        self.PortLabel.setGeometry(QtCore.QRect(20, 110, 61, 19))
        self.PortLabel.setObjectName("PortLabel")

        self.retranslateUi(AgentCompile)
        self.buttonBox.accepted.connect(AgentCompile.accept)
        self.buttonBox.rejected.connect(AgentCompile.reject)
        QtCore.QMetaObject.connectSlotsByName(AgentCompile)

    def retranslateUi(self, AgentCompile):
        _translate = QtCore.QCoreApplication.translate
        AgentCompile.setWindowTitle(_translate("AgentCompile", "Dialog"))
        self.Title.setText(_translate("AgentCompile", "Agent Compilation Options"))
        self.IPLable.setText(_translate("AgentCompile", "Server IP"))
        self.PortLabel.setText(_translate("AgentCompile", "Port"))
# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file './python/ui/agent_download_dialogue.ui'
#
# Created by: PyQt5 UI code generator 5.13.2
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_AgentDownload(object):
    def setupUi(self, AgentDownload):
        AgentDownload.setObjectName("AgentDownload")
        AgentDownload.resize(344, 171)
        self.buttonBox = QtWidgets.QDialogButtonBox(AgentDownload)
        self.buttonBox.setGeometry(QtCore.QRect(-90, 120, 341, 32))
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtWidgets.QDialogButtonBox.Cancel|QtWidgets.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName("buttonBox")
        self.Path = QtWidgets.QLineEdit(AgentDownload)
        self.Path.setGeometry(QtCore.QRect(30, 80, 271, 25))
        self.Path.setObjectName("Path")
        self.Title = QtWidgets.QLabel(AgentDownload)
        self.Title.setGeometry(QtCore.QRect(10, 20, 201, 19))
        self.Title.setObjectName("Title")
        self.AgentPathLabel = QtWidgets.QLabel(AgentDownload)
        self.AgentPathLabel.setGeometry(QtCore.QRect(30, 50, 131, 19))
        self.AgentPathLabel.setObjectName("AgentPathLabel")

        self.retranslateUi(AgentDownload)
        self.buttonBox.accepted.connect(AgentDownload.accept)
        self.buttonBox.rejected.connect(AgentDownload.reject)
        QtCore.QMetaObject.connectSlotsByName(AgentDownload)

    def retranslateUi(self, AgentDownload):
        _translate = QtCore.QCoreApplication.translate
        AgentDownload.setWindowTitle(_translate("AgentDownload", "Dialog"))
        self.Title.setText(_translate("AgentDownload", "Agent File Download Dialogue"))
        self.AgentPathLabel.setText(_translate("AgentDownload", "Path to File (Agent)"))
# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file './python/ui/agent_registration_dialogue.ui'
#
# Created by: PyQt5 UI code generator 5.13.2
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_AgentRegister(object):
    def setupUi(self, AgentRegister):
        AgentRegister.setObjectName("AgentRegister")
        AgentRegister.resize(198, 191)
        self.buttonBox = QtWidgets.QDialogButtonBox(AgentRegister)
        self.buttonBox.setGeometry(QtCore.QRect(-160, 150, 341, 32))
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtWidgets.QDialogButtonBox.Cancel|QtWidgets.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName("buttonBox")
        self.Title = QtWidgets.QLabel(AgentRegister)
        self.Title.setGeometry(QtCore.QRect(10, 10, 181, 19))
        self.Title.setObjectName("Title")
        self.Username = QtWidgets.QLineEdit(AgentRegister)
        self.Username.setGeometry(QtCore.QRect(20, 60, 161, 25))
        self.Username.setObjectName("Username")
        self.Password = QtWidgets.QLineEdit(AgentRegister)
        self.Password.setGeometry(QtCore.QRect(20, 120, 161, 25))
        self.Password.setObjectName("Password")
        self.UsernameLabel = QtWidgets.QLabel(AgentRegister)
        self.UsernameLabel.setGeometry(QtCore.QRect(20, 40, 181, 19))
        self.UsernameLabel.setObjectName("UsernameLabel")
        self.PasswordLabel = QtWidgets.QLabel(AgentRegister)
        self.PasswordLabel.setGeometry(QtCore.QRect(20, 100, 181, 19))
        self.PasswordLabel.setObjectName("PasswordLabel")

        self.retranslateUi(AgentRegister)
        self.buttonBox.accepted.connect(AgentRegister.accept)
        self.buttonBox.rejected.connect(AgentRegister.reject)
        QtCore.QMetaObject.connectSlotsByName(AgentRegister)

    def retranslateUi(self, AgentRegister):
        _translate = QtCore.QCoreApplication.translate
        AgentRegister.setWindowTitle(_translate("AgentRegister", "Dialog"))
        self.Title.setText(_translate("AgentRegister", "Agent Registration Options"))
        self.UsernameLabel.setText(_translate("AgentRegister", "Username"))
        self.PasswordLabel.setText(_translate("AgentRegister", "Password"))
# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file './python/ui/main_window.ui'
#
# Created by: PyQt5 UI code generator 5.13.2
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        MainWindow.setObjectName("MainWindow")
        MainWindow.resize(800, 600)
        self.centralwidget = QtWidgets.QWidget(MainWindow)
        self.centralwidget.setObjectName("centralwidget")
        self.AgentSelect = QtWidgets.QScrollArea(self.centralwidget)
        self.AgentSelect.setGeometry(QtCore.QRect(0, 0, 331, 551))
        self.AgentSelect.setWidgetResizable(True)
        self.AgentSelect.setObjectName("AgentSelect")
        self.scrollAreaWidgetContents = QtWidgets.QWidget()
        self.scrollAreaWidgetContents.setGeometry(QtCore.QRect(0, 0, 327, 547))
        self.scrollAreaWidgetContents.setObjectName("scrollAreaWidgetContents")
        self.AgentList = QtWidgets.QListWidget(self.scrollAreaWidgetContents)
        self.AgentList.setGeometry(QtCore.QRect(10, 10, 311, 531))
        self.AgentList.setObjectName("AgentList")
        self.AgentSelect.setWidget(self.scrollAreaWidgetContents)
        self.verticalLayoutWidget = QtWidgets.QWidget(self.centralwidget)
        self.verticalLayoutWidget.setGeometry(QtCore.QRect(340, 0, 451, 221))
        self.verticalLayoutWidget.setObjectName("verticalLayoutWidget")
        self.AgentInfo = QtWidgets.QVBoxLayout(self.verticalLayoutWidget)
        self.AgentInfo.setContentsMargins(0, 0, 0, 0)
        self.AgentInfo.setObjectName("AgentInfo")
        self.IP = QtWidgets.QLabel(self.verticalLayoutWidget)
        self.IP.setObjectName("IP")
        self.AgentInfo.addWidget(self.IP)
        self.ConnectionTime = QtWidgets.QLabel(self.verticalLayoutWidget)
        self.ConnectionTime.setObjectName("ConnectionTime")
        self.AgentInfo.addWidget(self.ConnectionTime)
        self.Hostname = QtWidgets.QLabel(self.verticalLayoutWidget)
        self.Hostname.setObjectName("Hostname")
        self.AgentInfo.addWidget(self.Hostname)
        self.Interfaces = QtWidgets.QLabel(self.verticalLayoutWidget)
        self.Interfaces.setObjectName("Interfaces")
        self.AgentInfo.addWidget(self.Interfaces)
        self.ProcOwner = QtWidgets.QLabel(self.verticalLayoutWidget)
        self.ProcOwner.setObjectName("ProcOwner")
        self.AgentInfo.addWidget(self.ProcOwner)
        self.verticalLayoutWidget_3 = QtWidgets.QWidget(self.centralwidget)
        self.verticalLayoutWidget_3.setGeometry(QtCore.QRect(340, 340, 451, 211))
        self.verticalLayoutWidget_3.setObjectName("verticalLayoutWidget_3")
        self.Terminal = QtWidgets.QVBoxLayout(self.verticalLayoutWidget_3)
        self.Terminal.setContentsMargins(0, 0, 0, 0)
        self.Terminal.setObjectName("Terminal")
        self.line = QtWidgets.QFrame(self.verticalLayoutWidget_3)
        self.line.setFrameShape(QtWidgets.QFrame.HLine)
        self.line.setFrameShadow(QtWidgets.QFrame.Sunken)
        self.line.setObjectName("line")
        self.Terminal.addWidget(self.line)
        self.TermOutput = QtWidgets.QFrame(self.verticalLayoutWidget_3)
        self.TermOutput.setFrameShape(QtWidgets.QFrame.StyledPanel)
        self.TermOutput.setFrameShadow(QtWidgets.QFrame.Raised)
        self.TermOutput.setObjectName("TermOutput")
        self.Terminal.addWidget(self.TermOutput)
        self.TermInput = QtWidgets.QLineEdit(self.verticalLayoutWidget_3)
        self.TermInput.setObjectName("TermInput")
        self.Terminal.addWidget(self.TermInput)
        self.gridLayoutWidget = QtWidgets.QWidget(self.centralwidget)
        self.gridLayoutWidget.setGeometry(QtCore.QRect(340, 230, 451, 101))
        self.gridLayoutWidget.setObjectName("gridLayoutWidget")
        self.gridLayout = QtWidgets.QGridLayout(self.gridLayoutWidget)
        self.gridLayout.setContentsMargins(0, 0, 0, 0)
        self.gridLayout.setObjectName("gridLayout")
        self.PushFile = QtWidgets.QPushButton(self.gridLayoutWidget)
        self.PushFile.setObjectName("PushFile")
        self.gridLayout.addWidget(self.PushFile, 0, 1, 1, 1)
        self.PullFile = QtWidgets.QPushButton(self.gridLayoutWidget)
        self.PullFile.setObjectName("PullFile")
        self.gridLayout.addWidget(self.PullFile, 0, 0, 1, 1)
        self.GetLoot = QtWidgets.QPushButton(self.gridLayoutWidget)
        self.GetLoot.setObjectName("GetLoot")
        self.gridLayout.addWidget(self.GetLoot, 1, 0, 1, 1)
        self.PushModule = QtWidgets.QPushButton(self.gridLayoutWidget)
        self.PushModule.setObjectName("PushModule")
        self.gridLayout.addWidget(self.PushModule, 1, 1, 1, 1)
        self.ExecComm = QtWidgets.QPushButton(self.gridLayoutWidget)
        self.ExecComm.setObjectName("ExecComm")
        self.gridLayout.addWidget(self.ExecComm, 2, 0, 1, 1)
        self.RevSh = QtWidgets.QPushButton(self.gridLayoutWidget)
        self.RevSh.setObjectName("RevSh")
        self.gridLayout.addWidget(self.RevSh, 2, 1, 1, 1)
        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QtWidgets.QMenuBar(MainWindow)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 800, 24))
        self.menubar.setObjectName("menubar")
        self.menuAgents_Available = QtWidgets.QMenu(self.menubar)
        self.menuAgents_Available.setObjectName("menuAgents_Available")
        self.menuLocal = QtWidgets.QMenu(self.menubar)
        self.menuLocal.setObjectName("menuLocal")
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QtWidgets.QStatusBar(MainWindow)
        self.statusbar.setObjectName("statusbar")
        MainWindow.setStatusBar(self.statusbar)
        self.actionCompile_Client = QtWidgets.QAction(MainWindow)
        self.actionCompile_Client.setObjectName("actionCompile_Client")
        self.actionRegister_Client_Creds = QtWidgets.QAction(MainWindow)
        self.actionRegister_Client_Creds.setObjectName("actionRegister_Client_Creds")
        self.actionOpen_Terminal = QtWidgets.QAction(MainWindow)
        self.actionOpen_Terminal.setObjectName("actionOpen_Terminal")
        self.actionChange_C2_Server = QtWidgets.QAction(MainWindow)
        self.actionChange_C2_Server.setObjectName("actionChange_C2_Server")
        self.menuAgents_Available.addAction(self.actionCompile_Client)
        self.menuAgents_Available.addAction(self.actionRegister_Client_Creds)
        self.menuAgents_Available.addSeparator()
        self.menuAgents_Available.addAction(self.actionChange_C2_Server)
        self.menuLocal.addAction(self.actionOpen_Terminal)
        self.menubar.addAction(self.menuAgents_Available.menuAction())
        self.menubar.addAction(self.menuLocal.menuAction())

        self.retranslateUi(MainWindow)
        QtCore.QMetaObject.connectSlotsByName(MainWindow)

    def retranslateUi(self, MainWindow):
        _translate = QtCore.QCoreApplication.translate
        MainWindow.setWindowTitle(_translate("MainWindow", "MainWindow"))
        self.IP.setText(_translate("MainWindow", "IP: "))
        self.ConnectionTime.setText(_translate("MainWindow", "Connection Time: "))
        self.Hostname.setText(_translate("MainWindow", "Hostname: "))
        self.Interfaces.setText(_translate("MainWindow", "Interfaces: "))
        self.ProcOwner.setText(_translate("MainWindow", "Process Owner: "))
        self.PushFile.setText(_translate("MainWindow", "Push File"))
        self.PullFile.setText(_translate("MainWindow", "Pull File"))
        self.GetLoot.setText(_translate("MainWindow", "Get Loot"))
        self.PushModule.setText(_translate("MainWindow", "Push Module"))
        self.ExecComm.setText(_translate("MainWindow", "Execute Command"))
        self.RevSh.setText(_translate("MainWindow", "Reverse Shell"))
        self.menuAgents_Available.setTitle(_translate("MainWindow", "Server"))
        self.menuLocal.setTitle(_translate("MainWindow", "Local"))
        self.actionCompile_Client.setText(_translate("MainWindow", "Compile Client"))
        self.actionRegister_Client_Creds.setText(_translate("MainWindow", "Register Client Creds"))
        self.actionOpen_Terminal.setText(_translate("MainWindow", "Open Terminal"))
        self.actionChange_C2_Server.setText(_translate("MainWindow", "Change C2 Server"))
# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file './python/ui/manager_connection_dialogue.ui'
#
# Created by: PyQt5 UI code generator 5.13.2
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_ManagerServerConnect(object):
    def setupUi(self, ManagerServerConnect):
        ManagerServerConnect.setObjectName("ManagerServerConnect")
        ManagerServerConnect.resize(272, 207)
        self.IP = QtWidgets.QLineEdit(ManagerServerConnect)
        self.IP.setGeometry(QtCore.QRect(30, 70, 211, 25))
        self.IP.setObjectName("IP")
        self.Connect = QtWidgets.QPushButton(ManagerServerConnect)
        self.Connect.setGeometry(QtCore.QRect(30, 170, 214, 29))
        self.Connect.setObjectName("Connect")
        self.Title = QtWidgets.QLabel(ManagerServerConnect)
        self.Title.setGeometry(QtCore.QRect(10, 20, 191, 19))
        self.Title.setObjectName("Title")
        self.Port = QtWidgets.QLineEdit(ManagerServerConnect)
        self.Port.setGeometry(QtCore.QRect(30, 130, 81, 25))
        self.Port.setObjectName("Port")
        self.IPLabel = QtWidgets.QLabel(ManagerServerConnect)
        self.IPLabel.setGeometry(QtCore.QRect(30, 50, 191, 19))
        self.IPLabel.setObjectName("IPLabel")
        self.PortLabel = QtWidgets.QLabel(ManagerServerConnect)
        self.PortLabel.setGeometry(QtCore.QRect(30, 110, 191, 19))
        self.PortLabel.setObjectName("PortLabel")

        self.retranslateUi(ManagerServerConnect)
        QtCore.QMetaObject.connectSlotsByName(ManagerServerConnect)

    def retranslateUi(self, ManagerServerConnect):
        _translate = QtCore.QCoreApplication.translate
        ManagerServerConnect.setWindowTitle(_translate("ManagerServerConnect", "Dialog"))
        self.Connect.setText(_translate("ManagerServerConnect", "Connect"))
        self.Title.setText(_translate("ManagerServerConnect", "Server Connection Dialogue"))
        self.IPLabel.setText(_translate("ManagerServerConnect", "Server IP Address"))
        self.PortLabel.setText(_translate("ManagerServerConnect", "Server Port (defualt 22)"))
# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file './python/ui/terminal_dialogue.ui'
#
# Created by: PyQt5 UI code generator 5.13.2
#
# WARNING! All changes made in this file will be lost!


from PyQt5 import QtCore, QtGui, QtWidgets


class Ui_Terminal(object):
    def setupUi(self, Terminal):
        Terminal.setObjectName("Terminal")
        Terminal.resize(890, 512)

        self.retranslateUi(Terminal)
        QtCore.QMetaObject.connectSlotsByName(Terminal)

    def retranslateUi(self, Terminal):
        _translate = QtCore.QCoreApplication.translate
        Terminal.setWindowTitle(_translate("Terminal", "Dialog"))
