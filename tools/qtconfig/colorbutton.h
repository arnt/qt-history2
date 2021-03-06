/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QAbstractButton>

QT_BEGIN_NAMESPACE

class ColorButton : public QAbstractButton
{
    Q_OBJECT

public:
    ColorButton(QWidget *);
    ColorButton(const QColor &, QWidget *);

    const QColor &color() const { return col; }

    void setColor(const QColor &);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void dragEnterEvent(QDragEnterEvent *);
    void dragMoveEvent(QDragMoveEvent *);
    void dropEvent(QDropEvent *);

signals:
    void colorChanged(const QColor &);

protected:
    void paintEvent(QPaintEvent *);
    void drawButton(QPainter *);
    void drawButtonLabel(QPainter *);

private slots:
    void changeColor();


private:
    QColor col;
    QPoint presspos;
    bool mousepressed;
};

QT_END_NAMESPACE

#endif // COLORBUTTON_H
