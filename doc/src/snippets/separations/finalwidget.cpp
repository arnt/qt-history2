/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
finalwidget.cpp

A widget to display an image and a label containing a description.
*/

#include <QApplication>
#include <QBuffer>
#include <QColorDialog>
#include <QDrag>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QPixmap>
#include <QPoint>
#include <QPushButton>
#include <QString>
#include <QWidget>

#include "finalwidget.h"

FinalWidget::FinalWidget(QWidget *parent, const QString &name)
    : QFrame(parent)
{
    hasImage = false;

    imageLabel = new QLabel(this);

    nameLabel = new QLabel(name, this);

    grid = new QGridLayout(this);
    grid->addWidget(imageLabel, 0, 0, 1, 2);
    grid->addWidget(nameLabel, 1, 0);
}

/*!
    If the mouse moves far enough when the left mouse button is held down,
    start a drag and drop operation.
*/

void FinalWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - dragStartPosition).manhattanLength()
         < QApplication::startDragDistance())
        return;
    if (!hasImage)
        return;

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    //mimeData->setPixmap(*imageLabel->pixmap());
    QByteArray output;
    QBuffer outputBuffer(&output);
    outputBuffer.open(QIODevice::WriteOnly);
    imageLabel->pixmap()->toImage().save(&outputBuffer, "PNG");
    mimeData->setData("image/png", output);

    drag->setMimeData(mimeData);
    drag->setPixmap(imageLabel->pixmap()->scaled(64, 64, Qt::KeepAspectRatio));
    drag->setHotSpot(QPoint(drag->pixmap().width()/2,
                            drag->pixmap().height()));

    drag->start();
}

/*!
    Check for left mouse button presses in order to enable drag and drop.
*/

void FinalWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        dragStartPosition = event->pos();
}

const QPixmap* FinalWidget::pixmap() const
{
    return imageLabel->pixmap();
}

void FinalWidget::setPixmap(const QPixmap &pixmap)
{
    imageLabel->setPixmap(pixmap);
    hasImage = true;
}
