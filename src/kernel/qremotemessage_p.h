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

#include <qpixmap.h>
class QString;
class QSocket;

class Q_EXPORT QRemoteMessage
{
public:
    enum retValue {Accepted, Rejected};

    QRemoteMessage();
    QRemoteMessage(QString event, QString params);
    QRemoteMessage(QString event, QString params, const QPixmap *pixmap);
    QRemoteMessage(QString event, QString params, const QByteArray *array);
    virtual ~QRemoteMessage();
	
    virtual void send(QSocket *socket);
    virtual void reply(QSocket *socket, int retValue);
    virtual bool receive(QSocket *socket);

    bool hasPixmap();
    bool getPixmap(QPixmap *&pixmap);

    bool hasByteArray();
    bool getByteArray(QByteArray *&byteArray);

    uint msgId();
    bool isReply();
    int retValue();
    QString event();
    QString params();

    void reset();

private:
    static uint m_nextMsgId;

protected:
    virtual uint primarySize();
    uint magicId();
    bool primaryDataRead;

    uint	m_magicId;
    Q_UINT32	m_size;
    uint	m_msgId;
    short	m_isReply;
    int		m_retValue;
    QCString	m_event;
    QCString	m_params;
    QPixmap	m_pixmap;
    QByteArray	m_byteArray;
};

#endif
