/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "alphashade.h"
#include "clipping.h"
#include "demoviewer.h"
#include "introscreen.h"
#include "paths.h"
#include "roads.h"
#include "rotatinggradient.h"
#include "warpix.h"
#ifndef QT_NO_OPENGL
#include <qgl.h>
#include "glpainter.h"
#endif
#include "textoutline.h"
#include "items.h"

#include <qapplication.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    DemoViewer viewer;
    viewer.addDemoWidget("Arthur - The Paint Engine", new IntroScreen, "introscreen.cpp");
    viewer.addDemoWidget("Alphablended primitives", new AlphaShade, "alphashade.cpp");
    viewer.addDemoWidget("Rotating Gradient", new RotatingGradient, "rotatinggradient.cpp");
    viewer.addDemoWidget("Clip Regions", new Clipping, "clipping.cpp");
    viewer.addDemoWidget("Paths", new Paths, "paths.cpp");
    viewer.addDemoWidget("On the road", new Roads, "roads.cpp");
    viewer.addDemoWidget("Stretched Pixmap", new Warpix, "warpix.cpp");
    viewer.addDemoWidget("Items", new Items, "items.cpp");
#if !(defined(QT_NO_FONTCONFIG) && defined(Q_WS_X11))
    viewer.addDemoWidget("Outline", new TextOutline, "textoutline.cpp");
#endif
#ifndef QT_NO_OPENGL
    QGLFormat f = QGLFormat::defaultFormat();
    f.setSampleBuffers(true);
    f.setStencil(true);
    QGLFormat::setDefaultFormat(f);
    if (QGLFormat::hasOpenGL())
	viewer.addDemoWidget("OpenGL Painter", new GLPainter, "glpainter.cpp");
#endif
    viewer.show();

    QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
    return app.exec();
}
