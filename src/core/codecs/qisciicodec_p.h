/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QISCIICODEC_P_H
#define QISCIICODEC_P_H

#include "qtextcodec.h"

#ifndef QT_NO_CODECS

class QIsciiCodec : public QTextCodec {
public:
    QIsciiCodec(int i):idx(i){}

    virtual int         mibEnum() const;
    virtual const char* mimeName () const;
    const   char*       name() const;

#if !defined(Q_NO_USING_KEYWORD)
    using QTextCodec::fromUnicode;
#endif

    QByteArray fromUnicode(const QString& uc, int& len_in_out) const;
    QString  toUnicode(const char* chars, int len) const;

    int heuristicContentMatch(const char* chars, int len) const;
    int heuristicNameMatch(const char* hint) const;

private:
    int idx;
};

#endif
#endif
