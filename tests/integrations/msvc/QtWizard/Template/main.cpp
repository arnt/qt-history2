// main.cpp -- Implements the main() function

$$IF(PROJECT_QT_NONGUI)
#include <stdio.h>
$$ENDIF
$$IF(PROJECT_QT_GUIREADY)
#include <QApplication.h>
#include <$$QT_CENTRAL_WIDGET_TYPE$$.h>
$$ENDIF
$$IF(PROJECT_QT_GUI)
#include "$$Root$$App.h"
$$ENDIF

int main( int argc, char** argv )
{
$$IF(PROJECT_QT_NONGUI)
$$IF(QT_COMMENTS)
// TODO: Add program functionality here
$$ENDIF
	printf( "Hello, world\n");
$$ENDIF
$$IF(PROJECT_QT_GUIREADY)
$$IF(QT_COMMENTS)
// We create the application object, and then create
// the label that will be our main widget.
$$ENDIF
	QApplication app( argc, argv );
	$$QT_CENTRAL_WIDGET_TYPE$$* pMain = new $$QT_CENTRAL_WIDGET_TYPE$$( NULL );

$$IF(QT_SUPPORT_SETTEXT)
	pMain->setText( "Hello, world" );

$$ENDIF
	pMain->resize( 128, 32 );
	app.setMainWidget( pMain );
	pMain->show();

	return app.exec();
$$ENDIF
$$IF(PROJECT_QT_GUI)
$$IF(QT_COMMENTS)
// The application object will take care of creating
// all the objects that is needed to run the application,
// so we just create it, and give it control
$$ENDIF
	C$$Root$$App app( argc, argv );

	return app.exec();
$$ENDIF
}
