#include <QCoreApplication>
#include <QMainWindow>
#include <QSettings>

void snippet_ctor1()
{
    QSettings settings("MySoft", "Star Runner");
}

void snippet_ctor2()
{
    QCoreApplication::setOrganizationName("MySoft");
    QCoreApplication::setOrganizationDomain("mysoft.com");
    QCoreApplication::setApplicationName("Star Runner");

    QSettings settings;

    settings.setValue("editor/wrapMargin", 68);
    int margin = settings.value("editor/wrapMargin").toInt();
    {
    int margin = settings.value("editor/wrapMargin", 80).toInt();
    }

    settings.setValue("mainwindow/size", win->size());
    settings.setValue("mainwindow/fullScreen", win->isFullScreen());
    settings.setValue("outputpanel/visible", panel->isVisible());

    settings.beginGroup("mainwindow");
    settings.setValue("size", win->size());
    settings.setValue("fullScreen", win->isFullScreen());
    settings.endGroup();

    settings.beginGroup("outputpanel");
    settings.setValue("visible", panel->isVisible());
    settings.endGroup();
}

void snippet_locations()
{
    QSettings obj1("MySoft", "Star Runner");
    QSettings obj2("MySoft");
    QSettings obj3(QSettings::SystemScope, "MySoft", "Star Runner");
    QSettings obj4(QSettings::SystemScope, "MySoft");

    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       "MySoft", "Star Runner");

    QSettings settings("starrunner.ini", QSettings::IniFormat);

    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft",
                       QSettings::NativeFormat);
}

class MainWindow : public QMainWindow
{
public:
    MainWindow();

    void writeSettings();
    void readSettings();

protected:
    void closeEvent(QCloseEvent *event);
};

void MainWindow::writeSettings()
{
    QSettings settings("Moose Soft", "Clipper");

    settings.beginGroup("MainWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.endGroup();
}

void MainWindow::readSettings()
{
    QSettings settings("Moose Soft", "Clipper");

    settings.beginGroup("MainWindow");
    resize(settings.value("size", QSize(400, 400)));
    move(settings.value("pos", QPoint(200, 200)));
    settings.endGroup();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    readSettings();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (userReallyWantsToQuit()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}
