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
    int num;
    QTextEdit * log3, * editor;
    QPushButton * btn1, *btn2, *btn3, *btn4;
    myf( int n = 100 ) : QVBox()
    {
	num = n;
	log3 = 0;
	btn1 = new QPushButton( "Delete", this );
	btn2 = new QPushButton( "Clear", this );
	btn3 = new QPushButton( "Recreate", this );
	btn4 = new QPushButton( "&Import from editor", this );
	editor = new QTextEdit( this );
	
	connect( btn1, SIGNAL(clicked()), SLOT(del()) );
	connect( btn2, SIGNAL(clicked()), SLOT(clear()) );
	connect( btn3, SIGNAL(clicked()), SLOT(recreate()) );
	connect( btn4, SIGNAL(clicked()), SLOT(import()) );
	
	recreate();

    }

public slots:
    void import()
    {
	if ( log3 ) {
	    log3->setText( editor->text() );
	}
    }
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
	log3->clear();
	log3->setText("<underline><red>The following:</red></underline>");
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
// 	str.sprintf("%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n"
// 		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n"
// 		    "%06d: 1234567890 1234567890 1234567890 1234567890\n"
// 		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890\n"
// 		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890\n"
// 		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890\n"
// 		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890\n"
// 		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890\n"
// 		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n"
// 		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890", cn, cn + 1, cn + 2, cn +3, cn+4, cn+5,cn+6,cn+7,cn+8,cn+9);
	    str.sprintf("\t\t\t%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n"
			"%06d: 1234567890 <green><b>1234567890 ygpqfh</b></green> 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n"
			"%06d: 1234567890 1234567890 1234567890 1234567890\n"
			"%06d: 1234567890 <i><u>1234567890</u></i> ygpqfh 1234567890 1234567890 1234567890\n"
			"%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890\n"
			"%06d: <red>1234567890</red> 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890\n"
			"%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890\n"
			"%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890\n"
			"%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n"
			"%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890", cn, cn + 1, cn + 2, cn +3, cn+4, cn+5,cn+6,cn+7,cn+8,cn+9);
	    cn +=10;
	    log3->append( str );
	    if ( (cn % 2000) == 0 ) {
		fprintf(stderr,"\rLines done: %08d", i );
	    }
	}
	gettimeofday( &et, 0 );
	fprintf(stderr,"\nLoop time: %d.%03d s\n", (int)(et.tv_sec - st.tv_sec),(int)(abs(et.tv_usec - st.tv_usec)%1000000) / 1000);
	fprintf(stderr,"Bytes: %.2f KB\n", (float)(log3->length()/1024) + (float)(log3->length()%1024)/1000.0 );
    }
};

int main( int argc, char ** argv )
{
    QApplication app( argc, argv );
    int num = 100;
    if ( argc > 1 )
 	num = QString(argv[1]).toInt();
    myf * mf = new myf( num );
    mf->show();
    app.setMainWidget( mf );
    return app.exec();
}

#include "main.moc"
