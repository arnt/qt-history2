#include "qcolordialog.h"

#include "qlabel.h"
#include <qapplication.h>


class ColorLabel : public QLabel
{
public:
    ColorLabel() : QLabel( "Click me", 0, "hello" ) {}
protected:
    void mouseReleaseEvent( QMouseEvent * )
    {
	QColor c = QColorDialog::getColor( backgroundColor() ); 
	if ( c.isValid() ) 
            setBackgroundColor( c );    
    }
};

main(int argc, char** argv)
{
    QApplication::setColorSpec( QApplication::ManyColor );
    QApplication app(argc, argv);
    QApplication::setFont( QFont("Helvetica") );


	ColorLabel l;
	l.show();
	   
   QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));
   return app.exec();
}



