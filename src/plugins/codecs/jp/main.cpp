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

#include "qeucjpcodec.h"
#include "qjiscodec.h"
#include "qsjiscodec.h"
#ifdef Q_WS_X11
#include "qfontjpcodec.h"
#endif

class JPTextCodecs : public QTextCodecPlugin
{
public:
    JPTextCodecs() {}

    QStringList names() const { return QStringList() << "eucJP" << "JIS7" << "SJIS" << "jisx0208.1983-0"; }
    QList<int> mibEnums() const { return QList<int>() << 16 << 17 << 18 << 63; }
    QTextCodec *createForMib(int);
    QTextCodec *createForName(const QString &);
};

QTextCodec *JPTextCodecs::createForMib(int mib)
{
    switch (mib) {
    case 16:
        return new QJisCodec;
    case 17:
        return new QSjisCodec;
    case 18:
        return new QEucJpCodec;
#ifdef Q_WS_X11
    case 15:
        return new QFontJis0201Codec;
    case 63:
        return new QFontJis0208Codec;
#endif
    default:
        ;
    }

    return 0;
}


QTextCodec *JPTextCodecs::createForName(const QString &name)
{
    if (name == "JIS7")
        return new QJisCodec;
    if (name == "SJIS")
        return new QSjisCodec;
    if (name == "eucJP")
        return new QEucJpCodec;
#ifdef Q_WS_X11
    if (name == "jisx0208.1983-0")
        return new QFontJis0208Codec;
    if (name == "jisx0201.1976-0")
        return new QFontJis0201Codec;
#endif
    return 0;
}


Q_EXPORT_PLUGIN(JPTextCodecs);
