#include "font.h"
#include <qpainter.h>
#include <qapplication.h>
#include <stdlib.h>

Main::Main(QWidget* parent, const char* name, int f) :
    QWidget(parent, name, f)
{
    startTimer(1);
    resize(100,100);
}

void Main::bang()
{
}

void Main::resizeEvent(QResizeEvent*)
{
}

void Main::keyPressEvent(QKeyEvent*)
{
}

void Main::keyReleaseEvent(QKeyEvent*)
{
}

void Main::timerEvent(QTimerEvent*)
{
    static const int nfamily=3;
    static const char* family[nfamily]={ "courier", "times", "helvetica" };
    QFont f(family[rand()%nfamily],rand()%100);
    setFont(f);
    repaint();
}

void Main::paintEvent(QPaintEvent* e)
{
    QPainter p(this);
    p.setClipRect(e->rect());
    p.drawText(rect(),0,"The quick brown fox jumps over the lazy dog.");
}

main(int argc, char** argv)
{
    QApplication::setColorSpec( QApplication::ManyColor );
    QApplication app(argc, argv);
    QApplication::setFont( QFont("Helvetica") );

    Main m;
    m.show();

    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

    return app.exec();
}
