#include <qapplication.h>
#include <qstylesheet.h>
#include <qfile.h>
#include <qtextstream.h>
#include "qtextview.h"

int main( int argc, char* argv[]  )
{
    QApplication a( argc, argv);
    // Many HTML files omit the </p> or </li>, so we add this for efficiency:
    QStyleSheet::defaultSheet()->item("p")->setSelfNesting( FALSE );
    QStyleSheet::defaultSheet()->item("i")->setSelfNesting( FALSE );
    QtTextEdit v;
    v.setTextFormat( QtTextEdit::RichText );
    //v.setFont( QFont("times", 12 ) );
    v.resize( 800, 1000 );
    QBrush paper;
     paper.setPixmap( QPixmap( "marble.xpm" ) );
//     v.setPaper( paper );
    a.setMainWidget( &v );
    

    if ( argc > 1 ) {
	QFile f( argv[1] );
	if ( f.open( IO_ReadOnly ) ) {
	    QTextStream ts( &f );
	    QString txt = ts.read();
	    f.close();
	    v.setText(txt, argv[1] );
	}
	else {
	    v.setText("Could not open file");
	}
    } else {
	qDebug("set text ");
	v.setText("No filename specified");
    }
    QtTextEdit second;
    second.setView( &v );
    v.show();
    second.show();
    return a.exec();
}
