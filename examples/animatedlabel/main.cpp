/****************************************************************************
**
** Copyright (C) 2003 Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qapplication.h>
#include "animatedlabel.h"

int main( int argc, char ** argv )
{
    QApplication app( argc, argv );

    bool busy = false;
    QString animation = "trolltech.gif";

    for (int arg=1; arg < argc; arg++) {
	QString a = argv[arg];
	if ( a == "--busy" )
	    busy = true;
	else
	    animation = a;
    }

    AnimatedLabel m(animation);
    m.setWindowTitle("Qt Examples - Animated Label");
    app.setMainWidget( &m );
    m.show();

    if ( busy ) {
	while (1) {
	    // ... do some processing ...

	    m.pushUpdate();
	}
    } else {
	// Normal event-based animation.
	return app.exec();
    }
}
