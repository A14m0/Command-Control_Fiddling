from PyQt5 import QtGui

class Colors():
    def __init__(self):
        self.pltActive = QtGui.QPalette()
        self.pltActive.setColor(QtGui.QPalette.Foreground, QtGui.QColor(206, 0, 0))
        self.pltActive.setColor(QtGui.QPalette.Text, QtGui.QColor(255,50,50))
        self.pltActive.setColor(QtGui.QPalette.Window, QtGui.QColor(30,30,30))
        self.pltActive.setColor(QtGui.QPalette.Button, QtGui.QColor(50,50,50))
        self.pltActive.setColor(QtGui.QPalette.ButtonText, QtGui.QColor(206,0,0))
        self.pltActive.setColor(QtGui.QPalette.Base, QtGui.QColor(50,50,50))
        self.pltActive.setColor(QtGui.QPalette.Highlight, QtGui.QColor(206,0,0))
        self.pltActive.setColor(QtGui.QPalette.HighlightedText, QtGui.QColor(255,255,255))
        self.pltActive.setColor(QtGui.QPalette.AlternateBase, QtGui.QColor(206,90,90))
        self.pltActive.setColor(QtGui.QPalette.ToolTipBase, QtGui.QColor(50,50,50))
        self.pltActive.setColor(QtGui.QPalette.ToolTipText, QtGui.QColor(206,90,90))
        self.pltActive.setColor(QtGui.QPalette.HighlightedText, QtGui.QColor(255,255,255))

    