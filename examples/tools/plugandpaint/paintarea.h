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

#ifndef PAINTAREA_H
#define PAINTAREA_H

#include <QColor>
#include <QImage>
#include <QPainterPath>
#include <QWidget>

class BrushInterface;

class PaintArea : public QWidget
{
    Q_OBJECT

public:
    PaintArea(QWidget *parent = 0);

    bool openImage(const QString &fileName);
    bool saveImage(const QString &fileName, const char *fileFormat);
    void setImage(const QImage &image);
    void insertShape(const QPainterPath &path);
    void setBrushColor(const QColor &color);
    void setBrushWidth(int width);
    void setBrush(BrushInterface *brushInterface, const QString &brush);

    QImage image() const { return theImage; }
    QColor brushColor() const { return color; }
    int brushWidth() const { return thickness; }
    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    void setupPainter(QPainter &painter);

    QImage theImage;
    QColor color;
    int thickness;

    BrushInterface *brushInterface;
    QString brush;
    QPoint lastPos;

    QPainterPath pendingPath;
};

#endif
