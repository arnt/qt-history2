#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include <QMainWindow>

#include "ui_mainwindow.h"

class QMdiArea;
class QMdiSubWindow;
class QGroupBox;
class QAction;
class QActionGroup;
class QMenu;
class QDialogButtonBox;
class QCloseEvent;

class MainWindow : public QMainWindow, Ui::MainWindow
{
    Q_OBJECT

public:
    enum Presets { SaveChanges, ReallyQuit, FileError, Empty };

    MainWindow();

private slots:
    void addButton();
    void deleteButton();
    void loadPresetBox();
    void newStyle(QAction *action);
    void newOrientation(QAction *action);
    void subWindowActivated(QMdiSubWindow *widget);

private:
    void connectActions();
    QWidget *createDialogButtonBox(Presets present);
    void setStyle(QDialogButtonBox *widget, QStyle *style);
    void resolveButtons();
    void resizeActiveWindow();
    
    int windowCount;

    QMdiArea *mdiArea;
    QMdiSubWindow *currentWindow;

    QActionGroup *styleGroup;
    QActionGroup *orientationGroup;

    QAbstractButton *myAddButton;
    QAbstractButton *myDeleteButton;
};

#endif
