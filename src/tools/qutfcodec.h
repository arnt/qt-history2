/****************************************************************************
** $Id: //depot/qt/main/src/tools/qutfcodec.h#4 $
**
** Definition of QEucCodec class
**
** Created : 981015
**
** Copyright (C) 1998-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QUTFCODEC_H
#define QUTFCODEC_H

#include "qtextcodec.h"

class QUtf8Codec : public QTextCodec {
public:
    virtual int mib() const;
    const char* name() const;

    QTextDecoder* makeDecoder() const;

    QCString fromUnicode(const QString& uc, int& len_in_out) const;

    int heuristicContentMatch(const char* chars, int len) const;
};

class QUtf16Codec : public QTextCodec {
public:
    virtual int mib() const;
    const char* name() const;

    QTextDecoder* makeDecoder() const;
    QTextEncoder* makeEncoder() const;

    int heuristicContentMatch(const char* chars, int len) const;
};

#endif
