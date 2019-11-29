from PyQt5 import QtGui, QtWidgets, QtCore

class File():
    def __init__(self, filename, size, data):
        self.filename = filename
        self.filesize = size
        self.data = data

class InterfaceModel(QtCore.QAbstractTableModel):
    def __init__(self, interface_list, address_list, parent=None):
        super(InterfaceModel, self).__init__(parent)
        self.datarr = []
        for i in range(len(address_list)):
            tmp = []
            tmp.append(interface_list[i])
            tmp.append(address_list[i])
            self.datarr.append(tmp)

        

    def columnCount(self, parent):
        #print("Updating column count")
        return 1
    
    def rowCount(self, parent):
        try:
            #print("Row count: %d" % len(self.datarr[0]))
            #print(self.datarr)
            return len(self.datarr[0])
        except IndexError:
            return 0

    def data(self, index, role):
        if not index.isValid():
            return QtCore.QVariant()

        try:
            if not (0<=index.row()<len(self.datarr[0])):
                return QtCore.QVariant()
        except IndexError:
            return QtCore.QVariant()

        if role == QtCore.Qt.DisplayRole or role == QtCore.Qt.EditRole:
            return self.datarr[1][index.row()]
        else:
            return QtCore.QVariant()

    def headerData(self, section, orientation, role):
        if role != QtCore.Qt.DisplayRole:
            return QtCore.QVariant()

        if(orientation == QtCore.Qt.Vertical):
            return self.datarr[0][section]
        else:
            return QtCore.QVariant()

    def update(self, arr):
        self.layoutAboutToBeChanged.emit()
        self.datarr = arr
        self.layoutChanged.emit()
        
    
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
        string = "There was an error opening \"%s\"" % filepath[0]
        QtWidgets.QMessageBox.information(None, "Unable to open file", string)
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