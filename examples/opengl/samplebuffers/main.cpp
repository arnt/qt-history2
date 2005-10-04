/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "glwidget.h"

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    QGLFormat f = QGLFormat::defaultFormat();
    f.setSampleBuffers(true);
    QGLFormat::setDefaultFormat(f);
    if (!QGLFormat::hasOpenGL()) {
	QMessageBox::information(0, "OpenGL samplebuffers", 
				 "This system does not support OpenGL.",
				 QMessageBox::Ok);
        return 0;
    }

    GLWidget widget(0);
    widget.resize(640, 480);
    widget.show();

    return a.exec();
}

