#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

class InvokeMethod;
class ChangeProperties;
class AmbientProperties;
class QAxScriptManager;

class QWorkspace;

class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected slots:
    void on_actionFileNew_triggered();
    void on_actionFileLoad_triggered();
    void on_actionFileSave_triggered();

    void on_actionContainerSet_triggered();
    void on_actionContainerClear_triggered();
    void on_actionContainerProperties_triggered();

    void on_actionControlInfo_triggered();
    void on_actionControlDocumentation_triggered();
    void on_actionControlPixmap_triggered();
    void on_actionControlProperties_triggered();
    void on_actionControlMethods_triggered();

    void on_actionScriptingLoad_triggered();
    void on_actionScriptingRun_triggered();

private:
    InvokeMethod *dlgInvoke;
    ChangeProperties *dlgProperties;
    AmbientProperties *dlgAmbient;
    QAxScriptManager *scripts;
    QWorkspace *workspace;

    QtMsgHandler oldDebugHandler;

private slots:
    void updateGUI();
    void logPropertyChanged(const QString &prop);
    void logSignal(const QString &signal, int argc, void *argv);
    void logException(int code, const QString&source, const QString&desc, const QString&help);
    void logMacro(int code, const QString &description, int sourcePosition, const QString &sourceText);
};


#endif // MAINWINDOW_H
