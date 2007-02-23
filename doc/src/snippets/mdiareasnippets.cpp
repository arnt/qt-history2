#include <QtGui>

void mainWindowExample()
{
    QMdiArea *mdiArea = new QMdiArea;
    QMainWindow *mainWindow = new QMainWindow;
    mainWindow->setCentralWidget(mdiArea);

    mdiArea->addSubWindow(new QPushButton("Push Me Now!"));

    mainWindow->show();
}

void addingSubWindowsExample()
{
    QWidget *internalWidget1 = new QWidget;
    QWidget *internalWidget2 = new QWidget;
    
    QMdiArea mdiArea;
    QMdiSubWindow *subWindow1 = new QMdiSubWindow;
    subWindow1->setWidget(internalWidget1);
    subWindow1->setAttribute(Qt::WA_DeleteOnClose);
    mdiArea.addSubWindow(subWindow1);

    QMdiSubWindow *subWindow2 =
	mdiArea.addSubWindow(internalWidget2);

    subWindow1->show();
    subWindow2->show();

    mdiArea.show();
}

int main(int argv, char **args)
{
    QApplication app(argv, args);

    mainWindowExample();
    //addingSubWindowsExample();

   QAction *act = new QAction(qApp);
   act->setShortcut(Qt::ALT + Qt::Key_S);
   act->setShortcutContext( Qt::ApplicationShortcut );
   QObject::connect(act, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    QWidget widget5;
    widget5.show();
    widget5.addAction(act);

    return app.exec();
}


