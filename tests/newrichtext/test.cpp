#include <qapplication.h>
#include <qpainter.h>

#include "qfontdatabase.h"

#include "qrtstring.h"
#include "qtextlayout.h"

#include <private/qcomplextext_p.h>
#include <qdatetime.h>
#include "editwidget.h"

// const char *family = "Arial Unicode Ms"; // generic
// const char *family = "Mangal"; // Devanagari
// const char *family = "Diwani Letter"; // arabic
// const char *family = "Serto Jerusalem"; // syriac
// const char *family = "Akaash"; // Bengali
const char *family = "Latha"; // Tamil

class MyWidget : public QWidget
{
public:
    MyWidget( QWidget *parent = 0,  const char *name = 0);

    QString string;
protected:
    void paintEvent( QPaintEvent *e);
    void mouseMoveEvent( QMouseEvent *e );
    void mousePressEvent( QMouseEvent *e );

    int getCursorPosition( int x );
    int cursor;
};


MyWidget::MyWidget( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    setMouseTracking( TRUE );
    cursor = 0;
}


void MyWidget::paintEvent( QPaintEvent * )
{

    QPainter p( this );
    QFont f(family);
    f.setPointSize( 48 );
    p.setFont( f );
    p.drawText( 10, 50, string );
    p.drawText( 10, 100, string, 0, 10 );
    p.drawText( 10, 150, string, 5, 10 );
}

void MyWidget::mouseMoveEvent( QMouseEvent *e )
{
//     getCursorPosition( e->x() - 10 );
}

void MyWidget::mousePressEvent( QMouseEvent *e )
{
    cursor = getCursorPosition( e->x() - 10 ) + 10;
    update();
}

int MyWidget::getCursorPosition( int _x )
{
    QFont f;
    f.setFamily( family );
    f.setPointSize( 48 );
    const TextLayout *layout = TextLayout::instance();
    ScriptItemArray items;
    layout->itemize( items, string );

    unsigned char levels[256];
    int visualOrder[256];
    int i;
    int cp = 0;
    int xcp = 0;

    for ( i = 0; i < items.size(); i++ )
	levels[i] = items[i].analysis.bidiLevel;
    layout->bidiReorder( items.size(), (unsigned char *)levels, (int *)visualOrder );

    int x = 0;

    int current;
//      qDebug("QPainter::drawText: num items=%d",  items.size() );
    for ( int i = 0; i < items.size(); i++ ) {
	current = visualOrder[i];
	ShapedItem shaped;
	layout->shape( shaped, f, string, items, current );

        cp = layout->xToCursor( shaped, _x );

	xcp = layout->cursorToX( shaped, cp, TextLayout::Leading );
	int xoff = 0;
	const Offset *advances = shaped.advances();
	int i = shaped.count();
	while ( i-- ) {
	    xoff += advances->x;
	    ++advances;
	}
	x += xoff;
	if ( _x < x )
	    break;
    }
    qDebug("cursor at position %d in item %d", items[current].position+cp, current );
    return xcp;
}


//const char *s = "some string";
//const char * s = "אי U יו";

// const char * s = "אירופה, תוכנה והאינטרנט: Unicode יוצא לשוק העולמי הירשמו כעת לכנס Unicode הבינלאומי העשירי, שייערך בין התאריכים 12־10 במרץ 1997, במיינץ שבגרמניה. בכנס ישתתפו מומחים מכל ענפי התעשייה בנושא האינטרנט העולמי וה־Unicode, בהתאמה לשוק הבינלאומי והמקומי, ביישום Unicode במערכות הפעלה וביישומים, בגופנים, בפריסת טקסט ובמחשוב רב־לשוני. some english inbetween כאשר העולם רוצה לדבר, הוא מדבר ב־Unicode אירופה, תוכנה והאינטרנט: Unicode יוצא לשוק העולמי הירשמו כעת לכנס Unicode הבינלאומי העשירי, שייערך בין התאריכים 12־10 במרץ 1997, במיינץ שבגרמניה. בכנס ישתתפו מומחים מכל ענפי התעשייה בנושא האינטרנט העולמי וה־Unicode, בהתאמה לשוק הבינלאומי והמקומי, ביישום Unicode במערכות הפעלה וביישומים, בגופנים, בפריסת טקסט ובמחשוב רב־לשוני. some english inbetween כאשר העולם רוצה לדבר, הוא מדבר ב־Unicode אירופה, תוכנה והאינטרנט: Unicode יוצא לשוק העולמי הירשמו כעת לכנס Unicode הבינלאומי העשירי, שייערך בין התאריכים 12־10 במרץ 1997, במיינץ שבגרמניה. בכנס ישתתפו מומחים מכל ענפי התעשייה בנושא האינטרנט העולמי וה־Unicode, בהתא";
//const char * s = "אירופה, תוכנה והאינטרנט: Unicode";


// const char *s = "أوروبا, برمجيات الحاسوب + انترنيت : تصبح عالميا مع يونيكود تسجّل الآن لحضور المؤتمر الدولي العاشر ليونيكود, الذي سيعقد في 10-12 آذار 1997 بمدينة ماينتس, ألمانيا. وسيجمع المؤتمر بين خبراء من  كافة قطاعات الصناعة على الشبكة العالمية انترنيت ويونيكود, حيث ستتم, على الصعيدين الدولي والمحلي على حد سواء مناقشة سبل استخدام يونكود  في النظم القائمة وفيما يخص التطبيقات الحاسوبية, الخطوط, these are some english words intermixed within the arabic text تصميم النصوص  والحوسبة متعددة اللغات. عندما يريد العالم أن يتكلّم, فهو يتحدّث بلغة يونيكود.";
// const char *s = "أوروبا, برمجيات الحاسو�";


// Thai
// const char *s = "ทำไมเขาถึงไม่พูด �าษาไทย";

// Vietnamese
// const char *s = "Tại sao họ không thể chỉ nói tiệ̣̣́ng.";

// Syriac
// const char *s = "ܠܡܢܐܠܐܡܡܠܠܝܢܣܘܪܝܝܐ";

// Devanagari
// const char *s = "रूस के राष्ट्रपति व्लादिमीर पुतिन ने बीजिंग पहुँचकर चीन के राष्ट्रपति जियांग ज़ेमिन से बातचीत की. बातचीत के बाद संयुक्त घोषणा में रूस और चीन ने उत्तर कोरिया, इराक़ और द्विपक्षीय मामलों पर अपना पक्ष रखा.";


// Bengali
// const char * s = "অাবার অাসিব ফিরে ধানসিড়িটির তীরে - এই বাংলায় হয়তো মানুষ নয় - হয়তো বা শঙ্খচিল শালিখের বেশে, হয়তো ভোরের কাক হয়ে এই কার্তিকের নবান্নের দেশে  কুয়াশার বুকে ভেসে একদিন অাসিব এই কাঁঠাল - ছায়ায়, হয়তো বা হাঁস হব - কিশোরীর - ঘুঙুর রহিবে লাল পায়ে, সারাদিন কেটে যাবে কলমীর গন্ধ ভরা জলে ভেসে ভেসে, অাবার অাসিব অামি বাংলার নদী মাঠ ক্ষেত ভালোবেসে জলঙ্গীর ঢেউয়ে ভেজা বাংলার এ সবুজ করুণ ডাঙায়";

// mixed
// const char *s = "Thai: ทำไมเขาถึงไม่พูด �าษาไทย "
// "Syriac: ܠܡܢܐܠܐܡܡܠܠܝܢܣܘܪܝܝܐ "
// "Arabic: أوروبا, برمجيات الحاسوب "
// "Hebrew: תוכנה והאינטרנט ";

const char *s = "";

int main( int argc, char **argv )
{
    QApplication a(argc, argv);

#if 0
    // QFontDatabase test
    QFontDatabase fdb;

    QStringList list1, list2;
    QStringList::const_iterator it1, end1, it2, end2;

    list1 = fdb.families();
    qDebug( "%d families", list1.count() );

    // each family
    for ( it1 = list1.begin(), end1 = list1.end(); it1 != end1; ++it1 ) {
	list2 = fdb.styles( *it1 );
	qDebug( "\n\nfamily '%s', fixed: %d, styles:",
		(*it1).latin1(), fdb.isFixedPitch( *it1, QString::null ) );

	// each style
	for ( it2 = list2.begin(), end2 = list2.end(); it2 != end2; ++it2 ) {
	    qDebug( "  %s:\n    scalable %d (smooth %d bitmap %d)",
		    (*it2).latin1(),
		    fdb.isScalable( *it1, *it2 ),
		    fdb.isSmoothlyScalable( *it1, *it2 ),
		    fdb.isBitmapScalable( *it1, *it2 ) );

	    qDebug( "    point sizes %d\n    smooth sizes %d",
		    fdb.pointSizes( *it1, *it2 ).count(),
		    fdb.smoothSizes( *it1, *it2 ).count() );
	}
    }

    return 0;
#endif

    QFont f( family );
    f.setPointSize( 38 );
    a.setFont( f );

#if 1
    EditWidget *w = new EditWidget( 0, 0 );
    w->setText( QString::fromUtf8( s ) );
    w->resize( 600, 300 );
    w->show();
    a.setMainWidget ( w );
#else

    MyWidget *w = new MyWidget;
    w->resize( 700,  300 );
    w->show();


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
#endif

    a.exec();
    delete w;
}
