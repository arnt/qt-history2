/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QDEBUG_H
#define QDEBUG_H

#include <QtCore/qalgorithms.h>
#include <QtCore/qhash.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qpair.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qstring.h>
#include <QtCore/qvector.h>
#include <QtCore/qset.h>

QT_BEGIN_HEADER

QT_MODULE(Core)

#if !defined(QT_NO_DEBUG_STREAM)
class Q_CORE_EXPORT QDebug
{
    struct Stream {
        Stream(QIODevice *device) : ts(device), ref(1), type(QtDebugMsg), space(true), message_output(false) {}
        Stream(QString *string) : ts(string, QIODevice::WriteOnly), ref(1), type(QtDebugMsg), space(true), message_output(false) {}
        Stream(QtMsgType t) : ts(&buffer, QIODevice::WriteOnly), ref(1), type(t), space(true), message_output(true) {}
        QTextStream ts;
        QString buffer;
        int ref;
        QtMsgType type;
        bool space;
        bool message_output;
    } *stream;
public:
    inline QDebug(QIODevice *device) : stream(new Stream(device)) {}
    inline QDebug(QString *string) : stream(new Stream(string)) {}
    inline QDebug(QtMsgType t) : stream(new Stream(t)) {}
    inline QDebug(const QDebug &o):stream(o.stream) { ++stream->ref; }
    inline QDebug &operator=(const QDebug &other);
    inline ~QDebug() {
        if (!--stream->ref) {
            if(stream->message_output)
                qt_message_output(stream->type, stream->buffer.toLocal8Bit().data());
            delete stream;
        }
    }
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

inline QDebug &QDebug::operator=(const QDebug &other)
{
    if (this != &other) {
        QDebug copy(other);
        qSwap(stream, copy.stream);
    }
    return *this;
}

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

template <typename T>
inline QDebug operator<<(QDebug debug, const QVector<T> &vec)
{
    debug.nospace() << "QVector";
    return operator<<(debug, vec.toList());
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

template <class aKey, class aT>
inline QDebug operator<<(QDebug debug, const QHash<aKey, aT> &hash)
{
    debug.nospace() << "QHash(";
    for (typename QHash<aKey, aT>::const_iterator it = hash.constBegin();
            it != hash.constEnd(); ++it)
        debug << "(" << it.key() << ", " << it.value() << ")";
    debug << ")";
    return debug.space();
}

template <class T1, class T2>
inline QDebug operator<<(QDebug debug, const QPair<T1, T2> &pair)
{
    debug.nospace() << "QPair(" << pair.first << "," << pair.second << ")";
    return debug.space();
}

template <typename T>
inline QDebug operator<<(QDebug debug, const QSet<T> &set)
{
    debug.nospace() << "QSet";
    return operator<<(debug, set.toList());
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

QT_END_HEADER

#endif // QDEBUG_H
