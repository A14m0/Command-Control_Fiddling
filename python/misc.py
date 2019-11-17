from PyQt5 import QtGui, QtWidgets

class File():
    def __init__(self, filename, size, data):
        self.filename = filename
        self.filesize = size
        self.data = data

def save_file(fileData):
    obj = QtWidgets.QFileDialog()
    filepath, filter = obj.getSaveFileName(None, "Save file", "", "All Files (*)")

    if not filepath:
        return 1

    try:
        file = open(str(filepath), 'wb')
    except IOError:
        QtWidgets.QMessageBox.information(None, "Unable to open file", "There was an error opening \"%s\"" % filepath)
        file.close()
        return 1

    file.write(fileData)
    file.close()
    QtWidgets.QMessageBox.information(None, "Success", "Successfully wrote data to \"%s\"" % filepath)
    return 0

def open_file():
    obj = QtWidgets.QFileDialog()
    filepath = obj.getOpenFileName(None, "Select file", "", "All Files (*)")
    
    if not filepath:
        return 0

    try:
        file = open(str(filepath[0]), 'rb')
    except IOError:
        string = "There was an error opeing \"%s\"" % filepath[0]
        QtWidgets.QMessageBox.information(None, "Unable to open file", string)
        file.close()
        return 0

    data = file.read()
    file.close()
    size = len(data)
    name = filepath[0].split('/')[-1]

    fileObj = File(name, size, data)
    print("[i] Successfully read data from file")
    return fileObj    

def getSavePath():
    obj = QtWidgets.QFileDialog()
    filepath = obj.getSaveFileName(None, "Save file", "", "All Files (*)")

    return filepath