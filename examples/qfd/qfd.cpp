/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "fontdisplayer.h"

#include <qapplication.h>
#include <qslider.h>
#include <qpainter.h>
#include <qstatusbar.h>



int main(int argc, char** argv)
{
    QApplication app(argc,argv);

    FontDisplayer m;
    QSize sh = m.centralWidget()->sizeHint();
    m.resize(sh.width(),
             sh.height()+3*m.statusBar()->height());
    app.setMainWidget(&m);
    m.setWindowTitle("Qt Example - QFD");
    m.show();

    return app.exec();
}
