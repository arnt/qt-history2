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

#include <private/qeuckrcodec_p.h>
#include <private/qfontcodecs_p.h>


class KRTextCodecs : public QTextCodecPlugin
{
public:
    KRTextCodecs() {}

    QStringList names() const { return QStringList() << "eucKR" << "ksc5601.1987-0"; }
    QList<int> mibEnums() const { return QList<int>() << 38 << 36; }
    QTextCodec *createForMib(int);
    QTextCodec *createForName(const QString &);
};


QTextCodec *KRTextCodecs::createForMib(int mib)
{
    switch (mib) {
    case 36:
        return new QFontKsc5601Codec;
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
    if (name == "ksc5601.1987-0")
        return new QFontKsc5601Codec;

    return 0;
}


Q_EXPORT_PLUGIN(KRTextCodecs);
