/****************************************************************************
**
** Definition of QDebug class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QDEBUG_H
#define QDEBUG_H
#ifndef QT_H
#include "qlist.h"
#include "qtextstream.h"
#include "qstring.h"
#endif

#ifndef QT_NO_DEBUG

class Q_CORE_EXPORT QDebug
{
    struct Stream {
        Stream():ts(&buffer, IO_WriteOnly),ref(1), space(true){}
        QTextStream ts;
        QString buffer;
        int ref;
        bool space;
    } *d;
public:
    inline QDebug():d(new Stream){}
    inline QDebug(const QDebug &o):d(o.d){++d->ref;}
    inline ~QDebug() {if (!--d->ref) { qDebug(d->buffer.latin1()); delete d; } }
    inline QDebug &space() { d->space = true; d->ts << ' '; return *this; }
    inline QDebug &nospace() { d->space = false; return *this; }
    inline QDebug &maybeSpace() { if (d->space) d->ts << ' '; return *this; }

    inline QDebug &operator<<(QChar t) { d->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(char t) { d->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(signed short t) { d->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(unsigned short t) { d->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(signed int t) { d->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(unsigned int t) { d->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(signed long t) { d->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(unsigned long t) { d->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(Q_LLONG t) { d->ts << QString::number(t); return maybeSpace(); }
    inline QDebug &operator<<(Q_ULLONG t) { d->ts << QString::number(t); return maybeSpace(); }
    inline QDebug &operator<<(float t) { d->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(double t) { d->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(const char* t) { d->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(const QString & t) { d->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(const QByteArray & t) { d->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(const void * t) { d->ts << t; return maybeSpace(); }
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

    inline QDebug &operator<<(QTSFUNC f)
        { if (f == endl) { qDebug(d->buffer.latin1()); d->buffer.clear(); } 
          else d->ts << f; 
          return *this; }

    inline QDebug &operator<<(QTSManip m)
        { d->ts << m; return *this; }
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
