#!/usr/bin/env python

import os, sys
from PyQt4.QtCore import *
from PyQt4.QtGui import *


class CustomWidget(QWidget):

    def __init__(self, parent, fake = False):
    
        QWidget.__init__(self, parent)
        gradient = QLinearGradient(QPointF(0, 0), QPointF(100.0, 100.0))
        baseColor = QColor(0xa6, 0xce, 0x39, 0x7f)
        gradient.setColorAt(0.0, baseColor.light(150))
        gradient.setColorAt(0.75, baseColor.light(75))
        self.brush = QBrush(gradient)
        self.fake = fake
        self.fakeBrush = QBrush(Qt.red, Qt.DiagCrossPattern)
        
        qtPath = QPainterPath()
        qtPath.setFillRule(Qt.OddEvenFill)
        qtPath.moveTo(37.000000, 50.000000)
        qtPath.lineTo(50.000000, 37.000000)
        qtPath.lineTo(41.252698, 28.252698)
        qtPath.arcTo(-50.000000, -50.000000, 100.000000, 100.000000, -34.406038, 338.812076)
        qtPath.closeSubpath()
        qtPath.moveTo(12.000000, -1.000000)
        qtPath.lineTo(28.910935, 15.910935)
        qtPath.arcTo(-33.000000, -33.000000, 66.000000, 66.000000, -28.825886, 327.651772)
        qtPath.lineTo(-1.000000, 12.000000)
        qtPath.lineTo(-11.000000, 22.000000)
        qtPath.lineTo(-24.000000, 10.000000)
        qtPath.lineTo(10.000000, -24.000000)
        qtPath.lineTo(22.000000, -11.000000)
        qtPath.closeSubpath()
        qtPath.addRect(-60, -60, 120, 120)
        self.path = qtPath

    def paintEvent(self, event):
    
        painter = QPainter()
        painter.begin(self)
        painter.setRenderHint(QPainter.Antialiasing)
        if self.fake:
            painter.fillRect(event.rect(), QBrush(Qt.white))
            painter.fillRect(event.rect(), self.fakeBrush)
        painter.setBrush(self.brush)
        painter.translate(60, 60)
        painter.drawPath(self.path)
        painter.end()
    
    def sizeHint(self):
    
        return QSize(120, 120)
    
    def minimumSizeHint(self):
    
        return QSize(120, 120)


if __name__ == "__main__":

    try:
        qt = sys.argv[1]
    except IndexError:
        qt = "4.1"
    
    if qt != "4.0" and qt != "4.1":
        sys.stderr.write("Usage: %s [4.0|4.1]\n" % sys.argv[0])
        sys.exit(1)
    
    app = QApplication(sys.argv)
    exec_dir = os.path.split(os.path.abspath(sys.argv[0]))[0]
    label = QLabel()
    label.setPixmap(QPixmap(os.path.join(exec_dir, "background.png")))
    
    layout = QGridLayout()
    label.setLayout(layout)
    if qt == "4.0":
        layout.addWidget(CustomWidget(label), 0, 0, Qt.AlignCenter)
        caption = QLabel("Opaque (Default)", label)
        caption.setMargin(2)
        layout.addWidget(caption, 1, 0, Qt.AlignCenter | Qt.AlignTop)
    elif qt == "4.1":
        layout.addWidget(CustomWidget(label), 0, 0, Qt.AlignCenter)
        caption = QLabel("Contents Propagated (Default)", label)
        caption.setAutoFillBackground(True)
        caption.setMargin(2)
        layout.addWidget(caption, 1, 0, Qt.AlignCenter | Qt.AlignTop)
    
    if qt == "4.0":
        contentsWidget = CustomWidget(label)
        contentsWidget.setAttribute(Qt.WA_ContentsPropagated, True)
        layout.addWidget(contentsWidget, 0, 1, Qt.AlignCenter)
        caption = QLabel("With WA_ContentsPropagated set", label)
        caption.setMargin(2)
        layout.addWidget(caption, 1, 1, Qt.AlignCenter | Qt.AlignTop)
    elif qt == "4.1":
        autoFillWidget = CustomWidget(label)
        autoFillWidget.setAutoFillBackground(True)
        layout.addWidget(autoFillWidget, 0, 1, Qt.AlignCenter)
        caption = QLabel("With autoFillBackground set", label)
        caption.setAutoFillBackground(True)
        caption.setMargin(2)
        layout.addWidget(caption, 1, 1, Qt.AlignCenter | Qt.AlignTop)
    
    if qt == "4.0":
        noBackgroundWidget = CustomWidget(label, fake = True)
        noBackgroundWidget.setAttribute(Qt.WA_NoBackground, True)
        layout.addWidget(noBackgroundWidget, 0, 2, Qt.AlignCenter)
        caption = QLabel("With WA_NoBackground set", label)
        caption.setWordWrap(True)
        caption.setMargin(2)
        layout.addWidget(caption, 1, 2, Qt.AlignCenter | Qt.AlignTop)
    elif qt == "4.1":
        opaqueWidget = CustomWidget(label, fake = True)
        opaqueWidget.setAttribute(Qt.WA_OpaquePaintEvent, True)
        layout.addWidget(opaqueWidget, 0, 2, Qt.AlignCenter)
        caption = QLabel("With WA_OpaquePaintEvent set", label)
        caption.setAutoFillBackground(True)
        caption.setMargin(2)
        layout.addWidget(caption, 1, 2, Qt.AlignCenter | Qt.AlignTop)
    
    if qt == "4.0":
        label.setWindowTitle("Qt 4.0: Painting Custom Widgets")
    elif qt == "4.1":
        label.setWindowTitle("Qt 4.1: Painting Custom Widgets")
    
    label.resize(404, 160)
    label.show()
    sys.exit(app.exec_())
