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

#ifndef QASSISTANTCLIENT_H
#define QASSISTANTCLIENT_H

#include <qobject.h>
#include <qstringlist.h>

class QSocket;
class QProcess;

class QAssistantClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY( bool open READ isOpen )

public:
    QAssistantClient( const QString &path, QObject *parent = 0);
    ~QAssistantClient();

    bool isOpen() const;

    void setArguments( const QStringList &args );

public slots:
    virtual void openAssistant();
    virtual void closeAssistant();
    virtual void showPage( const QString &page );

signals:
    void assistantOpened();
    void assistantClosed();
    void error( const QString &msg );

private slots:
    void socketConnected();
    void socketConnectionClosed();
    void readPort();
    void socketError( int );
    void readStdError();

private:
    QSocket *socket;
    QProcess *proc;
    Q_UINT16 port;
    QString host, assistantCommand, pageBuffer;
    bool opened;
};

#endif
