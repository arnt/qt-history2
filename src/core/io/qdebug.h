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

#if !defined( QT_NO_DEBUG_OUTPUT ) && !defined( QT_NO_TEXTSTREAM )
class Q_CORE_EXPORT QDebug
{
    struct Stream {
        Stream() : ts(&buffer, QIODevice::WriteOnly), ref(1), space(true){}
        QTextStream ts;
        QString buffer;
        int ref;
        bool space;
    } *stream;
public:
    inline QDebug():stream(new Stream){}
    inline QDebug(const QDebug &o):stream(o.stream) { ++stream->ref; }
    inline ~QDebug() { if (!--stream->ref) { stream->buffer.replace(QLatin1Char('%'), QLatin1String("%%"));
                                            qDebug(stream->buffer.toLocal8Bit().data()); delete stream; } }
    inline QDebug &space() { stream->space = true; stream->ts << ' '; return *this; }
    inline QDebug &nospace() { stream->space = false; return *this; }
    inline QDebug &maybeSpace() { if (stream->space) stream->ts << ' '; return *this; }

    inline QDebug &operator<<(QChar t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(bool t) { stream->ts << (t ? "true" : "false"); return maybeSpace(); }
    inline QDebug &operator<<(char t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(signed short t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(unsigned short t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(signed int t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(unsigned int t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(signed long t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(unsigned long t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(Q_LONGLONG t)
        { stream->ts << QString::number(t); return maybeSpace(); }
    inline QDebug &operator<<(Q_ULONGLONG t)
        { stream->ts << QString::number(t); return maybeSpace(); }
    inline QDebug &operator<<(float t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(double t) { stream->ts << t; return maybeSpace(); }
#ifdef QT_USE_FIXED_POINT
    inline QDebug &operator<<(QFixedPoint t) { stream->ts << t.toDouble(); return maybeSpace(); }
#endif
    inline QDebug &operator<<(const char* t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(const QString & t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(const QLatin1String &t) { stream->ts << t.latin1(); return maybeSpace(); }
    inline QDebug &operator<<(const QByteArray & t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(const void * t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(QTextStreamFunction f) {
        stream->ts << f;
        return *this;
    }

    inline QDebug &operator<<(QTextStreamManipulator m)
        { stream->ts << m; return *this; }
};

template <class T>
inline QDebug operator<<(QDebug debug, const QList<T> &list)
{
    debug << '(';
    for (Q_TYPENAME QList<T>::size_type i = 0; i < list.count(); ++i) {
        if (i)
            debug << ',';
        debug << list.at(i);
    }
    debug << ')';
    return debug;
}


inline Q_CORE_EXPORT QDebug qDebug() { return QDebug(); }

#else

class QNoDebug
{
public:
    inline QNoDebug(){}
    inline QNoDebug(const QDebug &){}
    inline ~QNoDebug(){}
#if !defined( QT_NO_TEXTSTREAM )
    inline QNoDebug &operator<<(QTextStreamFunction) { return *this; }
    inline QNoDebug &operator<<(QTextStreamManipulator) { return *this; }
#endif
    inline QNoDebug &space() { return *this; }
    inline QNoDebug &nospace() { return *this; }
    inline QNoDebug &maybeSpace() { return *this; }
};
#undef qDebug
inline QNoDebug qDebug() { return QNoDebug(); }
#define qDebug if(1) ; else qDebug

template<typename T>
inline QNoDebug operator<<(QNoDebug dbg, const T &) { return dbg; }

#endif


#endif // QDEBUG_H
