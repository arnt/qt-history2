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

#include <qdecorationplugin_qws.h>
#include <qdecorationwindows_qws.h>

class DecorationWindows : public QDecorationPlugin
{
public:
    DecorationWindows();

    QStringList keys() const;
    QDecoration *create(const QString&);
};

DecorationWindows::DecorationWindows()
: QDecorationPlugin()
{
}

QStringList DecorationWindows::keys() const
{
    QStringList list;
    list << "Windows";
    return list;
}

QDecoration* DecorationWindows::create(const QString& s)
{
    if (s.toLower() == "windows")
        return new QDecorationWindows();

    return 0;
}

Q_EXPORT_PLUGIN(DecorationWindows)
