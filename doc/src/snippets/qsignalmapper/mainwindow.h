#include <qmainwindow.h>
#include <qstatusbar.h>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent=0, const char *name=0)
        : QMainWindow(parent, name)
    {
        setCaption("QSignalMapper");
        statusBar()->message("Ready");
    }

public slots:
    void buttonPressed(const QString &caption)
    {
        statusBar()->message(QString("Chose %1").arg(caption));
    }
};


