/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QtGui>
#include <QPixmap>

class DemoTextItem;
class ImageItem;

class MainWindow : public QGraphicsView
{
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void enableMask(bool enable);
    void toggleFullscreen();
    int performBenchmark();    
    void switchTimerOnOff(bool on);
    void startLoop();
    
    QGraphicsScene *scene;
    bool loop;

    // FPS stuff:
    QList<QTime> frameTimeList;
    QList<float> fpsHistory;
    float currentFps;
    float fpsMedian;
    DemoTextItem *fpsLabel;

protected:
    // Overidden methods:
    void showEvent(QShowEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent *event);
    void drawBackground(QPainter *painter, const QRectF &rect);
    void drawItems(QPainter *painter, int numItems, QGraphicsItem ** items, const QStyleOptionGraphicsItem* options);
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);
        
private slots:
    void tick();

private:
    void setupWidget();
    void setupSceneItems();
    void drawBackgroundToPixmap();
    void setupScene();
    bool measureFps();
    void forceFpsMedianCalculation();
    void checkAdapt();
    QTimer updateTimer;
    QTime demoStartTime;
    QPixmap background;
    ImageItem *trolltechLogo;
    ImageItem *qtLogo;
    bool doneAdapt;
    bool useTimer;
};

#endif // MAIN_WINDOW_H

