#include "qcolordialog.h"

#include "qlabel.h"
#include <qapplication.h>

main(int argc, char** argv)
{
    QApplication::setColorSpec( QApplication::ManyColor );
    QApplication app(argc, argv);
    QApplication::setFont( QFont("Helvetica") );

    	QColor c = QColorDialog::getColor();
    if ( c.isValid() ) {

	QLabel l( "\n\nHello, world\n\n\n\n", 0 ); 
	l.setBackgroundColor( c );
	l.show();
	   
	QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

	return app.exec();
    }
}



