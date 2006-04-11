#include <QtGui>
#include <QApplication>

class MainWindow : public QMainWindow
{
public:
    MainWindow(QWidget *parent = 0);
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *myWidget = new QWidget;
    myWidget->setStatusTip(tr("This is my widget."));

    setCentralWidget(myWidget);

    QMenu *fileMenu = menuBar()->addMenu(tr("File"));

    QAction *newAct = new QAction(tr("&New"), this);
    newAct->setStatusTip(tr("Create a new file."));
    fileMenu->addAction(newAct);

    statusBar()->showMessage(tr("Ready"));
    setWindowTitle(tr("QStatusTipEvent"));
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}

