#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QAction;
class QActionGroup;
class QMenu;
class QStatusBar;
class TabletCanvas;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(TabletCanvas *canvas);

private slots:
    void brushColorAct();
    void alphaActionTriggered(QAction *action);
    void lineWidthActionTriggered(QAction *action);
    void saturationActionTriggered(QAction *action);
    void saveAct();
    void loadAct();
    void aboutAct();

private:
    void createActions();
    void createMenus();

    TabletCanvas *myCanvas;

    QAction *brushColorAction;
    QActionGroup *brushActionGroup;

    QActionGroup *alphaChannelGroup;
    QAction *alphaChannelPressureAction;
    QAction *alphaChannelTiltAction;
    QAction *noAlphaChannelAction;

    QActionGroup *colorSaturationGroup;
    QAction *colorSaturationVTiltAction;
    QAction *colorSaturationHTiltAction;
    QAction *colorSaturationPressureAction;
    QAction *noColorSaturationAction;

    QActionGroup *lineWidthGroup;
    QAction *lineWidthPressureAction;
    QAction *lineWidthTiltAction;
    QAction *lineWidthFixedAction;

    QAction *exitAction;
    QAction *saveAction;
    QAction *loadAction;

    QAction *aboutAction;
    QAction *aboutQtAction;
    
    QMenu *fileMenu;
    QMenu *brushMenu;
    QMenu *tabletMenu;
    QMenu *helpMenu;
    QMenu *colorSaturationMenu;
    QMenu *lineWidthMenu;
    QMenu *alphaChannelMenu;

};

#endif
