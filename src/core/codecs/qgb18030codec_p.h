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

// Contributed by James Su <suzhe@gnuchina.org>

#ifndef QGB18030CODEC_P_H
#define QGB18030CODEC_P_H

#ifndef QT_H
#include "qtextcodec.h"
#endif // QT_H


#ifndef QT_NO_BIG_CODECS

#if defined(QT_PLUGIN)
#define Q_EXPORT_CODECS_CN
#else
#define Q_EXPORT_CODECS_CN Q_CORE_EXPORT
#endif

class Q_EXPORT_CODECS_CN QGb18030Codec : public QTextCodec {
public:
    QGb18030Codec();

    int mibEnum() const;
    const char* name() const;

    QTextDecoder* makeDecoder() const;

#if !defined(Q_NO_USING_KEYWORD)
    using QTextCodec::fromUnicode;
#endif
    QByteArray fromUnicode(const QString& uc, int& lenInOut) const;
    QString toUnicode(const char* chars, int len) const;

    int heuristicContentMatch(const char* chars, int len) const;
    int heuristicNameMatch(const char* hint) const;
};

class Q_EXPORT_CODECS_CN QGbkCodec : public QGb18030Codec {
public:
    QGbkCodec();

    int mibEnum() const;
    const char* name() const;

    QTextDecoder* makeDecoder() const;

#if !defined(Q_NO_USING_KEYWORD)
    using QGb18030Codec::fromUnicode;
#endif
    QByteArray fromUnicode(const QString& uc, int& lenInOut) const;
    QString toUnicode(const char* chars, int len) const;

    int heuristicContentMatch(const char* chars, int len) const;
    int heuristicNameMatch(const char* hint) const;
};

class Q_EXPORT_CODECS_CN QGb2312Codec : public QGb18030Codec {
public:
    QGb2312Codec();

    int mibEnum() const;
    const char* name() const;

    QTextDecoder* makeDecoder() const;

#if !defined(Q_NO_USING_KEYWORD)
    using QGb18030Codec::fromUnicode;
#endif
    QByteArray fromUnicode(const QString& uc, int& lenInOut) const;
    QString toUnicode(const char* chars, int len) const;

    int heuristicContentMatch(const char* chars, int len) const;
    int heuristicNameMatch(const char* hint) const;
};

#endif
#endif
