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
    friend QDataStream &operator<<( QDataStream &, const QOffset & );
public:
    QOffset() {}
#if defined(QT_LARGEFILE_SUPPORT)
    QOffset( Q_UINT64 o ) : offset( o ) {}
#else
    QOffset( Q_UINT32 o ) : offset( o ) {}
#endif
    QOffset( const QOffset &o ) : offset(o.offset) {}
    QOffset &operator=( const QOffset &o ) { offset = o.offset; return *this; }
    bool operator==( const QOffset &o ) const { return offset == o.offset; }
    bool operator!=( const QOffset &o ) const { return !(operator==(o)); }
    bool operator<( const QOffset &o ) const { return offset < o.offset; }
    bool operator<=( const QOffset &o ) const { return !o.operator<( * this ); }
    bool operator>( const QOffset &o ) const { return o.operator<( * this ); }
    bool operator>=( const QOffset &o ) const { return !operator<( o ); }
    QOffset &operator+=( long n ) { offset += n; return *this; }
    QOffset &operator+=( ulong n ) { offset += n; return *this; }
    const QOffset operator+( long n ) { return *this += n; }
    const QOffset operator+( ulong n ) { return *this += n; }
    QOffset operator++() { offset++; return *this; }
    QOffset operator++(int) { return QOffset( offset++ ); }
    QOffset &operator-=( long n ) { offset -= n; return *this; }
    QOffset &operator-=( ulong n ) { offset -= n; return *this; }
    QOffset &operator-=( int n ) { offset -= n; return *this; }
    QOffset &operator-=( uint n ) { offset -= n; return *this; }
    const QOffset operator-( long n ) { return *this -= n; }
    const QOffset operator-( ulong n ) { return *this -= n; }
    const QOffset operator-( int n ) { return *this -= n; }
    const QOffset operator-( uint n ) { return *this -= n; }
    QOffset operator--() { offset--; return *this; }
    QOffset operator--(int) { return QOffset( offset-- ); }
private:
#if defined(QT_LARGEFILE_SUPPORT)
    Q_UINT64 offset;
#else
    Q_UINT32 offset;
#endif
};

/*****************************************************************************
  QOffset stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream &operator<<( QDataStream &, const QOffset & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QOffset & );
#endif

#endif
