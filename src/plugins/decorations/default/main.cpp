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
#include <qdecorationdefault_qws.h>

class DecorationDefault : public QDecorationPlugin
{
public:
    DecorationDefault();

    QStringList keys() const;
    QDecoration *create(const QString&);
};

DecorationDefault::DecorationDefault()
: QDecorationPlugin()
{
}

QStringList DecorationDefault::keys() const
{
    QStringList list;
    list << "Default";
    return list;
}

QDecoration* DecorationDefault::create(const QString& s)
{
    if (s.toLower() == "default")
        return new QDecorationDefault();

    return 0;
}

Q_EXPORT_PLUGIN(DecorationDefault)
