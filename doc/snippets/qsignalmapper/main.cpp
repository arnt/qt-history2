#include <qapplication.h>

#include "buttonwidget.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QStringList captions;
    captions << "January" << "February" << "March" << "April"
             << "May" << "June" << "July" << "August"
             << "September" << "October" << "November"
             << "December";
    MainWindow *mw = new MainWindow(0);
    ButtonWidget *buttons = new ButtonWidget(captions, mw);
    mw->setCentralWidget(buttons);
    mw->show();
    QObject::connect(buttons, SIGNAL(clicked(const QString&)),
                     mw, SLOT(buttonPressed(const QString&)));
    QObject::connect(qApp, SIGNAL(lastWindowClosed()),
                     qApp, SLOT(quit()));
    return app.exec();
}
