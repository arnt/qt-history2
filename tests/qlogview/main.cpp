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
#include <qstylesheet.h>


class myf : public QVBox
{
    Q_OBJECT
public:
    int num;
    QTextEdit * log3, * editor;
    QPushButton * btn1, *btn2, *btn3, *btn4, *btn5, *btn6;
    myf( int n = 100 ) : QVBox()
    {
	num = n;
	log3 = 0;
	btn1 = new QPushButton( "Delete", this );
	btn2 = new QPushButton( "Clear", this );
	btn3 = new QPushButton( "Recreate", this );
	btn4 = new QPushButton( "&Import from editor", this );
	btn5 = new QPushButton( "Append", this );
	btn6 = new QPushButton( "Toggle Log/RichText", this );
	editor = new QTextEdit( this );
	editor->installEventFilter( this );
// 	editor->setTextFormat( Qt::PlainText );
	editor->setTextFormat( Qt::LogText );
	editor->setWordWrap( QTextEdit::WidgetWidth );
	editor->setText( "per <b> ole</b> <u> odvin</u> <i>ole</i>" );
	editor->append( "per2 <b> ole</b> <u> odvin</u> <i>ole</i>" );
	connect( btn1, SIGNAL(clicked()), SLOT(del()) );
	connect( btn2, SIGNAL(clicked()), SLOT(clear()) );
	connect( btn3, SIGNAL(clicked()), SLOT(recreate()) );
	connect( btn4, SIGNAL(clicked()), SLOT(import()) );
	connect( btn5, SIGNAL(clicked()), SLOT(append()) );
	connect( btn6, SIGNAL(clicked()), SLOT(toggleRT()) );
	
// 	recreate();

    }

    bool eventFilter( QObject * o, QEvent * e )
    {
	if ( o == log3 && e->type() == QEvent::MouseButtonPress ) {
	    QMouseEvent * me = (QMouseEvent *) e;
	    qWarning("parag at: (%d,%d) - %d", me->x(), me->y(), log3->paragraphAt( me->pos() ) );
// 	} else if ( o == editor && e->type() == QEvent::KeyPress ) {
// 	    QKeyEvent * ke = (QKeyEvent *) e;
// 	    if ( ke->key() != Key_Backspace && ke->key() != Key_Delete && 
// 		 ke->key() != Key_Left && ke->key() != Key_Right && 
// 		 ke->key() != Key_Up && ke->key() != Key_Down && 
// 		 ke->key() != Key_Home && ke->key() != Key_End ) {
// 		if ( editor->length() > MAX_LENGTH )
// 		    return TRUE;
// 	    }
	}
	return FALSE;
    }
    
public slots:
    void toggleRT()
    {
	if ( log3 ) {
	    if ( log3->textFormat() == Qt::LogText ) {
		log3->setTextFormat( Qt::PlainText );
	    } else {
		log3->setTextFormat( Qt::LogText );
	    }
	    
	}
    }
    void clicked( int p, int i)
    {
	qWarning("clicked parag: %d - %d", p, i );
    }
    void doubleClicked( int p, int i )
    {
	qWarning("doubleClicked parag: %d - %d", p, i );
    }

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

    void append()
    {
	if ( log3 ) {
	    log3->append(editor->text());
	}
    }

    void recreate()
    {
	if ( !log3 ) {
	    log3 = new QTextEdit( this );
	    log3->installEventFilter( this );
	    connect( log3, SIGNAL(clicked(int,int)), SLOT(clicked(int,int)) );
	    connect( log3, SIGNAL(doubleClicked(int,int)), SLOT(doubleClicked(int,int)) );
// 	    log3->setFont( QFont("courier", 15) );
	    QPalette p = log3->palette();
	    p.setColor( QColorGroup::Highlight, QColor("paleturquoise") );
	    p.setColor( QColorGroup::HighlightedText, Qt::white );
	    p.setColor( QColorGroup::Text, Qt::white );
	    p.setColor( QColorGroup::Base, Qt::black );
//  	    log3->setReadOnly( TRUE );
//  	    log3->setWordWrap( QTextEdit::NoWrap );
   	    log3->setTextFormat( Qt::LogText );
	    log3->setPalette( p );
	    log3->show();
	    QStyleSheet * sh = log3->styleSheet();
	    QStyleSheetItem * itm = new QStyleSheetItem( sh, "mytag" );
	    log3->styleSheet()->item( "font" )->setDisplayMode( QStyleSheetItem::DisplayBlock );
	    itm->setColor("#33AAFF");
	    itm->setFontItalic( TRUE );	    
	    itm->setFontUnderline( TRUE );	    
	    itm->setFontWeight( QFont::Bold );
	    qWarning( "ss: %d, %d", log3->styleSheet()->item("mytag"), log3->styleSheet()->item("mytag")->fontItalic() );
	}
	log3->clear();
	log3->setText("<u><red>The following:</red></u>");
	log3->append("<mytag>This is a customtag!</mytag> per per per");
	log3->append("  This is some &lt;red&gt;red text&lt;/red&gt;, while this is some &lt;green&gt;green\n  text&lt;/green&gt;. &lt;blue&gt;&lt;yellow&gt;This is yellow&lt;/yellow&gt;, while this is blue.&lt;/blue&gt;");
	log3->append("<u><red>is turned into this:</red></u>");
	log3->append("  This is some <i><red>red text</red></i>, while this is some <b><green>green\n  text</green></b>. <blue><yellow><u>This is yellow</u></yellow>, while this is blue.</blue>");

	log3->append( "&lt;&gt;Per <red>K<green>å<yellow>r</yellow></green>e\nbott</red><blue>olfson</blue> jr<green>.</green>" );
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
	str.sprintf("%06d: <red><b>1234567890</b></red> 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n"
		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n"
		    "%06d: 1234567890 <green>1234567890</green> 1234567890 1234567890\n"
		    "%06d: <i>1234567890</i> 1234567890 ygpqfh 1234567890 1234567890 1234567890\n"
		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890\n"
		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890\n"
		    "<yellow>%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890\n"
		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890\n"
		    "</yellow>%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n"
		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890", cn, cn + 1, cn + 2, cn +3, cn+4, cn+5,cn+6,cn+7,cn+8,cn+9);
// 	    str.sprintf("<p>%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n"
// 			"%06d: 1234567890 <green><b>1234567890 ygpqfh</b></green> 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n"
// 			"%06d: 1234567890 1234567890 1234567890 1234567890\n"
// 			"%06d: 1234567890 <i><u>1234567890</u></i> ygpqfh 1234567890 1234567890 1234567890\n"
// 			"%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890\n"
// 			"%06d: <red>1234567890</red> 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890\n"
// 			"%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890\n"
// 			"%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890\n"
// 			"%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n"
// 			"%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890</p>", cn, cn + 1, cn + 2, cn +3, cn+4, cn+5,cn+6,cn+7,cn+8,cn+9);
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
