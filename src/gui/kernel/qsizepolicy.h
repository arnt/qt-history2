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

#include "QtCore/qglobal.h"

// Documentation is in qabstractlayout.cpp.

class Q_GUI_EXPORT QSizePolicy
{
private:
    enum SizePolicy {
        HSize = 6,
        HMask = 0x3f,
        VMask = HMask << HSize,
        MayGrow = 1,
        ExpMask = 2,
        MayShrink = 4
    };
public:
    enum SizeTypeFlag {
        Fixed = 0,
        Minimum = MayGrow,
        Maximum = MayShrink,
        Preferred = MayGrow | MayShrink,
        MinimumExpanding = MayGrow | ExpMask,
        Expanding = MayGrow | MayShrink | ExpMask,
        Ignored = ExpMask /* magic value */
    };

    Q_DECLARE_FLAGS(SizeType, SizeTypeFlag)

    enum ExpandData {
        NoDirection = 0,
        Horizontally = 1,
        Vertically = 2,
#ifdef QT_COMPAT
        Horizontal = Horizontally,
        Vertical = Vertically,
#endif
        BothDirections = Horizontally | Vertically
    };

    QSizePolicy() : data(0) { }

    QSizePolicy(SizeType horizontal, SizeType vertical)
        : data(horizontal | (vertical<<HSize)) { }

    SizeType horizontalData() const { return static_cast<SizeTypeFlag>(data & HMask); }
    SizeType verticalData() const { return static_cast<SizeTypeFlag>((data & VMask) >> HSize); }

    void setHorizontalData(SizeType d) { data = (data & ~HMask) | d; }
    void setVerticalData(SizeType d) { data = (data & ~(HMask << HSize)) | (d << HSize); }

    bool mayShrinkHorizontally() const { return horizontalData() & MayShrink || horizontalData() == Ignored; }
    bool mayShrinkVertically() const { return verticalData() & MayShrink || verticalData() == Ignored; }
    bool mayGrowHorizontally() const { return horizontalData() & MayGrow || horizontalData() == Ignored; }
    bool mayGrowVertically() const { return verticalData() & MayGrow || verticalData() == Ignored; }

    ExpandData expanding() const
    {
        return static_cast<ExpandData>(int(verticalData() & ExpMask ? Vertically : 0)
                                       | int(horizontalData() & ExpMask ? Horizontally : 0));
    }


    void setHeightForWidth(bool b) { data = b ? (data | (1 << 2*HSize)) : (data & ~(1 << 2*HSize));  }
    bool hasHeightForWidth() const { return data & (1 << 2*HSize); }

    bool operator==(const QSizePolicy& s) const { return data == s.data; }
    bool operator!=(const QSizePolicy& s) const { return data != s.data; }

    uint horizontalStretch() const { return data >> 24; }
    uint verticalStretch() const { return (data >> 16) & 0xff; }
    void setHorizontalStretch(uchar stretchFactor) { data = (data&0x00ffffff) | (uint(stretchFactor)<<24); }
    void setVerticalStretch(uchar stretchFactor) { data = (data&0xff00ffff) | (uint(stretchFactor)<<16); }

    void transpose();


#ifdef QT_COMPAT
public:
    QT_COMPAT_CONSTRUCTOR QSizePolicy(SizeType hor, SizeType ver, bool hfw)
        : data(hor | (ver<<HSize) | (hfw ? (1U<<2*HSize) : 0)) { }

    QT_COMPAT_CONSTRUCTOR QSizePolicy(SizeType hor, SizeType ver, uchar hors, uchar vers, bool hfw = false)
        : data(hor | (ver<<HSize) | (hfw ? (1U<<2*HSize) : 0)) {
        setHorizontalStretch(hors);
        setVerticalStretch(vers);
    }

    inline QT_COMPAT SizeType horData() const { return static_cast<SizeTypeFlag>(data & HMask); }
    inline QT_COMPAT SizeType verData() const { return static_cast<SizeTypeFlag>((data & VMask) >> HSize); }
    inline QT_COMPAT void setHorData(SizeType d) { setHorizontalData(d); }
    inline QT_COMPAT void setVerData(SizeType d) { setVerticalData(d); }

    inline QT_COMPAT uint horStretch() const { return horizontalStretch(); }
    inline QT_COMPAT uint verStretch() const { return verticalStretch(); }
    inline QT_COMPAT void setHorStretch(uchar sf) { setHorizontalStretch(sf); }
    inline QT_COMPAT void setVerStretch(uchar sf) { setVerticalStretch(sf); }
#endif

private:
    QSizePolicy(int i) : data(i) { }

    Q_UINT32 data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QSizePolicy::SizeType);

inline void QSizePolicy::transpose() {
    SizeType hData = horizontalData();
    SizeType vData = verticalData();
    uchar hStretch = horizontalStretch();
    uchar vStretch = verticalStretch();
    setHorizontalData(vData);
    setVerticalData(hData);
    setHorizontalStretch(vStretch);
    setVerticalStretch(hStretch);
}

#endif // QSIZEPOLICY_H
