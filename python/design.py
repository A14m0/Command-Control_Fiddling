# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'main_window.ui'
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
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QtGui.QStatusBar(MainWindow)
        self.statusbar.setObjectName(_fromUtf8("statusbar"))
        MainWindow.setStatusBar(self.statusbar)
        self.actionCompile_Client = QtGui.QAction(MainWindow)
        self.actionCompile_Client.setObjectName(_fromUtf8("actionCompile_Client"))
        self.actionRegister_Client_Creds = QtGui.QAction(MainWindow)
        self.actionRegister_Client_Creds.setObjectName(_fromUtf8("actionRegister_Client_Creds"))
        self.menuAgents_Available.addAction(self.actionCompile_Client)
        self.menuAgents_Available.addAction(self.actionRegister_Client_Creds)
        self.menubar.addAction(self.menuAgents_Available.menuAction())

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
        self.actionCompile_Client.setText(_translate("MainWindow", "Compile Client", None))
        self.actionRegister_Client_Creds.setText(_translate("MainWindow", "Register Client Creds", None))

