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

//     QPalette pal( qtgreen, Qt::black );
//     pal.setColor( QColorGroup::ButtonText, Qt::black );
//     app.setPalette( pal );

//     // compute the font size with the help of the screensize
//     QWidget *d = QApplication::desktop();
//     app.setFont( QFont( "monofonto", 22 ) );

    Launcher l;
    app.setMainWidget( &l );
    l.showMaximized();
    return app.exec();
}
