/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsocknot.h#14 $
**
** Definition of QSocketNotifier class
**
** Created : 951114
**
** Copyright (C) 1995-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QSOCKNOT_H
#define QSOCKNOT_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H


class QSocketNotifier : public QObject
{
    Q_OBJECT
public:
    enum Type { Read, Write, Exception };

    QSocketNotifier( int socket, Type, QObject *parent=0, const char *name=0 );
   ~QSocketNotifier();

    int		socket()	const;
    Type	type()		const;

    bool	isEnabled()	const;
    void	setEnabled( bool );

signals:
    void	activated( int socket );

protected:
    bool	event( QEvent * );

private:
    int		sockfd;
    Type	sntype;
    bool	snenabled;

private:	// Disabled copy constructor and operator=
    QSocketNotifier( const QSocketNotifier & );
    QSocketNotifier &operator=( const QSocketNotifier & );
};


inline int QSocketNotifier::socket() const
{ return sockfd; }

inline QSocketNotifier::Type QSocketNotifier::type() const
{ return sntype; }

inline bool QSocketNotifier::isEnabled() const
{ return snenabled; }


#endif // QSOCKNOT_H
