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

#include "qeuckrcodec.h"


class KRTextCodecs : public QTextCodecPlugin
{
public:
    KRTextCodecs() {}

    QStringList names() const { return QStringList() << "eucKR"
#ifdef Q_WS_X11
                                                     << "ksc5601.1987-0"
#endif
            ; }
    QList<int> mibEnums() const { return QList<int>() << 38
#ifdef Q_WS_X11
                                                      << 36
#endif
            ; }
    QTextCodec *createForMib(int);
    QTextCodec *createForName(const QString &);
};


QTextCodec *KRTextCodecs::createForMib(int mib)
{
    switch (mib) {
#ifdef Q_WS_X11
    case 36:
        return new QFontKsc5601Codec;
#endif
    case 38:
        return new QEucKrCodec;
    default:
        ;
    }

    return 0;
}


QTextCodec *KRTextCodecs::createForName(const QString &name)
{
    if (name == "eucKR")
        return new QEucKrCodec;
#ifdef Q_WS_X11
    if (name == "ksc5601.1987-0")
        return new QFontKsc5601Codec;
#endif
    return 0;
}


Q_EXPORT_PLUGIN(KRTextCodecs);
