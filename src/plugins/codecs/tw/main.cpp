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

#include "qbig5codec.h"


class TWTextCodecs : public QTextCodecPlugin
{
public:
    TWTextCodecs() {}

    QStringList names() const { return QStringList() << "Big5"
#ifdef Q_WS_X11
                                                     << "big5*-0"
                                                     << "big5hkscs-0"
#endif
            ;
    }
    QList<int> mibEnums() const { return QList<int>() << 2026
#ifdef Q_WS_X11
                                                      << -2026
                                                      << -2101
#endif
            ;
    }
    QTextCodec *createForMib(int);
    QTextCodec *createForName(const QString &);
};

QTextCodec *TWTextCodecs::createForMib(int mib)
{
    switch (mib) {
#ifdef Q_WS_X11
    case -2026:
        return new QFontBig5Codec;
    case -2101:
        return new QFontBig5hkscsCodec;
#endif
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
#ifdef Q_WS_X11
    if (name == "big5*-0")
        return new QFontBig5Codec;
    if (name ==  "big5hkscs-0")
        return new QFontBig5hkscsCodec;
#endif
    return 0;
}


Q_EXPORT_PLUGIN(TWTextCodecs);

