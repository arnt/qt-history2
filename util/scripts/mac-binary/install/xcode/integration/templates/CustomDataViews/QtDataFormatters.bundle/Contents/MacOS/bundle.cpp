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
****************************************************************************///xcode
extern "C"  {
#include "DataFormatterPlugin.h"
    _pbxgdb_plugin_function_list *_pbxgdb_plugin_functions = 0;
}

//qt stuff
#include <QtCore/QtCore>

static int serialId(int id)
{
#if 0
    static int serialRet = 0xF00F00F0;
    return serialRet--;
#else
    return id;
#endif
}

static void *doAllocate(int id, int size)
{
    if(_pbxgdb_plugin_functions && _pbxgdb_plugin_functions->allocate)
        return (*_pbxgdb_plugin_functions->allocate)(id, size);
    return malloc(size);
}

static char *doSprintf(int id, const char *format, va_list ap)
{
    if(_pbxgdb_plugin_functions && _pbxgdb_plugin_functions->vmessage)
        return (*_pbxgdb_plugin_functions->vmessage)(id, format, ap);
    return "Unable to sprintf!";
}

static char *doSprintf(int id, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    char *ret = doSprintf(id, format, ap);
    va_end(ap);
    return ret;
}

char *
Qt_QPointSummary(QPoint &point, int id)
{
    return doSprintf(serialId(id), "(%d,%d)", point.x(), point.y());
}

char *
Qt_QSizeSummary(QSize &size, int id)
{
    return doSprintf(serialId(id), "[%dx%d]", size.width(), size.height());
}

char *
Qt_QStringSummary(QString &string, int id)
{
    return doSprintf(serialId(id), "%s", string.toLatin1().constData());
}

char *
Qt_QDateSummary(QDate &v, int id)
{
    return doSprintf(serialId(id), "%s", v.toString().toLatin1().constData());
}

char *
Qt_QTimeSummary(QTime &v, int id)
{
    return doSprintf(serialId(id), "%s", v.toString().toLatin1().constData());
}

char *
Qt_QDateTimeSummary(QDateTime &v, int id)
{
    return doSprintf(serialId(id), "%s", v.toString().toLatin1().constData());
}

char *
Qt_QVariantSummary(QVariant &v, int id)
{
    return doSprintf(serialId(id), "%s", v.toString().toLatin1().constData());
}

char *
Qt_QRectSummary(QRect &rect, int id)
{
    return doSprintf(serialId(id), "(%d,%d)[%dx%d]", rect.x(), rect.y(),
                     rect.width(), rect.height());
}
