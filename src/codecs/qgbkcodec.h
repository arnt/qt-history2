/****************************************************************************
** $Id$
**
** Definition of QGbkCodec template/macro class
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
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

// Contributed by Justin Yu <justiny@turbolinux.com.cn>
//                Sean Chen <seanc@turbolinux.com.cn>
// See the documentation for their license statement for the code as
// it was at the time of contribution.

#ifndef QGBKCODEC_H
#define QGBKCODEC_H

#ifndef QT_H
#include "qglobal.h"
#include "qtextcodec.h"
#endif // QT_H

#ifndef QT_NO_BIG_CODECS

#if defined(QT_PLUGIN)
#define Q_EXPORT_CODECS_CN
#else
#define Q_EXPORT_CODECS_CN Q_EXPORT
#endif

class Q_EXPORT_CODECS_CN QGbkCodec : public QTextCodec {
public:
    QGbkCodec();

    virtual int mibEnum() const;
    const char* name() const;
    const char* mimeName() const;

    QTextDecoder* makeDecoder() const;

#if defined(Q_USING)
    using QTextCodec::fromUnicode;
#endif
    QCString fromUnicode(const QString& uc, int& len_in_out) const;
    QString toUnicode(const char* chars, int len) const;

    int heuristicContentMatch(const char* chars, int len) const;
    int heuristicNameMatch(const char* hint) const;
};

#endif
#endif
