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
#include <qdecorationkde2_qws.h>

class DecorationKDE2 : public QDecorationPlugin
{
public:
    DecorationKDE2();

    QStringList keys() const;
    QDecoration *create(const QString&);
};

DecorationKDE2::DecorationKDE2()
: QDecorationPlugin()
{
}

QStringList DecorationKDE2::keys() const
{
    QStringList list;
    list << "KDE2";
    return list;
}

QDecoration* DecorationKDE2::create(const QString& s)
{
    if (s.toLower() == "kde2")
        return new QDecorationKDE2();

    return 0;
}

Q_EXPORT_PLUGIN(DecorationKDE2)
