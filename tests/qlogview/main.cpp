#include <qapplication.h>
#include <qfont.h>
#include <qtextview.h>
#include "qlogview.h"
#include <qframe.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <qvbox.h>
#include <qpushbutton.h>


class myf : public QVBox
{
    Q_OBJECT
public:
    QTextEdit * log3;
    QPushButton * btn1, *btn2, *btn3;
    myf() : QVBox()
    {
	log3 = 0;
	btn1 = new QPushButton( "Delete", this );
	btn2 = new QPushButton( "Clear", this );
	btn3 = new QPushButton( "Recreate", this );
	
	connect( btn1, SIGNAL(clicked()), SLOT(del()) );
	connect( btn2, SIGNAL(clicked()), SLOT(clear()) );
	connect( btn3, SIGNAL(clicked()), SLOT(recreate()) );
	
	recreate();
    
    }

public slots:
    void clear() 
    {
	if ( log3 )
	    log3->setText("");
    }
    void del() 
    {
	delete log3;
	log3 = 0;
    }

    void recreate()
    {
	if ( !log3 ) {
	    log3 = new QTextEdit( this );
// 	    log3->setFont( QFont("courier", 15) );
	    QPalette p = log3->palette();
	    p.setColor( QColorGroup::Highlight, QColor("paleturquoise") );
	    p.setColor( QColorGroup::HighlightedText, Qt::black );
	    p.setColor( QColorGroup::Text, Qt::white );
	    p.setColor( QColorGroup::Base, Qt::black );
	    log3->setReadOnly( TRUE );
	    log3->setWordWrap( QTextEdit::NoWrap );
	    log3->setTextFormat( Qt::PlainText );
	    log3->setPalette( p );
	    log3->show();
	}
	int num = 1000;
	log3->clear();
	log3->append("<underline><red>The following:</red></underline>");
	log3->append("  This is some <<red>>red text<</red>>, while this is some <<green>>green\n  text<</green>>. <<blue>><<yellow>>This is yellow<</yellow>>, while this is blue.<</blue>>");
	log3->append("<u><red>is turned into this:</red></u>");
	log3->append("  This is some <italic><red>red text</red></italic>, while this is some <bold><green>green\n  text</green></bold>. <blue><yellow><underline>This is yellow</underline></yellow>, while this is blue.</blue>");

	log3->append( "<<>>Per <red>K<green>å<yellow>r</yellow></green>e\nbott</red><blue>olfson</blue> jr<green>.</green>" );
	log3->append( "<pink>Pink, <red>Red, <palegreen>Palegreen, </palegreen></red></pink>"
		      "<paleturquoise>Paleturquoise</paleturquoise>" );
	log3->append( "Per <red><blue>o</blue>lfson jr.</red>" );
	log3->append( "Per <red>Kåre\nbott<blue>olf</blue>son jr.</red>" );
	log3->append( "Insert test del<green>uxe\ninsert.</green>" );
	log3->append( "per <red>ole <yellow>bob\nroper</yellow> olle </red>bob");
    
	QString str;
	timeval st, et;
	gettimeofday( &st, 0 );
	fprintf( stderr,"\n");
	int cn = 0;
	for ( int i = 0; i < num; i++ ) {
	str.sprintf("%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n"
		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n"
		    "%06d: 1234567890 1234567890 1234567890 1234567890\n"
		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890\n"
		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890\n"
		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890\n"
		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890\n"
		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890\n"
		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n"
		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890", cn, cn + 1, cn + 2, cn +3, cn+4, cn+5,cn+6,cn+7,cn+8,cn+9);
// 	    str.sprintf("%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n"
// 			"%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n"
// 			"<deepskyblue>%06d</deepskyblue>: 1234567890 1234567890 1234567890 1234567890\n"
// 			"%06d: 1234567890 <red>1234567890</red> ygpqfh 1234567890 1234567890 1234567890\n"
// 			"%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890\n"
// 			"%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890\n"
// 			"%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890\n"
// 			"%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890\n"
// 			"%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n"
// 			"%06d: 1234567890 <slategray>1234567890</slategray> ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890", cn, cn + 1, cn + 2, cn +3, cn+4, cn+5,cn+6,cn+7,cn+8,cn+9);
	    cn +=10;
	    log3->append( str );
	    if ( (cn % 2000) == 0 ) {
		fprintf(stderr,"\rLines done: %08d", i );
	    }
	}
	gettimeofday( &et, 0 );
	fprintf(stderr,"\nLoop time: %d.%03d s\n", (int)(et.tv_sec - st.tv_sec),(int)(abs(et.tv_usec - st.tv_usec)%1000000) / 1000);
	fprintf(stderr,"\n");
    }
};

int main( int argc, char ** argv )
{
    QApplication app( argc, argv );
    int num = 1000;
    
    if ( argc > 1 )
	num = QString(argv[1]).toInt();
    myf * mf = new myf;
    mf->show();
    
//     QTextEdit * log3 = new QTextEdit;
//    QLogView* log = new QLogView;
//     app.setMainWidget( log3 );
    app.setMainWidget( mf );
    
//     log3->setFont( QFont("courier", 15) );
    
//     QPalette p = log3->palette();
//     p.setColor( QColorGroup::Highlight, QColor("paleturquoise") );
//     p.setColor( QColorGroup::HighlightedText, Qt::black );
//     p.setColor( QColorGroup::Text, Qt::white );
//     p.setColor( QColorGroup::Base, Qt::black );
//     log3->setReadOnly( TRUE );
//     log3->setWordWrap( QTextEdit::NoWrap );
//     log3->setTextFormat( Qt::PlainText );
//     log3->setPalette( p );
//     log3->append("The following:");
//     log3->append("  This is some <<red>>red text<</red>>, while this is some <<green>>green\n  text<</green>>. <<blue>><<yellow>>This is yellow<</yellow>>, while this is blue.<</blue>>");
//     log3->append("is turned into this:");
//     log3->append("  This is some <red>red text</red>, while this is some <green>green\n  text</green>. <blue><yellow>This is yellow</yellow>, while this is blue.</blue>");

//     log3->append( "<<>><indianred>Per <red>K<green>å<yellow>r</yellow></green>e\nbott</red><blue>olfson</blue> jr<green>.</green>" );
//     log3->append( "<pink>Pink, <red>Red, <palegreen>Palegreen, </palegreen></red></pink>"
// 		  "<paleturquoise>Paleturquoise</paleturquoise>" );
//     log3->append( "Per <red><blue>o</blue>lfson jr.</red>" );
//     log3->append( "Per <red>Kåre\nbott<blue>olf</blue>son jr.</red>" );
//     log3->append( "Insert test del<green>uxe\ninsert.</green>" );
//     log3->append( "per <red>ole <yellow>bob\nroper</yellow> olle </red>bob");
    
//     QString str;
//     timeval st, et;
//     gettimeofday( &st, 0 );
//     fprintf( stderr,"\n");
//     int cn = 0;
//     for ( int i = 0; i < num; i++ ) {
// // 	str.sprintf("%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n"
// // 		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n"
// // 		    "%06d: 1234567890 1234567890 1234567890 1234567890\n"
// // 		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890\n"
// // 		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890\n"
// // 		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890\n"
// // 		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890\n"
// // 		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890\n"
// // 		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n"
// // 		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890", cn, cn + 1, cn + 2, cn +3, cn+4, cn+5,cn+6,cn+7,cn+8,cn+9);
// 	str.sprintf("%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n"
// 		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n"
// 		    "<deepskyblue>%06d</deepskyblue>: 1234567890 1234567890 1234567890 1234567890\n"
// 		    "%06d: 1234567890 <red>1234567890</red> ygpqfh 1234567890 1234567890 1234567890\n"
// 		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890\n"
// 		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890\n"
// 		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890\n"
// 		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890\n"
// 		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n"
// 		    "%06d: 1234567890 <slategray>1234567890</slategray> ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890", cn, cn + 1, cn + 2, cn +3, cn+4, cn+5,cn+6,cn+7,cn+8,cn+9);
// 	cn +=10;
// 	log3->append( str );
// 	if ( (cn % 2000) == 0 ) {
// 	    fprintf(stderr,"\rLines done: %08d", i );
// 	}
//     }
//     gettimeofday( &et, 0 );
//     fprintf(stderr,"\nLoop time: %d.%03d s\n", (int)(et.tv_sec - st.tv_sec),(int)(abs(et.tv_usec - st.tv_usec)%1000000) / 1000);
//     fprintf(stderr,"\n");
//     str.sprintf("Trond Kjgyfzp\n");
//     log3->show();
//     int para = 5561, ind = 0;
//     qWarning("Found: %d", log3->find( "5555:", FALSE, FALSE, FALSE ));
// //				      &para, &ind) );
//     qWarning("para: %d, ind: %d", para, ind );
//     para = 0; ind = 0;
//     qWarning("2. Found: %d", log3->find( "9999: ", FALSE, FALSE, TRUE ));
// //				      &para, &ind) );
//     qWarning("para: %d, ind: %d", para, ind );

//     fprintf( stderr,"%d.%03d Kb\n", log3->length()/1024,
// 	     log3->length() % 1024 );

//     qWarning( log3->text( 500 ) );
//     qWarning( "%d",log3->paragraphs() );
//     qWarning( "%d",log3->paragraphLength( 500 ) );
//     qWarning( "%d",log3->lines() );
//     log3->show();
    return app.exec();
}

#include "main.moc"
