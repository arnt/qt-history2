/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qmenubar.h>
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qimage.h>
#include <qtimer.h>

#include "canvas.h"

#include <stdlib.h>

extern QString butterfly_fn;
extern QString logo_fn;

int main(int argc, char** argv)
{
    QApplication app(argc,argv);
    
    if ( argc > 1 )
	butterfly_fn = argv[1];
    else
	butterfly_fn = ":/trolltech/examples/graphicsview/portedcanvas/butterfly.png";
    
    if ( argc > 2 )
	logo_fn = argv[2];
    else
	logo_fn = ":/trolltech/examples/graphicsview/portedcanvas/qtlogo.png";
    
    QGraphicsScene canvas;
    canvas.setSceneRect(0, 0, 800, 600);
    Main m(canvas);
    m.resize(m.sizeHint());
    m.setCaption("Ported Canvas Example");
    if ( QApplication::desktop()->width() > m.width() + 10
	&& QApplication::desktop()->height() > m.height() +30 )
	m.show();
    else
	m.showMaximized();
    
    QTimer timer;
    QObject::connect(&timer, SIGNAL(timeout()), &canvas, SLOT(advance()));
    timer.start(30);

    QObject::connect( qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()) );
    
    return app.exec();
}

