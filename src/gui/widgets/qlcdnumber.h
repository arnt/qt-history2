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

#ifndef QLCDNUMBER_H
#define QLCDNUMBER_H

#include "QtGui/qframe.h"
#include "QtCore/qbitarray.h"

#ifndef QT_NO_LCDNUMBER


class QLCDNumberPrivate;
class Q_GUI_EXPORT QLCDNumber : public QFrame // LCD number widget
{
    Q_OBJECT
    Q_ENUMS(Mode SegmentStyle)
    Q_PROPERTY(bool smallDecimalPoint READ smallDecimalPoint WRITE setSmallDecimalPoint)
    Q_PROPERTY(int numDigits READ numDigits WRITE setNumDigits)
    Q_PROPERTY(Mode mode READ mode WRITE setMode)
    Q_PROPERTY(SegmentStyle segmentStyle READ segmentStyle WRITE setSegmentStyle)
    Q_PROPERTY(double value READ value WRITE display)
    Q_PROPERTY(int intValue READ intValue WRITE display)

public:
    QLCDNumber(QWidget* parent = 0);
    QLCDNumber(uint numDigits, QWidget* parent = 0);
    ~QLCDNumber();

    enum Mode {
        Hex, Dec, Oct, Bin
#ifdef QT_COMPAT
        , HEX = Hex, DEC = Dec, OCT = Oct, BIN = Bin
#endif
    };
    enum SegmentStyle {
        Outline, Filled, Flat
    };

    bool smallDecimalPoint() const;

    int numDigits() const;
    void setNumDigits(int nDigits);

    bool checkOverflow(double num) const;
    bool checkOverflow(int num) const;

    Mode mode() const;
    void setMode(Mode);

    SegmentStyle segmentStyle() const;
    void setSegmentStyle(SegmentStyle);

    double value() const;
    int intValue() const;

    QSize sizeHint() const;

public slots:
    void display(const QString &str);
    void display(int num);
    void display(double num);
    void setHexMode();
    void setDecMode();
    void setOctMode();
    void setBinMode();
    void setSmallDecimalPoint(bool);

signals:
    void overflow();

protected:
    void paintEvent(QPaintEvent *);

public:
#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QLCDNumber(QWidget* parent, const char* name);
    QT_COMPAT_CONSTRUCTOR QLCDNumber(uint numDigits, QWidget* parent, const char* name);
#endif

private:
    Q_DISABLE_COPY(QLCDNumber)
    Q_DECLARE_PRIVATE(QLCDNumber)
};

#endif // QT_NO_LCDNUMBER

#endif // QLCDNUMBER_H
