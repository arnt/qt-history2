#include "???.h"
#include <qpainter.h>
#include <qapp.h>

Main::Main(QWidget* parent, const char* name, int f) :
    QWidget(parent, name, f)
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
}

void Main::keyReleaseEvent(QKeyEvent*)
{
}

void Main::paintEvent(QPaintEvent* e)
{
    QPainter p(this);
    p.setClipRect(e->rect());

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
