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

#ifndef QDEBUG_H
#define QDEBUG_H
#include "qlist.h"
#include "qtextstream.h"
#include "qstring.h"

#ifndef QT_NO_DEBUG

class Q_CORE_EXPORT QDebug
{
    struct Stream {
        Stream():ts(&buffer, IO_WriteOnly),ref(1), space(true){}
        QTextStream ts;
        QString buffer;
        int ref;
        bool space;
    } *stream;
public:
    inline QDebug():stream(new Stream){}
    inline QDebug(const QDebug &o):stream(o.stream){++stream->ref;}
    inline ~QDebug() {if (!--stream->ref) { qDebug(stream->buffer.latin1()); delete stream; } }
    inline QDebug &space() { stream->space = true; stream->ts << ' '; return *this; }
    inline QDebug &nospace() { stream->space = false; return *this; }
    inline QDebug &maybeSpace() { if (stream->space) stream->ts << ' '; return *this; }

    inline QDebug &operator<<(QChar t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(char t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(signed short t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(unsigned short t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(signed int t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(unsigned int t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(signed long t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(unsigned long t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(Q_LLONG t) { stream->ts << QString::number(t); return maybeSpace(); }
    inline QDebug &operator<<(Q_ULLONG t) { stream->ts << QString::number(t); return maybeSpace(); }
    inline QDebug &operator<<(float t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(double t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(const char* t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(const QString & t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(const QByteArray & t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(const void * t) { stream->ts << t; return maybeSpace(); }
    template <class T>
    inline QDebug &operator<<(const QList<T> &list)
    {
        (*this) << '(';
        for (Q_TYPENAME QList<T>::size_type i = 0; i < list.count(); ++i) {
            if (i)
                (*this) << ',';
            (*this) << list.at(i);
        }
        (*this) << ')';
        return *this;
    }

    inline QDebug &operator<<(QTSFUNC f) {
        stream->ts << f;
        return *this;
    }

    inline QDebug &operator<<(QTSManip m)
        { stream->ts << m; return *this; }
};
inline Q_CORE_EXPORT QDebug qDebug() { return QDebug(); }

#else

class QNoDebug
{
public:
    inline QNoDebug(){}
    inline QNoDebug(const QDebug &){}
    inline ~QNoDebug(){}
    template <typename T> inline QNoDebug &operator<<(const T &) {return *this; }
    inline QNoDebug &operator<<(QTSFUNC){ return *this; }
    inline QNoDebug &operator<<(QTSManip){ return *this; }
    inline QNoDebug &space() { return *this; }
    inline QNoDebug &nospace() { return *this; }
    inline QNoDebug &maybeSpace() { return *this; }
};
#undef qDebug
inline QNoDebug qDebug() { return QNoDebug(); }
#define qDebug if(1) ; else qDebug

#endif


#endif // QDEBUG_H
