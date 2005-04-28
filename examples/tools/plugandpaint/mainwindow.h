#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QAction;
class QActionGroup;
class QMenu;
class QScrollArea;
class PaintArea;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private slots:
    void open();
    bool saveAs();
    void brushColor();
    void brushWidth();
    void changeBrush();
    void insertShape();
    void applyFilter();
    void about();

private:
    void createActions();
    void createMenus();
    void loadPlugins();
    void addToMenu(QObject *plugin, const QStringList &texts, QMenu *menu,
                   const char *member, QActionGroup *actionGroup = 0);

    PaintArea *paintArea;
    QScrollArea *scrollArea;

    QMenu *fileMenu;
    QMenu *brushMenu;
    QMenu *shapesMenu;
    QMenu *filterMenu;
    QMenu *helpMenu;
    QActionGroup *brushActionGroup;
    QAction *openAct;
    QAction *saveAsAct;
    QAction *exitAct;
    QAction *brushWidthAct;
    QAction *brushColorAct;
    QAction *aboutAct;
    QAction *aboutQtAct;
};

#endif
