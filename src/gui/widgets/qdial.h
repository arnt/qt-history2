/****************************************************************************
**
** Definition of the dial widget.
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


#ifndef QDIAL_H
#define QDIAL_H

#ifndef QT_H
#include "qwidget.h"
#include "qrangecontrol.h"
#endif // QT_H

#ifndef QT_NO_DIAL

class QDialPrivate;

class Q_GUI_EXPORT QDial: public QWidget, public QRangeControl
{
    Q_OBJECT
    Q_PROPERTY(bool tracking READ tracking WRITE setTracking)
    Q_PROPERTY(bool wrapping READ wrapping WRITE setWrapping)
    Q_PROPERTY(int notchSize READ notchSize)
    Q_PROPERTY(double notchTarget READ notchTarget WRITE setNotchTarget)
    Q_PROPERTY(bool notchesVisible READ notchesVisible WRITE setNotchesVisible)
    Q_PROPERTY(int minValue READ minValue WRITE setMinValue)
    Q_PROPERTY(int maxValue READ maxValue WRITE setMaxValue)
    Q_PROPERTY(int lineStep READ lineStep WRITE setLineStep)
    Q_PROPERTY(int pageStep READ pageStep WRITE setPageStep)
    Q_PROPERTY(int value READ value WRITE setValue)

public:
    QDial(QWidget* parent=0, const char* name=0, WFlags f = 0);
    QDial(int minValue, int maxValue, int pageStep, int value,
           QWidget* parent=0, const char* name=0);
    ~QDial();

    bool tracking() const;

    bool wrapping() const;

    int notchSize() const;

    virtual void setNotchTarget(double);
    double notchTarget() const;

    bool notchesVisible() const;

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    int         minValue() const;
    int         maxValue() const;
    void setMinValue(int);
    void setMaxValue(int);
    int         lineStep() const;
    int         pageStep() const;
    void setLineStep(int);
    void setPageStep(int);
    int  value() const;

public slots:
    virtual void setValue(int);
    void addLine();
    void subtractLine();
    void addPage();
    void subtractPage();
    virtual void setNotchesVisible(bool b);
    virtual void setWrapping(bool on);
    virtual void setTracking(bool enable);

signals:
    void valueChanged(int value);
    void dialPressed();
    void dialMoved(int value);
    void dialReleased();

protected:
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);

    void keyPressEvent(QKeyEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
#ifndef QT_NO_WHEELEVENT
    void wheelEvent(QWheelEvent *);
#endif
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);

    void valueChange();
    void rangeChange();

    virtual void repaintScreen(const QRect *cr = 0);

private:
    QDialPrivate * d;

    int valueFromPoint(const QPoint &) const;
    double angle(const QPoint &, const QPoint &) const;
    QPointArray calcArrow(double &a) const;
    QRect calcDial() const;
    int calcBigLineSize() const;
    void calcLines();

private: // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QDial(const QDial &);
    QDial &operator=(const QDial &);
#endif

};

#endif  // QT_NO_DIAL

#endif
