/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSCROLLBAR_H
#define QSCROLLBAR_H

#include "QtGui/qwidget.h"
#include "QtGui/qabstractslider.h"

class QScrollBarPrivate;

class Q_GUI_EXPORT QScrollBar : public QAbstractSlider
{
    Q_OBJECT
public:
    explicit QScrollBar(QWidget *parent=0);
    explicit QScrollBar(Qt::Orientation, QWidget *parent=0);
    ~QScrollBar();

    QSize sizeHint() const;
    bool event(QEvent *event);

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void hideEvent(QHideEvent*);
    void sliderChange(SliderChange change);

#ifdef QT3_SUPPORT
public:
    QT3_SUPPORT_CONSTRUCTOR QScrollBar(QWidget *parent, const char* name);
    QT3_SUPPORT_CONSTRUCTOR QScrollBar(Qt::Orientation, QWidget *parent, const char* name);
    QT3_SUPPORT_CONSTRUCTOR QScrollBar(int minValue, int maxValue, int lineStep, int pageStep,
                int value, Qt::Orientation, QWidget *parent=0, const char* name = 0);
    inline QT3_SUPPORT bool draggingSlider() { return isSliderDown(); }
#endif

private:
    Q_DISABLE_COPY(QScrollBar)
    Q_DECLARE_PRIVATE(QScrollBar)
};

#endif // QSCROLLBAR_H
