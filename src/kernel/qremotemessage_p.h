/****************************************************************************
** $Id: $
**
** Definition of QRemoteMessage class
**
** Created : 010301
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QREMOTEMESSAGE_H
#define QREMOTEMESSAGE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Remote Control. This header file may change from version to 
// version without notice, or even be removed.
//
// We mean it.
//
//

#ifndef QT_H
#include "qpixmap.h"
#endif // QT_H

#ifndef QT_NO_REMOTE

class QString;
class QSocket;

class Q_EXPORT QRemoteMessage
{
public:
    QRemoteMessage();
    QRemoteMessage(const QString &eventName, const QString &eventInfo);
    QRemoteMessage(const QString &eventName, const QString &eventInfo, const QPixmap *pixmap);
    QRemoteMessage(const QString &eventName, const QString &eventInfo, const QByteArray *array);
    virtual ~QRemoteMessage();
	
    virtual void send(QSocket *socket);
    virtual void reply(QSocket *socket, const QString &result);
    virtual bool receive(QSocket *socket);

    bool hasPixmap() const;
    bool getPixmap(QPixmap *&pixmap);

    bool hasByteArray() const;
    bool getByteArray(QByteArray *&byteArray);

    uint messageId() const;
    bool isReply() const;
    QString result() const;
    QString msgType() const;
    QString message() const;

    void reset();

private:
    static uint next_msg_id;

protected:
    virtual uint primarySize();
    uint magicId() const;

    bool	primary_data_read;
    uint	magic_id;
    Q_UINT32	size;
    uint	msg_id;
    short	is_reply;
    QString	retvalue;
    QString	msg_type;
    QString	msg;
    QPixmap	internal_pixmap;
    QByteArray	internal_bytearray;
};

#endif //QT_NO_REMOTE

#endif
