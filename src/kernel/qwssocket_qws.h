/****************************************************************************
** $Id: //depot/qt/main/extensions/network/src/qsocketdevice.h#14 $
**
** Definition of QWSSocket and related classes.
**
** Created : 970521
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QWSSOCKETDEVICE_H
#define QWSSOCKETDEVICE_H

#ifndef QT_H
#include "qsocket.h"
#include "qserversocket.h"
#endif // QT_H


class QWSSocket : public QSocket
{
    Q_OBJECT
public:
    QWSSocket( QObject *parent=0, const char *name=0 );
   ~QWSSocket();
	        
    virtual void connectToLocalFile( const QString &file );

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QWSSocket( const QWSSocket & );
    QWSSocket &operator=( const QWSSocket & );
#endif
};


class QWSServerSocket : public QServerSocket
{
    Q_OBJECT
public:
    QWSServerSocket( const QString& file, int backlog = 0,
		     QObject *parent=0, const char *name=0 );
   ~QWSServerSocket();

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QWSServerSocket( const QWSServerSocket & );
    QWSServerSocket &operator=( const QWSServerSocket & );
#endif
};


#endif // QWSSOCKETDEVICE_H
