/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsessionmanager.h#3 $
**
** Definition of QSessionManager class
**
** Created : 990510
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
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

#ifndef QSESSIONMANAGER_H
#define QSESSIONMANAGER_H

#include "qnamespace.h"
#include "qstring.h"
#include "qstringlist.h"

class QApplication;
class QSessionManagerData;

class Q_EXPORT  QSessionManager : public Qt
{
    QSessionManager( QApplication *app, QString &session );
    ~QSessionManager();
public:
    QString sessionId();

    bool allowsInteraction();
    bool allowsErrorInteraction();
    void release();

    void cancel();

    enum RestartHint { RestartIfRunning, RestartAnyway, RestartImmediately, RestartNever };
    void setRestartHint( RestartHint );
    RestartHint restartHint() const;

    void setRestartCommand( const QStringList& );
    QStringList restartCommand() const;
    void setDiscardCommand( const QStringList& );
    QStringList discardCommand() const;

    void setProperty( const QString& name, const QString& value );
    void setProperty( const QString& name, const QStringList& value );

    bool isPhase2();
    void requestPhase2();

private:
    friend class QApplication;
    QSessionManagerData* d;
};


#endif
