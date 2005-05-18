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

#include "qnamespace.h"
#include "qlist.h"
#include "qiodevice.h"
#include "qbytearray.h"

class QWidget;
class QPainter;
class QPixmap;

class QCircularBuffer {
    QByteArray buf[2];
    uint curr_used, start_off, start_buff, curr_buff, buff_growth;
public:
    inline QCircularBuffer(uint growth) : curr_used(0), start_off(0), start_buff(0),
                                          curr_buff(0), buff_growth(growth) { }

    char *alloc(uint buflen);
    char *take(uint maxsize, uint *realsize=0);
    inline void free(uint buflen);
    void push(char c);
    inline void truncate(uint len) { curr_used -= len; }

    inline int growth() const { return buff_growth; }
    inline int numBuffers() const { return 2; }
    inline bool isEmpty() const { return !used(); }
    inline uint used() const { return curr_used; }
    inline void clear() { if(!isEmpty()) free(used()); }
};

inline char *QCircularBuffer::alloc(uint size)
{
    if(buf[curr_buff].size() <
       (int)(curr_used+size+(curr_buff == start_buff ? start_off : 0))) {
        if(curr_buff == start_buff && buf[curr_buff].size()) {
            buf[curr_buff].resize(start_off + curr_used);
            curr_buff = !curr_buff;
            if(!buf[curr_buff].size())
                buf[curr_buff].resize(buff_growth*2);
        } else {
            int sz = buf[curr_buff].size();
            buf[curr_buff].resize(qMax((uint)sz + (sz / 2), (buff_growth*2)));
        }
    }
    int off = curr_used;
    curr_used += size;
    if(curr_buff != start_buff)
        off -= buf[start_buff].size() - start_off;
    else
        off += start_off;
    return buf[curr_buff].data()+off;
}
inline char *QCircularBuffer::take(uint size, uint *real_size)
{
    if(size > curr_used) {
        qWarning("Warning: asked to take too much %d [%d]", size, curr_used);
        size = curr_used;
    }
    if(real_size)
        *real_size = qMin(size, buf[start_buff].size() - start_off);
    return buf[start_buff].data()+start_off;
}

inline void QCircularBuffer::free(uint size)
{
    if(size > curr_used) {
        qWarning("Warning: asked to free too much %d [%d]", size, curr_used);
        size = curr_used;
    }
    curr_used -= size;
    if(curr_used == 0) {
        curr_buff = start_buff = start_off = 0;
        return;
    }

    uint start_size = buf[start_buff].size() - start_off;
    if(start_size > size) {
        start_off += size;
    } else if(start_buff != curr_buff) {
        start_buff = curr_buff;
        start_off = start_size - size;
    } else {
        start_off = 0;
    }
}

inline void QCircularBuffer::push(char ch)
{
    curr_used++;
    buf[start_buff].insert(start_off, ch);
}

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
