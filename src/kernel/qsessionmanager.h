/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsessionmanager.h#7 $
**
** Definition of QSessionManager class
**
** Created : 990510
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

#ifndef QSESSIONMANAGER_H
#define QSESSIONMANAGER_H

#ifndef QT_H
#include "qobject.h"
#include "qwindowdefs.h"
#include "qnamespace.h"
#include "qstring.h"
#include "qstringlist.h"
#endif // QT_H
#ifndef QT_NO_SESSIONMANAGER
class QSessionManagerData;

class Q_EXPORT  QSessionManager : public QObject
{
    Q_OBJECT
    QSessionManager( QApplication *app, QString &session );
    ~QSessionManager();
public:
    QString sessionId() const;
#if defined(_WS_X11_)
    void* handle() const;
#endif

    bool allowsInteraction();
    bool allowsErrorInteraction();
    void release();

    void cancel();

    enum RestartHint {
	RestartIfRunning,
	RestartAnyway,
	RestartImmediately,
	RestartNever
    };
    void setRestartHint( RestartHint );
    RestartHint restartHint() const;

    void setRestartCommand( const QStringList& );
    QStringList restartCommand() const;
    void setDiscardCommand( const QStringList& );
    QStringList discardCommand() const;

    void setProperty( const QString& name, const QString& value );
    void setProperty( const QString& name, const QStringList& value );

    bool isPhase2() const;
    void requestPhase2();

private:
    friend class QApplication;
    friend class QBaseApplication;
    QSessionManagerData* d;
};

#endif // QT_NO_SESSIONMANAGER
#endif // QSESSIONMANAGER_H
