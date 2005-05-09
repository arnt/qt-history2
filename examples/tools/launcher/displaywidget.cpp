/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>

#include "displayshape.h"
#include "displaywidget.h"

DisplayWidget::DisplayWidget(QWidget *parent)
    : QWidget(parent)
{
    empty = true;
    emptying = false;

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateShapes()));
    timer->setSingleShot(false);
    startTimer();
}

void DisplayWidget::appendShape(DisplayShape *shape)
{
    shapes.append(shape);
    empty = false;
    startTimer();
}

void DisplayWidget::insertShape(int position, DisplayShape *shape)
{
    shapes.insert(position, shape);
    empty = false;
    startTimer();
}

QSize DisplayWidget::minimumSizeHint() const
{
    return QSize(640, 480);
}

void DisplayWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    foreach (DisplayShape *shape, shapes) {
        if (shape->rect().contains(event->pos()) && !emptying \
            && !shape->contains("fade")) {
            if (shape->contains("launch")) {
                emit launchRequested(shape->metaData("launch").toString());
                shape->setMetaData("fade", -5);
                startTimer();
            } else if (shape->contains("category"))
                emit categoryRequested(shape->metaData("category").toString());
            else if (shape->contains("example"))
                emit exampleRequested(shape->metaData("example").toString());
            else if (shape->contains("documentation")) {
                emit documentationRequested(
                    shape->metaData("documentation").toString());
                shape->setMetaData("fade", -5);
                startTimer();
            }
        }
    }
}

void DisplayWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);
    //painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(event->rect(), Qt::white);
    foreach (DisplayShape *shape, shapes)
        shape->paint(&painter);
    painter.end();
}

void DisplayWidget::reset()
{
    if (emptying)
        return;

    if (shapes.size() == 0) {
        empty = true;
        timer->stop();
        emit displayEmpty();    // Note: synchronous signal
    } else {
        startTimer();
        emptying = true;
        empty = false;
        foreach (DisplayShape *shape, shapes)
            shape->setMetaData("fade", -15);
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

void DisplayWidget::startTimer()
{
    if (!timer->isActive())
        timer->start(50);
}

void DisplayWidget::updateShapes()
{
    QVector<DisplayShape*> discard;

    int updated = 0;

    foreach (DisplayShape *shape, shapes) {
        update(shape->rect().toRect().adjusted(-1,-1,1,1));
        if (shape->animate(rect()))
            ++updated;

        if (shape->contains("target")) {
            QPointF target = shape->metaData("target").toPointF();
            if (shape->position().toPoint() == target.toPoint()) {
                shape->removeMetaData("target");
                if (!emptying)
                    emit targetReached(shape);
            }
        }

        if (shape->contains("destroy")) {
            discard.append(shape);
        } else {
            update(shape->rect().toRect().adjusted(-1,-1,1,1));
        }
    }

    if (updated == 0)
        timer->stop();

    foreach (DisplayShape *shape, discard) {
        shapes.removeAll(shape);
        delete shape;
    }

    if (shapes.size() == 0 && !empty) {
        empty = true;
        emptying = false;
        timer->stop();
        emit displayEmpty();    // Note: synchronous signal
    }
}
