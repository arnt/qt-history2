#include "metrics.h"
#include <qapplication.h>
#include <qfontdialog.h>
#include <qmenubar.h>
#include <qpaintdevicemetrics.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qprinter.h>
#include <qscrollview.h>

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
    font1 = QFont("Helvetica", 24);
    font2 = QFont("Times", 12);
    sizeToA4();
}

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
    int dpix = metrics.logicalDpiX();
    int dpiy = metrics.logicalDpiY();

    const int margin = 72; // pt
    QString text = "Text";

    p.setFont(font1);
    QRect r = p.boundingRect(0,0,0,0,DontClip,text);
    r.moveTopLeft(QPoint(margin*dpix/72, margin*dpiy/72));
    p.drawRect(r);
    QRect full(margin*dpix/72, margin*dpiy/72,
	       metrics.width()-margin*dpix/72*2,
		metrics.height()-margin*dpiy/72*2 );
    p.drawRect(full);
    p.drawText(r,DontClip,text);

    p.setFont(font2);
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

    int x = full.left();
    for ( int i = 0; word[i]; i++ ) {
	p.drawText(x, r.bottom()+fm.lineSpacing(), word[i]);
	x += fm.width(word[i]);
    }
    p.drawText(full.left(), r.bottom()+fm.lineSpacing()*2,
	"These lines may be spaced differently. ");
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
