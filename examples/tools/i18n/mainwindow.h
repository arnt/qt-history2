#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QAction;
class QGroupBox;
class QLabel;
class QListWidget;
class QMenu;
class QRadioButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private:
    void createGroupBox();

    QWidget *centralWidget;
    QLabel *label;
    QGroupBox *groupBox;
    QListWidget *listWidget;
    QRadioButton *perspectiveRadioButton;
    QRadioButton *isometricRadioButton;
    QRadioButton *obliqueRadioButton;
    QMenu *fileMenu;
    QAction *exitAction;
};

#endif
