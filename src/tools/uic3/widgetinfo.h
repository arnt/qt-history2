/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef WIDGETINFO_H
#define WIDGETINFO_H

class QString;
struct QMetaObject;
class QMetaEnum;

class WidgetInfo
{
protected:
    WidgetInfo();

public:
    static bool isValidProperty(const QString &className, const char *name);
    static bool isValidEnumerator(const QString &className, const char *name);
    static bool isValidSignal(const QString &className, const char *name);
    static bool isValidSlot(const QString &className, const char *name);

    static QString resolveEnumerator(const QString &className, const char *name);

private:
    static const QMetaObject *metaObject(const QString &widgetName);
    static bool checkEnumerator(const QMetaObject *meta, const char *name);
    static bool checkEnumerator(const QMetaEnum &metaEnum, const char *name);

    static QString resolveEnumerator(const QMetaObject *meta, const char *name);
    static QString resolveEnumerator(const QMetaEnum &metaEnum, const char *name);
};

#endif
