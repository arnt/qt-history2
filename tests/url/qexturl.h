/****************************************************************************
** $Id: //depot/qt/main/tests/url/qexturl.h#2 $
**
** Implementation of QFileDialog class
**
** Created : 950429
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

#ifndef QEXTURL_H
#define QEXTURL_H

#include "qurl.h"
#include "qftp.h"

class QExtUrl : public QUrl
{
    Q_OBJECT
public:
    QExtUrl();
    QExtUrl( const QString& url );
    QExtUrl( const QUrl& url );
    QExtUrl( const QUrl& url, const QString& relUrl_ );

    virtual void listEntries( const QString &nameFilter, int filterSpec = QDir::DefaultFilter,
			      int sortSpec   = QDir::DefaultSort );
    virtual void mkdir( const QString &dirname );
    virtual QString toString() const;

private:
    QFtp ftp;
    
};

#endif
