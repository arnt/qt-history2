/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui/QApplication>
#include <QtGui/QMessageBoxEx>
#include "glwidget.h"

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    QGLFormat f = QGLFormat::defaultFormat();
    f.setSampleBuffers(true);
    QGLFormat::setDefaultFormat(f);
    if (!QGLFormat::hasOpenGL()) {
	QMessageBoxEx::information(0, "OpenGL samplebuffers",
				 "This system does not support OpenGL.");
        return 0;
    }

    GLWidget widget(0);

    if (!widget.format().sampleBuffers()) {
	QMessageBoxEx::information(0, "OpenGL samplebuffers",
				 "This system does not have sample buffer support.");
        return 0;
    }

    widget.resize(640, 480);
    widget.show();

    return a.exec();
}

