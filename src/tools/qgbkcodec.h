/****************************************************************************
** $Id: $
**
** Definition of QGbkCodec template/macro class
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

// Contributed by Justin Yu <justiny@turbolinux.com.cn>
//                Sean Chen <seanc@turbolinux.com.cn>
// See the documentation for their license statement for the code as
// it was at the time of contribution.

#ifndef QGBKCODEC_H
#define QGBKCODEC_H
#ifndef QT_NO_CODECS

#ifndef QT_H
#include "qtextcodec.h"
#endif // QT_H

class Q_EXPORT QGbkCodec : public QTextCodec {
public:
    QGbkCodec();

    virtual int mibEnum() const;
    const char* name() const;

    QTextDecoder* makeDecoder() const;

    QCString fromUnicode(const QString& uc, int& len_in_out) const;
    QString toUnicode(const char* chars, int len) const;

    int heuristicContentMatch(const char* chars, int len) const;
    int heuristicNameMatch(const char* hint) const;
};

#endif
#endif
