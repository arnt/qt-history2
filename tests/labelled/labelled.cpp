#include "labelled.h"
#include <qlabelled.h>
#include <qlabel.h>
#include <qapp.h>

main(int argc, char** argv)
{
    QApplication::setColorSpec( QApplication::ManyColor );
    QApplication app(argc, argv);
    QApplication::setFont( QFont("Helvetica") );

    QLabelled m("Labelled widget");
    //m.setAlignment(AlignLeft);
    QLabel l("Label for the guts",&m);
    l.setMinimumSize(l.sizeHint());

    m.show();

    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

    return app.exec();
}
