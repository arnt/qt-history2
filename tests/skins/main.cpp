#include <qapplication.h>
#include <qfile.h>
#include <qtextstream.h>
#include "demo.h"
#include "qskin.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    
    if (argc < 2) {
        /* load from default */
        QApplication::setStyle(new QSkinStyle());
    } else {
        QFile f("example.skin");
        f.open(IO_ReadOnly);
        QTextStream t(&f);
        QApplication::setStyle(new QSkinStyle(t));
        f.close();
    }

    Skinable *w = new Skinable;
    a.setMainWidget(w);
    w->setFixedSize(330, 130);
    w->show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    return a.exec();
}
