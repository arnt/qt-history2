#include <qapplication.h>
#include <qpainter.h>

#include "qrtstring.h"
#include "qtextlayout.h"

#include <private/qcomplextext_p.h>
#include <qdatetime.h>

#if 1
class MyWidget : public QWidget
{
public:
    MyWidget( QWidget *parent = 0,  const char *name = 0);

    QRTString string;
protected:
    void paintEvent( QPaintEvent *e);

};


MyWidget::MyWidget( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
}


void MyWidget::paintEvent( QPaintEvent * )
{

    QPainter p( this );

    int x = 10, y = 50;
    // just for testing we draw char by char
    QRTFormat oldformat;
    for ( int i = 0; i < string.length(); i++ ) {
	QRTFormat format = string.format(i);
	if ( i == 0 || format != oldformat ) {
	    qDebug("format change at pos %d", i );
	}

	p.setFont( format.font() );
	p.setPen( format.color() );

	p.drawText( x, y, string.str(), i, 1 );
	x += p.fontMetrics().charWidth( string.str(), i );
	oldformat = format;
    }

}

#endif

//const char * s = "אי U יו";

const char * s = "אירופה, תוכנה והאינטרנט: Unicode יוצא לשוק העולמי הירשמו כעת לכנס Unicode הבינלאומי העשירי, שייערך בין התאריכים 12־10 במרץ 1997, במיינץ שבגרמניה. בכנס ישתתפו מומחים מכל ענפי התעשייה בנושא האינטרנט העולמי וה־Unicode, בהתאמה לשוק הבינלאומי והמקומי, ביישום Unicode במערכות הפעלה וביישומים, בגופנים, בפריסת טקסט ובמחשוב רב־לשוני. some english inbetween כאשר העולם רוצה לדבר, הוא מדבר ב־Unicode";


int main( int argc, char **argv )
{
    QApplication a(argc, argv);
#if 0
    MyWidget *w = new MyWidget;
    w->resize( 300,  300 );
    w->show();
    a.setMainWidget ( w );

    QRTString string("This is a test string");

    QFont f;
    f.setPointSize( 24 );
    QRTFormat fmt( f, Qt::red );
    string.setFormat( fmt, 5, 5 );
    QRTFormat fmt2( f,  Qt::blue );
    QRTString str2("foo", fmt2 );
    string.insert( 20, str2 );

    w->string = string;
    a.exec();
    delete w;
#endif

    {
	QRTString string = QString::fromUtf8( s );

#if 0
	ScriptItemArray items;
	items.itemize( string );
	qDebug("itemization: ");
	for ( int i = 0; i < items.size(); i++ ) {
	    qDebug("    (%d): start: %d, level: %d, script: %d", i, items[i].position, items[i].analysis.bidiLevel,
		   items[i].analysis.script );
	}
#else
	QTime t;
	t.start();
	ScriptItemArray items;
	for ( int i = 0; i < 1000; i++ ) {
	    items.itemize( string );
	}
	qDebug("itemize: %dms", t.elapsed() );
	t.start();
	for ( int i = 0; i < 1000; i++ ) {
	    QString str = QComplexText::bidiReorderString( string.str() );
	}
	qDebug("itemize: %dms", t.elapsed() );
#endif
    }

    qDebug("at exit:");
    QRTFormat::statistics();
}
