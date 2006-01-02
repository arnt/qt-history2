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

#ifndef QINTERNAL_P_H
#define QINTERNAL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of a number of Qt sources files.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include "QtCore/qnamespace.h"
#include "QtCore/qlist.h"
#include "QtCore/qiodevice.h"
#include "QtCore/qvector.h"

class QWidget;
class QPainter;
class QPixmap;

class Q_CORE_EXPORT QRingBuffer
{
public:
    QRingBuffer(int growth = 4096);

    int nextDataBlockSize() const;
    char *readPointer() const;
    void free(int bytes);
    char *reserve(int bytes);
    void truncate(int bytes);

    bool isEmpty() const;

    int getChar();
    void putChar(char c);
    void ungetChar(char c);

    int size() const;
    void clear();
    int indexOf(char c) const;
    int readLine(char *data, int maxLength);
    bool canReadLine() const;

private:
    QList<QByteArray> buffers;
    int head, tail;
    int tailBuffer;
    int basicBlockSize;
    int bufferSize;
};

#endif // QINTERNAL_P_H
