#include <qapplication.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qdatetime.h>
#include <qfontmetrics.h>
#include <qpixmap.h>

QString str1 = "&String &with &underline";
QString str2 = "a\tb\tc\td\te";
QString str3 = "Some rather long string without line breaks";
QString str4 = "Some rather long string with\na forced line break";
QString str5 = "Some rather long string without line breaks";

QString rstr1 = QString::fromUtf8( "&אירופה, &תוכנ&ה &והאי�" );
QString rstr2 = QString::fromUtf8("א\tי\tר\tו\tפה");
QString rstr3 = QString::fromUtf8( "אירופה, תוכנה והאינטרנט: Unicode יוצא לשוק העולמי הירשמו כעת לכנס" );
QString rstr4 = QString::fromUtf8( "אירופה, תוכנה והאינטרנט:\nUnicode יוצא לשוק העולמי הירשמו כעת לכנס" );

QString rstr5 = QString::fromUtf8( "אירופה, תוכנה והאינטרנט: Unicode יוצא לשוק העולמי הירשמו כעת לכנס" );


static inline void draw( QPainter &p, int x, int y, int w, int h, int flags, const QString &str )
{
    p.drawRect( x, y, w, h);
    p.drawText( x, y, w, h, flags, str );
    p.setPen( Qt::red );
    QRect br = p.boundingRect( x, y, w, h, flags, str );
    p.drawRect( br );
    p.drawLine( br.topLeft(),  br.bottomRight() );
    p.drawLine( br.topRight(),  br.bottomLeft() );
    p.setPen( Qt::black );
}

class MyWidget: public QWidget
{
public:
    MyWidget() : rot( 0 ), x( 0 ), y( 0 ), scale( 1. ), rtl( FALSE ), opaque( FALSE ) {}
    void paintEvent( QPaintEvent * )
	{
	    QPainter p( this );
	    if ( opaque ) {
		p.setBackgroundColor( Qt::yellow );
		p.setBackgroundMode( Qt::OpaqueMode );
	    }
	    p.rotate( rot );
	    p.translate( x,  y );
	    p.scale( scale, scale );
	    draw( p, 10, 10, 180, 80, Qt::ShowPrefix, rtl ? rstr1 : str1 );
	    draw( p, 210, 10, 180, 80, Qt::AlignLeft|Qt::ShowPrefix, rtl ? rstr1 : str1 );
	    draw( p, 410, 10, 180, 80, Qt::AlignCenter|Qt::ShowPrefix, rtl ? rstr1 : str1 );
	    draw( p, 610, 10, 180, 80, Qt::AlignRight|Qt::ShowPrefix, rtl ? rstr1 : str1 );

	    int tabs[5] = { 20, 100, 120, 150, 0 };
	    p.setTabArray( tabs );
	    draw( p, 10, 110, 180, 80, Qt::ExpandTabs, rtl ? rstr2 : str2 );
	    draw( p, 210, 110, 180, 80, Qt::AlignLeft|Qt::ExpandTabs, rtl ? rstr2 : str2 );
	    draw( p, 410, 110, 180, 80, Qt::AlignCenter|Qt::ExpandTabs, rtl ? rstr2 : str2 );
	    draw( p, 610, 110, 180, 80, Qt::AlignRight|Qt::ExpandTabs, rtl ? rstr2 : str2 );

	    p.setPen( Qt::blue );
	    for ( int i = 0; i < 4; i++ ) {
		int x = rtl ? 10+180-tabs[i] : tabs[i]+10;
		p.drawLine( x, 110, x, 110+80 );
	    }
	    p.setPen( Qt::black );
	    draw( p, 10, 210, 180, 80, Qt::WordBreak, rtl ? rstr3 : str3 );
	    draw( p, 210, 210, 180, 80, Qt::AlignLeft|Qt::WordBreak, rtl ? rstr3 : str3 );
	    draw( p, 410, 210, 180, 80, Qt::AlignCenter|Qt::WordBreak, rtl ? rstr3 : str3 );
	    draw( p, 610, 210, 180, 80, Qt::AlignRight|Qt::WordBreak, rtl ? rstr3 : str3 );

	    draw( p, 10, 310, 180, 80, 0, rtl ? rstr4 : str4 );
	    draw( p, 210, 310, 180, 80, Qt::AlignLeft, rtl ? rstr4 : str4 );
	    draw( p, 410, 310, 180, 80, Qt::AlignCenter, rtl ? rstr4 : str4 );
	    draw( p, 610, 310, 180, 80, Qt::AlignRight, rtl ? rstr4 : str4 );

	    draw( p, 10, 410, 180, 80, Qt::SingleLine, rtl ? rstr3 : str3 );
	    draw( p, 210, 410, 180, 80, Qt::AlignLeft|Qt::SingleLine, rtl ? rstr3 : str3 );
	    draw( p, 410, 410, 180, 80, Qt::AlignCenter|Qt::SingleLine, rtl ? rstr3 : str3 );
	    draw( p, 610, 410, 180, 80, Qt::AlignRight|Qt::SingleLine, rtl ? rstr3 : str3 );

	    p.drawText( 10, 510, rtl ? rstr1 : str1 );
	    p.drawText( 210, 510, rtl ? rstr1 : str1 );
	    p.drawText( 410, 510, rtl ? rstr1 : str1 );
	    p.drawText( 610, 510, rtl ? rstr1 : str1 );


	}

    void keyPressEvent( QKeyEvent *e ) {
	switch( e->key() ) {
	case Key_Plus:
	    rot += 10;
	    break;
	case Key_Minus:
	    rot -= 10;
	    break;
	case Key_PageUp: {
	    QFont f = font();
	    f.setPointSize( f.pointSize()+1 );
	    setFont( f );
	    break;
	}
	case Key_PageDown: {
	    QFont f = font();
	    f.setPointSize( f.pointSize()-1 );
	    setFont( f );
	    break;
	}
	case Key_R:
	    rtl = !rtl;
	    break;
	case Key_Right:
	    x += 10;
	    break;
	case Key_Left:
	    x -= 10;
	    break;
	case Key_Up:
	    y -= 10;
	    break;
	case Key_Down:
	    y += 10;
	    break;
	case Key_Home:
	    scale *= 1.2;
	    break;
	case Key_End:
	    scale /= 1.2;
	    break;
	case Key_O:
	    opaque = !opaque;
	    break;
	}

	update();
    }
    int rot;
    int x, y;
    double scale;
    bool rtl;
    bool opaque;
};


class MyWidget2: public QWidget
{
public:
    MyWidget2() : scale( 1. ) {}
    void paintEvent( QPaintEvent * )
	{
	    QPainter p( this );
	    p.scale( scale, scale );

	    QFont fnt;
	    fnt.setPointSize( fnt.pointSize() /scale );
	    p.setFont( fnt );

	    QString str = "scale is %1, pointSize = %2";
	    str = str.arg( scale ).arg( fnt.pointSize() );
	    p.drawText((int)(50/scale), (int)(height()/2/scale), str );

	}

    void keyPressEvent( QKeyEvent *e ) {
	switch( e->key() ) {
	case Key_Plus:
	    scale *= 2;
	    break;
	case Key_Minus:
	    scale /= 2;
	    break;
	}

	update();
    }
    double scale;
};


QString latinString =
"KDE is a powerful Open Source graphical desktop environment for Unix workstations. It combines ease of use, contemporary functionality, and out";
;

QString i18nString = QString::fromUtf8(
"ทำไมเขาถึงไม่พูด "
"ܠܡܢܐܠܐܡܡܠܠܝܢܣܘܪܝܝܐ "
"أوروبا, برمجيات الحاسوب "
"תוכנה והאינטרנט "
"रूस के राष्ट्रपति "
"অাবার অাসিব ফিরে "
"रूस के राष्ट्रपति "
"לְמָה לָא יאםרוּן" );

const int loops = 2000;

static void timeSpeed()
{
    QFont fnt;
    QFontMetrics fm( fnt );

    QPixmap pm( 500,  500 );
    QPainter p( &pm );

    QTime t;

    qDebug("\n\ntesting speed of drawing for %s", QT_VERSION_STR );

    qDebug("------------------------------------------------------------------\n" );

//     qDebug("string lengths are: latin=%d i18n=%d", latinString.length(), i18nString.length() );

    for ( int test = 0; test < 2; test++ ) {
	QString str;
	if ( test == 0 ) {
	    qDebug("\nTesting for Latin text:\n");
	    str = latinString;
	} else {
	    qDebug("\nTesting for i18n text:\n");
	    str = i18nString;
	}
// 	qDebug("string = '%s'",  str.utf8().data() );
	fm.width( str );

#if 1
	qDebug("    Font Metrics:");
	t.start();
	int w = 0;
	const QChar *qch = str.unicode();
	for ( int i = 0; i < loops; i++ ) {
	    const QChar *ch = qch + str.length();
	    while ( ch-- > qch )
		fm.width( *qch );
	}
	qDebug("        width, QChar\t\t\t%02.2f us/char",
	       ((float)t.elapsed())/loops/str.length()*1000 );

#if QT_VERSION >= 300
	t.start();
	for ( int i = 0; i < loops; i++ ) {
	    const QChar *ch = qch + str.length();
	    while ( ch-- > qch )
		w += fm.charWidth( str, ch-qch );
	}
	qDebug("        charWidth\t\t\t%02.2f us/char",
	       ((float)t.elapsed())/loops/str.length()*1000 );
// 	qDebug("w = %d", w );
#endif

	t.start();
	int w2 = 0;
	for ( int i = 0; i < loops; i++ )
	    w2 += fm.width( str );
	qDebug("        width, QString\t\t\t%02.2f us/char",
	       ((float)t.elapsed())/loops/str.length()*1000 );
// 	qDebug("w2 = %d", w2 );
#if QT_VERSION >= 300
	Q_ASSERT( w2 == w );
#endif

	t.start();
	for ( int i = 0; i < loops; i++ )
	    fm.boundingRect( 0, 0, 500, 500, Qt::SingleLine, str );
	qDebug("        boundingRect, Qt::SingleLine\t%02.2f us/char",
	       ((float)t.elapsed())/loops/str.length()*1000 );

	t.start();
	for ( int i = 0; i < loops; i++ )
	    fm.boundingRect( 0, 0, 500, 500, Qt::WordBreak, str );
	qDebug("        boundingRect, Qt::WordBreak\t%02.2f us/char",
	       ((float)t.elapsed())/loops/str.length()*1000 );
#endif

	qDebug("    QPainter drawText:");
	t.start();
	for ( int i = 0; i < loops; i++ )
	    p.drawText( 0, 100, str );
	qDebug("        simple\t\t\t\t%02.2f us/char",
	       ((float)t.elapsed())/loops/str.length()*1000 );

#if 1
	t.start();
	for ( int i = 0; i < loops; i++ )
	    p.drawText( 0, 0, 500, 500, Qt::SingleLine, str );
	qDebug("        Qt::SingleLine\t\t\t%02.2f us/char",
	       ((float)t.elapsed())/loops/str.length()*1000 );
	t.start();
	for ( int i = 0; i < loops; i++ )
	    p.drawText( 0, 0, 500, 500, Qt::WordBreak, str );
	qDebug("        Qt::WordBreak\t\t\t%02.2f us/char",
	       ((float)t.elapsed())/loops/str.length()*1000 );
#endif
    }
}


int main( int argc, char** argv )
{

    QApplication app( argc, argv );
    app.connect( &app, SIGNAL(lastWindowClosed()), SLOT(quit()) );


    int mode = 0;
    if ( argc == 2 ) {
	if ( strcmp( argv[1], "-big" ) == 0 )
	    mode = 1;
	else if ( strcmp( argv[1], "-draw" ) == 0 )
	    mode = 0;
	else if ( strcmp( argv[1], "-speed" ) == 0 )
	    mode = 2;
    }

    QWidget *w;
    if ( mode == 0 )
	w = new MyWidget;
    else if ( mode == 1 )
	w = new MyWidget2;
    else if ( mode == 2 ) {
	timeSpeed();
	return 0;
    }

    w->resize( 800,  600 );
    w->show();
    return app.exec();
}
