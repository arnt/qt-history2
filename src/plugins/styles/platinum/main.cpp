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

#include <qplatinumstyle.h>
#include <qstyleplugin.h>

class PlatinumStyle : public QStylePlugin
{
public:
    PlatinumStyle();

    QStringList keys() const;
    QStyle *create(const QString&);
};

PlatinumStyle::PlatinumStyle()
: QStylePlugin()
{
}

QStringList PlatinumStyle::keys() const
{
    QStringList list;
    list << "Platinum";
    return list;
}

QStyle* PlatinumStyle::create(const QString& s)
{
    if (s.toLower() == "platinum")
        return new QPlatinumStyle();

    return 0;
}


Q_EXPORT_PLUGIN(PlatinumStyle)
