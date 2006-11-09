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

#include <QtGui>

#if defined(Q_WS_X11)
#include <QTime>
#include <private/qt_x11_p.h>
#endif

#include "displayshape.h"
#include "displaywidget.h"

DisplayWidget::DisplayWidget(QWidget *parent)
    : QWidget(parent)
{
#if defined(Q_WS_X11)
    frameTime = 0;
    avgRate = 0;
    numFrames = 0;
    testDrawSpeed = true;
#endif
    empty = true;
    emptying = false;

    enableUpdates();

    setBackgroundRole(QPalette::Base);
    setMouseTracking(true);
}

void DisplayWidget::appendShape(DisplayShape *shape)
{
    shapes.append(shape);
    empty = false;
    enableUpdates();
}

void DisplayWidget::insertShape(int position, DisplayShape *shape)
{
    shapes.insert(position, shape);
    empty = false;
    enableUpdates();
}

QSize DisplayWidget::minimumSizeHint() const
{
    return QSize(800, 600);
}

void DisplayWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (emptying)
        return;

    bool updated = false;

    foreach (DisplayShape *shape, shapes) {
        if (shape->rect().contains(event->pos())) {
            if (shape->isInteractive() && !shape->contains("fade")
                && !shape->contains("highlight")) {
                shape->setMetaData("highlight", true);
                updated = true;
            }
        } else if (shape->isInteractive() && shape->contains("highlight")
                   && shape->metaData("highlight").toBool()) {
            shape->setMetaData("highlight", false);
            updated = true;
        }
    }

    if (updated)
        enableUpdates();
}

void DisplayWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    if (emptying)
        return;

    foreach (DisplayShape *shape, shapes) {
        if (shape->rect().contains(event->pos()) && !shape->contains("fade")) {
            if (shape->contains("action"))
                emit actionRequested(shape->metaData("action").toString());
            else if (shape->contains("category"))
                emit categoryRequested(shape->metaData("category").toString());
            else if (shape->contains("example"))
                emit exampleRequested(shape->metaData("example").toString());
            else if (shape->contains("documentation")) {
                emit documentationRequested(
                    shape->metaData("documentation").toString());
                shape->setMetaData("highlight", false);
                enableUpdates();
            } else if (shape->contains("launch")) {
                emit launchRequested(shape->metaData("launch").toString());
                shape->setMetaData("fade", -5);
                enableUpdates();
            }
        }
    }
}

void DisplayWidget::paintEvent(QPaintEvent *event)
{
#if defined(Q_WS_X11)
    QTime renderTime;
    if (testDrawSpeed)
        renderTime.restart();
#endif

    QPainter painter;
    painter.begin(this);
    painter.fillRect(event->rect(), Qt::white);
    foreach (DisplayShape *shape, shapes) {
        if (shape->rect().intersects(event->rect()))
            shape->paint(&painter);
    }
    painter.end();

#if defined(Q_WS_X11)
    if (testDrawSpeed) {
        numFrames++;
        frameTime += renderTime.elapsed();
        avgRate = frameTime/numFrames;
        if (numFrames > 20) {
            testDrawSpeed = false;
            if (avgRate > 50 && X11->use_xrender)
                QTimer::singleShot(0, this, SLOT(toggleXRender()));
        }
    }
#endif
}

#if defined(Q_WS_X11)
void DisplayWidget::toggleXRender()
{
    timer.stop();
    int result = QMessageBox::question(this,
                                       QObject::tr("Disable XRender?"),
                                       QObject::tr("Your XRender implementation does not appear to be accelerated.\n"
                                                   "This may cause this demo to run very slowly.\n"
                                                   "Do you wish to turn XRender support off in this demo to improve\n"
                                                   "the frame rate?"),
                                       QMessageBox::Yes|QMessageBox::No);
    if (result == QMessageBox::Yes)
        X11->use_xrender = false;
    enableUpdates();
}
#endif

void DisplayWidget::reset()
{
    if (emptying)
        return;

    if (shapes.size() == 0) {
        empty = true;
        timer.stop();
        emit displayEmpty();    // Note: synchronous signal
    } else {
        enableUpdates();
        emptying = true;
        empty = false;
        foreach (DisplayShape *shape, shapes) {
            shape->setMetaData("fade", -15);
            shape->setMetaData("fade minimum", 0);
        }
    }
}

DisplayShape *DisplayWidget::shape(int index) const
{
    return shapes.value(index);
}

int DisplayWidget::shapesCount() const
{
    return shapes.size();
}

void DisplayWidget::enableUpdates()
{
    if (!timer.isActive())
        timer.start(25, this);
}

void DisplayWidget::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == timer.timerId()) {
        QVector<DisplayShape*> discard;

        int updated = 0;

        foreach (DisplayShape *shape, shapes) {
            QRect oldRect = shape->rect().toRect().adjusted(-1,-1,1,1);
            if (shape->animate()) {

                update(oldRect);
                QRect newRect = shape->rect().toRect().adjusted(-1,-1,1,1);
                ++updated;

                if (shape->contains("destroy")) {
                    discard.append(shape);
                } else {
                    update(newRect);
                }
            }
        }

        if (updated == 0)
            timer.stop();

        foreach (DisplayShape *shape, discard) {
            shapes.removeAll(shape);
            delete shape;
        }

        if (shapes.size() == 0 && !empty) {
            empty = true;
            emptying = false;
            timer.stop();
            emit displayEmpty();    // Note: synchronous signal
        }
    } else {
	QWidget::timerEvent(event);
    }
}
