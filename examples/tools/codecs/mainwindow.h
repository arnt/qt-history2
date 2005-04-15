#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QList>
#include <QMainWindow>

class QAction;
class QMenu;
class QTextCodec;
class QTextEdit;
class PreviewForm;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private slots:
    void open();
    void save();
    void about();
    void aboutToShowSaveAsMenu();

private:
    void findCodecs();
    void createActions();
    void createMenus();

    QTextEdit *textEdit;
    PreviewForm *previewForm;
    QList<QTextCodec *> codecs;

    QMenu *fileMenu;
    QMenu *helpMenu;
    QMenu *saveAsMenu;
    QAction *openAct;
    QList<QAction *> saveAsActs;
    QAction *exitAct;
    QAction *aboutAct;
    QAction *aboutQtAct;
};

#endif
