#include <qapplication.h>
#include <qpainter.h>

#include "qrtstring.h"
#include "qtextlayout.h"

#include <private/qcomplextext_p.h>
#include <qdatetime.h>

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
    QFont f("Tahoma");
    f.setPointSize( 24 );
    p.setFont( f );
    p.drawText( 10, 100, string );
}

//const char *s = "some string";
//const char * s = "אי U יו";

//const char * s = "אירופה, תוכנה והאינטרנט: Unicode יוצא לשוק העולמי הירשמו כעת לכנס Unicode הבינלאומי העשירי, שייערך בין התאריכים 12־10 במרץ 1997, במיינץ שבגרמניה. בכנס ישתתפו מומחים מכל ענפי התעשייה בנושא האינטרנט העולמי וה־Unicode, בהתאמה לשוק הבינלאומי והמקומי, ביישום Unicode במערכות הפעלה וביישומים, בגופנים, בפריסת טקסט ובמחשוב רב־לשוני. some english inbetween כאשר העולם רוצה לדבר, הוא מדבר ב־Unicode אירופה, תוכנה והאינטרנט: Unicode יוצא לשוק העולמי הירשמו כעת לכנס Unicode הבינלאומי העשירי, שייערך בין התאריכים 12־10 במרץ 1997, במיינץ שבגרמניה. בכנס ישתתפו מומחים מכל ענפי התעשייה בנושא האינטרנט העולמי וה־Unicode, בהתאמה לשוק הבינלאומי והמקומי, ביישום Unicode במערכות הפעלה וביישומים, בגופנים, בפריסת טקסט ובמחשוב רב־לשוני. some english inbetween כאשר העולם רוצה לדבר, הוא מדבר ב־Unicode אירופה, תוכנה והאינטרנט: Unicode יוצא לשוק העולמי הירשמו כעת לכנס Unicode הבינלאומי העשירי, שייערך בין התאריכים 12־10 במרץ 1997, במיינץ שבגרמניה. בכנס ישתתפו מומחים מכל ענפי התעשייה בנושא האינטרנט העולמי וה־Unicode, בהתא";
//const char * s = "אירופה, תוכנה והאינטרנט: Unicode";


const char *s = "أوروبا, برمجيات الحاسوب + انترنيت: some english אירופה, תוכנה והאינטרנט";



int main( int argc, char **argv )
{
    QApplication a(argc, argv);

    MyWidget *w = new MyWidget;
    w->resize( 600,  300 );
    w->show();
    a.setMainWidget ( w );


    {
	QString string = QString::fromUtf8( s );
#if 1
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
}
