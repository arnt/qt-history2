#include <qapplication.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qdatetime.h>
#include <qfontmetrics.h>

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
    MyWidget() : rot( 0 ), x( 0 ), y( 0 ), scale( 1. ), rtl( FALSE ) {}
    void paintEvent( QPaintEvent * )
	{
	    QPainter p( this );
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
	}

	update();
    }
    int rot;
    int x, y;
    double scale;
    bool rtl;
};

int main( int argc, char** argv )
{

    QApplication app( argc, argv );

    QFont fnt;
    QFontMetrics fm( fnt );

    QTime t;
    t.start();
    MyWidget w;
//     QPainter p( &w );
//     //QString str = "Some test string.Some test string.Some test string.";
//     QString str = "אירופה, תוכנה והאינטרנט: ";
//     for ( int i = 0; i < 10000; i++ ) {
// 	p.drawText( 0, 0, 500, 100, Qt::WordBreak, str );
//     }

//     qDebug("t=%dms", t.elapsed() );

    w.resize( 800,  600 );
    w.show();
    return app.exec();
    return 0;
}
