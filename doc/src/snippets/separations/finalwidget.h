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

#ifndef FINALWIDGET_H
#define FINALWIDGET_H

#include <QFrame>
#include <QImage>
#include <QPoint>

class QGridLayout;
class QLabel;
class QMouseEvent;
class QWidget;

class FinalWidget : public QFrame
{
    Q_OBJECT

public:
    FinalWidget(QWidget *parent, const QString &name);
    void setPixmap(const QPixmap &pixmap);
    const QPixmap *pixmap() const;

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);

private:
    void createImage();

    bool dragging;
    bool hasImage;
    QGridLayout *grid;
    QImage originalImage;
    QLabel *imageLabel;
    QLabel *nameLabel;
    QPoint dragStartPosition;
};

#endif
