#include <qapplication.h>
#include <qstylesheet.h>
#include <qfile.h>
#include <qtextstream.h>
#include "qtextview.h"
#include "qtextbrowser.h"

int main( int argc, char* argv[]  )
{
    QApplication a( argc, argv);
    // Ignore any bodytext in <head>...</head>:
    QStyleSheetItem* style = new QStyleSheetItem( QStyleSheet::defaultSheet(), "head" );
    style->setDisplayMode(QStyleSheetItem::DisplayNone);
    // Many HTML files omit the </p> or </li>, so we add this for efficiency:
    QStyleSheet::defaultSheet()->item("p")->setSelfNesting( FALSE );
    QStyleSheet::defaultSheet()->item("i")->setSelfNesting( FALSE );
    QStyleSheet::defaultSheet()->item("multicol")->setDisplayMode( QStyleSheetItem::DisplayBlock );

    QStyleSheet::defaultSheet()->item("h3")->setMargin( QStyleSheetItem::MarginTop, 8 );
    QStyleSheet::defaultSheet()->item("h3")->setMargin( QStyleSheetItem::MarginBottom, 4 );
    QStyleSheet::defaultSheet()->item("p")->setMargin( QStyleSheetItem::MarginVertical, 0 );
    QStyleSheet::defaultSheet()->item("p")->setMargin( QStyleSheetItem::MarginTop, 4 );

    
    style = new QStyleSheetItem( QStyleSheet::defaultSheet(), "right" );
    style->setAlignment( QStyleSheetItem::AlignCenter );
    style->setDisplayMode( QStyleSheetItem::DisplayBlock );
    style->setMargin( QStyleSheetItem::MarginLeft, 80 );
    style->setMargin( QStyleSheetItem::MarginRight, 40 );

    QtTextBrowser v;
    v.setTextFormat( QtTextEdit::RichText );
    //v.setFont( QFont("times", 12 ) );
    v.resize( 800, 600 );
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
//     QtTextEdit second;
//     second.setView( &v );
    v.show();
//     second.show();
    return a.exec();
}
