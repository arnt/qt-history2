/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSIMPLECODEC_P_H
#define QSIMPLECODEC_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtCore/qtextcodec.h"

#ifndef QT_NO_TEXTCODEC

template <typename T> class QAtomicPointer;

class QSimpleTextCodec: public QTextCodec
{
public:
    enum { numSimpleCodecs = 30 };
    explicit QSimpleTextCodec(int);
    ~QSimpleTextCodec();

    QString convertToUnicode(const char *, int, ConverterState *) const;
    QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const;

    QByteArray name() const;
    QList<QByteArray> aliases() const;
    int mibEnum() const;

private:
    int forwardIndex;
    mutable QAtomicPointer<QByteArray> reverseMap;
};

#endif // QT_NO_TEXTCODEC

#endif // QSIMPLECODEC_P_H
