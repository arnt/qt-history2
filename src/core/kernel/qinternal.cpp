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

#include "private/qinternal_p.h"
#include "qcleanuphandler.h"

// *******************************************************************
// QMembuf declaration and implementation
// *******************************************************************

/*  \internal
    This class implements an efficient buffering of data that is often used by
    asynchronous IO classes like QSocket, QHttp and QProcess.
*/

QMembuf::QMembuf() : _size(0), _index(0)
{
}

QMembuf::~QMembuf()
{
    while (!buf.isEmpty())
        delete buf.takeFirst();
}

/*! \internal
    This function consumes \a nbytes bytes of data from the
    buffer and copies it into \a sink. If \a sink is a 0 pointer
    the data goes into the nirvana.
*/
bool QMembuf::consumeBytes(Q_ULONG nbytes, char *sink)
{
    if (nbytes <= 0 || nbytes > _size)
        return false;
    _size -= nbytes;
    while (!buf.isEmpty()) {
        QByteArray *a = buf.first();
        if ((int)(_index + nbytes) >= a->size()) {
            // Here we skip the whole byte array and get the next later
            int len = a->size() - _index;
            if (sink) {
                memcpy(sink, a->constData()+_index, len);
                sink += len;
            }
            nbytes -= len;
            buf.removeFirst();
            _index = 0;
            if (nbytes == 0)
                break;
        } else {
            // Here we skip only a part of the first byte array
            if (sink)
                memcpy(sink, a->constData()+_index, nbytes);
            _index += nbytes;
            break;
        }
    }
    return true;
}

/*! \internal
    Scans for any occurrence of '\n' in the buffer. If \a store
    is not 0 the text up to the first '\n' (or terminating 0) is
    written to \a store, and a terminating 0 is appended to \a store
    if necessary. Returns true if a '\n' was found; otherwise returns
    false.
*/
bool QMembuf::scanNewline(QByteArray *store)
{
    if (_size == 0)
        return false;
    int i = 0; // index into 'store'
    QByteArray *a = 0;
    char *p;
    int n;
    bool retval = false;
    for (int j = 0; j < buf.size(); ++j) {
        a = buf.at(j);
        p = a->data();
        n = a->size();
        if (!j) {
            // first buffer
            p += _index;
            n -= _index;
        }
        if (store) {
            while (n-- > 0) {
                *(store->data()+i) = *p;
                if (++i == (int)store->size())
                    store->resize(store->size() < 256
                                   ? 1024 : store->size()*4);
                if (*p == '\n') {
                    retval = true;
                    goto end;
                }
                p++;
            }
        } else {
            while (n-- > 0) {
                if(*p == '\n')
                    return true;
                p++;
            }
        }
    }
 end:
    if (store)
        store->resize(i);
    return retval;
}

int QMembuf::ungetch(int ch)
{
    if (buf.isEmpty() || _index==0) {
        // we need a new QByteArray
        QByteArray *ba = new QByteArray;
        ba->resize(1);
        buf.prepend(ba);
        _size++;
        (*ba)[0] = ch;
    } else {
        // we can reuse a place in the buffer
        QByteArray *ba = buf.first();
        _index--;
        _size++;
        (*ba)[(int)_index] = ch;
    }
    return ch;
}

/*! \internal

    Creates an empty ring buffer. The buffer will grow in steps of \a
    growth as data is written to it.
*/
QRingBuffer::QRingBuffer(int growth)
    : basicBlockSize(growth)
{
    buffers << QByteArray();
    clear();
}

/*! \internal

    Returns the number of bytes that can be read in one operation. The
    data is read from readPointer().
*/
int QRingBuffer::nextDataBlockSize() const
{
    return (tailBuffer == 0 ? tail : buffers.at(0).size()) - head;
}

/*! \internal

    Returns a pointer to where no more than nextDataBlockSize() bytes
    of data can be read. Call free() to remove data after reading.
*/
char *QRingBuffer::readPointer() const
{
    if (buffers.count() == 0)
        return 0;
    return const_cast<char *>(buffers[0].data()) + head;
}

/*! \internal

    Removes \a bytes bytes from the front of the buffer. If \a bytes
    is larger than the size of the buffer, the buffer is cleared.
*/
void QRingBuffer::free(int bytes)
{
    bufferSize -= bytes;
    if (bufferSize < 0)
        bufferSize = 0;

    for (;;) {
        int nextBlockSize = nextDataBlockSize();
        if (bytes < nextBlockSize) {
            head += bytes;
            if (head == tail && tailBuffer == 0)
                head = tail = 0;
            return;
        }

        bytes -= nextBlockSize;
        if (buffers.count() == 1) {
            if (buffers.at(0).size() != basicBlockSize)
                buffers[0].resize(basicBlockSize);
            head = tail = 0;
            tailBuffer = 0;
            return;
        }

        buffers.removeAt(0);
        --tailBuffer;
        head = 0;
    }
}

/*! \internal

    Reserves space in the buffer for \a bytes new bytes, and returns a
    pointer to the first byte.
*/
char *QRingBuffer::reserve(int bytes)
{
    bufferSize += bytes;

    // if there is already enough space, simply return.
    if (tail + bytes <= buffers.at(tailBuffer).size()) {
        char *writePtr = buffers[tailBuffer].data() + tail;
        tail += bytes;
        return writePtr;
    }

    // if our buffer isn't half full yet, simply resize it.
    if (tail < buffers.at(tailBuffer).size() / 2) {
        buffers[tailBuffer].resize(tail + bytes);
        char *writePtr = buffers[tailBuffer].data() + tail;
        tail += bytes;
        return writePtr;
    }

    // shrink this buffer to its current size
    buffers[tailBuffer].resize(tail);

    // create a new QByteArray with the right size
    buffers << QByteArray();
    ++tailBuffer;
    buffers[tailBuffer].resize(qMax(basicBlockSize, bytes));
    tail = bytes;
    return buffers[tailBuffer].data();
}

/*! \internal

    Removes \a bytes bytes from the end of the buffer. If \a bytes is
    larger than the buffer size, the buffer is cleared.
*/
void QRingBuffer::truncate(int bytes)
{
    bufferSize -= bytes;
    if (bufferSize < 0)
        bufferSize = 0;

    for (;;) {
        // special case: head and tail are in the same buffer
        if (tailBuffer == 0) {
            tail -= bytes;
            if (tail <= head)
                tail = head = 0;
            return;
        }

        if (bytes <= tail) {
            tail -= bytes;
            return;
        }

        bytes -= tail;
        buffers.removeAt(tailBuffer);

        --tailBuffer;
        tail = buffers.at(tailBuffer).size();
    }
}

/*! \internal

    Returns and removes the first character in the buffer. Returns -1
    if the buffer is empty.
*/
int QRingBuffer::getChar()
{
    if (isEmpty())
       return -1;
    char c = *readPointer();
    free(1);
    return c;
}

/*! \internal

    Appends the character \a c to the end of the buffer.
*/
void QRingBuffer::putChar(char c)
{
    char *ptr = reserve(1);
    *ptr = c;
}

/*! \internal

    Prepends the character \a c to the front of the buffer.
*/
void QRingBuffer::ungetChar(char c)
{
    --head;
    if (head < 0) {
        buffers.prepend(QByteArray());
        buffers[0].resize(basicBlockSize);
        head = basicBlockSize - 1;
        ++tailBuffer;
    }
    buffers[0][head] = c;
    ++bufferSize;
}

/*! \internal

    Returns the size of the buffer; e.g. the number of bytes
    currently in use.
*/
int QRingBuffer::size() const
{
    return bufferSize;
}

/*! \internal

    Removes all data from the buffer and resets its size to 0.
*/
void QRingBuffer::clear()
{
    QByteArray tmp = buffers[0];
    buffers.clear();
    buffers << tmp;

    if (buffers.at(0).size() != basicBlockSize)
        buffers[0].resize(basicBlockSize);

    head = tail = 0;
    tailBuffer = 0;
    bufferSize = 0;
}

/*! \internal

    Returns true if the buffer is empty; otherwise returns false.
*/
bool QRingBuffer::isEmpty() const
{
    return tailBuffer == 0 && tail == 0;
}

/*! \internal

    Returns the index of the first occurrence of the character \a c in
    the buffer. In no such character is found, -1 is returned.
*/
int QRingBuffer::indexOf(char c) const
{
    int index = 0;
    for (int i = 0; i < buffers.size(); ++i) {
        int start = 0;
        int end = buffers.at(i).size();

        if (i == 0)
            start = head;
        if (i == tailBuffer)
            end = tail;
        const char *ptr = buffers.at(i).data() + start;
        for (int j = start; j < end; ++j) {
            if (*ptr++ == c)
                return index;
            ++index;
        }
    }

    return -1;
}

/*! \internal

    Reads one line of data (all data up to and including the '\n'
    character), no longer than \a maxSize - 1 bytes, and stores it in \a
    data. If the line is too long, maxSize bytes of the line are read.
    \a data is always terminated by a '\0' byte.
*/
int QRingBuffer::readLine(char *data, int maxSize)
{
    int index = indexOf('\n');
    if (index == -1 || maxSize <= 0)
        return -1;

    int readSoFar = 0;
    while (readSoFar < index && readSoFar < maxSize - 1) {
        int bytesToRead = qMin((index + 1) - readSoFar, nextDataBlockSize());
        bytesToRead = qMin(bytesToRead, (maxSize - 1) - readSoFar);
        memcpy(data + readSoFar, readPointer(), bytesToRead);
        readSoFar += bytesToRead;
        free(bytesToRead);
    }

    // Terminate it.
    data[readSoFar] = '\0';
    return readSoFar;
}

/*! \internal

    Returns true if a line can be read from the buffer; otherwise
    returns false.
*/
bool QRingBuffer::canReadLine() const
{
    return indexOf('\n') != -1;
}
