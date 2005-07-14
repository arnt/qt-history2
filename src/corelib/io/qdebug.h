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

#include "QtCore/qlist.h"
#include "QtCore/qmap.h"
#include "QtCore/qtextstream.h"
#include "QtCore/qstring.h"

QT_MODULE(Core)

#if !defined(QT_NO_DEBUG_STREAM)
class Q_CORE_EXPORT QDebug
{
    struct Stream {
        Stream(QtMsgType t) : ts(&buffer, QIODevice::WriteOnly), ref(1), type(t), space(true){}
        QTextStream ts;
        QString buffer;
        int ref;
        QtMsgType type;
        bool space;
    } *stream;
public:
    inline QDebug(QtMsgType t) : stream(new Stream(t)) {}
    inline QDebug(const QDebug &o):stream(o.stream) { ++stream->ref; }
    inline ~QDebug()
        { if (!--stream->ref) { qt_message_output(stream->type, stream->buffer.toLocal8Bit().data()); delete stream; } }
    inline QDebug &space() { stream->space = true; stream->ts << " "; return *this; }
    inline QDebug &nospace() { stream->space = false; return *this; }
    inline QDebug &maybeSpace() { if (stream->space) stream->ts << " "; return *this; }

    inline QDebug &operator<<(QChar t) { stream->ts << "\'" << t << "\'"; return maybeSpace(); }
    inline QDebug &operator<<(bool t) { stream->ts << (t ? "true" : "false"); return maybeSpace(); }
    inline QDebug &operator<<(char t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(signed short t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(unsigned short t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(signed int t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(unsigned int t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(signed long t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(unsigned long t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(qint64 t)
        { stream->ts << QString::number(t); return maybeSpace(); }
    inline QDebug &operator<<(quint64 t)
        { stream->ts << QString::number(t); return maybeSpace(); }
    inline QDebug &operator<<(float t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(double t) { stream->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(const char* t) { stream->ts  << t; return maybeSpace(); }
    inline QDebug &operator<<(const QString & t) { stream->ts << "\"" << t  << "\""; return maybeSpace(); }
    inline QDebug &operator<<(const QLatin1String &t) { stream->ts << "\""  << t.latin1() << "\""; return maybeSpace(); }
    inline QDebug &operator<<(const QByteArray & t) { stream->ts  << "\"" << t << "\""; return maybeSpace(); }
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
    debug.nospace() << "(";
    for (Q_TYPENAME QList<T>::size_type i = 0; i < list.count(); ++i) {
        if (i)
            debug << ", ";
        debug << list.at(i);
    }
    debug << ")";
    return debug.space();
}

template <class aKey, class aT>
inline QDebug operator<<(QDebug debug, const QMap<aKey, aT> &map)
{
    debug.nospace() << "QMap(";
    for (typename QMap<aKey, aT>::const_iterator it = map.constBegin();
         it != map.constEnd(); ++it) {
        debug << "(" << it.key() << ", " << it.value() << ")";
    }
    debug << ")";
    return debug.space();
}

inline Q_CORE_EXPORT QDebug qDebug() { return QDebug(QtDebugMsg); }
inline Q_CORE_EXPORT QDebug qWarning() { return QDebug(QtWarningMsg); }
inline Q_CORE_EXPORT QDebug qCritical() { return QDebug(QtCriticalMsg); }

#else // QT_NO_DEBUG_STREAM

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

#ifndef QT_NO_MEMBER_TEMPLATES
    template<typename T>
    inline QNoDebug &operator<<(const T &) { return *this; }
#endif
};
#undef qDebug
inline QNoDebug qDebug() { return QNoDebug(); }
#define qDebug if(1) ; else qDebug

#ifdef QT_NO_MEMBER_TEMPLATES
template<typename T>
inline QNoDebug operator<<(QNoDebug debug, const T &) { return debug; }
#endif

#endif

#endif // QDEBUG_H
