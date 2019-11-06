#!/usr/bin/env python
#-*- coding:utf-8 -*-

import  sys
from PyQt4 import QtCore, QtGui

class TerminalWidget(QtGui.QWidget):

    def __init__(self):
        QtGui.QWidget.__init__(self)
        self.process = QtCore.QProcess(self)
        self.terminal = QtGui.QWidget(self)
        layout = QtGui.QVBoxLayout(self)
        layout.addWidget(self.terminal)
        self.process.start(
                'xterm',['-into', str(self.terminal.winId())])
        # Works also with urxvt:
        #self.process.start(
                #'urxvt',['-embed', str(self.terminal.winId())])
