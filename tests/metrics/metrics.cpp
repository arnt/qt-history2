#include "metrics.h"
#include <qapplication.h>
#include <qfontdialog.h>
#include <qmenubar.h>
#include <qpaintdevicemetrics.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qprinter.h>
#include <qscrollview.h>
#include <qsimplerichtext.h>

Main::Main(QWidget* parent, const char* name, int f) :
    QMainWindow(parent, name, f)
{
    QScrollView* central = new QScrollView(this);
    Paper* paper = new Paper(central);
    central->addChild( paper );
    setCentralWidget(central);

    QPopupMenu *file = new QPopupMenu( this );
    file->insertItem( "&Print", paper, SLOT(print()), CTRL+Key_P );
    file->insertSeparator();
    file->insertItem( "E&xit", qApp, SLOT(quit()), CTRL+Key_Q );
    menuBar()->insertItem("&File", file);

    QPopupMenu *view = new QPopupMenu( this );
    view->insertItem( "&Upper font...", paper, SLOT(changeFont1()) );
    view->insertItem( "&Lower font...", paper, SLOT(changeFont2()) );
    view->insertSeparator();
    view->insertItem( "&A5 paper", paper, SLOT(sizeToA5()) );
    view->insertItem( "&A4 paper", paper, SLOT(sizeToA4()) );
    menuBar()->insertItem("&View", view);
}

Paper::Paper(QWidget* parent, const char* name, int f) :
    QWidget(parent, name, f)
{
    setBackgroundMode(PaletteBase);
    font1 = QFont("Arial", 48);
    font2 = QFont("Times new roman", 10);

    sizeToA4();
}

Paper::~Paper()
{
}

static ushort some_unicode[] = {
    0xfeff, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x000a, 0x00e1,
    0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7, 0x00e8, 0x00e9,
    0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef, 0x00f0, 0x00f1,
    0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f8, 0x00f9, 0x00e1,
    0x000a, 0x3042, 0x3044, 0x3046, 0x3048, 0x304a, 0x000a, 0x304b,
    0x304d, 0x304f, 0x3051, 0x3053, 0x000a, 0x3055, 0x3057, 0x3059,
    0x305b, 0x305d, 0x000a, 0x0419, 0x0426, 0x0423, 0x041a, 0x0415,
    0x041d, 0x000a, 0x0439, 0x0446, 0x0443, 0x043a, 0x0435, 0x043d
};

void Paper::changeFont1()
{
    QFont nf = QFontDialog::getFont( 0, font1, this );
    if ( nf != font1 ) {
	font1 = nf;
	update();
    }
}

void Paper::changeFont2()
{
    QFont nf = QFontDialog::getFont( 0, font2, this );
    if ( nf != font2 ) {
	font2 = nf;
	update();
    }
}

void Paper::sizeToA5()
{
    QPaintDeviceMetrics metrics(this);
    setFixedSize( 297/2*10*metrics.logicalDpiX()/254,
	          210*10*metrics.logicalDpiY()/254 );
    ps = QPrinter::A5;
}

void Paper::sizeToA4()
{
    QPaintDeviceMetrics metrics(this);
    setFixedSize( 210*10*metrics.logicalDpiX()/254,
	          297*10*metrics.logicalDpiY()/254 );
    ps = QPrinter::A4;
}

void Paper::print()
{
    QPrinter printer;
    printer.setFullPage(TRUE);
    printer.setPageSize(ps);
    if ( printer.setup() ) {
	QPainter p(&printer);
	paint(p);
    }
}

void Paper::paint(QPainter& p)
{
    QPaintDeviceMetrics metrics(p.device());

    int width = int(metrics.width()-metrics.logicalDpiX()*2.0);
    int height = int(metrics.height()-metrics.logicalDpiY()*2.0);
    p.setViewport( int(metrics.logicalDpiX()*1.0),
		 int(metrics.logicalDpiY()*1.0),
		 width, height );
    const int scale=2;
    width *= scale;
    height *= scale;
    p.setWindow( 0, 0, width, height );
    QFont font1_scaled = font1; font1_scaled.setPointSize(font1.pointSize()*scale);
    QFont font2_scaled = font2; font2_scaled.setPointSize(font2.pointSize()*scale);

    QString text = "Text";

    p.setFont(font1_scaled);
    QRect r = p.boundingRect(0,0,0,0,DontClip,text);
    p.drawRect(r);
    QRect body(0, 0, width, height );
    p.drawRect(body);
    p.drawText(r,DontClip,text);

    p.setFont(font2_scaled);
    QFontMetrics fm = p.fontMetrics();

    static const char* word[] = {
	"These ",
	"lines ",
	"may ",
	"be ",
	"spaced ",
	"differently. ",
	0
    };

    int x = body.left();
    for ( int i = 0; word[i]; i++ ) {
	p.drawText(x, r.bottom()+fm.lineSpacing(), word[i]);
	x += fm.width(word[i]);
    }
    p.drawText(body.left(), r.bottom()+fm.lineSpacing()*2,
	"These lines may be spaced differently. ");


    QString html =
	"<h1>Rich Text</h1>"
	"<p>The Qt <i>rich text</i> features allow you to use many "
	"HTML tags to improve the presentation of text information.<br>";
    html += QString().setUnicodeCodes(some_unicode,sizeof(some_unicode)/sizeof(ushort));
    QSimpleRichText rtext(html, font2_scaled);
    rtext.setWidth(&p,body.width());
    rtext.draw(&p, body.left(), r.bottom()+fm.lineSpacing()*2, body, palette().active());
}

void Paper::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    paint(p);
}

main(int argc, char** argv)
{
    QApplication app(argc, argv);

    Main m;
    app.setMainWidget(&m);
    m.show();

    return app.exec();
}
