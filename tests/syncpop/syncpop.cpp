#include "syncpop.h"
#include <qpainter.h>
#include <qcursor.h>
#include <qpopupmenu.h>
#include <qapplication.h>

Main::Main(QWidget* parent, const char* name, int f) :
    QWidget(parent, name, f)
{
}

void Main::bang()
{
    static int level=0;
    debug("level %d in",level);
    level++;
    QPopupMenu p;
    p.insertItem("Nothing");
    p.insertItem("Deeper",this,SLOT(bang()));
    p.move(QCursor::pos());
    p.exec();
    level--;
    debug("level %d out",level);
}

void Main::mousePressEvent(QMouseEvent*)
{
    bang();
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
