/****************************************************************************
**
** Definition of QScrollBar class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSCROLLBAR_H
#define QSCROLLBAR_H

#ifndef QT_H
#include "qwidget.h"
#include "qabstractslider.h"
#endif // QT_H

class QScrollBarPrivate;

class Q_GUI_EXPORT QScrollBar : public QAbstractSlider
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QScrollBar)
public:
    QScrollBar(QWidget *parent=0);
    QScrollBar(Qt::Orientation, QWidget *parent=0);

    ~QScrollBar();

    QSize sizeHint() const;

protected:
    void        paintEvent(QPaintEvent *);

    void        mousePressEvent(QMouseEvent *);
    void        mouseReleaseEvent(QMouseEvent *);
    void        mouseMoveEvent(QMouseEvent *);
    void        hideEvent(QHideEvent*);

    void     changeEvent(QEvent *);
    void sliderChange(SliderChange change);

#ifdef QT_COMPAT
public:
    QScrollBar(QWidget *parent, const char* name);
    QScrollBar(Qt::Orientation, QWidget *parent, const char* name);
    QScrollBar(int minValue, int maxValue, int lineStep, int pageStep,
                int value, Qt::Orientation, QWidget *parent=0, const char* name = 0);
    inline QT_COMPAT bool draggingSlider() { return isSliderDown(); }
#endif

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QScrollBar(const QScrollBar &);
    QScrollBar &operator=(const QScrollBar &);
#endif
};



#endif // QSCROLLBAR_H
