/****************************************************************************
** $Id$
**
** Definition of QTsciiCodec class
**
** Copyright (C) 2000 Hans Petter Bieker <bieker@kde.org>.  All rights reserved.
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** Contributed by Hans Petter Bieker <bieker@kde.org>
** See the documentation for their license statement for the code as
** it was at the time of contribution.
**
**********************************************************************/

#ifndef QTSCIICODEC_H
#define QTSCIICODEC_H

#ifndef QT_H
#include "qtextcodec.h"
#endif // QT_H

#ifndef QT_NO_CODECS

class Q_EXPORT QTsciiCodec : public QTextCodec {
public:
    virtual int mibEnum() const;
    const char* name() const;

#if !defined(Q_NO_USING_KEYWORD)
    using QTextCodec::fromUnicode;
#endif
    QCString fromUnicode(const QString& uc, int& lenInOut) const;
    QString toUnicode(const char* chars, int len) const;

    int heuristicContentMatch(const char* chars, int len) const;
    int heuristicNameMatch(const char* hint) const;
};

#endif

#endif
