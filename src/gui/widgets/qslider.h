/****************************************************************************
**
** Definition of QSlider class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
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
    Q_ENUMS(TickSetting)
    Q_PROPERTY(TickSetting tickmarks READ tickmarks WRITE setTickmarks)
    Q_PROPERTY(int tickInterval READ tickInterval WRITE setTickInterval)

public:
    enum TickSetting { NoMarks = 0, Above = 1, Left = Above, Below = 2, Right = Below, Both = 3 };

    QSlider(QWidget *parent = 0);
    QSlider(Orientation orientation, QWidget *parent = 0);

    ~QSlider();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    void setTickmarks(TickSetting ts);
    TickSetting tickmarks() const;

    void setTickInterval(int ti);
    int tickInterval() const;

protected:
    void paintEvent(QPaintEvent *ev);
    void keyPressEvent(QKeyEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);

private:
    Q_DECL_PRIVATE(QSlider);

#ifdef QT_COMPAT
public:
    QSlider(QWidget *parent = 0, const char *name = 0);
    QSlider(Orientation, QWidget *parent = 0, const char *name = 0);
    QSlider(int minValue, int maxValue, int pageStep, int value, Orientation orientation,
                      QWidget *parent = 0, const char *name = 0);
    inline QT_COMPAT int sliderStart() const { return sliderPosition(); }
    QT_COMPAT QRect sliderRect() const;
public slots:
    inline QT_COMPAT void addStep() { triggerAction(SliderSingleStepAdd); };
    inline QT_COMPAT void subtractStep() { triggerAction(SliderSingleStepSub); };
#endif
private:
#if defined(Q_DISABLE_COPY)     // Disabled copy constructor and operator
    QSlider(const QSlider &);
    QSlider &operator=(const QSlider &);
#endif
};

#endif // QT_NO_SLIDER

#endif // QSLIDER_H
