#include <qapplication.h>
#include <qcolor.h>
#include <qpalette.h>

#include "launcher.h"

static QColor qtgreen(0xa1,0xc4,0x10);


void silent(QtMsgType, const char *)
{
}


main(int argc, char** argv)
{
    QApplication app( argc, argv );

//    qInstallMsgHandler(silent);

    QPalette pal( qtgreen, Qt::black );
    pal.setColor( QColorGroup::ButtonText, Qt::black );
    app.setPalette( pal );
#if defined(UNIX)
    app.setFont( QFont("Coolvetica",22) );
#else
    app.setFont( QFont("Coolvetica",16) );
#endif

    Launcher l;
    app.setMainWidget(&l);
    l.showMaximized();
    return app.exec();
}
