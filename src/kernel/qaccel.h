/****************************************************************************
**
** Definition of QAccel class.
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

#ifndef QACCEL_H
#define QACCEL_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H

#if defined(QT_COMPAT) || defined(QT_INCLUDE_COMPAT)
#include "qkeysequence.h"
#endif

#ifndef QT_NO_ACCEL

class QAccelPrivate;
class QWidget;

class Q_GUI_EXPORT QAccel : public QObject			// accelerator class
{
    Q_OBJECT
public:
    QAccel( QWidget *parent, const char *name=0 );
    QAccel( QWidget* watch, QObject *parent, const char *name=0 );
    ~QAccel();

    bool isEnabled() const;
    void setEnabled( bool );

    uint count() const;

    int insertItem( const QKeySequence& key, int id=-1);
    void removeItem( int id );
    void clear();

    QKeySequence key( int id );
    int findKey( const QKeySequence& key ) const;

    bool isItemEnabled( int id ) const;
    void setItemEnabled( int id, bool enable );

    bool connectItem( int id,  const QObject *receiver, const char* member );
    bool disconnectItem( int id,  const QObject *receiver, const char* member );

    void setWhatsThis( int id, const QString& );
    QString whatsThis( int id ) const;
    void setIgnoreWhatsThis( bool );
    bool ignoreWhatsThis() const;

    static QKeySequence shortcutKey( const QString & );
#ifdef QT_COMPAT
    static QT_COMPAT QString keyToString(const QKeySequence &k );
    static QT_COMPAT QKeySequence stringToKey( const QString & );
#endif

signals:
    void activated( int id );
    void activatedAmbiguously( int id );

private:
    Q_DECL_PRIVATE(QAccel);

private:
#if defined(Q_DISABLE_COPY)
    QAccel( const QAccel & );
    QAccel &operator=( const QAccel & );
#endif
    friend class QAccelManager;
};

#ifdef QT_COMPAT
inline QString QAccel::keyToString( const QKeySequence &k )
{
    return (QString) k;
}

inline QKeySequence QAccel::stringToKey( const QString & s )
{
    return QKeySequence( s );
}
#endif

#endif // QT_NO_ACCEL
#endif // QACCEL_H
