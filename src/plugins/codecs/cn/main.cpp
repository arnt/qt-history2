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

#include "qgb18030codec.h"


class CNTextCodecs : public QTextCodecPlugin
{
public:
    CNTextCodecs() {}

    QStringList names() const  {
        return QStringList() << "GB18030" << "GBK" << "GB2312"
#ifdef Q_WS_X11
                             << "gb2312.1980-0"
                             << "gbk-0"
                             << "gb18030-0"
#endif
        ;
    }
    QList<int> mibEnums() const {
        return QList<int>() << 114 << 113 << 2025
#ifdef Q_WS_X11
                            << 57 << -113
                            << -114
#endif
            ;
    }

    QTextCodec *createForMib(int);
    QTextCodec *createForName(const QString &);
};

QTextCodec *CNTextCodecs::createForMib(int mib)
{
    switch (mib) {
    case 2025:
        return new QGb2312Codec;
    case 113:
        return new QGbkCodec;
    case -2025:
        return new QGb18030Codec;
#ifdef Q_WS_X11
    case 57:
        return new QFontGb2312Codec;
    case -113:
        return new QFontGbkCodec;
    case -114:
        return new QFontGb18030_0Codec;
#endif
    default:
        ;
    }

    return 0;
}


QTextCodec *CNTextCodecs::createForName(const QString &name)
{
    if (name == "GB18030")
        return new QGb18030Codec;
    if (name == "GBK" || name == "gbk-0")
        return new QGbkCodec;
    if (name == "gb2312.1980-0")
        return new QGb2312Codec;
#ifdef Q_WS_X11
    if (name == "gb2312.1980-0")
        return new QFontGb2312Codec;
    if (name == "gbk-0")
        return new QFontGbkCodec;
    if (name == "gb18030-0")
        return new QFontGb18030_0Codec;
#endif
    return 0;
}


Q_EXPORT_PLUGIN(CNTextCodecs);
