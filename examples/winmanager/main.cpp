
#include <qapplication.h>
#include <qwidget.h>
#include <qfont.h>
#include "../hello/hello.h"
#include "minimal.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QFont f(app.font());
    f.setPointSize(12);
    app.setFont(f);

    app.qwsSetDecorator( new QWSMinimalDecorator() );

    Hello hello("Hello World");
    hello.setCaption("Minimal window manager");
    hello.setFont( QFont("times",32,QFont::Bold) );
    hello.show();

    app.setMainWidget(&hello);

    app.exec();
}

