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
#include <qdecorationbeos_qws.h>

class DecorationBeOS : public QDecorationPlugin
{
public:
    DecorationBeOS();

    QStringList keys() const;
    QDecoration *create(const QString&);
};

DecorationBeOS::DecorationBeOS()
: QDecorationPlugin()
{
}

QStringList DecorationBeOS::keys() const
{
    QStringList list;
    list << "BeOS";
    return list;
}

QDecoration* DecorationBeOS::create(const QString& s)
{
    if (s.toLower() == "beos")
        return new QDecorationBeOS();

    return 0;
}

Q_EXPORT_PLUGIN(DecorationBeOS)
