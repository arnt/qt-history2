#include "recreate.h"
#include <qpainter.h>
#include <qapp.h>

Main::Main(QWidget* parent, const char* name, int f) :
    QLabel("Press key to recreate", parent, name, f)
{
}

void Main::bang()
{
}

void Main::resizeEvent(QResizeEvent*)
{
}

void Main::keyPressEvent(QKeyEvent*)
{
    recreate(0,0,QPoint(0,0),isVisible());
}

void Main::keyReleaseEvent(QKeyEvent*)
{
}

void Main::paintEvent(QPaintEvent* e)
{
    QLabel::paintEvent(e);

    //QPainter p(this);
    //p.setClipRect(e->rect());

    // ...
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
