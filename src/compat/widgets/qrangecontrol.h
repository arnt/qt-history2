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

#ifndef QRANGECONTROL_H
#define QRANGECONTROL_H

#include "qglobal.h"
#include "qwidget.h"

#ifndef QT_NO_RANGECONTROL

class QRangeControlPrivate;

class Q_COMPAT_EXPORT QRangeControl
{
public:
    QRangeControl();
    QRangeControl(int minValue, int maxValue,
                   int lineStep, int pageStep, int value);
    virtual ~QRangeControl();

    int                value()                const;
    void        setValue(int);
    void        addPage();
    void        subtractPage();
    void        addLine();
    void        subtractLine();

    int                minValue()        const;
    int                maxValue()        const;
    void        setRange(int minValue, int maxValue);
    void        setMinValue(int minVal);
    void        setMaxValue(int minVal);

    int                lineStep()        const;
    int                pageStep()        const;
    void        setSteps(int line, int page);

    int                bound(int) const;

protected:
    int                positionFromValue(int val, int space) const;
    int                valueFromPosition(int pos, int space) const;
    void        directSetValue(int val);
    int                prevValue()        const;

    virtual void valueChange();
    virtual void rangeChange();
    virtual void stepChange();

private:
    int                minVal, maxVal;
    int                line, page;
    int                val, prevVal;

    QRangeControlPrivate * d;

private:
    Q_DISABLE_COPY(QRangeControl)
};


inline int QRangeControl::value() const
{ return val; }

inline int QRangeControl::prevValue() const
{ return prevVal; }

inline int QRangeControl::minValue() const
{ return minVal; }

inline int QRangeControl::maxValue() const
{ return maxVal; }

inline int QRangeControl::lineStep() const
{ return line; }

inline int QRangeControl::pageStep() const
{ return page; }


#endif // QT_NO_RANGECONTROL

#ifndef QT_NO_SPINWIDGET

class QSpinWidgetPrivate;
class Q_COMPAT_EXPORT QSpinWidget : public QWidget
{
    Q_OBJECT
public:
    QSpinWidget(QWidget* parent=0, const char* name=0);
    ~QSpinWidget();

    void         setEditWidget(QWidget * widget);
    QWidget *         editWidget();

    QRect upRect() const;
    QRect downRect() const;

    void setUpEnabled(bool on);
    void setDownEnabled(bool on);

    bool isUpEnabled() const;
    bool isDownEnabled() const;

    enum ButtonSymbols { UpDownArrows, PlusMinus };
    virtual void        setButtonSymbols(ButtonSymbols bs);
    ButtonSymbols        buttonSymbols() const;

    void arrange();

signals:
    void stepUpPressed();
    void stepDownPressed();

public slots:
    void stepUp();
    void stepDown();

protected:
    void mousePressEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent* ev);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
#ifndef QT_NO_WHEELEVENT
    void wheelEvent(QWheelEvent *);
#endif
    void changeEvent(QEvent *);
    void paintEvent(QPaintEvent *);

private slots:
    void timerDone();
    void timerDoneEx();

private:
    QSpinWidgetPrivate * d;

    void updateDisplay();

private:
    Q_DISABLE_COPY(QSpinWidget)
};

#endif // QT_NO_SPINWIDGET

#endif // QRANGECONTROL_H
