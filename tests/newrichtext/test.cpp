#include <qapplication.h>
#include <qpainter.h>

#include "qfontdatabase.h"

#include <private/qcomplextext_p.h>
#include <qdatetime.h>
#include "editwidget.h"

// const char *family = "Arial Unicode Ms"; // generic
const char *family = "Diwani Letter,Verdana,Latha,Akaash,Serto Jerusalem,Mangal,Rama"; // Devanagari
// const char *family = ""; // arabic
// const char *family = ""; // syriac
// const char *family = ""; // Bengali
// const char *family = ""; // Tamil

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
const char *s = "रूस के राष्ट्रपति व्लादिमीर पुतिन ने बीजिंग पहुँचकर चीन के राष्ट्रपति जियांग ज़ेमिन से बातचीत की. बातचीत के बाद संयुक्त घोषणा में रूस और चीन ने उत्तर कोरिया, इराक़ और द्विपक्षीय मामलों पर अपना पक्ष रखा. रूस के राष्ट्रपति व्लादिमीर पुतिन ने बीजिंग पहुँचकर चीन के राष्ट्रपति जियांग ज़ेमिन से बातचीत की. बातचीत के बाद संयुक्त घोषणा में रूस और चीन ने उत्तर कोरिया, इराक़ और द्विपक्षीय मामलों पर अपना पक्ष रखा.";


// Bengali
// const char * s = "অাবার অাসিব ফিরে ধানসিড়িটির তীরে - এই বাংলায় হয়তো মানুষ নয় - হয়তো বা শঙ্খচিল শালিখের বেশে, হয়তো ভোরের কাক হয়ে এই কার্তিকের নবান্নের দেশে  কুয়াশার বুকে ভেসে একদিন অাসিব এই কাঁঠাল - ছায়ায়, হয়তো বা হাঁস হব - কিশোরীর - ঘুঙুর রহিবে লাল পায়ে, সারাদিন কেটে যাবে কলমীর গন্ধ ভরা জলে ভেসে ভেসে, অাবার অাসিব অামি বাংলার নদী মাঠ ক্ষেত ভালোবেসে জলঙ্গীর ঢেউয়ে ভেজা বাংলার এ সবুজ করুণ ডাঙায়";

// mixed
// const char *s =
// "Thai: ทำไมเขาถึงไม่พูด "
// "Syriac: ܠܡܢܐܠܐܡܡܠܠܝܢܣܘܪܝܝܐ "
// "Arabic: أوروبا, برمجيات الحاسوب "
// "Hebrew: תוכנה והאינטרנט "
// "Devanagari: रूस के राष्ट्रपति "
// "Bengali: অাবার অাসিব ফিরে "
// ;

// latin
// const char *s =
// "KDE is a powerful Open Source graphical desktop environment for Unix workstations. It combines ease of use, contemporary functionality, and outstanding graphical design with the technological superiority of the Unix operating system. KDE is an Internet project that is truly open in every sense. Development takes place on the Internet and is discussed on our mailing lists, USENET news groups, and IRC channels to which we invite and welcome everyone."
// ;



// const char *s = "";

int main( int argc, char **argv )
{
    QApplication a(argc, argv);

    QFontDatabase fdb;

    return 0;

    QFont f( family );
    f.setPointSize( 24 );
    a.setFont( f );

#if 0
    QString str = QString::fromUtf8( s );
    QTime t;
    t.start();
    QFontMetrics fm( f );
    (void)fm.width( str );
    qDebug("loading took %d ms",  t.elapsed() );

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

    QFontMetrics( f ).boundingRect( 0, 0, 50000, 1000, Qt::SingleLine, QString::fromUtf8( s ) );
    QTime t;
    t.start();
    QFontMetrics( f ).boundingRect( 0, 0, 50000, 1000, Qt::WordBreak, QString::fromUtf8( s ) );
    qDebug("fm.boundingrect() took %dms (%dus/char)", t.elapsed(), t.elapsed()*1000/QString::fromUtf8( s ).length() );

    EditWidget *w = new EditWidget( 0, 0 );
    w->setText( QString::fromUtf8( s ) );
    w->resize( 600, 300 );
    w->show();
    a.setMainWidget ( w );

    a.exec();
    delete w;
}
