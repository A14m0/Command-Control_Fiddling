from PyQt4 import QtGui

class File():
    def __init__(self, filename, size, data):
        self.filename = filename
        self.filesize = size
        self.data = data

def save_file(fileData):
    obj = QtGui.QFileDialog()
    filepath = obj.getSaveFileName(None, "Save file", "", "All Files (*)")

    if not filepath:
        return 1

    try:
        file = open(str(filepath), 'wb')
    except IOError:
        QtGui.QMessageBox.information(None, "Unable to open file", "There was an error opening \"%s\"" % filepath)
        file.close()
        return 1

    file.write(fileData)
    file.close()
    QtGui.QMessageBox.information(None, "Success", "Successfully wrote data to \"%s\"" % filepath)
    return 0

def open_file():
    obj = QtGui.QFileDialog()
    filepath = obj.getOpenFileName(None, "Select file", "", "All Files (*)")
    
    if not filepath:
        return 0

    try:
        file = open(str(filepath), 'rb')
    except IOError:
        QtGui.QMessageBox.information(None, "Unable to open file", "There was an error opening \"%s\"" % filepath)
        file.close()
        return 0

    data = file.read()
    file.close()
    size = len(data)
    name = filepath.split('/')[-1]

    fileObj = File(name, size, data)
    print("[i] Successfully read data from file")
    return fileObj    