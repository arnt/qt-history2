#include <qapplication.h>
#include <qfile.h>
#include <qtextstream.h>
#include "qtextview.h"

int main( int argc, char* argv[]  )
{
    QApplication a( argc, argv);
    QtTextView v;
    QBrush paper;
    paper.setPixmap( QPixmap( "marble.xpm" ) );
    v.setPaper( paper );
    a.setMainWidget( &v );
    
    if ( argc > 1 ) {
	QFile f( argv[1] );
	if ( f.open( IO_ReadOnly ) ) {
	    QTextStream ts( &f );
	    QString txt = ts.read();
	    f.close();
	    v.setText(txt);
	}
	else {
	    v.setText("Could not open file");
	}
    } else {
	v.setText("No filename specified");
    }
    v.show();
    return a.exec();
}
