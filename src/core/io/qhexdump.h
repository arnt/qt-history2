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

#ifndef QHEXDUMP_H
#define QHEXDUMP_H

#include <qbytearray.h>
#include <qtextstream.h>

#define QHEXDUMP_MAX 32
class QHexDump
{
public:

    QHexDump(const void *adress, int len, int wrapAt = 16)
        : wrap(wrapAt), dataSize(len)
    {
        init();
        data = reinterpret_cast<const char*>(adress);
        if (len < 0)
            dataSize = 0;
    }

    QHexDump(const char *str, int len = -1, int wrapAt = 16)
        : wrap(wrapAt), dataSize(len)
    {
        init();
        data = str;
        if (len == -1)
            dataSize = strlen(str);
    }

    QHexDump(const QByteArray &array, int wrapAt = 16)
        : wrap(wrapAt)
    {
        init();
        data = array.data();
        dataSize = array.size();
    }

    // Sets a customized prefix for the hexdump
    void setPrefix(const char *str) { prefix = str; }

    // Sets number of bytes to cluster together
    void setClusterSize(uint num) { clustering = num; }

    // Output hexdump to a text stream
    void output(QTextStream &strm) {
        outstrm = &strm;
        hexDump();
    }

    // Output hexdump to a QString
    QString output() {
        QString result;
        QTextStream strm(&result, IO_WriteOnly);
        outstrm = &strm;
        hexDump();
        return result;
    }

protected:
    void init()
    {
        prefix = "> ";           // Standard line prefix
        clustering = 2;          // Word-size clustering by default
        if (wrap > QHEXDUMP_MAX) // No wider than QHEXDUMP_MAX bytes
            wrap = QHEXDUMP_MAX;
    }

    void hexDump()
    {
        *outstrm << "(" << dataSize << " bytes):\n" << prefix;
        sprintf(sideviewLayout, " [%%-%ds]", wrap);
        dataWidth = (2 * wrap) + (wrap / clustering);

        dirty = false;
        uint wrapIndex = 0;
        for (uint i = 0; i < dataSize; i++) {
            uint c = static_cast<uchar>(data[i]);
            sideview[wrapIndex = i%wrap] = isprint(c) ? c : '.';

            if (wrapIndex && (wrapIndex % clustering == 0))
                *outstrm << " ";

            outstrm->width(2);
            outstrm->fill('0');
            outstrm->setf(0, QTextStream::showbase);
            *outstrm << hex << c;
            dirty = true;

            if (wrapIndex == wrap-1) {
                sideviewDump(wrapIndex);
                wrapIndex = 0;
                if (i+1 < dataSize)
                    *outstrm << endl << prefix;
            }

        }
        sideviewDump(wrapIndex);
    }

    void sideviewDump(int at)
    {
        if (dirty) {
            dirty = false;
            ++at;
            sideview[at] = '\0';
            int currentWidth = (2 * at) + (at / clustering) - (at%clustering?0:1);
            int missing = qMax(dataWidth - currentWidth, 0);
            while (missing--)
                *outstrm << " ";

            *outstrm << " [";
            outstrm->fill(' ');
            outstrm->width(wrap);
            outstrm->setf(QTextStream::left, QTextStream::adjustfield);
            *outstrm << sideview;
            *outstrm << "]";
        }
    }

private:
    uint wrap;
    uint clustering;
    uint dataSize;
    int dataWidth;
    const char *data;
    const char *prefix;
    bool dirty;

    char sideviewLayout[QHEXDUMP_MAX + 1];
    char sideview[15];

    QTextStream *outstrm;
};
#undef QHEXDUMP_MAX

#ifndef QT_NO_DEBUG
#include <qdebug.h>
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
#endif // QT_NO_DEBUG

#endif // QHEXDUMP_H
