#include <qdial.h>
#include <qapplication.h>
#include <qvbox.h>
#include <qlcdnumber.h>

main(int argc, char** argv)
{
    QApplication::setColorSpec( QApplication::ManyColor );
    QApplication app(argc, argv);
    QApplication::setFont( QFont("Helvetica") );

    QVBox box;
    QLCDNumber lcd( &box );
    QDial dial( &box );
    dial.setFocusPolicy(QWidget::StrongFocus);
    dial.setFocus();
    box.show();
 
    QObject::connect(&dial, SIGNAL(valueChanged(int)), &lcd, SLOT(display(int)));
    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

    return app.exec();
}
