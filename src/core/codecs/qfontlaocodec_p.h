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

#ifndef QFONTLAOCODEC_P_H
#define QFONTLAOCODEC_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qfontencodings_x11.cpp and qfont_x11.cpp.  This header file may
// change from version to version without notice, or even be removed.
//
// We mean it.
//
//

#include "qglobal.h"
#include "qstring.h"
#include "qtextcodec.h"


#ifndef QT_NO_CODECS

class Q_CORE_EXPORT QFontLaoCodec : public QTextCodec
{
public:
    QFontLaoCodec();

    QByteArray name() const;
    int mibEnum() const;

    QString convertToUnicode(const char *, int, ConverterState *) const;
    QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const;
};

#endif // QT_NO_CODECS

#endif // QFONTCODECS_P_H
