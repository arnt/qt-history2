#include <QtGui>

class MainWindow : public QMainWindow
{
public:
    MainWindow();

    QAction *newAct;
};

MainWindow()
{
    newAct = new QAction(tr("&New"), this);
    newAct->setShortcut(tr("Ctrl+N"));
    newAct->setStatusTip(tr("Create a new file"));
    newAct->setWhatsThis(tr("Click this option to create a new file."));
}

int main()
{
    return 0;
}
