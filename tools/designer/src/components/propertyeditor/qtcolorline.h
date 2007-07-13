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

#ifndef QTCOLORLINE_H
#define QTCOLORLINE_H

#include <QtGui/QWidget>

namespace qdesigner_internal {

class QtColorLine : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(int indicatorSpace READ indicatorSpace WRITE setIndicatorSpace)
    Q_PROPERTY(int indicatorSize READ indicatorSize WRITE setIndicatorSize)
    Q_PROPERTY(bool flip READ flip WRITE setFlip)
    Q_PROPERTY(bool backgroundTransparent READ backgroundTransparent WRITE setBackgroundTransparent)
    Q_PROPERTY(ColorComponent colorComponent READ colorComponent WRITE setColorComponent)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
    Q_ENUMS(ColorComponent)
public:

    enum ColorComponent {
        Red,
        Green,
        Blue,
        Hue,
        Saturation,
        Value,
        Alpha
    };

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    QtColorLine(QWidget *parent = 0);
    ~QtColorLine();

    QColor color() const;

    void setIndicatorSize(int size);
    int indicatorSize() const;

    void setIndicatorSpace(int space);
    int indicatorSpace() const;

    void setFlip(bool flip);
    bool flip() const;

    void setBackgroundTransparent(bool transparent);
    bool backgroundTransparent() const;

    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation() const;

    void setColorComponent(ColorComponent component);
    ColorComponent colorComponent() const;

public slots:

    void setColor(const QColor &color);

signals:

    void colorChanged(const QColor &color);

protected:

    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:

    class QtColorLinePrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtColorLine)
    Q_DISABLE_COPY(QtColorLine)
};

}

#endif
