/****************************************************************************
** $Id: //depot/qt/main/src/tools/qbig5codec.h#4 $
**
** Definition of QBig5Codec class
**
** Created : 990713
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the tools
** module and therefore may only be used if the tools module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

// Most of the code here was originally written by Ming-Che Chuang and
// is included in Qt with the author's permission, and the grateful
// thanks of the Trolltech team.

#ifndef QBIG5CODEC_H
#define QBIG5CODEC_H

#ifndef QT_H
#include "qtextcodec.h"
#endif // QT_H

#ifndef QT_NO_CODECS

class QBig5Codec : public QTextCodec {
public:
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
