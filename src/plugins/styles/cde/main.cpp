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
#include <qcdestyle.h>

class CDEStyle : public QStylePlugin
{
public:
    CDEStyle();

    QStringList keys() const;
    QStyle *create(const QString&);
};

CDEStyle::CDEStyle()
: QStylePlugin()
{
}

QStringList CDEStyle::keys() const
{
    QStringList list;
    list << "CDE";
    return list;
}

QStyle* CDEStyle::create(const QString& s)
{
    if (s.toLower() == "cde")
        return new QCDEStyle();

    return 0;
}

Q_EXPORT_PLUGIN(CDEStyle)
