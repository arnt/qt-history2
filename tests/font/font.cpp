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
    repaint();
}

void Main::paintEvent(QPaintEvent* e)
{
    QPainter p(this);
    p.setClipRect(e->rect());
    char str[]=" - The quick brown fox jumps over the lazy dog.";
    static const int nfamily=1;
    static char* family[nfamily]={ "helvetica" };
    str[0]=rand();
    char* fam = family[rand()%nfamily];
    int pt = 20+rand()%300;
    QString msg;
    msg.sprintf("Trying char %d in %s at %dpt",str[0],fam,pt);
    p.drawText(rect(),0,msg);
    QFont f(fam,pt);

    p.setFont(f);
    p.drawText(rect(),0,str);
    msg.sprintf("Trying %s at %dpt - DONE",fam,pt);
    p.drawText(rect(),0,msg);

    //f.handle();
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
