#include <qapplication.h>
#include <qstylesheet.h>
#include <qfile.h>
#include <qtextstream.h>
#include "qtextview.h"
#include "qtextbrowser.h"
#include <qvbox.h>
#include <qpushbutton.h>


int main( int argc, char* argv[]  )
{
    QApplication a( argc, argv);
    // Ignore any body text in <head>...</head>:
    QStyleSheetItem* style = new QStyleSheetItem( QStyleSheet::defaultSheet(), "head" );
    style->setDisplayMode(QStyleSheetItem::DisplayNone);

    style = new QStyleSheetItem( QStyleSheet::defaultSheet(), "td" );
    style->setContexts("tr");
    style = new QStyleSheetItem( QStyleSheet::defaultSheet(), "th" );
    style->setFontWeight( QFont::Bold );
    style->setAlignment( Qt::AlignCenter );
    style->setContexts("tr");
    style = new QStyleSheetItem( QStyleSheet::defaultSheet(), "tr" );
    style->setContexts("table");


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

    QVBox box;

    QPushButton* b = new QPushButton( "Push me", &box, 0 );
    QtTextBrowser*  v = new QtTextBrowser( &box );
    v->setFocus();
    b->connect( b, SIGNAL( clicked() ), v, SLOT( temporary() ) );
    v->setTextFormat( QtTextEdit::RichText );
    //v->setFont( QFont("times", 12 ) );
//     QBrush paper;
//     paper.setPixmap( QPixmap( "marble.xpm" ) );
//     v->setPaper( paper );
    a.setMainWidget( &box );


    if ( argc > 1 ) {
 	v->setSource(argv[1] );
//  	QFile f( argv[1] );
//  	if ( f.open( IO_ReadOnly ) ) {
//  	    QTextStream ts( &f );
//  	    QString txt = ts.read();
//  	    f.close();
//  	    v->setText(txt, argv[1] );
//  	}
//  	else {
//  	    v->setText("Could not open file");
//  	}
    } else {
	qDebug("set text ");
	v->setText("No filename specified");
    }
//     QtTextEdit second;
//     second.setView( &v );
    box.resize( 500, 720 );
    box.show();
//     second.show();
    return a.exec();
}
