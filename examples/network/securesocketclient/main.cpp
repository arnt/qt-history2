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

#include <QApplication>
#include <QMessageBox>

#include "sslclient.h"

int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(securesocketclient);

    QApplication app(argc, argv);

    if (!QSslSocket::supportsSsl()) {
	QMessageBox::information(0, "Secure Socket Client",
				 "This system does not support OpenSSL.");
        return -1;
    }

    SslClient client;
    client.show();

    return app.exec();
}
