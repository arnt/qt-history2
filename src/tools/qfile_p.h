/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QFILE_P_H
#define QFILE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qfileinfo.h and qfile.h.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QT_H
#include "qglobal.h"
#endif // QT_H

class QOffset
{
public:
#if defined(QT_LARGEFILE_SUPPORT)
    typedef Q_ULLONG type;
#else
    typedef Q_ULONG type;
#endif
    QOffset() {}
    QOffset(Q_ULONG o) : offset(o) {}
    QOffset(const QOffset &o) : offset(o.offset) {}
    QOffset &operator=(Q_ULONG o) { offset = o; return *this; }
    QOffset &operator=(const QOffset &o) { offset = o.offset; return *this; }
    QOffset &operator+=(Q_ULONG n) { offset += n; return *this; }
    QOffset operator++() { ++offset; return *this; }
    QOffset operator++(int) { QOffset o(*this); ++offset; return o; }
    QOffset &operator-=(Q_ULONG n) { offset -= n; return *this; }
    QOffset operator--() { --offset; return *this; }
    QOffset operator--(int) { QOffset o(*this); --offset; return o; }
    operator Q_ULONG() const { return (Q_ULONG)offset; }
private:
    Q_ULONG offset;
};

/*****************************************************************************
  QOffset stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream &operator<<(QDataStream &, const QOffset &);
Q_EXPORT QDataStream &operator>>(QDataStream &, QOffset &);
#endif

#endif
