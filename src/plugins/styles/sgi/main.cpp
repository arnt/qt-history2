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
#include <qsgistyle.h>

class SGIStyle : public QStylePlugin
{
public:
    SGIStyle();

    QStringList keys() const;
    QStyle *create(const QString&);
};

SGIStyle::SGIStyle()
: QStylePlugin()
{
}

QStringList SGIStyle::keys() const
{
    QStringList list;
    list << "SGI";
    return list;
}

QStyle* SGIStyle::create(const QString& s)
{
    if (s.toLower() == "sgi")
        return new QSGIStyle();

    return 0;
}

Q_EXPORT_PLUGIN(SGIStyle)
