/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
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
#include <QtGui/QMessageBox>
#include "glwidget.h"

int main(int argc, char **argv)
{
    QApplication a(argc, argv);
    if (!QGLFormat::hasOpenGL() || !QGLFramebufferObject::hasOpenGLFramebufferObjects()) {
	QMessageBox::information(0, "OpenGL framebuffer objects 2",
				 "This system does not support OpenGL/framebuffer objects.",
				 QMessageBox::Ok);
        return -1;
    }

    GLWidget widget(0);
    widget.resize(640, 480);
    widget.show();
    return a.exec();
}

