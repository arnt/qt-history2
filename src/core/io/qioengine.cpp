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
    \class QIOEngine
    \reentrant

    \brief The QIOEngine class provies an abstraction for block reading and writing.

    \ingroup io
    \mainclass

    ###
*/


/*!

    ###

 */
QIOEngine::QIOEngine() : d_ptr(new QIOEnginePrivate)
{
    d_ptr->q_ptr = this;
}

/*!

    ###

 */
QIOEngine::QIOEngine(QIOEnginePrivate &dd) : d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

/*!

    ###

 */
QIOEngine::~QIOEngine()
{
    delete d_ptr;
    d_ptr = 0;
}

/*!

    ###

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

    ###

 */
int QIOEngine::getch()
{
    uchar ret;
    if(readBlock((char*)&ret, 1) != 1) 
        return EOF;
    return (int)ret;
}
 
/*!

    ###

 */
int QIOEngine::putch(int ch)
{
    uchar ret = ch;
    if(writeBlock((char*)&ret, 1) != 1)
        return EOF;
    return (int)ret;
}

/*!

    ###

 */
bool QIOEngine::atEnd() const
{
    return at() == size();
}

/*!

    ###

 */
QIODevice::Status QIOEngine::errorStatus() const
{
    return QIODevice::UnspecifiedError;
}

/*!

    ###

 */
uchar *QIOEngine::map(QIODevice::Offset /*offset*/, Q_LONG /*len*/)
{
    return 0;
}

/*!

    ###

 */
void QIOEngine::unmap(uchar */*data*/)
{

}

/*!

    ###

 */
QString QIOEngine::errorString() const
{
    return QString::null;
}



/* 
    \fn QIOEngine::Type QIOEngine::type() const
  
    ###

    This virtual function must be reimplemented by all subclasses.
 */

/* 
    \fn bool QIOEngine::open(int flags)
  
    ###

    This virtual function must be reimplemented by all subclasses.
 */

/* 
    \fn bool QIOEngine::close()
  
    ###

    This virtual function must be reimplemented by all subclasses.
 */

/* 
    \fn void QIOEngine::flush()
  
    ###

    This virtual function must be reimplemented by all subclasses.
 */

/* 
    \fn QIODevice::Offset QIOEngine::size() const
  
    ###

    This virtual function must be reimplemented by all subclasses.
 */

/* 
    \fn QIODevice::Offset QIOEngine::at() const
  
    ###

    This virtual function must be reimplemented by all subclasses.
 */

/* 
    \fn bool QIOEngine::seek(QIODevice::Offset)
  
    ###

    This virtual function must be reimplemented by all subclasses.
 */

/* 
    \fn bool QIOEngine::isSequential() const
  
    ###

    This virtual function must be reimplemented by all subclasses.
 */

/* 
    \fn Q_LONG QIOEngine::readBlock(char *data, Q_LONG maxlen)
  
    ###

    This virtual function must be reimplemented by all subclasses.
 */

/* 
    \fn Q_LONG QIOEngine::writeBlock(const char *data, Q_LONG len)
  
    ###

    This virtual function must be reimplemented by all subclasses.
 */

/* 
    \fn int QIOEngine::ungetch(int c)
  
    ###

    This virtual function must be reimplemented by all subclasses.
 */
