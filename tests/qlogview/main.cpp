#include <qapplication.h>
#include <qfont.h>
#include <qtextview.h>
#include <qframe.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qstylesheet.h>
#include <qbuffer.h>
#include <qtextstream.h>
#include <qdatetime.h>


class myf : public QVBox
{
    Q_OBJECT
public:
    int num;
    QTextEdit * log3, * editor;
    QPushButton * btn1, *btn2, *btn3, *btn4, *btn5, *btn6, *btn7, *btn8;
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
	btn7 = new QPushButton( "Text 1", this );
	btn8 = new QPushButton( "Text 2", this );
	editor = new QTextEdit( this );
	editor->installEventFilter( this );
// 	editor->setTextFormat( Qt::PlainText );
 	editor->setTextFormat( Qt::LogText );
	editor->setWordWrap( QTextEdit::WidgetWidth );
	editor->setText( "per <b>ole</b> odvin ole" );
	editor->append( "per2 ole odvin ole" );
	connect( btn1, SIGNAL(clicked()), SLOT(del()) );
	connect( btn2, SIGNAL(clicked()), SLOT(clear()) );
	connect( btn3, SIGNAL(clicked()), SLOT(recreate()) );
	connect( btn4, SIGNAL(clicked()), SLOT(import()) );
	connect( btn5, SIGNAL(clicked()), SLOT(append()) );
	connect( btn6, SIGNAL(clicked()), SLOT(toggleRT()) );
	connect( btn7, SIGNAL(clicked()), SLOT(text1()) );
	connect( btn8, SIGNAL(clicked()), SLOT(text2()) );
	
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
    void timerEvent( QTimerEvent *)
    {
	static int num = 0;
	if ( log3 ) {
	    log3->insertAt( "Test ", log3->lines(), 100000);
// 	    log3->append( QString("%1 This is a <red>log</red> line test. ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz").arg(++num) );
// 	    log3->repaint();
	}
    }
    
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
//  	    log3->setLogLimit( 0 );
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
// 	    qWarning( "ss: %d, %d", log3->styleSheet()->item("mytag"), log3->styleSheet()->item("mytag")->fontItalic() );
	}
	log3->clear();
  	startTimer( 500 );
// 	log3->setText("<u><red>The following:</red></u>");
// 	log3->append("<mytag>This is a customtag!</mytag> per per per");
// 	log3->append("  This is some &lt;red&gt;red text&lt;/red&gt;, while this is some &lt;green&gt;green\n  text&lt;/green&gt;. &lt;blue&gt;&lt;yellow&gt;This is yellow&lt;/yellow&gt;, while this is blue.&lt;/blue&gt;");
// 	log3->append("<u><red>is turned into this:</red></u>");
// 	log3->append("  This is some <i><red>red text</red></i>, while this is some <b><green>green\n  text</green></b>. <blue><yellow><u>This is yellow</u></yellow>, while this is blue.</blue>");

// 	log3->append( "&lt;&gt;Per <red>K<green>å<yellow>r</yellow></green>e\nbott</red><blue>olfson</blue> jr<green>.</green>" );
// 	log3->append( "<pink>Pink, <red>Red, <palegreen>Palegreen, </palegreen></red></pink>"
// 		      "<paleturquoise>Paleturquoise</paleturquoise>" );
// 	log3->append( "Per <red><blue>o</blue>lfson jr.</red>" );
// 	log3->append( "Per <red>Kåre\nbott<blue>olf</blue>son jr.</red>" );
// 	log3->append( "Insert test del<green>uxe\ninsert.</green>" );
// 	log3->append( "per <red>ole <yellow>bob\nroper</yellow> olle </red>bob");

// 	QString str;
// 	timeval st, et;
// 	gettimeofday( &st, 0 );
// 	fprintf( stderr,"\n");
// 	int cn = 0;
// 	for ( int i = 0; i < num; i++ ) {
// 	str.sprintf("%06d: <red><b>1234567890</b></red> 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n"
// 		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n"
// 		    "%06d: 1234567890 <green>1234567890</green> 1234567890 1234567890\n"
// 		    "%06d: <i>1234567890</i> 1234567890 ygpqfh 1234567890 1234567890 1234567890\n"
// 		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890\n"
// 		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890\n"
// 		    "<yellow>%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890\n"
// 		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890\n"
// 		    "</yellow>%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n"
// 		    "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890", cn, cn + 1, cn + 2, cn +3, cn+4, cn+5,cn+6,cn+7,cn+8,cn+9);
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
// 	    cn +=10;
// 	    log3->append( str );
// 	    if ( (cn % 2000) == 0 ) {
// 		fprintf(stderr,"\rLines done: %08d", i );
// 	    }
// 	}
// 	gettimeofday( &et, 0 );
// 	fprintf(stderr,"\nLoop time: %d.%03d s\n", (int)(et.tv_sec - st.tv_sec),(int)(abs(et.tv_usec - st.tv_usec)%1000000) / 1000);
// 	fprintf(stderr,"Bytes: %.2f KB\n", (float)(log3->length()/1024) + (float)(log3->length()%1024)/1000.0 );
    }

    void text1()
    {
	if ( !log3 )
	    return;
	int cn = 0;
	QString str;
// 	str.sprintf(
 	log3->setText("<u><red>The following:</red></u>\nThis is a little test with the best\nYes it ldkskkkkkkkkkkkkkkkkkkkkkkk\n" + str);
	log3->setUpdatesEnabled( FALSE );
	for ( int i = 0; i < 2; i++ ){
	log3->append( "%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n" );
	log3->append( "%06d: 1234567890 <green><b>1234567890 ygpqfh</b></green> 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n" );
	log3->append("%06d: 1234567890 1234567890 1234567890 1234567890\n" );
	log3->append("%06d: 1234567890 <i><u>1234567890</u></i> ygpqfh 1234567890 1234567890 1234567890\n");
	log3->append("%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890\n");
	log3->append("%06d: <red>1234567890</red> 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890\n");
	log3->append("%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890\n");
	log3->append("%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890\n");
	log3->append("%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n");
	log3->append("%06d: 1234567890 1234567890 ygpqfh 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890");
	}
	log3->setUpdatesEnabled( TRUE );
 	log3->scrollToBottom();
// 	    , cn, cn + 1, cn + 2, cn +3, cn+4, cn+5,cn+6,cn+7,cn+8,cn+9);
    }

    void text2()
    {
	if ( !log3 )
	    return;
	log3->setTextFormat( Qt::LogText );
//  	log3->setText("<u><red>The following:</red></u> fooooooooooooooooooff\n fofffffffffffffffffffffffffffffff\n of offfffffffffffffffffo f");
	//             012     345678901234         567890123456789012345
 	log3->setText("<blue>The<red> following: </red><b>fooooooooooooooooooff</b>\nLine <green>Number 2\nLine</green> Number 3");
	qWarning( log3->text() );
 	log3->insertParagraph("<orange>#orange#</orange>", 1);
  	log3->insertAt(" #\nper\nka<blue>ar<yellow>e\nreo</yellow>le</blue># \n", 2, 3 );
  	log3->insertAt("\n#<b>o</b><i>l</i><u>e</u>#\n\n", 100, 100);
 	log3->insertAt("<u>#o\nle#</u>", 0, 10);
 	log3->append("Dette er f;rst");
	qWarning( log3->text() );
    }
};

#define LOGOFFSET(i) (log+i) % 2147483647
int main( int argc, char ** argv )
{
    QApplication::setDesktopSettingsAware(FALSE);
    QMap<int,int> pp;
    QApplication app( argc, argv );

//     if (TRUE) {
//     	QBuffer stamp;
//  	QDataStream s(stamp.buffer(), IO_WriteOnly);
//  	s << "per";
//     }
    
    int num = 100;
    if ( argc > 1 )
 	num = QString(argv[1]).toInt();
    
//     uint log = 2147483646;
//     for(int x= 0; x< 5; x++ )
// 	qWarning("num: %d", (uint)LOGOFFSET(x) );
    
//     pp[2147483647] = 1;
//     pp[-2147483640] = 2;
//     qWarning( "dd: %d, %d", pp[2147483647], pp[-2147483640]);
    
    myf * mf = new myf( num );
    mf->show();
    app.setMainWidget( mf );
    return app.exec();
}

#include "main.moc"
