/****************************************************************************
**
** Implementation of QIOEngine class.
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qioengine.h"
#include "qioengine_p.h"
#include <stdio.h>

#define d d_func()
#define q q_func()

/*!
    \class QIOEngine qioengine.h
    \reentrant

    \brief The QIOEngine class provides an abstraction for block reading and writing.

    \ingroup io
    \mainclass

    The QIOEngine class is used to directly read and write to the
    backing store for a QIODevice. All read/write operations will be
    based on simple char's thus completely platform independent,
    QTextStream will in turn intrepret the bytes the same when read.
*/


/*!
   Construct a QIOEngine.
 */
QIOEngine::QIOEngine() : d_ptr(new QIOEnginePrivate)
{
    d_ptr->q_ptr = this;
}

/*!
   \internal

   Construct a QIOEngine.
 */
QIOEngine::QIOEngine(QIOEnginePrivate &dd) : d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

/*!
  Destroys a QIOEngine.
 */
QIOEngine::~QIOEngine()
{
    delete d_ptr;
    d_ptr = 0;
}

/*!
  Reads a single line (as ended with \\n) into \a data with a maximum
  length of \a maxlen. The length of the data filled is returned.

  Many QIOEngine subclasses can be optimized for this function, the
  base implementation will simply read a single character at a time.

  \sa readBlock
 */
Q_LONG QIOEngine::readLine(char *data, Q_LONG maxlen)
{
    if (maxlen == 0)                                // application bug?
        return 0;
    char *p = data;
    while (--maxlen && (readBlock(p,1)>0)) {        // read one byte at a time
        if (*p++ == '\n')                        // end of line
            break;
    }
    if(p != data) {
        *p++ = '\0';
        return p - data;
    } 
    return -1;
}

/*!
   Reads all the remaining data from the source and returns it in a
   QByteArray this can be optimized in many subclasses but the base
   implementation will just read a block at a time.

   \sa readBlock
 */
QByteArray QIOEngine::readAll()
{
    if (isSequential()) {     // read until we reach the end
        const int blocksize = 512;
        int nread = 0;
        QByteArray ba;
        while (!atEnd()) {
            ba.resize(nread + blocksize);
            int r = readBlock(ba.data()+nread, blocksize);
            if (r < 0) {
                ba.resize(nread);
                return ba;
            }
            nread += r;
        }
        ba.resize(nread);
        return ba;
    }
    // we know the size
    int n = size() - at(); // ### fix for 64-bit or large files?
    int totalRead = 0;
    QByteArray ba;
    ba.resize(n);
    char* c = ba.data();
    while (n) {
        int r = readBlock(c, n);
        if (r < 0)
            return QByteArray();
        n -= r;
        c += r;
        totalRead += r;
        // If we have a translated file, then it is possible that
        // we read less bytes than size() reports
        if (atEnd()) {
            ba.resize(totalRead);
            break;
        }
    }
    return ba;
}


/*!
  Gets a single character from a QIODevice, if the end of the file is
  met EOF should be returned. The base implementation will simply
  readBlock a single character.

  \sa readBlock
 */
int QIOEngine::getch()
{
    uchar ret;
    if(readBlock((char*)&ret, 1) != 1) 
        return EOF;
    return (int)ret;
}
 
/*!
  Puts a single chacters into the QIODevice, if this fails EOF is
  returned. The base implementation will simply writeBlock a single
  character.

  \sa writeBlock
 */
int QIOEngine::putch(int ch)
{
    uchar ret = ch;
    if(writeBlock((char*)&ret, 1) != 1)
        return EOF;
    return (int)ret;
}

/*!
   Returns true if the end of file has been reached; otherwise false.
 */
bool QIOEngine::atEnd() const
{
    return at() == size();
}

/*!
  Returns the QIODevice::Status that resulted from the last failed
  operation. If QIOevice::UnspecifiedError is returned QIODevice will
  use its own idea of the error status.

  \sa QIODeivce::Status, errorString
 */
QIODevice::Status QIOEngine::errorStatus() const
{
    return QIODevice::UnspecifiedError;
}

/*!
  Returns the message that goes along with errorStatus. If there is no
  string to display the error return QString::null.

  \sa errorStatus, QString::isNull()
 */
QString QIOEngine::errorString() const
{
    return QString::null;
}

/*!
  Attempt to map the file contents from \a offset for \a len number of
  bytes and return it as a uchar *. If this fails returning 0 will
  fall back to block reading/writing.

  \sa unmap
 */
uchar *QIOEngine::map(QIODevice::Offset /*offset*/, Q_LONG /*len*/)
{
    return 0;
}

/*!
   Unmap previously mapped file contents from memory.

   \sa map
 */
void QIOEngine::unmap(uchar */*data*/)
{

}

/* 
    \fn QIOEngine::Type QIOEngine::type() const
  
    Return your IO type, this can be used as simple RTTI information.

    This virtual function must be reimplemented by all subclasses.
 */

/* 
    \fn bool QIOEngine::open(int flags)
  
    Requests the IO be opened with flags set to \a flags being a set
    or or'd together members from QIODevice::OpenModes. Upon success
    return true; otherwise false and if necessary set your
    QIOEngine::errorStatus.

    This virtual function must be reimplemented by all subclasses.

    \sa errorStatus, QIODevice::OpenModes, close
 */

/* 
    \fn bool QIOEngine::close()
  
    Requests the a previously opened QIOEngine be closed, and
    references into the IO be removed. Upon success return true;
    otherwise false and if necessary set your QIOEngine::errorStatus.

    This virtual function must be reimplemented by all subclasses.
 */

/* 
    \fn void QIOEngine::flush()
  
    Requests the all read/write's be flushed to your destination.

    This virtual function must be reimplemented by all subclasses.
 */

/* 
    \fn QIODevice::Offset QIOEngine::size() const
  
    Requests the size of your IO destination.

    This virtual function must be reimplemented by all subclasses.
 */

/* 
    \fn QIODevice::Offset QIOEngine::at() const
  
    Requests the current position of your IO destination.

    This virtual function must be reimplemented by all subclasses.
 */

/* 
    \fn bool QIOEngine::seek(QIODevice::Offset off)
  
    Requests that the engine's position be set to \a off relative the
    beginning of the line. If the engine is a sequential device then
    \a off will be relative the current position.

    This virtual function must be reimplemented by all subclasses.

    \sa at, isSequential
 */

/* 
    \fn bool QIOEngine::isSequential() const
  
    If the engine cannot be used for random read/write access return
    true; otherwise false.

    This virtual function must be reimplemented by all subclasses.
 */

/* 
    \fn Q_LONG QIOEngine::readBlock(char *data, Q_LONG maxlen)
  
    Requests that input from the source be read into the values
    pointed to by \a data with a maximum length of \a maxlen. The
    number of bytes read will be returned; if a failure occurs EOF is
    returned.

    This virtual function must be reimplemented by all subclasses.
 */

/* 
    \fn Q_LONG QIOEngine::writeBlock(const char *data, Q_LONG len)
  
    Requests that values pointed to by \a data be written to the
    destination with a maximum length of \a maxlen. The number of
    bytes written will be returned; if a failure occurs EOF is
    returned.

    This virtual function must be reimplemented by all subclasses.
 */

/* 
    \fn int QIOEngine::ungetch(int c)
  
    Places \a c back into the stream at the current posisition; upon
    failure -1 is returned, otherwise \a c is returned.

    This virtual function must be reimplemented by all subclasses.
 */
