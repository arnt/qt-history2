/****************************************************************************
** $Id: //depot/qt/main/src/tools/qutfcodec.h#8 $
**
** Definition of QEucCodec class
**
** Created : 981015
**
** Copyright (C) 1998-2000 Troll Tech AS.  All rights reserved.
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

#ifndef QUTFCODEC_H
#define QUTFCODEC_H

#ifndef QT_H
#include "qtextcodec.h"
#endif // QT_H

#ifndef QT_NO_TEXTCODEC

class Q_EXPORT QUtf8Codec : public QTextCodec {
public:
    virtual int mibEnum() const;
    const char* name() const;

    QTextDecoder* makeDecoder() const;

    QCString fromUnicode(const QString& uc, int& len_in_out) const;

    int heuristicContentMatch(const char* chars, int len) const;
};

class Q_EXPORT QUtf16Codec : public QTextCodec {
public:
    virtual int mibEnum() const;
    const char* name() const;

    QTextDecoder* makeDecoder() const;
    QTextEncoder* makeEncoder() const;

    int heuristicContentMatch(const char* chars, int len) const;
};

#endif //QT_NO_TEXTCODEC
#endif // QUTFCODEC_H
