#include <qapplication.h>
#include <qdialog.h>
#include <qpushbutton.h>

main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QPushButton pb1("1",0);
    QDialog d;
    QPushButton pb2("2",&d);
    QObject::connect(&pb1,SIGNAL(clicked()),&d,SLOT(show()));
    QObject::connect(&pb2,SIGNAL(clicked()),&d,SLOT(accept()));
    pb1.show();

    return app.exec();
}
