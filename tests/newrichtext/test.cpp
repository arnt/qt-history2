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

    QString string;
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
// 	QRTFormat format = string.format(0);
// 	p.setFont( format.font() );
// 	p.setPen( format.color() );

#if 0
	ScriptItemArray items;
	TextLayout::instance()->itemize( items, string );
	qDebug("itemization: ");
	for ( int i = 0; i < items.size(); i++ ) {
	    qDebug("    (%d): start: %d, level: %d, script: %d", i, items[i].position, items[i].analysis.bidiLevel,
		   items[i].analysis.script );
	}

	unsigned char levels[256];
	int visualOrder[256];
	for ( int i = 0; i < items.size(); i++ )
	    levels[i] = items[i].analysis.bidiLevel;
	TextLayout::instance()->bidiReorder( items.size(), (unsigned char *)levels, (int *)visualOrder );

	int x = 0;
	int y = 30;
	QString str = string.qstring();
	for ( int i = 0; i < items.size(); i++ ) {
	    int current = visualOrder[i];
	    const ScriptItem &it = items[ current ];
	    ShapedItem shaped;
	    TextLayout::instance()->shape( shaped, string, items, current );
	    str = *(QString *)shaped.d;

	    int pos = it.position;
	    int length = str.length();
	    int xold = x;
	    qDebug("visual %d x=%d length=%d", visualOrder[i], x,  length);
	    for ( int j = 0; j < length; j++ ) {
		x += p.fontMetrics().width( str[j] );
		p.setPen( Qt::red );
		p.drawLine( xold, y, x, y );
		p.setPen( Qt::black );
		p.drawText( xold, y, str, j, 1,
			    it.analysis.bidiLevel % 2 ? QPainter::RTL : QPainter::LTR );
		xold = x;
		y += 2;  //j % 2 ? 4 : -4;
	    }

	    y += 20;
	}

#else

	p.drawText( 10, 50, string );

#endif
}

#endif

//const char *s = "some string";
//const char * s = "אי U יו";

//const char * s = "אירופה, תוכנה והאינטרנט: Unicode יוצא לשוק העולמי הירשמו כעת לכנס Unicode הבינלאומי העשירי, שייערך בין התאריכים 12־10 במרץ 1997, במיינץ שבגרמניה. בכנס ישתתפו מומחים מכל ענפי התעשייה בנושא האינטרנט העולמי וה־Unicode, בהתאמה לשוק הבינלאומי והמקומי, ביישום Unicode במערכות הפעלה וביישומים, בגופנים, בפריסת טקסט ובמחשוב רב־לשוני. some english inbetween כאשר העולם רוצה לדבר, הוא מדבר ב־Unicode אירופה, תוכנה והאינטרנט: Unicode יוצא לשוק העולמי הירשמו כעת לכנס Unicode הבינלאומי העשירי, שייערך בין התאריכים 12־10 במרץ 1997, במיינץ שבגרמניה. בכנס ישתתפו מומחים מכל ענפי התעשייה בנושא האינטרנט העולמי וה־Unicode, בהתאמה לשוק הבינלאומי והמקומי, ביישום Unicode במערכות הפעלה וביישומים, בגופנים, בפריסת טקסט ובמחשוב רב־לשוני. some english inbetween כאשר העולם רוצה לדבר, הוא מדבר ב־Unicode אירופה, תוכנה והאינטרנט: Unicode יוצא לשוק העולמי הירשמו כעת לכנס Unicode הבינלאומי העשירי, שייערך בין התאריכים 12־10 במרץ 1997, במיינץ שבגרמניה. בכנס ישתתפו מומחים מכל ענפי התעשייה בנושא האינטרנט העולמי וה־Unicode, בהתא";
const char * s = "אירופה, תוכנה והאינטרנט: Unicode";


//const char *s = "أوروبا, برمجيات الحاسوب + انترنيت : some english تصبح";



int main( int argc, char **argv )
{
    QApplication a(argc, argv);

    MyWidget *w = new MyWidget;
    w->resize( 600,  300 );
    w->show();
    a.setMainWidget ( w );


    {
	QString string = QString::fromUtf8( s );
	//qDebug("string length=%d",  string.qstring().length() );
#if 1
// 	string.setFormat( QRTFormat( QFont( "Arial", 24 ), Qt::black ) );
// 	const TextLayout *textLayout = TextLayout::instance();
	w->string = string;
#else
	QTime t;
	t.start();
	const TextLayout * const textLayout = TextLayout::instance();
	for ( int i = 0; i < 1000; i++ ) {
	    ScriptItemArray items;
	    textLayout->itemize( items, string );
	}
	qDebug("itemize: %dms", t.elapsed() );
// 	t.start();
// 	for ( int i = 0; i < 1000; i++ ) {
// 	    QString str = QComplexText::bidiReorderString( string.qstring() );
// 	}
// 	qDebug("itemize: %dms", t.elapsed() );
#endif
    }

    a.exec();
    delete w;

    qDebug("at exit:");
    QRTFormat::statistics();
}
