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

#include "mainwindow.h"
#include "menumanager.h"
#include "colors.h"
#include "dockitem.h"
#include "demotextitem.h"
#include "imageitem.h"
#include "demoitem.h"
#include "demoscene.h"

#ifndef QT_NO_OPENGL    
    #include <QGLWidget>
#endif
//#define QT_NO_OPENGL

MainWindow::MainWindow(QWidget *parent) : QGraphicsView(parent), updateTimer(this)
{
    this->currentFps = Colors::fps;
    this->loop = true;
    this->fpsMedian = -1;
    this->fpsLabel = 0;
    this->doneAdapt = false;
    this->trolltechLogo = 0;
    this->qtLogo = 0;
    this->setupWidget();
    this->setupScene();
    this->setupSceneItems();
    this->drawBackgroundToPixmap();
    this->switchTimerOnOff(true);
    this->demoStartTime.restart();
}

MainWindow::~MainWindow()
{
    delete this->trolltechLogo;
    delete this->qtLogo;
}

void MainWindow::setupWidget()
{
    QRect rect(0, 0, 800, 600);
    rect.moveCenter(QApplication::desktop()->screenGeometry().center());
    this->setGeometry(rect);
    setWindowTitle(tr("Qt Examples and Demos"));
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);

#ifndef QT_NO_OPENGL    
    if (!Colors::noOpenGl){
        // Use OpenGL
        QGLWidget *widget = new QGLWidget(QGLFormat(QGL::SampleBuffers));
        if (Colors::noScreenSync)
            widget->format().setSwapInterval(0);
        widget->setAutoFillBackground(false);
        setViewport(widget);
    }
#endif
    
    if (Colors::noOpenGl)
        setCacheMode(QGraphicsView::CacheBackground);

    connect(&this->updateTimer, SIGNAL(timeout()), this, SLOT(tick()));
}

void MainWindow::startLoop()
{
    this->updateTimer.stop();    
    while (this->loop){
        this->tick();
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
}

void MainWindow::enableMask(bool enable)
{
    if (!enable || Colors::noWindowMask)
        this->clearMask();
    else {
        QPolygon region; 
        region.setPoints(9,
                                // north side:
                                0, 0,
                                800, 0,
                                // east side:
                                // 800, 70,
                                // 790, 90,
                                // 790, 480,
                                // 800, 500,
                                800, 600,
                                // south side:
                                700, 600,
                                670, 590,
                                130, 590,
                                100, 600,
                                0, 600,
                                // west side:
                                // 0, 550,
                                // 10, 530,
                                // 10, 520,
                                // 0, 520,
                                0, 0); 
        this->setMask(QRegion(region));
    }
}

void MainWindow::setupScene()
{
    this->scene = new DemoScene(this);
    this->scene->setSceneRect(0, 0, size().width(), size().height());
    setScene(this->scene);
    this->scene->setItemIndexMethod(QGraphicsScene::NoIndex);
}

void MainWindow::drawItems(QPainter *painter, int numItems, QGraphicsItem **items, const QStyleOptionGraphicsItem* options)
{
    QGraphicsView::drawItems(painter, numItems, items, options);
}

void MainWindow::switchTimerOnOff(bool on)
{
    bool ticker = MenuManager::instance()->ticker && MenuManager::instance()->ticker->scene();
    if (ticker)
        MenuManager::instance()->ticker->tickOnPaint = !on || Colors::noTimerUpdate;

    if (on && !Colors::noTimerUpdate){
        this->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);
        this->updateTimer.start(int(1000 / Colors::fps));
    }
    else{
        this->updateTimer.stop();
        if (Colors::low)
            this->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
        else if (!Colors::noOpenGl)
            this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
        else
            this->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    }
}	

bool MainWindow::measureFps()
{ 
    static QTime prev = QTime::currentTime();
    float t = prev.msecsTo(QTime::currentTime());
    if (t == 0)
        t = 0.01f;
        
    this->currentFps = (1000.0f / t);
    this->fpsHistory += this->currentFps;
    prev = QTime::currentTime();

    // Calculate median:
    int size = this->fpsHistory.size();
    if (size == 10){
        qSort(this->fpsHistory.begin(), this->fpsHistory.end());
        this->fpsMedian = this->fpsHistory.at(int(size/2));
        if (this->fpsMedian == 0)
            this->fpsMedian = 0.01f;
        this->fpsHistory.clear();
        return true;
    }
    return false;
}

/**
    Used for adaption in case things are so slow
    that no median  yet has been calculated
*/
void MainWindow::forceFpsMedianCalculation()
{
    if (this->fpsMedian != -1)
        return;
    
    int size = this->fpsHistory.size();
    if (size == 0){
        this->fpsMedian = 0.01f;
        return;
    }
    
    qSort(this->fpsHistory.begin(), this->fpsHistory.end());
    this->fpsMedian = this->fpsHistory.at(int(size/2));
    if (this->fpsMedian == 0)
        this->fpsMedian = 0.01f;
}

void MainWindow::tick()
{
    bool medianChanged = this->measureFps();
    this->checkAdapt();

    if (medianChanged && this->fpsLabel && Colors::showFps)
        this->fpsLabel->setText(QString("FPS: ") + QString::number(int(this->currentFps)));
    
    if (MenuManager::instance()->ticker)
        MenuManager::instance()->ticker->tick();
    
    this->viewport()->update();
}

void MainWindow::setupSceneItems()
{
    if (Colors::showFps){    
        this->fpsLabel = new DemoTextItem(QString("FPS: --"), Colors::buttonFont(), Qt::white, -1, this->scene, 0, DemoTextItem::DYNAMIC_TEXT);
        this->fpsLabel->setZValue(100);
        this->fpsLabel->setPos(Colors::stageStartX, 600 - QFontMetricsF(Colors::buttonFont()).height() - 5);
    }

    this->trolltechLogo = new ImageItem(":/images/trolltech-logo.png", 1000, 1000, this->scene, 0, true, 0.5f);
    this->qtLogo = new ImageItem(":/images/qtlogo_small.png", 1000, 1000, this->scene, 0, true, 0.5f);
    this->trolltechLogo->setZValue(100);
    this->qtLogo->setZValue(100);
}

void MainWindow::checkAdapt()
{
    if (this->doneAdapt
        || Colors::noAdapt
        || Colors::noTimerUpdate
        || this->demoStartTime.elapsed() < 2000)
       return;

    this->doneAdapt = true;
    this->forceFpsMedianCalculation();
    
    if (this->fpsMedian < 30){
       if (MenuManager::instance()->ticker && MenuManager::instance()->ticker->scene()){
            this->scene->removeItem(MenuManager::instance()->ticker);
            Colors::noTimerUpdate = true;
            Colors::noAnimations = true;
        }

       if (this->fpsLabel)
           this->fpsLabel->setText(QString("FPS: (") + QString::number(this->fpsMedian) + QString(")"));       

       this->switchTimerOnOff(false);
    }
}

int MainWindow::performBenchmark()
{
/*    
    QTime time;
    time.restart();
    while (time.elapsed() < 2000)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
*/    
    return 0;    
}

void MainWindow::drawBackgroundToPixmap()
{
    const QRectF r = this->scene->sceneRect();
    this->background = QPixmap(qRound(r.width()), qRound(r.height()));
    this->background.fill(Qt::black);
    QPainter painter(&this->background);
    
    if (false && Colors::useEightBitPalette){
        painter.fillRect(r, Colors::sceneBg1);
    } else {
        QImage bg(":/images/demobg.png");
        painter.drawImage(0, 0, bg);
    }
}

void MainWindow::drawBackground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);
    painter->drawPixmap(QPoint(0, 0), this->background);
}

void MainWindow::showEvent(QShowEvent * event)
{
    Q_UNUSED(event);
    QGraphicsView::showEvent(event);
}

void MainWindow::toggleFullscreen()
{        
    if (this->isFullScreen()){
        this->enableMask(true);
        this->showNormal();
    }
    else {
        this->enableMask(false);
        this->showFullScreen();
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape){
        this->loop = false;
        QApplication::quit();
    }
    else if (event->key() == Qt::Key_1){
            QString s("");
            s += "Low settings: ";
            s += Colors::low ? "yes" : "no";
            s += "\nOpenGL: ";
            s += Colors::noOpenGl ? "off" : "on";
            s += "\nAnimations: ";
            s += Colors::noAnimations ? "off" : "on";
            s += "\nBlending: ";
            s += Colors::noBlending ? "off" : "on";
            s += "\nTicker: ";
            s += Colors::noTicker ? "off" : "on";
            s += "\nPixmaps: ";
            s += Colors::usePixmaps ? "on" : "off";
            s += "\nAdapt: ";
            s += Colors::noAdapt ? "off" : "on";
            s += "\nWanted FPS: ";
            s += QString::number(Colors::fps);
            s += "\nRescale images on resize: ";
            s += Colors::noRescale ? "off" : "on";
            s += "\nTimer based updates: ";
            s += Colors::noTimerUpdate ? "off" : "on";
            s += "\nSeparate loop: ";
            s += Colors::useLoop ? "yes" : "no";
            s += "\nScreen sync: ";
            s += Colors::noScreenSync ? "no" : "yes";
            QWidget w;
            s += "\nColor bit depth: ";
            s += QString::number(w.depth());
            QMessageBox::information(0, QString("Current configuration"), s);
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    this->resetMatrix();
    this->scale(event->size().width() / 800.0, event->size().height() / 600.0);
    QGraphicsView::resizeEvent(event);
    DemoItem::setMatrix(this->matrix());

    if (this->trolltechLogo){
        const QRectF r = this->scene->sceneRect();
        QRectF ttb = this->trolltechLogo->boundingRect();
        this->trolltechLogo->setPos(int((r.width() - ttb.width()) / 2), 595 - ttb.height());
        QRectF qtb = this->qtLogo->boundingRect();
        this->qtLogo->setPos(802 - qtb.width(), 0);
    }
    
    // Changing size will almost always
    // hurt FPS during the changing. So
    // ignore it.
    this->fpsHistory.clear();
}


