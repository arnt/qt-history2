#include "alphashade.h"
#include "clipping.h"
#include "demoviewer.h"
#include "introscreen.h"
#include "paths.h"
#include "rotatinggradient.h"
#include "warpix.h"

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
    viewer.show();

    QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
    return app.exec();
}
