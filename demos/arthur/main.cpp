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
#include "rotatinggradient.h"
#include "warpix.h"
#ifndef QT_NO_OPENGL
#include <qgl.h>
#include "glpainter.h"
#endif

#include <qapplication.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    DemoViewer viewer;
    viewer.addDemoWidget("Arthur - The Paint Engine", new IntroScreen);
    viewer.addDemoWidget("Alphablended primitives", new AlphaShade);
    viewer.addDemoWidget("Rotating Gradient", new RotatingGradient);
    viewer.addDemoWidget("Clip Regions", new Clipping);
    viewer.addDemoWidget("Paths", new Paths);
    viewer.addDemoWidget("Stretched Pixmap", new Warpix);
#ifndef QT_NO_OPENGL
    if (QGLFormat::hasOpenGL())
	viewer.addDemoWidget("OpenGL Painter", new GLPainter);
#endif
    viewer.show();

    QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
    return app.exec();
}
