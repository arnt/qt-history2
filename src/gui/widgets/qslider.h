/****************************************************************************
**
** Definition of QSlider class.
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

#ifndef QSLIDER_H
#define QSLIDER_H

#ifndef QT_H
#include <qabstractslider.h>
#endif // QT_H

#ifndef QT_NO_SLIDER

class QSliderPrivate;

class Q_GUI_EXPORT QSlider : public QAbstractSlider
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSlider)

    Q_ENUMS(TickSetting)
    Q_PROPERTY(TickSetting tickmarks READ tickmarks WRITE setTickmarks)
    Q_PROPERTY(int tickInterval READ tickInterval WRITE setTickInterval)

public:
    enum TickSetting { NoMarks = 0, Above = 1, Left = Above, Below = 2, Right = Below, Both = 3 };

    QSlider(QWidget *parent = 0);
    QSlider(Qt::Orientation orientation, QWidget *parent = 0);

    ~QSlider();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    void setTickmarks(TickSetting ts);
    TickSetting tickmarks() const;

    void setTickInterval(int ti);
    int tickInterval() const;

protected:
    void paintEvent(QPaintEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);

#ifdef QT_COMPAT
public:
    QT_COMPAT_CONSTRUCTOR QSlider(QWidget *parent, const char *name);
    QT_COMPAT_CONSTRUCTOR QSlider(Qt::Orientation, QWidget *parent, const char *name);
    QT_COMPAT_CONSTRUCTOR QSlider(int minValue, int maxValue, int pageStep, int value,
                                  Qt::Orientation orientation,
                                  QWidget *parent = 0, const char *name = 0);
public slots:
    inline QT_MOC_COMPAT void addStep() { triggerAction(SliderSingleStepAdd); };
    inline QT_MOC_COMPAT void subtractStep() { triggerAction(SliderSingleStepSub); };
#endif
private:
#if defined(Q_DISABLE_COPY)     // Disabled copy constructor and operator
    QSlider(const QSlider &);
    QSlider &operator=(const QSlider &);
#endif
};

#endif // QT_NO_SLIDER

#endif // QSLIDER_H
