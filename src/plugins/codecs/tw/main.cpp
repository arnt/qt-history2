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

#include <qtextcodecplugin.h>
#include <qtextcodec.h>
#include <qstringlist.h>

#include <private/qbig5codec_p.h>
#include <private/qfontcodecs_p.h>


class TWTextCodecs : public QTextCodecPlugin
{
public:
    TWTextCodecs() {}

    QStringList names() const { return QStringList() << "Big5" << "big5*-0"; }
    QList<int> mibEnums() const { return QList<int>() << 2026 << -2026; }
    QTextCodec *createForMib(int);
    QTextCodec *createForName(const QString &);
};

QTextCodec *TWTextCodecs::createForMib(int mib)
{
    switch (mib) {
    case -2026:
        return new QFontBig5Codec;
    case 2026:
        return new QBig5Codec;
    default:
        ;
    }

    return 0;
}


QTextCodec *TWTextCodecs::createForName(const QString &name)
{
    if (name == "Big5")
        return new QBig5Codec;
    if (name == "big5*-0")
        return new QFontBig5Codec;

    return 0;
}


Q_EXPORT_PLUGIN(TWTextCodecs);

