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

#ifndef QSIZEPOLICY_H
#define QSIZEPOLICY_H

#include "QtCore/qobject.h"
#include "QtCore/qobjectdefs.h"


// Documentation is in qabstractlayout.cpp.

class QVariant;

class Q_GUI_EXPORT QSizePolicy
{
    Q_GADGET
    Q_ENUMS(Policy)
private:
    enum {
        HSize = 4,
        HMask = 0x0f,
        VMask = HMask << HSize,
    };
public:
    enum PolicyFlag {
        GrowFlag = 1,
        ExpandFlag = 2,
        ShrinkFlag = 4,
        IgnoreFlag = 8
    };

    enum Policy {
        Fixed = 0,
        Minimum = GrowFlag,
        Maximum = ShrinkFlag,
        Preferred = GrowFlag | ShrinkFlag,
        MinimumExpanding = GrowFlag | ExpandFlag,
        Expanding = GrowFlag | ShrinkFlag | ExpandFlag,
        Ignored = ShrinkFlag|GrowFlag|IgnoreFlag
    };

    QSizePolicy() : data(0) { }

    QSizePolicy(Policy horizontal, Policy vertical)
        : data(horizontal | (vertical<<HSize)) { }


    Policy horizontalPolicy() const { return static_cast<Policy>(data & HMask); }
    Policy verticalPolicy() const { return static_cast<Policy>((data & VMask) >> HSize); }

    void setHorizontalPolicy(Policy d) { data = (data & ~HMask) | d; }
    void setVerticalPolicy(Policy d) { data = (data & ~(HMask << HSize)) | (d << HSize); }

    Qt::Orientations expandingDirections() const {
        Qt::Orientations result;
        if (verticalPolicy() & ExpandFlag)
            result |= Qt::Vertical;
        if (horizontalPolicy() & ExpandFlag)
            result |= Qt::Horizontal;
        return result;
    }

    void setHeightForWidth(bool b) { data = b ? (data | (1 << 2*HSize)) : (data & ~(1 << 2*HSize));  }
    bool hasHeightForWidth() const { return data & (1 << 2*HSize); }

    bool operator==(const QSizePolicy& s) const { return data == s.data; }
    bool operator!=(const QSizePolicy& s) const { return data != s.data; }
    operator QVariant() const; // implemented in qabstractlayout.cpp

    int horizontalStretch() const { return data >> 24; }
    int verticalStretch() const { return (data >> 16) & 0xff; }
    void setHorizontalStretch(uchar stretchFactor) { data = (data&0x00ffffff) | (uint(stretchFactor)<<24); }
    void setVerticalStretch(uchar stretchFactor) { data = (data&0xff00ffff) | (uint(stretchFactor)<<16); }

    void transpose();



#ifdef QT3_SUPPORT
    typedef Qt::Orientations ExpandData;
    typedef Policy SizeType;

    enum {
        NoDirection = 0,
        Horizontally = 1,
        Vertically = 2,
        BothDirections = Horizontally | Vertically
    };
#endif


#if 0//###def QT3_SUPPORT
    bool mayShrinkHorizontally() const { return horizontalPolicy() & ShrinkFlag; }
    bool mayShrinkVertically() const { return verticalPolicy() & ShrinkFlag; }
    bool mayGrowHorizontally() const { return horizontalPolicy() & GrowFlag; }
    bool mayGrowVertically() const { return verticalPolicy() & GrowFlag; }

    Qt::Orientations expanding() const
    {
        return expandingDirections();
    }
#endif


#ifdef QT3_SUPPORT
public:
    QT3_SUPPORT_CONSTRUCTOR QSizePolicy(Policy hor, Policy ver, bool hfw)
        : data(hor | (ver<<HSize) | (hfw ? (1U<<2*HSize) : 0)) { }

    QT3_SUPPORT_CONSTRUCTOR QSizePolicy(Policy hor, Policy ver, uchar hors, uchar vers, bool hfw = false)
        : data(hor | (ver<<HSize) | (hfw ? (1U<<2*HSize) : 0)) {
        setHorizontalStretch(hors);
        setVerticalStretch(vers);
    }

    inline QT3_SUPPORT Policy horData() const { return static_cast<Policy>(data & HMask); }
    inline QT3_SUPPORT Policy verData() const { return static_cast<Policy>((data & VMask) >> HSize); }
    inline QT3_SUPPORT void setHorData(Policy d) { setHorizontalPolicy(d); }
    inline QT3_SUPPORT void setVerData(Policy d) { setVerticalPolicy(d); }

    inline QT3_SUPPORT uint horStretch() const { return horizontalStretch(); }
    inline QT3_SUPPORT uint verStretch() const { return verticalStretch(); }
    inline QT3_SUPPORT void setHorStretch(uchar sf) { setHorizontalStretch(sf); }
    inline QT3_SUPPORT void setVerStretch(uchar sf) { setVerticalStretch(sf); }
#endif

private:
    QSizePolicy(int i) : data(i) { }

    quint32 data;
};

inline void QSizePolicy::transpose() {
    Policy hData = horizontalPolicy();
    Policy vData = verticalPolicy();
    uchar hStretch = horizontalStretch();
    uchar vStretch = verticalStretch();
    setHorizontalPolicy(vData);
    setVerticalPolicy(hData);
    setHorizontalStretch(vStretch);
    setVerticalStretch(hStretch);
}

#endif // QSIZEPOLICY_H
