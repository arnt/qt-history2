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

#include "qdebug.h"
#include "qhexdump.h"
#include "qfile.h"

// Output hexdump to a QString
QString QHexDump::output() {
    QString result;
    QTextStream strm(&result, QFile::WriteOnly);
    outstrm = &strm;
    hexDump();
    return result;
}

#ifndef QT_NO_DEBUG
QDebug &operator<<(QDebug &dbg, QHexDump *hd) {
    if (!hd)
        return dbg << "QHexDump(0x0)";
    QString result = hd->output();
    dbg.nospace() << result;
    return dbg.space();
}

// GCC & Intel wont handle references here
QDebug operator<<(QDebug dbg, QHexDump hd) {
    return dbg << &hd;
}
#endif
