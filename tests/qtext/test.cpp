#include <qapplication.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qfont.h>

#include <math.h>
#include "test.h"

const char *testStr1 = "This page contains text in many languages, advertising the Tenth International Unicode Conference.  The page is encoded using Unicode UTF-8 and to view it you need a Unicode-capable browser.  If you don't have access to such a browser, you may want to visit the individual Web pages, each containing the text in a single language. The text on these Web pages was provided by volunteer translators (see the Credits page).  If you would like to offer a language we don't yet have, or have a comment about one of the existing pages, please mail us.";

#if 1
const char *testStr = "أوروبا, برمجيات الحاسوب + انترنيت : some english تصبح عالميا مع يونيكود more english تسجّل الآن لحضور المؤتمر الدولي العاشر ليونيكود, الذي سيعقد في 10-12 آذار 1997 بمدينة ماينتس, ألمانيا. وسيجمع المؤتمر بين خبراء من  كافة قطاعات الصناعة على الشبكة العالمية انترنيت ويونيكود, حيث ستتم, على الصعيدين الدولي والمحلي على حد سواء مناقشة سبل استخدام يونكود  في النظم القائمة وفيما يخص التطبيقات الحاسوبية, الخطوط, تصميم النصوص  والحوسبة متعددة اللغات. english عندما يريد العالم أن يتكلّم, فهو يتحدّث بلغة يونيكود. english... now some hebrew: אירופה, תוכנה והאינטרנט: Unicode יוצא לשוק העולמי הירשמו כעת לכנס Unicode הבינלאומי העשירי, שייערך בין התאריכים 12־10 במרץ 1997, במיינץ שבגרמניה. בכנס ישתתפו מומחים מכל ענפי התעשייה בנושא האינטרנט העולמי וה־Unicode, בהתאמה לשוק הבינלאומי והמקומי, ביישום Unicode במערכות הפעלה וביישומים, בגופנים, בפריסת טקסט ובמחשוב רב־לשוני. some english inbetween כאשר העולם רוצה לדבר, הוא מדבר ב־Unicode";
#else
const char *testStr = "english אירופה, תוכנה והאינטרנט: english english אירופה, תוכנה והאינטרנט: english english אירופה, תוכנה והאינטרנט: english english אירופה, תוכנה והאינטרנט: english ";
#endif


MyArea::MyArea()
    : QTextArea()
{
}

int MyArea::lineWidth(int x, int y, int h = 0) const
{
    if(y>100 && y< 500) {
	return 700-2*(sqrt(40000- (y-300)*(y-300)) );
    }  else {
	return 700;
    }
}

QRect MyArea::lineRect(int x, int y, int h) const
{
    if(y>100 && y< 700) {
	return QRect(sqrt(40000- (y-300)*(y-300)) , y, 700-2*(sqrt(40000- (y-300)*(y-300)) ), h);
    }  else {
	return QRect(0, y, 700, h);
    }
}


MyView::MyView()
    : QWidget()
{
    area = new MyArea();
    resize(500, 400);

    QRichTextFormatCollection fc;
    QRichTextFormat *f = fc.defaultFormat();
    
    QString aStr = testStr1;
    aStr += aStr;

    area->appendParagraph( QRichTextString(aStr, f) );

    aStr = QString::fromUtf8(testStr);

    area->appendParagraph( QRichTextString(aStr, f) );
    //   area->appendParagraph(aStr);
}


void MyView::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    area->paint(&p, 0, 50);
}


int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    QWidget *w = new MyView();
    QFont f("tahoma");
    f.setCharSet(QFont::Unicode);
    w->setFont(f);

    a.setMainWidget(w);

    w->show();

    return a.exec();
}
