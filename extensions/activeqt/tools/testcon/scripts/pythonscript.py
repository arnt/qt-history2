def QAxWidget2_Click():
    QAxWidget2.lineWidth = QAxWidget2.lineWidth + 1;

def fatLines():
    QAxWidget2.lineWidth = 25;

def thinLines():
    QAxWidget2.lineWidth = 1;

def setLineWidth(width):
    QAxWidget2.lineWidth = width;

def getLineWidth():
    return QAxWidget2.lineWidth;
