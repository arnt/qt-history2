/****************************************************************************
**
** Definition of QTextCodecPlugin class.
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

#ifndef QTEXTCODECPLUGIN_H
#define QTEXTCODECPLUGIN_H

#ifndef QT_H
#include "qgplugin.h"
#endif // QT_H

#ifndef QT_NO_TEXTCODECPLUGIN
class QTextCodec;
class QTextCodecPluginPrivate;
class QStringList;
template <typename T> class QList;

class Q_CORE_EXPORT QTextCodecPlugin : public QGPlugin
{
    Q_OBJECT
public:
    QTextCodecPlugin();
    ~QTextCodecPlugin();

    virtual QStringList names() const = 0;
    virtual QTextCodec *createForName(const QString &name) = 0;

    virtual QList<int> mibEnums() const = 0;
    virtual QTextCodec *createForMib(int mib) = 0;

private:
    QTextCodecPluginPrivate *d;
};
#endif // QT_NO_TEXTCODECPLUGIN
#endif // QTEXTCODECPLUGIN_H
