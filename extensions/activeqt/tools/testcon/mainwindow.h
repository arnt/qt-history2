#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

class InvokeMethod;
class ChangeProperties;
class AmbientProperties;
class QAxScriptManager;

class Q3Workspace;

class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected slots:
    void setControl();
    void on_actionControlInfo_triggered();
    void on_actionFileNew_triggered();
    void on_actionFileLoad_triggered();
    void on_actionFileSave_triggered();
    void showDocumentation();
    void renderPixmap();
    void runMacro();
    void loadScript();
    void changeProperties();
    void clearControl();
    void containerProperties();
    void invokeMethods();

private:
    void updateGUI();

    InvokeMethod *dlgInvoke;
    ChangeProperties *dlgProperties;
    AmbientProperties *dlgAmbient;
    QAxScriptManager *scripts;
    Q3Workspace *workspace;

    QtMsgHandler oldDebugHandler;

private slots:
    void windowActivated(QWidget *widget);
    void logPropertyChanged(const QString &prop);
    void logSignal(const QString &signal, int argc, void *argv);
    void logException(int code, const QString&source, const QString&desc, const QString&help);
    void logMacro(int code, const QString &description, int sourcePosition, const QString &sourceText);
};


#endif // MAINWINDOW_H
