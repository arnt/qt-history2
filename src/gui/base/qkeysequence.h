/****************************************************************************
**
** Definition of QKeySequence class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QKEYSEQUENCE_H
#define QKEYSEQUENCE_H

#ifndef QT_H
#ifndef QT_H
#include "qnamespace.h"
#include "qstring.h"
#endif // QT_H
#endif

#ifndef QT_NO_ACCEL

/*****************************************************************************
  QKeySequence stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
class QKeySequence;
Q_GUI_EXPORT QDataStream &operator<<( QDataStream &, const QKeySequence & );
Q_GUI_EXPORT QDataStream &operator>>( QDataStream &, QKeySequence & );
#endif

class QKeySequencePrivate;

class Q_GUI_EXPORT QKeySequence : public Qt
{
public:
    QKeySequence();
    QKeySequence( const QString& key );
    QKeySequence( int key );
    QKeySequence( int k1, int k2, int k3 = 0, int k4 = 0 );
    QKeySequence( const QKeySequence & );
    ~QKeySequence();

    uint count() const;
    bool isEmpty() const;
    Qt::SequenceMatch matches( const QKeySequence & ) const;

    operator QString() const;
    operator int () const;
    int operator[]( uint ) const;
    QKeySequence &operator=( const QKeySequence & );
    bool operator==( const QKeySequence& ) const;
    bool operator!= ( const QKeySequence& ) const;

private:
    static int decodeString( const QString & );
    static QString encodeString( int );
    int assign( QString );
    void setKey( int key, int index );

    QKeySequencePrivate* d;

    friend Q_GUI_EXPORT QDataStream &operator<<( QDataStream &, const QKeySequence & );
    friend Q_GUI_EXPORT QDataStream &operator>>( QDataStream &, QKeySequence & );
    friend class QAccelManager;
};

#ifndef QT_NO_DEBUG
Q_GUI_EXPORT QDebug operator<<(QDebug, const QKeySequence &);
#endif

#else

class Q_GUI_EXPORT QKeySequence : public Qt
{
public:
    QKeySequence() {}
    QKeySequence( int ) {}
};

#endif //QT_NO_ACCEL

#endif
