/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef SSLCLIENT_H
#define SSLCLIENT_H

#include <QtGui/QWidget>
#include <QtNetwork/QAbstractSocket>
#include <QtNetwork/QSslSocket>

class QSslSocket;
class QToolButton;
class Ui_Form;

class SslClient : public QWidget
{
    Q_OBJECT
public:
    SslClient(QWidget *parent = 0);
    ~SslClient();
    
private slots:
    void updateEnabledState();
    void secureConnect();
    void socketStateChanged(QAbstractSocket::SocketState state);
    void socketEncrypted();
    void socketReadyRead();
    void sendData();
    void sslErrors(const QList<QSslError> &errors);
    void displayCertificateInfo();

private:
    void appendString(const QString &line);

    QSslSocket *socket;
    QToolButton *padLock;
    Ui_Form *form;
};

#endif
