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


#ifndef QDIAL_H
#define QDIAL_H

#include "QtGui/qabstractslider.h"

#ifndef QT_NO_DIAL

class QDialPrivate;

class Q_GUI_EXPORT QDial: public QAbstractSlider
{
    Q_OBJECT

    Q_PROPERTY(bool wrapping READ wrapping WRITE setWrapping)
    Q_PROPERTY(int notchSize READ notchSize)
    Q_PROPERTY(double notchTarget READ notchTarget WRITE setNotchTarget)
    Q_PROPERTY(bool notchesVisible READ notchesVisible WRITE setNotchesVisible)
public:
    QDial(QWidget *parent = 0);

    ~QDial();

    bool wrapping() const;

    int notchSize() const;

    void setNotchTarget(double target);
    double notchTarget() const;
    bool notchesVisible() const;

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

public slots:
    void setNotchesVisible(bool visible);
    void setWrapping(bool on);

protected:
    void resizeEvent(QResizeEvent *re);
    void paintEvent(QPaintEvent *pe);

    void mousePressEvent(QMouseEvent *me);
    void mouseReleaseEvent(QMouseEvent *me);
    void mouseMoveEvent(QMouseEvent *me);

    void focusInEvent(QFocusEvent *fe);
    void focusOutEvent(QFocusEvent *fe);

    void sliderChange(SliderChange change);

#ifdef QT_COMPAT
public:
    QT_COMPAT_CONSTRUCTOR QDial(int minValue, int maxValue, int pageStep, int value,
                                QWidget* parent = 0, const char* name = 0);
    QT_COMPAT_CONSTRUCTOR QDial(QWidget *parent, const char *name);

signals:
    QT_MOC_COMPAT void dialPressed();
    QT_MOC_COMPAT void dialMoved(int value);
    QT_MOC_COMPAT void dialReleased();
#endif

private:
    Q_DECLARE_PRIVATE(QDial)
    Q_DISABLE_COPY(QDial)
};

#endif  // QT_NO_DIAL

#endif
