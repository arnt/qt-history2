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

#ifndef BASICTOOLSPLUGIN_H
#define BASICTOOLSPLUGIN_H

#include <QRect>
#include <QObject>
#include <QStringList>
#include <QPainterPath>
#include <QImage>

#include <plugandpaint/interfaces.h>

class BasicToolsPlugin : public QObject,
                         public BrushInterface,
                         public ShapeInterface,
                         public FilterInterface
{
    Q_OBJECT
    Q_INTERFACES(BrushInterface ShapeInterface FilterInterface)

public:
    // BrushInterface
    QStringList brushes() const;
    QRect mousePress(const QString &brush, QPainter &painter,
                     const QPoint &pos);
    QRect mouseMove(const QString &brush, QPainter &painter,
                    const QPoint &oldPos, const QPoint &newPos);
    QRect mouseRelease(const QString &brush, QPainter &painter,
                       const QPoint &pos);

    // ShapeInterface
    QStringList shapes() const;
    QPainterPath generateShape(const QString &shape, QWidget *parent);

    // FilterInterface
    QStringList filters() const;
    QImage filterImage(const QString &filter, const QImage &image,
                       QWidget *parent);
};

#endif
