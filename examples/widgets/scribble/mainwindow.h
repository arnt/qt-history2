#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QList>
#include <QMainWindow>

#include "scribblearea.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void open();
    void save();
    void penColor();
    void penWidth();

private:
    void createActions();
    void createMenus();
    bool maybeSave();
    bool saveFile(const QString &fileFormat);

    ScribbleArea *scribbleArea;
    bool modified;

    QAction *openAct;
    QList<QAction *> saveAsActs;
    QMenu *fileMenu;
    QMenu *optionMenu;
    QMenu *saveAsMenu;
    QAction *exitAct;
    QAction *penColorAct;
    QAction *penWidthAct;
    QAction *clearScreenAct;
};

#endif
