/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsocketnotifier.h#22 $
**
** Definition of QSocketNotifier class
**
** Created : 951114
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QSOCKETNOTIFIER_H
#define QSOCKETNOTIFIER_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H


class Q_EXPORT QSocketNotifier : public QObject
{
    Q_OBJECT
public:
    enum Type { Read, Write, Exception };

    QSocketNotifier( int socket, Type, QObject *parent=0, const char *name=0 );
   ~QSocketNotifier();

    int		 socket()	const;
    Type	 type()		const;

    bool	 isEnabled()	const;
    virtual void setEnabled( bool );

signals:
    void	 activated( int socket );

protected:
    bool	 event( QEvent * );

private:
    int		 sockfd;
    Type	 sntype;
    bool	 snenabled;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QSocketNotifier( const QSocketNotifier & );
    QSocketNotifier &operator=( const QSocketNotifier & );
#endif
};


inline int QSocketNotifier::socket() const
{ return sockfd; }

inline QSocketNotifier::Type QSocketNotifier::type() const
{ return sntype; }

inline bool QSocketNotifier::isEnabled() const
{ return snenabled; }


#endif // QSOCKETNOTIFIER_H
