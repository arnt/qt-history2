#include "qcolordialog.h"

#include "qlabel.h"
#include <qapplication.h>

#include <qdatastream.h>
#include <qfile.h>


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

    
    QFile f( "colours.dat" );
    if ( f.open(IO_ReadOnly) ) {
        QDataStream t( &f );
        int n = 0;
	QColor c;
	t >> c;
        while ( !t.eof() ) {        
	    QColorDialog::setCustomColor( n++, c.rgb() );
	    t >> c;
        }
        f.close();
    }

    
    QColor c = QColorDialog::getColor( QApplication::palette().color( QPalette::Normal, QColorGroup::Background ));
    
    
    
    ColorLabel l;
    if ( c.isValid() )
	l.setBackgroundColor( c );
    l.show();
	
   QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));
   int r =  app.exec();
   if ( f.open(IO_WriteOnly) ) {
        QDataStream t( &f );
        for ( int i = 0; i < QColorDialog::customCount(); i++ ) {
	    QColor c( QColorDialog::customColor(i) );
            t << c;
        }
        f.close();
   }
   return r;
}



