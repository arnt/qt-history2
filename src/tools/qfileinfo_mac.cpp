/****************************************************************************
** $Id: //depot/qt/main/src/tools/qfileinfo.cpp#61 $
**
** Implementation of QFileInfo class
**
** Created : 950628
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

#include "qglobal.h"
#include "qfileinfo.h"
#include "qfiledefs.h"
#include "qdatetime.h"
#include "qdir.h"

void QFileInfo::slashify( QString& )
{
    return;
}

extern bool qt_file_access( const QString& fn, int t );

bool QFileInfo::isFile() const
{
    return true;
}

bool QFileInfo::isDir() const
{
    return true;
}

bool QFileInfo::isSymLink() const
{
    return true;
}


QString QFileInfo::readLink() const
{
    QString r;
    return r;
}

static const uint nobodyID = (uint) -2;

QString QFileInfo::owner() const
{
    return QString::null;
}

uint QFileInfo::ownerId() const
{
    return nobodyID;
}

QString QFileInfo::group() const
{
    return QString::null;
}

uint QFileInfo::groupId() const
{
    return nobodyID;
}


bool QFileInfo::permission( int permissionSpec ) const
{
    return TRUE;
}

uint QFileInfo::size() const
{
    return 0;
}


QDateTime QFileInfo::lastModified() const
{
    QDateTime dt;
    return dt;
}

QDateTime QFileInfo::lastRead() const
{
    QDateTime dt;
    return dt;
}


void QFileInfo::doStat() const
{
}
