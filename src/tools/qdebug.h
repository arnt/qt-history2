/****************************************************************************
**
** Definition of QDebug class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
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
#include "qtextstream.h"
#endif

#ifndef QT_NO_DEBUG
class Q_CORE_EXPORT QDebug
{
    struct Stream {
	Stream():ts(stderr, IO_WriteOnly),ref(1), space(true){}
	QTextStream ts;
	int ref;
	bool space;
    } *d;
public:
    inline QDebug():d(new Stream){}
    inline QDebug(const QDebug &o):d(o.d){++d->ref;}
    inline ~QDebug() {if (!--d->ref) { d->ts << endl; delete d; } }
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
    inline QDebug &operator<<(float t) { d->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(double t) { d->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(const char* t) { d->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(const QString & t) { d->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(const QByteArray & t) { d->ts << t; return maybeSpace(); }
    inline QDebug &operator<<(void * t) { d->ts << t; return maybeSpace(); }

    inline QDebug &operator<<(QTSFUNC f)
	{ d->ts << f; return *this; }

    inline QDebug &operator<<(QTSManip m)
	{ d->ts << m; return *this; }
};
inline Q_CORE_EXPORT QDebug qDebug() { return QDebug(); }

#else

class Q_CORE_EXPORT QDebug
{
public:
    inline QDebug(){}
    inline QDebug(const QDebug &){}
    inline ~QDebug(){}
    template <typename T> inline QDebug &operator<<(const T &) {return *this; }
    inline QDebug &operator<<(QTSFUNC){ return *this; }
    inline QDebug &operator<<(QTSManip){ return *this; }
};
#undef qDebug
inline Q_CORE_EXPORT QDebug qDebug() { return QDebug(); }
#define qDebug qt_noop(),1?(void)0:qDebug

#endif


#endif // QDEBUG_H
