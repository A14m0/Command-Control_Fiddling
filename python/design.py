
# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file './python/ui/agent_compile_dialogue.ui'
#
# Created by: PyQt4 UI code generator 4.12.1
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    def _fromUtf8(s):
        return s

try:
    _encoding = QtGui.QApplication.UnicodeUTF8
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig, _encoding)
except AttributeError:
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig)

class Ui_AgentCompile(object):
    def setupUi(self, AgentCompile):
        AgentCompile.setObjectName(_fromUtf8("AgentCompile"))
        AgentCompile.resize(357, 227)
        self.buttonBox = QtGui.QDialogButtonBox(AgentCompile)
        self.buttonBox.setGeometry(QtCore.QRect(-10, 170, 341, 32))
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.Title = QtGui.QLabel(AgentCompile)
        self.Title.setGeometry(QtCore.QRect(10, 10, 191, 19))
        self.Title.setObjectName(_fromUtf8("Title"))
        self.ServerIP = QtGui.QLineEdit(AgentCompile)
        self.ServerIP.setGeometry(QtCore.QRect(20, 80, 181, 25))
        self.ServerIP.setObjectName(_fromUtf8("ServerIP"))
        self.ServerPort = QtGui.QLineEdit(AgentCompile)
        self.ServerPort.setGeometry(QtCore.QRect(20, 140, 101, 25))
        self.ServerPort.setObjectName(_fromUtf8("ServerPort"))
        self.IPLable = QtGui.QLabel(AgentCompile)
        self.IPLable.setGeometry(QtCore.QRect(20, 50, 61, 19))
        self.IPLable.setObjectName(_fromUtf8("IPLable"))
        self.PortLabel = QtGui.QLabel(AgentCompile)
        self.PortLabel.setGeometry(QtCore.QRect(20, 110, 61, 19))
        self.PortLabel.setObjectName(_fromUtf8("PortLabel"))

        self.retranslateUi(AgentCompile)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), AgentCompile.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), AgentCompile.reject)
        QtCore.QMetaObject.connectSlotsByName(AgentCompile)

    def retranslateUi(self, AgentCompile):
        AgentCompile.setWindowTitle(_translate("AgentCompile", "Dialog", None))
        self.Title.setText(_translate("AgentCompile", "Agent Compilation Options", None))
        self.IPLable.setText(_translate("AgentCompile", "Server IP", None))
        self.PortLabel.setText(_translate("AgentCompile", "Port", None))

# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file './python/ui/agent_download_dialogue.ui'
#
# Created by: PyQt4 UI code generator 4.12.1
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    def _fromUtf8(s):
        return s

try:
    _encoding = QtGui.QApplication.UnicodeUTF8
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig, _encoding)
except AttributeError:
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig)

class Ui_AgentDownload(object):
    def setupUi(self, AgentDownload):
        AgentDownload.setObjectName(_fromUtf8("AgentDownload"))
        AgentDownload.resize(344, 171)
        self.buttonBox = QtGui.QDialogButtonBox(AgentDownload)
        self.buttonBox.setGeometry(QtCore.QRect(-90, 120, 341, 32))
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.Path = QtGui.QLineEdit(AgentDownload)
        self.Path.setGeometry(QtCore.QRect(30, 80, 271, 25))
        self.Path.setObjectName(_fromUtf8("Path"))
        self.Title = QtGui.QLabel(AgentDownload)
        self.Title.setGeometry(QtCore.QRect(10, 20, 201, 19))
        self.Title.setObjectName(_fromUtf8("Title"))
        self.AgentPathLabel = QtGui.QLabel(AgentDownload)
        self.AgentPathLabel.setGeometry(QtCore.QRect(30, 50, 131, 19))
        self.AgentPathLabel.setObjectName(_fromUtf8("AgentPathLabel"))

        self.retranslateUi(AgentDownload)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), AgentDownload.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), AgentDownload.reject)
        QtCore.QMetaObject.connectSlotsByName(AgentDownload)

    def retranslateUi(self, AgentDownload):
        AgentDownload.setWindowTitle(_translate("AgentDownload", "Dialog", None))
        self.Title.setText(_translate("AgentDownload", "Agent File Download Dialogue", None))
        self.AgentPathLabel.setText(_translate("AgentDownload", "Path to File (Agent)", None))

# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file './python/ui/agent_registration_dialogue.ui'
#
# Created by: PyQt4 UI code generator 4.12.1
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    def _fromUtf8(s):
        return s

try:
    _encoding = QtGui.QApplication.UnicodeUTF8
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig, _encoding)
except AttributeError:
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig)

class Ui_AgentRegister(object):
    def setupUi(self, AgentRegister):
        AgentRegister.setObjectName(_fromUtf8("AgentRegister"))
        AgentRegister.resize(198, 191)
        self.buttonBox = QtGui.QDialogButtonBox(AgentRegister)
        self.buttonBox.setGeometry(QtCore.QRect(-160, 150, 341, 32))
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.Title = QtGui.QLabel(AgentRegister)
        self.Title.setGeometry(QtCore.QRect(10, 10, 181, 19))
        self.Title.setObjectName(_fromUtf8("Title"))
        self.Username = QtGui.QLineEdit(AgentRegister)
        self.Username.setGeometry(QtCore.QRect(20, 60, 161, 25))
        self.Username.setObjectName(_fromUtf8("Username"))
        self.Password = QtGui.QLineEdit(AgentRegister)
        self.Password.setGeometry(QtCore.QRect(20, 120, 161, 25))
        self.Password.setObjectName(_fromUtf8("Password"))
        self.UsernameLabel = QtGui.QLabel(AgentRegister)
        self.UsernameLabel.setGeometry(QtCore.QRect(20, 40, 181, 19))
        self.UsernameLabel.setObjectName(_fromUtf8("UsernameLabel"))
        self.PasswordLabel = QtGui.QLabel(AgentRegister)
        self.PasswordLabel.setGeometry(QtCore.QRect(20, 100, 181, 19))
        self.PasswordLabel.setObjectName(_fromUtf8("PasswordLabel"))

        self.retranslateUi(AgentRegister)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), AgentRegister.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), AgentRegister.reject)
        QtCore.QMetaObject.connectSlotsByName(AgentRegister)

    def retranslateUi(self, AgentRegister):
        AgentRegister.setWindowTitle(_translate("AgentRegister", "Dialog", None))
        self.Title.setText(_translate("AgentRegister", "Agent Registration Options", None))
        self.UsernameLabel.setText(_translate("AgentRegister", "Username", None))
        self.PasswordLabel.setText(_translate("AgentRegister", "Password", None))

# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file './python/ui/main_window.ui'
#
# Created by: PyQt4 UI code generator 4.12.1
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    def _fromUtf8(s):
        return s

try:
    _encoding = QtGui.QApplication.UnicodeUTF8
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig, _encoding)
except AttributeError:
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig)

class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        MainWindow.setObjectName(_fromUtf8("MainWindow"))
        MainWindow.resize(800, 600)
        self.centralwidget = QtGui.QWidget(MainWindow)
        self.centralwidget.setObjectName(_fromUtf8("centralwidget"))
        self.AgentSelect = QtGui.QScrollArea(self.centralwidget)
        self.AgentSelect.setGeometry(QtCore.QRect(0, 0, 331, 551))
        self.AgentSelect.setWidgetResizable(True)
        self.AgentSelect.setObjectName(_fromUtf8("AgentSelect"))
        self.scrollAreaWidgetContents = QtGui.QWidget()
        self.scrollAreaWidgetContents.setGeometry(QtCore.QRect(0, 0, 327, 547))
        self.scrollAreaWidgetContents.setObjectName(_fromUtf8("scrollAreaWidgetContents"))
        self.AgentList = QtGui.QListWidget(self.scrollAreaWidgetContents)
        self.AgentList.setGeometry(QtCore.QRect(10, 10, 311, 531))
        self.AgentList.setObjectName(_fromUtf8("AgentList"))
        self.AgentSelect.setWidget(self.scrollAreaWidgetContents)
        self.verticalLayoutWidget = QtGui.QWidget(self.centralwidget)
        self.verticalLayoutWidget.setGeometry(QtCore.QRect(340, 0, 451, 221))
        self.verticalLayoutWidget.setObjectName(_fromUtf8("verticalLayoutWidget"))
        self.AgentInfo = QtGui.QVBoxLayout(self.verticalLayoutWidget)
        self.AgentInfo.setMargin(0)
        self.AgentInfo.setObjectName(_fromUtf8("AgentInfo"))
        self.IP = QtGui.QLabel(self.verticalLayoutWidget)
        self.IP.setObjectName(_fromUtf8("IP"))
        self.AgentInfo.addWidget(self.IP)
        self.ConnectionTime = QtGui.QLabel(self.verticalLayoutWidget)
        self.ConnectionTime.setObjectName(_fromUtf8("ConnectionTime"))
        self.AgentInfo.addWidget(self.ConnectionTime)
        self.Hostname = QtGui.QLabel(self.verticalLayoutWidget)
        self.Hostname.setObjectName(_fromUtf8("Hostname"))
        self.AgentInfo.addWidget(self.Hostname)
        self.Interfaces = QtGui.QLabel(self.verticalLayoutWidget)
        self.Interfaces.setObjectName(_fromUtf8("Interfaces"))
        self.AgentInfo.addWidget(self.Interfaces)
        self.ProcOwner = QtGui.QLabel(self.verticalLayoutWidget)
        self.ProcOwner.setObjectName(_fromUtf8("ProcOwner"))
        self.AgentInfo.addWidget(self.ProcOwner)
        self.verticalLayoutWidget_3 = QtGui.QWidget(self.centralwidget)
        self.verticalLayoutWidget_3.setGeometry(QtCore.QRect(340, 340, 451, 211))
        self.verticalLayoutWidget_3.setObjectName(_fromUtf8("verticalLayoutWidget_3"))
        self.Terminal = QtGui.QVBoxLayout(self.verticalLayoutWidget_3)
        self.Terminal.setMargin(0)
        self.Terminal.setObjectName(_fromUtf8("Terminal"))
        self.line = QtGui.QFrame(self.verticalLayoutWidget_3)
        self.line.setFrameShape(QtGui.QFrame.HLine)
        self.line.setFrameShadow(QtGui.QFrame.Sunken)
        self.line.setObjectName(_fromUtf8("line"))
        self.Terminal.addWidget(self.line)
        self.TermOutput = QtGui.QFrame(self.verticalLayoutWidget_3)
        self.TermOutput.setFrameShape(QtGui.QFrame.StyledPanel)
        self.TermOutput.setFrameShadow(QtGui.QFrame.Raised)
        self.TermOutput.setObjectName(_fromUtf8("TermOutput"))
        self.Terminal.addWidget(self.TermOutput)
        self.TermInput = QtGui.QLineEdit(self.verticalLayoutWidget_3)
        self.TermInput.setObjectName(_fromUtf8("TermInput"))
        self.Terminal.addWidget(self.TermInput)
        self.gridLayoutWidget = QtGui.QWidget(self.centralwidget)
        self.gridLayoutWidget.setGeometry(QtCore.QRect(340, 230, 451, 101))
        self.gridLayoutWidget.setObjectName(_fromUtf8("gridLayoutWidget"))
        self.gridLayout = QtGui.QGridLayout(self.gridLayoutWidget)
        self.gridLayout.setMargin(0)
        self.gridLayout.setObjectName(_fromUtf8("gridLayout"))
        self.PushFile = QtGui.QPushButton(self.gridLayoutWidget)
        self.PushFile.setObjectName(_fromUtf8("PushFile"))
        self.gridLayout.addWidget(self.PushFile, 0, 1, 1, 1)
        self.PullFile = QtGui.QPushButton(self.gridLayoutWidget)
        self.PullFile.setObjectName(_fromUtf8("PullFile"))
        self.gridLayout.addWidget(self.PullFile, 0, 0, 1, 1)
        self.GetLoot = QtGui.QPushButton(self.gridLayoutWidget)
        self.GetLoot.setObjectName(_fromUtf8("GetLoot"))
        self.gridLayout.addWidget(self.GetLoot, 1, 0, 1, 1)
        self.PushModule = QtGui.QPushButton(self.gridLayoutWidget)
        self.PushModule.setObjectName(_fromUtf8("PushModule"))
        self.gridLayout.addWidget(self.PushModule, 1, 1, 1, 1)
        self.ExecComm = QtGui.QPushButton(self.gridLayoutWidget)
        self.ExecComm.setObjectName(_fromUtf8("ExecComm"))
        self.gridLayout.addWidget(self.ExecComm, 2, 0, 1, 1)
        self.RevSh = QtGui.QPushButton(self.gridLayoutWidget)
        self.RevSh.setObjectName(_fromUtf8("RevSh"))
        self.gridLayout.addWidget(self.RevSh, 2, 1, 1, 1)
        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QtGui.QMenuBar(MainWindow)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 800, 24))
        self.menubar.setObjectName(_fromUtf8("menubar"))
        self.menuAgents_Available = QtGui.QMenu(self.menubar)
        self.menuAgents_Available.setObjectName(_fromUtf8("menuAgents_Available"))
        self.menuLocal = QtGui.QMenu(self.menubar)
        self.menuLocal.setObjectName(_fromUtf8("menuLocal"))
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QtGui.QStatusBar(MainWindow)
        self.statusbar.setObjectName(_fromUtf8("statusbar"))
        MainWindow.setStatusBar(self.statusbar)
        self.actionCompile_Client = QtGui.QAction(MainWindow)
        self.actionCompile_Client.setObjectName(_fromUtf8("actionCompile_Client"))
        self.actionRegister_Client_Creds = QtGui.QAction(MainWindow)
        self.actionRegister_Client_Creds.setObjectName(_fromUtf8("actionRegister_Client_Creds"))
        self.actionOpen_Terminal = QtGui.QAction(MainWindow)
        self.actionOpen_Terminal.setObjectName(_fromUtf8("actionOpen_Terminal"))
        self.actionChange_C2_Server = QtGui.QAction(MainWindow)
        self.actionChange_C2_Server.setObjectName(_fromUtf8("actionChange_C2_Server"))
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
        MainWindow.setWindowTitle(_translate("MainWindow", "MainWindow", None))
        self.IP.setText(_translate("MainWindow", "IP: ", None))
        self.ConnectionTime.setText(_translate("MainWindow", "Connection Time: ", None))
        self.Hostname.setText(_translate("MainWindow", "Hostname: ", None))
        self.Interfaces.setText(_translate("MainWindow", "Interfaces: ", None))
        self.ProcOwner.setText(_translate("MainWindow", "Process Owner: ", None))
        self.PushFile.setText(_translate("MainWindow", "Push File", None))
        self.PullFile.setText(_translate("MainWindow", "Pull File", None))
        self.GetLoot.setText(_translate("MainWindow", "Get Loot", None))
        self.PushModule.setText(_translate("MainWindow", "Push Module", None))
        self.ExecComm.setText(_translate("MainWindow", "Execute Command", None))
        self.RevSh.setText(_translate("MainWindow", "Reverse Shell", None))
        self.menuAgents_Available.setTitle(_translate("MainWindow", "Server", None))
        self.menuLocal.setTitle(_translate("MainWindow", "Local", None))
        self.actionCompile_Client.setText(_translate("MainWindow", "Compile Client", None))
        self.actionRegister_Client_Creds.setText(_translate("MainWindow", "Register Client Creds", None))
        self.actionOpen_Terminal.setText(_translate("MainWindow", "Open Terminal", None))
        self.actionChange_C2_Server.setText(_translate("MainWindow", "Change C2 Server", None))

# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file './python/ui/manager_connection_dialogue.ui'
#
# Created by: PyQt4 UI code generator 4.12.1
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    def _fromUtf8(s):
        return s

try:
    _encoding = QtGui.QApplication.UnicodeUTF8
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig, _encoding)
except AttributeError:
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig)

class Ui_ManagerServerConnect(object):
    def setupUi(self, ManagerServerConnect):
        ManagerServerConnect.setObjectName(_fromUtf8("ManagerServerConnect"))
        ManagerServerConnect.resize(272, 207)
        self.IP = QtGui.QLineEdit(ManagerServerConnect)
        self.IP.setGeometry(QtCore.QRect(30, 70, 211, 25))
        self.IP.setObjectName(_fromUtf8("IP"))
        self.Connect = QtGui.QPushButton(ManagerServerConnect)
        self.Connect.setGeometry(QtCore.QRect(30, 170, 214, 29))
        self.Connect.setObjectName(_fromUtf8("Connect"))
        self.Title = QtGui.QLabel(ManagerServerConnect)
        self.Title.setGeometry(QtCore.QRect(10, 20, 191, 19))
        self.Title.setObjectName(_fromUtf8("Title"))
        self.Port = QtGui.QLineEdit(ManagerServerConnect)
        self.Port.setGeometry(QtCore.QRect(30, 130, 81, 25))
        self.Port.setObjectName(_fromUtf8("Port"))
        self.IPLabel = QtGui.QLabel(ManagerServerConnect)
        self.IPLabel.setGeometry(QtCore.QRect(30, 50, 191, 19))
        self.IPLabel.setObjectName(_fromUtf8("IPLabel"))
        self.PortLabel = QtGui.QLabel(ManagerServerConnect)
        self.PortLabel.setGeometry(QtCore.QRect(30, 110, 191, 19))
        self.PortLabel.setObjectName(_fromUtf8("PortLabel"))

        self.retranslateUi(ManagerServerConnect)
        QtCore.QMetaObject.connectSlotsByName(ManagerServerConnect)

    def retranslateUi(self, ManagerServerConnect):
        ManagerServerConnect.setWindowTitle(_translate("ManagerServerConnect", "Dialog", None))
        self.Connect.setText(_translate("ManagerServerConnect", "Connect", None))
        self.Title.setText(_translate("ManagerServerConnect", "Server Connection Dialogue", None))
        self.IPLabel.setText(_translate("ManagerServerConnect", "Server IP Address", None))
        self.PortLabel.setText(_translate("ManagerServerConnect", "Server Port (defualt 22)", None))

# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file './python/ui/terminal_dialogue.ui'
#
# Created by: PyQt4 UI code generator 4.12.1
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    def _fromUtf8(s):
        return s

try:
    _encoding = QtGui.QApplication.UnicodeUTF8
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig, _encoding)
except AttributeError:
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig)

class Ui_Terminal(object):
    def setupUi(self, Terminal):
        Terminal.setObjectName(_fromUtf8("Terminal"))
        Terminal.resize(890, 512)

        self.retranslateUi(Terminal)
        QtCore.QMetaObject.connectSlotsByName(Terminal)

    def retranslateUi(self, Terminal):
        Terminal.setWindowTitle(_translate("Terminal", "Dialog", None))

