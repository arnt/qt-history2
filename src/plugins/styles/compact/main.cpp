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

#include <qstyleplugin.h>
#include <qcompactstyle.h>

class CompactStyle : public QStylePlugin
{
public:
    CompactStyle();

    QStringList keys() const;
    QStyle *create(const QString&);
};

CompactStyle::CompactStyle()
: QStylePlugin()
{
}

QStringList CompactStyle::keys() const
{
    QStringList list;
    list << "Compact";
    return list;
}

QStyle* CompactStyle::create(const QString& s)
{
    if (s.lower() == "compact")
        return new QCompactStyle();

    return 0;
}


Q_EXPORT_PLUGIN(CompactStyle)

