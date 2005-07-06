/****************************************************************************
**
** Copyright (C) 2004-2005 Trolltech AS. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef RATECONTROLLER_H
#define RATECONTROLLER_H

#include <QtCore/QObject>

class PeerWireClient;
template<typename T> class QList;
class QTimerEvent;

class RateController : public QObject
{
    Q_OBJECT
public:
    RateController();
    static RateController *instance();

    int uploadLimit() const;
    void setUploadLimit(int bytesPerSecond);

    int downloadLimit() const;
    void setDownloadLimit(int bytesPerSecond);

    void addClient(PeerWireClient *client);
    void removeClient(PeerWireClient *client);

protected:
    void timerEvent(QTimerEvent *event);

private:
    int transmitTimer;
    int timerDelay;
    int uploadLimitBytes;
    int downloadLimitBytes;
    QList<PeerWireClient *> clients;
};

#endif

